#pragma once
#include "Engines.h"
#include <vector>

// ─────────────────────────────────────────────────────────────────────────────
//  MICROTONAL SCALE
// ─────────────────────────────────────────────────────────────────────────────
struct MicrotonalScale {
    static constexpr const char* kNames[] = {
        "Chromatic","Pentatonic","Dorian","Phrygian","Lydian","Mixolydian",
        "Locrian","Harmonic Minor","Phrygian Dom","Double Harmonic",
        "Hirajoshi","Maqam Rast","Maqam Nahawand"
    };
    static constexpr int kCount = 13;

    int scaleIdx = 0, rootNote = 0;

    bool semitoneActive(int semi) const {
        static const int T[kCount][8] = {
            {0,1,2,3,4,5,6,7},{0,2,4,7,9,-1},   {0,2,3,5,7,9,10,-1},
            {0,1,3,5,7,8,10,-1},{0,2,4,6,7,9,11,-1},{0,2,4,5,7,9,10,-1},
            {0,1,3,5,6,8,10,-1},{0,2,3,5,7,8,11,-1},{0,1,4,5,7,8,10,-1},
            {0,1,4,5,7,8,11,-1},{0,2,3,7,8,-1},  {0,2,4,5,7,9,11,-1},
            {0,2,3,5,7,8,10,-1}
        };
        for (int i = 0; i < 8 && T[scaleIdx%kCount][i] >= 0; ++i)
            if (T[scaleIdx%kCount][i] == semi) return true;
        return false;
    }

    int quantize(int midi) const {
        static const int T[kCount][8] = {
            {0,1,2,3,4,5,6,7},{0,2,4,7,9,-1},   {0,2,3,5,7,9,10,-1},
            {0,1,3,5,7,8,10,-1},{0,2,4,6,7,9,11,-1},{0,2,4,5,7,9,10,-1},
            {0,1,3,5,6,8,10,-1},{0,2,3,5,7,8,11,-1},{0,1,4,5,7,8,10,-1},
            {0,1,4,5,7,8,11,-1},{0,2,3,7,8,-1},  {0,2,4,5,7,9,11,-1},
            {0,2,3,5,7,8,10,-1}
        };
        int oct = (midi / 12) * 12;
        int semi = (midi - rootNote + 120) % 12;
        int best = 0;
        for (int i = 0; i < 8 && T[scaleIdx%kCount][i] >= 0; ++i)
            if (T[scaleIdx%kCount][i] <= semi) best = T[scaleIdx%kCount][i];
        return oct + rootNote + best;
    }
};

// ─────────────────────────────────────────────────────────────────────────────
//  FILTER — SVF + Ladder with all types including Comb
// ─────────────────────────────────────────────────────────────────────────────
struct AncientFilter {
    enum Type { LP12=0,LP24,HP,BP,Notch,Comb,Ladder };
    float s1=0,s2=0,s3=0,s4=0;
    float cutoff=8000,res=0.5f,drive=1.f;
    // Smoothed parameters to prevent clicks from instant jumps
    float sCut=8000,sRes=0.5f,sDrv=1.f;
    Type  type=LP12;
    double sr=48000;

    void reset() { s1=0; s2=0; s3=0; s4=0; sCut=cutoff; sRes=res; sDrv=drive; }

    // Smooth saturator — cubic soft-clip, NO discontinuity anywhere
    static float sat(float x) {
        if (x > 1.5f) return 1.f;
        if (x < -1.5f) return -1.f;
        return x * (1.f - x * x / 6.75f);  // smooth everywhere, derivative continuous
    }

    float process(float x) {
        // Smooth parameters toward targets — prevents click from instant jumps
        const float smooth = 0.002f;  // ~5ms at 48kHz
        sCut += smooth * (cutoff - sCut);
        sRes += smooth * (res - sRes);
        sDrv += smooth * (drive - sDrv);

        float f = 2.f * std::sin(juce::MathConstants<float>::pi
                                  * juce::jlimit(20.f,18000.f,sCut) / float(sr));
        f = juce::jlimit(0.0001f, 0.85f, f);  // was 0.95 — too high, causes aliasing

        float curRes = juce::jlimit(0.f, 0.95f, sRes);
        float curDrv = juce::jlimit(0.5f, 1.8f, sDrv);

        if (type == Ladder) {
            float fb = s4 * curRes * 2.8f;
            x = std::tanh((x - fb) * curDrv);
            float g = f * 0.5f;
            s1+=g*(x -s1); s2+=g*(s1-s2); s3+=g*(s2-s3); s4+=g*(s3-s4);
            s1=sat(s1); s2=sat(s2); s3=sat(s3); s4=sat(s4);
            return s4;
        }

        // SVF
        float q  = juce::jlimit(0.05f, 1.f, 1.f - curRes * 0.95f);
        float twoQ = 2.f * q;
        float hp = (x * curDrv - s1 * twoQ - s2) / (1.f + twoQ * f + f*f);
        float bp = s1 + f * hp;
        float lp = s2 + f * bp;
        s1 = sat(bp + f * hp);
        s2 = sat(lp + f * bp);  // FIXED: trapezoidal correction

        switch (type) {
            case HP:    return hp;
            case BP:    return bp;
            case Notch: return lp + hp;
            case Comb:  return lp + bp * curRes * 2.f;
            case LP24: {
                float hp2 = (lp - s3 * twoQ - s4) / (1.f + twoQ * f + f*f);
                float bp2 = s3 + f * hp2;
                float lp2 = s4 + f * bp2;
                s3=sat(bp2+f*hp2);
                s4=sat(lp2+f*bp2);  // FIXED: trapezoidal correction
                return lp2;
            }
            default:    return lp;
        }
    }
};

// ─────────────────────────────────────────────────────────────────────────────
//  ADSR
// ─────────────────────────────────────────────────────────────────────────────
struct ADSR {
    float a=0.01f,d=0.2f,s=0.8f,r=0.5f,val=0,phase=0;
    float startVal=0;  // level at moment of trigger — attack ramps from here
    bool  gate=false; double sr=44100;
    float tick() {
        float dt = 1.f/float(sr);
        if (gate) {
            if      (phase < a)   {
                float t = phase/std::max(a,.001f);  // 0→1 over attack time
                val = startVal + (1.f - startVal) * t;  // ramp from startVal to 1.0
                phase+=dt;
            }
            else if (phase < a+d) { val = 1.f-(phase-a)/std::max(d,.001f)*(1.f-s); phase+=dt; }
            else                    val = s;
        } else { val -= dt/std::max(r,.001f); if(val<0)val=0; }
        return val;
    }
    void on()  { gate=true; phase=0; startVal=val; }  // NO val=0 — attack from current level
    void off() { gate=false; }
    bool done() const { return !gate && val < 0.001f; }
};

// ─────────────────────────────────────────────────────────────────────────────
//  LFO
// ─────────────────────────────────────────────────────────────────────────────
struct LFO {
    float rate=1,depth=0,phase=0,held=0; int shape=0; double sr=44100;
    std::mt19937 rng{99};
    float tick() {
        float inc = rate / float(sr), v = 0;
        switch(shape) {
            case 0: v=std::sin(phase*juce::MathConstants<float>::twoPi); break;
            case 1: v=phase<.5f?4*phase-1:3-4*phase; break;
            case 2: v=phase*2-1; break;
            case 3: v=phase<.5f?1.f:-1.f; break;
            case 4: if(phase+inc>=1) held=(rng()&0xFFFF)/32767.f-1.f; v=held; break;
        }
        phase+=inc; if(phase>=1)phase-=1;
        return v * depth;
    }
};

// ─────────────────────────────────────────────────────────────────────────────
//  SYNTH VOICE
// ─────────────────────────────────────────────────────────────────────────────
struct AncientSound : public juce::SynthesiserSound {
    bool appliesToNote(int) override    { return true; }
    bool appliesToChannel(int) override { return true; }
};

// ─────────────────────────────────────────────────────────────────────────────
//  MODULATION MATRIX
// ─────────────────────────────────────────────────────────────────────────────
struct ModMatrix {
    enum Source  { None=0, LFO1, LFO2, Env1, Env2, Velocity, ModWheel, Aftertouch, NumSources };
    enum Dest    { Pitch=0, Cutoff, Resonance, FMDepth, WTPos, AmpLevel, Pan, GrSize, NumDests };

    static constexpr const char* kSrcNames[] = {
        "None","LFO 1","LFO 2","Amp Env","Filt Env","Velocity","Mod Wheel","Aftertouch"
    };
    static constexpr const char* kDstNames[] = {
        "Pitch","Cutoff","Resonance","FM Depth","WT Position","Amp Level","Pan","Grain Size"
    };

    struct Slot { int src=0; int dst=0; float amt=0; };
    static constexpr int kSlots = 4;
    std::array<Slot,kSlots> slots;

    struct Outputs {
        float pitch=0,cutoff=0,reso=0,fmDepth=0,wtPos=0,amp=1,pan=0,grSize=0;
    };

    Outputs compute(float l1, float l2, float aEnv, float fEnv,
                    float vel, float mw, float at) const
    {
        float srcs[NumSources] = {0,l1,l2,aEnv,fEnv,vel,mw,at};
        Outputs out; out.amp = 1.f;
        for (auto& s : slots) {
            if (s.src==None || s.amt==0.f) continue;
            float v = srcs[s.src] * s.amt;
            switch (s.dst) {
                case Pitch:      out.pitch   += v; break;
                case Cutoff:     out.cutoff  += v * 8000.f; break;
                case Resonance:  out.reso    += v; break;
                case FMDepth:    out.fmDepth += v; break;
                case WTPos:      out.wtPos   += v; break;
                case AmpLevel:   out.amp     *= (1.f + v); break;
                case Pan:        out.pan     += v; break;
                case GrSize:     out.grSize  += v * 0.2f; break;
                default: break;
            }
        }
        return out;
    }
};

// ─────────────────────────────────────────────────────────────────────────────
//  HISTORICAL TUNING TABLES
// ─────────────────────────────────────────────────────────────────────────────
struct HistoricalTuning {
    enum System {
        EqualTemperament=0,
        JustIntonation,
        Pythagorean,
        SumerianPentatonic,
        EgyptianModal,
        MesopotamianDiatonic,
        NumSystems
    };

    static constexpr const char* kNames[] = {
        "Equal Temperament","Just Intonation","Pythagorean",
        "Sumerian Pentatonic","Egyptian Modal","Mesopotamian Diatonic"
    };

    static const float* getCents(System s) {
        static const float ET[12]   = {0,0,0,0,0,0,0,0,0,0,0,0};
        static const float JI[12]   = {0,11.7f,-3.9f,15.6f,-13.7f,-2.0f,-9.8f,2.0f,13.7f,-15.6f,17.6f,-11.7f};
        static const float PY[12]   = {0,13.7f,3.9f,17.6f,7.8f,-2.0f,11.7f,2.0f,15.6f,5.9f,19.6f,9.8f};
        static const float SU[12]   = {0,0,-27.f,0,-14.f,0,0,-2.f,0,-16.f,0,-29.f};
        static const float EG[12]   = {0,0,-49.f,0,-14.f,0,0,2.f,0,-16.f,-49.f,0};
        static const float ME[12]   = {0,5.f,-8.f,10.f,-6.f,2.f,-5.f,3.f,8.f,-3.f,-35.f,7.f};
        switch(s) {
            case JustIntonation:       return JI;
            case Pythagorean:          return PY;
            case SumerianPentatonic:   return SU;
            case EgyptianModal:        return EG;
            case MesopotamianDiatonic: return ME;
            default:                   return ET;
        }
    }

    // FIX: Standard MIDI→freq with microtuning.  rootNote only affects
    // which pitch class receives which cent offset — NOT the base octave.
    static float getMidiFreq(int midiNote, System sys, int rootNote=0) {
        int   pc = ((midiNote - rootNote) % 12 + 12) % 12;
        float cents = getCents(sys)[pc];
        return 440.f * std::pow(2.f, (float(midiNote - 69) + cents / 100.f) / 12.f);
    }
};

class AncientVoice : public juce::SynthesiserVoice {
public:
    struct Params {
        int   engine=0,wtShape=0,fmAlgo=0,filtType=0;
        float wtPos=0,fmFb=0.3f;
        int   grSource=0;  // granular source timbre (0-7)
        float grSz=0.08f,grSc=0.3f,grDen=0.6f,grPr=0.1f;
        float ksBody=0.5f,ksPick=0.25f,ksStretch=0.5f; int ksColor=0;
        float wgStiff=0.3f,wgDamp=0.4f;
        float ampA=0.01f,ampD=0.2f,ampS=0.8f,ampR=0.5f;
        float fCut=8000,fRes=0.3f,fDrv=1,fEnvAmt=0.3f,fKeyTrack=0;
        float fA=0.01f,fD=0.2f,fS=0.5f,fR=0.5f;
        float l1Rate=1,l1Depth=0,l1Shape=0,l1Dest=0;
        float l2Rate=2,l2Depth=0,l2Shape=0,l2Dest=1;
        float glide=0,detune=0,uniSpread=0,masterTune=0;
        float uniDetuneCents=0;
        float modWheel=0,aftertouch=0;
        int   tuningSystem=0,tuningRoot=0;
        ModMatrix modMatrix;
    } p;

    bool canPlaySound(juce::SynthesiserSound* s) override { return s != nullptr; }

    void startNote(int note, float vel, juce::SynthesiserSound*, int) override {
        note_ = note; vel_ = vel;
        double sr = getSampleRate();
        float freq = HistoricalTuning::getMidiFreq(
            note,
            HistoricalTuning::System(p.tuningSystem),
            p.tuningRoot
        );
        freq *= std::pow(2.f, p.uniDetuneCents / 1200.f);
        tFreq_ = freq;
        cFreq_ = (p.glide < 0.001f) ? freq : cFreq_;

        wt_.buildShape(p.wtShape, freq, sr);
        wt_.setNote(freq, sr);
        wt_.reset();  // reset phase to 0 — stale phase causes click
        fm_.setAlgo(p.fmAlgo); fm_.setNote(freq, sr); fm_.feedback = p.fmFb;
        for (auto& op : fm_.ops) op.phase = 0;  // reset phases to prevent click
        fm_.on();
        gran_.grainSize=p.grSz; gran_.scatter=p.grSc;
        gran_.density=p.grDen; gran_.pitchRnd=p.grPr;
        gran_.prepare(sr, p.grSource);  // pass source type!
        gran_.setNote(freq, sr);        // pitch tracking!
        kp_.body=p.ksBody; kp_.pick=p.ksPick; kp_.stretch=p.ksStretch;
        kp_.color=p.ksColor;
        kp_.setNote(freq, sr); kp_.excite();
        wg_.damping=p.wgDamp; wg_.stiffness=p.wgStiff;
        wg_.setNote(freq, sr); wg_.pluck();

        // Do NOT reset filter — states persist between re-triggers.
        // fadeIn_ ramp handles the transition from previous state smoothly.
        filt_.sr = sr;
        filt_.cutoff = juce::jlimit(20.f, 20000.f, p.fCut);
        filt_.res = p.fRes;
        filt_.drive = p.fDrv;
        filt_.type = AncientFilter::Type(p.filtType);
        // Set up LFOs FIRST so we can compute their initial contribution
        lfo1_.sr=lfo2_.sr=sr;
        lfo1_.rate=p.l1Rate; lfo1_.depth=p.l1Depth; lfo1_.shape=int(p.l1Shape);
        lfo2_.rate=p.l2Rate; lfo2_.depth=p.l2Depth; lfo2_.shape=int(p.l2Shape);
        lfo1_.phase = 0; lfo2_.phase = 0;
        // At phase=0: sin(0)=0, so LFO contribution starts at 0 — matches renderNextBlock's first tick

        // Initialize smoothed params INCLUDING filter envelope + keytracking + LFO initial state.
        // This must match the EXACT formula in renderNextBlock to prevent any 1-sample mismatch.
        float initialFiltEnv = filtEnv_.val * p.fEnvAmt * 8000.f;
        float ktOffset = (note_ - 60) * p.fKeyTrack * 100.f;
        float initCut = p.fCut * std::pow(2.f, ktOffset / 1200.f) + initialFiltEnv;
        // LFO at phase=0 contributes 0, so no addition needed
        filt_.sCut = juce::jlimit(20.f, 18000.f, initCut);
        filt_.sRes = filt_.res;
        filt_.sDrv = filt_.drive;

        ampEnv_.sr=filtEnv_.sr=sr;
        ampEnv_.a=p.ampA; ampEnv_.d=p.ampD; ampEnv_.s=p.ampS; ampEnv_.r=p.ampR;
        filtEnv_.a=p.fA; filtEnv_.d=p.fD; filtEnv_.s=p.fS; filtEnv_.r=p.fR;
        ampEnv_.on(); filtEnv_.on();
        fadeIn_ = 0; releaseSamples_ = 0; silenceCount_ = 0;
    }

    void stopNote(float, bool tail) override {
        ampEnv_.off(); filtEnv_.off(); fm_.off();
        if (!tail) {
            // Kill everything immediately
            ampEnv_.val = 0; filtEnv_.val = 0;
            std::fill(kp_.buf.begin(), kp_.buf.end(), 0.f);
            kp_.dcState = 0.f;
            std::fill(wg_.buf.begin(), wg_.buf.end(), 0.f);
            wg_.lp1 = 0; wg_.lp2 = 0; wg_.ap1 = 0; wg_.dcState = 0;
            // Kill all active grains
            for (auto& g : gran_.grains) g.on = false;
            gran_.spawnTimer = 99.f;
            filt_.reset();  // clean slate for when this voice is reused
            fadeIn_ = 256; releaseSamples_ = 0; silenceCount_ = 0;
            clearCurrentNote();
        }
    }

    void pitchWheelMoved(int v) override { pb_ = (v-8192)/8192.f * 2.f; }
    void controllerMoved(int ctrl, int val) override {
        if (ctrl == 1)  p.modWheel   = val / 127.f;
        if (ctrl == 64) gate64_ = val > 63;
    }
    void aftertouchChanged(int v) override { p.aftertouch = v / 127.f; }

    void renderNextBlock(juce::AudioBuffer<float>& buf, int start, int num) override {
        juce::ScopedNoDenormals noDenormals;  // CRITICAL: prevents denormal CPU spikes
        const double sr = getSampleRate();
        if (sr <= 0.0) return;
        filt_.sr = sr;
        ampEnv_.sr = filtEnv_.sr = lfo1_.sr = lfo2_.sr = sr;
        for (int i = start; i < start+num; ++i) {
            float gc = (p.glide < 0.001f) ? 1.f
                      : std::exp(-1.f / (p.glide * float(sr)));
            cFreq_ = tFreq_ + (cFreq_ - tFreq_) * gc;
            float freq = cFreq_ * std::pow(2.f, (pb_ + p.detune/100.f + p.masterTune/100.f) / 12.f);

            float l1 = lfo1_.tick(), l2 = lfo2_.tick();

            if (int(p.l1Dest)==0) freq *= (1.f + l1 * 0.05f);
            if (int(p.l2Dest)==0) freq *= (1.f + l2 * 0.05f);

            float aEnvVal = ampEnv_.val, fEnvVal = filtEnv_.val;
            auto mod = p.modMatrix.compute(l1, l2, aEnvVal, fEnvVal,
                                           vel_, p.modWheel, p.aftertouch);
            freq *= std::pow(2.f, mod.pitch / 12.f);

            wt_.phaseInc = freq / float(sr) * WavetableEngine::kSize;
            fm_.setNote(freq, sr);
            gran_.setNote(freq, sr);  // update granular pitch every sample
            wt_.wtPos = juce::jlimit(0.f,1.f, p.wtPos + mod.wtPos);
            gran_.grainSize = juce::jlimit(0.005f,0.5f, p.grSz + mod.grSize);

            for (auto& op : fm_.ops) op.level = juce::jlimit(0.f,1.f, 1.f + mod.fmDepth);

            // Stop granular spawning during release
            if (!ampEnv_.gate) gran_.density = 0.f;
            else gran_.density = juce::jlimit(0.01f, 1.f, p.grDen);

            float raw = 0.f;
            switch (p.engine) {
                case 0: raw = wt_.tick();   break;
                case 1: raw = fm_.tick();   break;
                case 2: raw = gran_.tick(); break;
                case 3: raw = kp_.tick();   break;
                case 4: raw = wg_.tick();   break;
            }

            // Fade ALL engines — KS/WG step into resonant filter = crunch
            if (fadeIn_ < 256) {
                raw *= float(fadeIn_) / 256.f;
                ++fadeIn_;
            }

            // Track output energy — if voice is producing silence, kill it
            silenceCount_ = (std::abs(raw) < 0.0005f) ? silenceCount_ + 1 : 0;

            float ktOffset = (note_ - 60) * p.fKeyTrack * 100.f;
            float fc = p.fCut * std::pow(2.f, ktOffset/1200.f)
                      + filtEnv_.tick() * p.fEnvAmt * 8000.f
                      + mod.cutoff;
            if (int(p.l1Dest)==1) fc += l1 * 2000.f;
            if (int(p.l2Dest)==1) fc += l2 * 2000.f;
            filt_.cutoff = juce::jlimit(20.f,20000.f,fc);
            filt_.res    = juce::jlimit(0.f,1.f, p.fRes + mod.reso);
            filt_.drive  = p.fDrv;
            filt_.type   = AncientFilter::Type(p.filtType);

            float amp = ampEnv_.tick() * vel_ * mod.amp;
            if (int(p.l1Dest)==3) amp *= (1.f + l1 * 0.5f);
            if (int(p.l2Dest)==3) amp *= (1.f + l2 * 0.5f);
            // Hard-kill after 1s of release
            if (!ampEnv_.gate) {
                releaseSamples_++;
                if (releaseSamples_ > int(1.f * float(sr))) {
                    amp = 0; ampEnv_.val = 0;
                }
            }
            float out = filt_.process(raw) * amp;
            out = juce::jlimit(-1.2f, 1.2f, out);

            float pan = juce::jlimit(-1.f,1.f, mod.pan);
            float gainL = (pan <= 0.f) ? 1.f : (1.f - pan);
            float gainR = (pan >= 0.f) ? 1.f : (1.f + pan);

            if (buf.getNumChannels() > 1) {
                buf.addSample(0, i, out * gainL);
                buf.addSample(1, i, out * gainR);
            } else {
                buf.addSample(0, i, out);
            }
        }
        // Kill voice if: envelope done, envelope very low, or engine producing silence
        if (ampEnv_.done()
            || (!ampEnv_.gate && ampEnv_.val < 0.0001f)
            || silenceCount_ > 4096)  // ~85ms of silence = voice has decayed
            clearCurrentNote();
    }

private:
    WavetableEngine wt_;
    FMEngine        fm_;
    GranularEngine  gran_;
    KarplusEngine   kp_;
    WaveguideEngine wg_;
    AncientFilter   filt_;
    ADSR            ampEnv_, filtEnv_;
    LFO             lfo1_, lfo2_;
    int   note_=60; float vel_=1, pb_=0, tFreq_=440, cFreq_=440;
    int   fadeIn_=256, releaseSamples_=0, silenceCount_=0;
    bool  gate64_=false;
};

// ─────────────────────────────────────────────────────────────────────────────
//  ARPEGGIATOR
// ─────────────────────────────────────────────────────────────────────────────
struct Arpeggiator {
    bool  on=false; int mode=0, rateDiv=8, step=0, dir=1;
    float gate=0.8f;
    std::vector<int> held;
    double phase=0; std::mt19937 rng{55};

    void noteOn (int n) { held.push_back(n); }
    void noteOff(int n) { held.erase(std::remove(held.begin(),held.end(),n),held.end()); }

    int tick(double sr, double bpm) {
        if (!on || held.empty()) return -1;
        phase++;
        double dur = 60.0 / (bpm * rateDiv / 4.0) * sr;
        if (phase < dur) return -1;
        phase = 0;
        int n = int(held.size());
        switch (mode) {
            case 0: step=(step+1)%n; break;
            case 1: step=(step-1+n)%n; break;
            case 2: step+=dir; if(step>=n){step=n-2;dir=-1;} if(step<0){step=1;dir=1;} break;
            case 3: step=rng()%n; break;
            default:step=(step+1)%n; break;
        }
        return held[step % held.size()];
    }
};

// ═══════════════════════════════════════════════════════════════════════
//  MASTER FX CHAIN
// ═══════════════════════════════════════════════════════════════════════
struct MasterFX {

    struct BiquadState { float x1=0,x2=0,y1=0,y2=0; };
    struct BiquadCoeffs {
        float b0=1,b1=0,b2=0,a1=0,a2=0;
        void lowShelf(float freq,float gainDb,double sr){
            float A=std::pow(10.f,gainDb/40.f),w0=juce::MathConstants<float>::twoPi*freq/float(sr);
            float cw=std::cos(w0),alpha=std::max(std::sin(w0)/2.f*std::sqrt((A+1.f/A)*(1.f)+2.f),0.001f);
            float a0=(A+1.f)-(A-1.f)*cw+2.f*std::sqrt(A)*alpha;
            b0=A*((A+1.f)-(A-1.f)*cw+2.f*std::sqrt(A)*alpha)/a0;
            b1=2.f*A*((A-1.f)-(A+1.f)*cw)/a0;
            b2=A*((A+1.f)-(A-1.f)*cw-2.f*std::sqrt(A)*alpha)/a0;
            a1=-2.f*((A-1.f)+(A+1.f)*cw)/a0;
            a2=((A+1.f)+(A-1.f)*cw-2.f*std::sqrt(A)*alpha)/a0;
        }
        void highShelf(float freq,float gainDb,double sr){
            float A=std::pow(10.f,gainDb/40.f),w0=juce::MathConstants<float>::twoPi*freq/float(sr);
            float cw=std::cos(w0),alpha=std::max(std::sin(w0)/2.f*std::sqrt((A+1.f/A)*(1.f)+2.f),0.001f);
            float a0=(A+1.f)-(A-1.f)*cw+2.f*std::sqrt(A)*alpha;
            b0=A*((A+1.f)+(A-1.f)*cw+2.f*std::sqrt(A)*alpha)/a0;
            b1=-2.f*A*((A-1.f)+(A+1.f)*cw)/a0;
            b2=A*((A+1.f)+(A-1.f)*cw-2.f*std::sqrt(A)*alpha)/a0;
            a1=2.f*((A-1.f)-(A+1.f)*cw)/a0;
            a2=((A+1.f)-(A-1.f)*cw-2.f*std::sqrt(A)*alpha)/a0;
        }
        void peakEQ(float freq,float gainDb,float Q,double sr){
            float A=std::pow(10.f,gainDb/40.f),w0=juce::MathConstants<float>::twoPi*freq/float(sr);
            float alpha=std::sin(w0)/(2.f*std::max(Q,0.1f)),a0=1.f+alpha/A;
            b0=(1.f+alpha*A)/a0; b1=(-2.f*std::cos(w0))/a0; b2=(1.f-alpha*A)/a0;
            a1=(-2.f*std::cos(w0))/a0; a2=(1.f-alpha/A)/a0;
        }
        float process(float x,BiquadState& s) const {
            float y=b0*x+b1*s.x1+b2*s.x2-a1*s.y1-a2*s.y2;
            s.x2=s.x1; s.x1=x; s.y2=s.y1; s.y1=y; return y;
        }
    };
    BiquadCoeffs eqLoC,eqMidC,eqHiC;
    BiquadState  eqLoL,eqLoR,eqMidL,eqMidR,eqHiL,eqHiR;
    float eqLo=0,eqMid=0,eqHi=0,eqLoFreq=200.f,eqMidFreq=1000.f,eqMidQ=1.f,eqHiFreq=6000.f;

    void rebuildEQ(double sr){
        eqLoC.lowShelf(eqLoFreq,eqLo,sr);
        eqMidC.peakEQ(eqMidFreq,eqMid,eqMidQ,sr);
        eqHiC.highShelf(eqHiFreq,eqHi,sr);
    }

    // Saturation
    float distAmt=0,distTone=0.5f,distPostGain=1.f; int distType=0;
    float distLPS=0,distRPS=0,distLHS=0,distRHS=0;
    float saturate(float x,int type,float drive){
        x*=drive;
        switch(type){
            case 0: return juce::jlimit(-1.f,1.f,x);
            case 1: return std::tanh(x);
            case 2: return x/(1.f+std::abs(x));
            case 3: { float a=std::abs(x); return a<1.f?x*(2.f-a):juce::jlimit(-1.f,1.f,x); }
            case 4: { float f=x; while(f>1.f)f=2.f-f; while(f<-1.f)f=-2.f-f; return f; }
            default: return x;
        }
    }

    // Chorus
    static constexpr int kCBSize = 8192;
    std::vector<float> cBufL, cBufR;
    int cWp=0; float chrLFO[4]{};
    float chrMix=0,chrRate=0.3f,chrDepth=0.005f,chrSpread=0.7f;

    // Phaser
    static constexpr int kPH=8;
    float phAP[kPH][2]{},phLFO=0,phFBL=0,phFBR=0;
    float phaserMix=0,phaserRate=0.5f,phaserDepth=0.8f,phaserFB=0.6f;
    float allpass(float x,float c,float& s){float o=-c*x+s;s=x+c*o;return o;}

    // Delay
    static constexpr int kDBSize = 131072;
    std::vector<float> dBufL, dBufR;
    int dWpL=0,dWpR=0;
    float dlyMix=0,dlyTime=0.3f,dlyFB=0.4f,dlyDampL=0,dlyDampR=0,dlyDamping=0.3f;

    // Reverb
    static constexpr int kRN=8;
    static constexpr int kRL[kRN]{1557,1617,1491,1422,1277,1356,1188,1116};
    static constexpr int kRBufSize = 16384;
    static constexpr int kAPL=2;
    static constexpr int kAPLen[kAPL]{556,441};
    static constexpr int kAPBufSize = 1024;
    std::vector<std::vector<float>> rBuf, apBufL, apBufR;
    std::vector<int> rPos, apPos;
    std::vector<float> rLP, rLen, apLen;
    static constexpr int kPDBSize = 8192;
    std::vector<float> preDelBufL, preDelBufR;
    int preDelPos=0;
    float revMix=0.3f,revSize=0.7f,revDamp=0.4f,revPreDelay=0.f,revWidth=0.8f,revShimmer=0.f;
    static constexpr int kShimSize = 4096;
    std::vector<float> shimBufL, shimBufR;
    float shimPhase=0; int shimWp=0;

    // Master
    float masterVol=0.8f,stereoWidth=1.f,gain=1.f;
    double sr=44100;

    void prepare(double _sr){
        sr=_sr;
        cBufL.assign(kCBSize,0.f);  cBufR.assign(kCBSize,0.f);
        dBufL.assign(kDBSize,0.f);  dBufR.assign(kDBSize,0.f);
        shimBufL.assign(kShimSize,0.f); shimBufR.assign(kShimSize,0.f);
        preDelBufL.assign(kPDBSize,0.f); preDelBufR.assign(kPDBSize,0.f);

        rBuf.assign(kRN, std::vector<float>(kRBufSize,0.f));
        apBufL.assign(kAPL, std::vector<float>(kAPBufSize,0.f));
        apBufR.assign(kAPL, std::vector<float>(kAPBufSize,0.f));
        rPos.assign(kRN,0);   apPos.assign(kAPL,0);
        rLP.assign(kRN,0.f);
        rLen.assign(kRN,0.f); for(int i=0;i<kRN;++i) rLen[i]=float(kRL[i])*float(sr)/44100.f;
        apLen.assign(kAPL,0.f);
        apLen[0]=float(kAPLen[0])*float(sr)/44100.f;
        apLen[1]=float(kAPLen[1])*float(sr)/44100.f;
        reset(sr);
    }

    void reset(double _sr) {
        sr = _sr;
        for(int i=0;i<kRN;++i) { if(!rLen.empty()) rLen[i]=float(kRL[i])*float(sr)/44100.f; }
        if(apLen.size()>=2){apLen[0]=float(kAPLen[0])*float(sr)/44100.f; apLen[1]=float(kAPLen[1])*float(sr)/44100.f;}
        eqLoL={};eqLoR={};eqMidL={};eqMidR={};eqHiL={};eqHiR={};
        for(auto& v:{std::ref(cBufL),std::ref(cBufR),std::ref(dBufL),std::ref(dBufR),
                     std::ref(shimBufL),std::ref(shimBufR),std::ref(preDelBufL),std::ref(preDelBufR)})
            std::fill(v.get().begin(),v.get().end(),0.f);
        for(auto& row:rBuf)  std::fill(row.begin(),row.end(),0.f);
        for(auto& row:apBufL) std::fill(row.begin(),row.end(),0.f);
        for(auto& row:apBufR) std::fill(row.begin(),row.end(),0.f);
        std::fill(rPos.begin(),rPos.end(),0);
        std::fill(apPos.begin(),apPos.end(),0);
        std::fill(rLP.begin(),rLP.end(),0.f);
        std::fill(chrLFO,chrLFO+4,0.f);
        std::fill(phAP[0],phAP[0]+kPH*2,0.f);
        cWp=0; dWpL=0; dWpR=0; phLFO=0; phFBL=0; phFBR=0;
        distLPS=0; distRPS=0; distLHS=0; distRHS=0;
        dlyDampL=0; dlyDampR=0; shimPhase=0; shimWp=0; preDelPos=0;
        rebuildEQ(sr);
    }

    std::pair<float,float> process(float L,float R){
        // Pre-FX gain stage (dB converted to linear by caller)
        L *= gain; R *= gain;

        if(std::abs(eqLo)>0.05f){L=eqLoC.process(L,eqLoL);R=eqLoC.process(R,eqLoR);}
        if(std::abs(eqMid)>0.05f){L=eqMidC.process(L,eqMidL);R=eqMidC.process(R,eqMidR);}
        if(std::abs(eqHi)>0.05f){L=eqHiC.process(L,eqHiL);R=eqHiC.process(R,eqHiR);}

        if(distAmt>0.001f){
            float lpC=juce::jlimit(0.01f,0.99f,0.3f+distTone*0.65f);
            distLPS+=lpC*(L-distLPS); L=distLPS; distRPS+=lpC*(R-distRPS); R=distRPS;
            float drive=1.f+distAmt*6.f;
            L=saturate(L,distType,drive)*distPostGain;
            R=saturate(R,distType,drive)*distPostGain;
            float hpC=0.05f+distTone*0.25f;
            distLHS+=hpC*(L-distLHS); L=L-distLHS*(1.f-distTone);
            distRHS+=hpC*(R-distRHS); R=R-distRHS*(1.f-distTone);
        }

        if(chrMix>0.001f){
            static constexpr float spd[4]={1.f,1.13f,0.87f,1.27f};
            static constexpr float del[4]={0.010f,0.017f,0.013f,0.021f};
            static constexpr float pan[4]={-1.f,1.f,-0.6f,0.6f};
            cBufL[size_t(cWp%kCBSize)]=L; cBufR[size_t(cWp%kCBSize)]=R;
            float wL=0,wR=0;
            for(int v=0;v<4;++v){
                chrLFO[v]+=spd[v]*chrRate/float(sr);
                if(chrLFO[v]>1.f)chrLFO[v]-=1.f;
                float lfo=std::sin(chrLFO[v]*juce::MathConstants<float>::twoPi);
                int d=juce::jlimit(1,kCBSize-1,int((del[v]+chrDepth*lfo)*float(sr)));
                int ri=((cWp-d)%kCBSize+kCBSize)%kCBSize;
                float p=pan[v]*chrSpread;
                wL+=cBufL[size_t(ri)]*(p<=0.f?1.f:1.f-p)*0.25f;
                wR+=cBufR[size_t(ri)]*(p>=0.f?1.f:1.f+p)*0.25f;
            }
            L+=wL*chrMix; R+=wR*chrMix; cWp++;
        }

        if(phaserMix>0.001f){
            phLFO+=phaserRate/float(sr); if(phLFO>1.f)phLFO-=1.f;
            float lfo=(std::sin(phLFO*juce::MathConstants<float>::twoPi)+1.f)*0.5f;
            float coeff=std::cos(juce::MathConstants<float>::pi*(0.1f+phaserDepth*lfo*0.4f));
            float pL=L+phFBL*phaserFB,pR=R+phFBR*phaserFB;
            for(int s=0;s<kPH;++s){pL=allpass(pL,coeff,phAP[s][0]);pR=allpass(pR,coeff,phAP[s][1]);}
            phFBL=pL; phFBR=pR; L+=pL*phaserMix; R+=pR*phaserMix;
        }

        if(dlyMix>0.001f){
            int ds=juce::jlimit(1,kDBSize-1,int(dlyTime*float(sr)));
            float dL=dBufL[size_t(((dWpL-ds)%kDBSize+kDBSize)%kDBSize)];
            float dR=dBufR[size_t(((dWpR-ds)%kDBSize+kDBSize)%kDBSize)];
            dlyDampL+=dlyDamping*(dL-dlyDampL);
            dlyDampR+=dlyDamping*(dR-dlyDampR);
            dBufL[size_t(dWpL%kDBSize)]=R+dlyDampR*dlyFB;
            dBufR[size_t(dWpR%kDBSize)]=L+dlyDampL*dlyFB;
            dWpL++; dWpR++; L+=dL*dlyMix; R+=dR*dlyMix;
        }

        if(revMix>0.001f){
            int preD=juce::jlimit(0,kPDBSize-1,int(revPreDelay*0.001f*float(sr)));
            float pdL=L,pdR=R;
            if(preD>0){
                int rp=((preDelPos-preD)%kPDBSize+kPDBSize)%kPDBSize;
                pdL=preDelBufL[size_t(rp)]; pdR=preDelBufR[size_t(rp)];
                preDelBufL[size_t(preDelPos%kPDBSize)]=L;
                preDelBufR[size_t(preDelPos%kPDBSize)]=R;
                preDelPos++;
            }
            float mono=(pdL+pdR)*0.5f,combL=0,combR=0;
            for(int i=0;i<kRN;++i){
                int p2=rPos[size_t(i)],rl=juce::jmax(1,int(rLen[size_t(i)]));
                float s=rBuf[size_t(i)][size_t(p2%kRBufSize)];
                rLP[size_t(i)]=s*(1.f-revDamp*0.4f)+rLP[size_t(i)]*revDamp*0.4f;
                rBuf[size_t(i)][size_t(p2%kRBufSize)]=mono+rLP[size_t(i)]*revSize*0.98f;
                rPos[size_t(i)]=(p2+1)%rl;
                if(i%2==0)combL+=s; else combR+=s;
            }
            for(int i=0;i<kAPL;++i){
                int p2=apPos[size_t(i)],al=juce::jmax(1,int(apLen[size_t(i)]));
                float aL=apBufL[size_t(i)][size_t(p2%kAPBufSize)];
                float aR=apBufR[size_t(i)][size_t(p2%kAPBufSize)];
                apBufL[size_t(i)][size_t(p2%kAPBufSize)]=combL+0.5f*aL;
                apBufR[size_t(i)][size_t(p2%kAPBufSize)]=combR+0.5f*aR;
                combL=aL-0.5f*(combL+0.5f*aL); combR=aR-0.5f*(combR+0.5f*aR);
                apPos[size_t(i)]=(p2+1)%al;
            }
            float rm=(combL+combR)*0.5f,rs=(combL-combR)*0.5f*revWidth;
            combL=rm+rs; combR=rm-rs;
            if(revShimmer>0.001f){
                shimBufL[size_t(shimWp%kShimSize)]=combL;
                shimBufR[size_t(shimWp%kShimSize)]=combR;
                shimPhase+=2.f/kShimSize; if(shimPhase>=1.f)shimPhase-=1.f;
                int sr2=int(shimPhase*kShimSize);
                combL+=shimBufL[size_t(sr2%kShimSize)]*revShimmer;
                combR+=shimBufR[size_t(sr2%kShimSize)]*revShimmer; shimWp++;
            }
            L=L*(1.f-revMix)+combL*revMix*0.25f;
            R=R*(1.f-revMix)+combR*revMix*0.25f;
        }

        float mid=(L+R)*0.5f,side=(L-R)*0.5f*stereoWidth;
        return {(mid+side)*masterVol,(mid-side)*masterVol};
    }
};
