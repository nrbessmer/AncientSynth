#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_dsp/juce_dsp.h>
#include <array>
#include <random>

// ─────────────────────────────────────────────────────────────────────────────
//  WAVETABLE ENGINE  — 8 ancient shapes, 8-frame morph, 32 harmonics
// ─────────────────────────────────────────────────────────────────────────────
struct WavetableEngine {
    static constexpr int kFrames = 8, kSize = 2048, kHarmonics = 32;
    using Table = std::vector<float>;
    std::vector<Table> tables;
    float phase = 0.f, phaseInc = 0.f, wtPos = 0.f;
    int   shape = 0;
    int   builtShape_ = -1, builtMaxH_ = 0;  // cache to skip rebuild
    WavetableEngine() : tables(kFrames, Table(kSize, 0.f)) {}

    static float harmonicAmp(int s, int h) {
        float hf = float(h);
        switch (s) {
            case 0: // Plucked string — bright attack, rapid harmonic decay
                return (1.f / hf) * (h % 2 == 0 ? 0.85f : 1.f);
            case 1: { // Kithara — ancient lyre: warm, rich evens, slow rolloff
                float base = 1.f / std::pow(hf, 0.55f);
                if (h % 2 == 0) base *= 1.35f;
                if (h >= 3 && h <= 6) base *= 1.2f;  // body resonance
                return base;
            }
            case 2: { // Ney flute — breathy fundamental, sparse upper partials
                if (h == 1) return 1.f;
                if (h == 2) return 0.04f;
                if (h == 3) return 0.25f;
                if (h % 2 == 1) return 0.12f / std::pow(hf, 0.7f);
                return 0.008f / hf;
            }
            case 3: { // Reed/Mizmar — nasal, buzzy, all harmonics present
                float base = 1.f / std::pow(hf, 0.35f);
                // Nasal formant around harmonics 3-7
                float formant = 0.5f * std::exp(-0.3f * (hf - 5.f) * (hf - 5.f));
                return base + formant;
            }
            case 4: { // Brass/Hasosra — bright, blazing, formant peak 6-14
                float base = (h % 2 == 1) ? 1.f / std::pow(hf, 0.45f)
                                           : 0.6f / std::pow(hf, 0.55f);
                float f1 = 0.5f * std::exp(-0.08f * (hf - 10.f) * (hf - 10.f));
                return base + f1;
            }
            case 5: { // Organ/Temple pipes — stopped pipe: strong odds + sub coupling
                if (h % 2 == 0) return (h == 2) ? 0.2f : 0.0f;
                return 1.f / std::pow(hf, 0.8f);
            }
            case 6: { // Bell/Sistrum — shimmering inharmonic partials
                static const float g[] = {
                    1.0f,0.5f,0.95f,0.1f,0.75f,0.05f,0.55f,0.85f,
                    0.03f,0.45f,0.7f,0.02f,0.35f,0.6f,0.01f,0.25f,
                    0.5f,0.01f,0.2f,0.4f,0.01f,0.15f,0.35f,0.01f,
                    0.1f,0.25f,0.01f,0.08f,0.2f,0.01f,0.06f,0.15f
                };
                return g[h - 1] / std::pow(hf, 0.2f);
            }
            case 7: { // Vocal/Chant — deep "om" with throat formants
                float base = 0.12f / std::pow(hf, 0.7f);
                // Throat singing formants: ~150Hz, ~700Hz, ~1200Hz, ~2500Hz, ~3500Hz
                float f1 = 0.9f  * std::exp(-3.f  * (hf-1.5f)*(hf-1.5f));  // chest
                float f2 = 0.75f * std::exp(-1.2f * (hf-3.f)*(hf-3.f));    // throat
                float f3 = 0.55f * std::exp(-0.4f * (hf-6.f)*(hf-6.f));    // nasal
                float f4 = 0.35f * std::exp(-0.2f * (hf-11.f)*(hf-11.f));  // head
                float f5 = 0.2f  * std::exp(-0.15f* (hf-16.f)*(hf-16.f));  // air
                return base + f1 + f2 + f3 + f4 + f5;
            }
            default: return 1.f / hf;
        }
    }

    void buildShape(int s, float freq = 440.f, double sr = 44100.0) {
        // Only rebuild when shape changes — NOT per-note frequency
        if (s == builtShape_) return;
        builtShape_ = s;
        shape = s;
        const int nH = kHarmonics;  // fixed count — filter handles brightness
        for (int f = 0; f < kFrames; ++f) {
            float blur = f / float(kFrames - 1);
            for (int n = 0; n < kSize; ++n) {
                float v = 0.f;
                for (int h = 1; h <= nH; ++h) {
                    float amp = harmonicAmp(s, h);
                    amp *= (1.f - blur * 0.7f * float(h - 1) / float(kHarmonics));
                    v += amp * std::sin(juce::MathConstants<float>::twoPi * h * n / kSize);
                }
                tables[f][n] = v;
            }
            float pk = 0.f;
            for (float x : tables[f]) pk = std::max(pk, std::abs(x));
            if (pk > 0.f) for (float& x : tables[f]) x /= pk;
        }
    }

    void setNote(float freq, double sr) {
        phaseInc = freq / float(sr) * kSize;
    }

    float tick() {
        float ff = juce::jlimit(0.f, float(kFrames-1), wtPos * (kFrames - 1));
        int   f0 = int(ff), f1 = std::min(f0 + 1, kFrames - 1);
        float t  = ff - f0;
        int   i0 = int(phase) % kSize, i1 = (i0 + 1) % kSize;
        float fi = phase - int(phase);
        float s0 = tables[f0][i0] + fi * (tables[f0][i1] - tables[f0][i0]);
        float s1 = tables[f1][i0] + fi * (tables[f1][i1] - tables[f1][i0]);
        phase += phaseInc;
        if (phase >= kSize) phase -= kSize;
        return s0 + t * (s1 - s0);
    }

    void reset() { phase = 0.f; }
};

// ─────────────────────────────────────────────────────────────────────────────
//  FM ENGINE  — 4-operator, 8 algorithms
// ─────────────────────────────────────────────────────────────────────────────
struct FMEngine {
    struct Op {
        float phase=0, inc=0, level=1, prevOut=0, ratio=1;
        float eVal=0, ePhase=0, eA=0.01f, eD=0.3f, eS=0.7f, eR=0.5f;
        bool  gate=false;
        double sr=44100;

        void setNote(float freq, double _sr) {
            sr = _sr;
            inc = (freq * ratio) / float(sr) * juce::MathConstants<float>::twoPi;
        }
        void on()  { gate=true; ePhase=0; }
        void off() { gate=false; }

        float tickEnv() {
            float dt = 1.f / float(sr);
            if (gate) {
                if      (ePhase < eA)    { eVal = ePhase / std::max(eA,.001f); ePhase+=dt; }
                else if (ePhase < eA+eD) { eVal = 1.f-(ePhase-eA)/std::max(eD,.001f)*(1.f-eS); ePhase+=dt; }
                else                       eVal = eS;
            } else {
                eVal -= dt / std::max(eR,.001f);
                if (eVal < 0) eVal = 0;
            }
            return eVal;
        }

        float tick(float mod = 0.f) {
            float out = std::sin(phase + mod) * level * tickEnv();
            phase += inc;
            if (phase > juce::MathConstants<float>::twoPi)
                phase -= juce::MathConstants<float>::twoPi;
            prevOut = out;
            return out;
        }
    };

    std::array<Op,4> ops;
    int   algo = 0;
    float feedback = 0.3f;

    static constexpr float kRatios[8][4] = {
        {1,2,3,4},{1,1,2,3},{1,2,4,8},{1,3,5,7},
        {1,2,2,4},{1,1,3,5},{1,2,7,14},{1,4,7,11}
    };

    void setAlgo(int a) {
        algo = a & 7;
        for (int i = 0; i < 4; ++i) ops[i].ratio = kRatios[algo][i];
    }
    void setNote(float f, double sr) { for (auto& op : ops) op.setNote(f, sr); }
    void on()  { for (auto& op : ops) op.on(); }
    void off() { for (auto& op : ops) op.off(); }

    float tick() {
        float fb = ops[0].prevOut * feedback * 0.5f;
        float result = 0.f;
        switch (algo) {
            case 0: { float m3=ops[3].tick(),m2=ops[2].tick(m3),m1=ops[1].tick(m2);
                      result=ops[0].tick(m1+fb); } break;
            case 1: { float m3=ops[3].tick(),m1=ops[1].tick();
                      result=(ops[2].tick(m3)+ops[0].tick(m1+fb))*0.5f; } break;
            case 2: { float m3=ops[3].tick(),m2=ops[2].tick();
                      result=ops[0].tick(ops[1].tick((m2+m3)*0.5f)+fb); } break;
            case 3: { float m3=ops[3].tick(),m2=ops[2].tick(m3);
                      result=(ops[0].tick(m2+fb)+ops[1].tick())*0.5f; } break;
            case 4: { float m3=ops[3].tick();
                      result=(ops[0].tick(fb)+ops[1].tick(m3)+ops[2].tick(m3))*0.33f; } break;
            case 5: { float m3=ops[3].tick();
                      result=(ops[0].tick(m3+fb)+ops[1].tick()+ops[2].tick())*0.33f; } break;
            case 6: { result=(ops[0].tick(fb)+ops[1].tick()+ops[2].tick()+ops[3].tick())*0.25f; } break;
            default:{ float m3=ops[3].tick(),m2=ops[2].tick(m3),m1=ops[1].tick(m2);
                      result=ops[0].tick(m1+fb); } break;
        }
        return juce::jlimit(-1.f, 1.f, result);
    }
};

// ─────────────────────────────────────────────────────────────────────────────
//  GRANULAR ENGINE  — 32 grains, 8 source timbres, PITCH TRACKING
// ─────────────────────────────────────────────────────────────────────────────
struct GranularEngine {
    static constexpr int kGrains = 32, kBuf = 4096;
    std::vector<float> src = std::vector<float>(kBuf, 0.f);
    float   grainSize=0.08f, scatter=0.3f, density=0.6f, pitchRnd=0.1f;
    float   baseInc = 1.f;
    int     sourceType = 0;
    double  sr = 44100;
    std::mt19937 rng{42};
    float   spawnTimer = 0.1f;

    struct Grain { float pos=0,inc=1,age=0,dur=0; bool on=false; };
    std::array<Grain,kGrains> grains{};

    void buildSource(int type) {
        sourceType = type;
        for (int i = 0; i < kBuf; ++i) {
            float t = float(i) / kBuf;
            float ph = t * juce::MathConstants<float>::twoPi;
            switch (type) {
                case 0: { // Warm pad
                    float v = 0.f;
                    for (int h=1;h<=16;++h) v+=std::sin(ph*h)/std::pow(float(h),0.8f);
                    src[i] = v + ((rng()&0xFFFF)/65535.f-.5f)*0.04f;
                } break;
                case 1: { // Vocal/choir
                    src[i] = std::sin(ph) + 0.7f*std::sin(ph*2) + 0.9f*std::sin(ph*3)
                           + 0.4f*std::sin(ph*4) + 0.6f*std::sin(ph*5)
                           + 0.3f*std::sin(ph*7) + 0.5f*std::sin(ph*10)
                           + 0.2f*std::sin(ph*14);
                } break;
                case 2: { // Hollow/ethereal
                    src[i] = std::sin(ph)+0.5f*std::sin(ph*3)+0.3f*std::sin(ph*5)
                           +0.2f*std::sin(ph*7)+0.15f*std::sin(ph*9)+0.1f*std::sin(ph*11);
                } break;
                case 3: { // Bell/metallic
                    src[i] = std::sin(ph)+0.7f*std::sin(ph*2.2f)+0.5f*std::sin(ph*3.517f)
                           +0.4f*std::sin(ph*4.7f)+0.3f*std::sin(ph*6.3f)+0.2f*std::sin(ph*8.1f);
                } break;
                case 4: { // Breathy/wind
                    src[i] = std::sin(ph)*0.4f+std::sin(ph*2)*0.2f+std::sin(ph*3)*0.1f
                           +((rng()&0xFFFF)/65535.f-.5f)*0.5f;
                } break;
                case 5: { // Deep drone
                    float v = std::sin(ph)+0.8f*std::sin(ph*2)+0.6f*std::sin(ph*3)+0.4f*std::sin(ph*4);
                    for (int h=5;h<=24;++h) v+=(0.3f/h)*std::sin(ph*h);
                    src[i] = v;
                } break;
                case 6: { // Bright crystalline
                    float v = 0.3f*std::sin(ph);
                    for (int h=4;h<=20;++h) v+=(0.8f/std::sqrt(float(h)))*std::sin(ph*h);
                    src[i] = v;
                } break;
                default: { // Noise
                    src[i] = ((rng()&0xFFFF)/65535.f-.5f);
                } break;
            }
        }
        float pk=0; for (float x:src) pk=std::max(pk,std::abs(x));
        if (pk>0) for (float& x:src) x/=pk;
    }

    int builtSource_ = -1;  // cache to avoid rebuilding on every noteOn

    void prepare(double _sr, int type = 0) {
        sr = _sr;
        // Only rebuild source if type changed — buildSource has thousands of sin() calls
        if (type != builtSource_) {
            std::fill(src.begin(), src.end(), 0.f);
            buildSource(type);
            builtSource_ = type;
        }
        for (auto& g : grains) g.on = false;
        spawnTimer = 0.01f;
    }

    void setNote(float freq, double /*_sr*/) {
        baseInc = freq * kBuf / float(sr);
        baseInc = juce::jlimit(0.01f, 64.f, baseInc);
    }

    void spawnGrain() {
        for (auto& g : grains) {
            if (!g.on) {
                std::uniform_real_distribution<float> d(-1,1);
                g.pos = float(rng() % kBuf);
                g.inc = baseInc * (1.f + d(rng) * pitchRnd);
                g.dur = std::max(1.f, grainSize * float(sr));
                g.age = 0; g.on = true;
                break;
            }
        }
    }

    float tick() {
        if (src.empty()) return 0.f;
        float dt = 1.f / float(sr);
        spawnTimer -= dt;
        if (spawnTimer <= 0.f && density > 0.01f) {
            std::uniform_real_distribution<float> d(0,1);
            spawnTimer = grainSize*(1.f-density*0.8f) + d(rng)*scatter*grainSize;
            spawnGrain();
        }
        float out = 0.f;
        int active = 0;
        for (auto& g : grains) {
            if (!g.on) continue;
            ++active;
            float env = std::sin(g.age/std::max(1.f,g.dur) * juce::MathConstants<float>::pi);
            int idx = int(g.pos) % kBuf;
            if (idx < 0) idx += kBuf;
            int idx2 = (idx+1) % kBuf;
            float frac = g.pos - std::floor(g.pos);
            out += (src[idx] + frac*(src[idx2]-src[idx])) * env;
            g.pos += g.inc;
            while (g.pos >= kBuf) g.pos -= kBuf;
            while (g.pos < 0)     g.pos += kBuf;
            g.age++;
            if (g.age >= g.dur) g.on = false;
        }
        float norm = (active > 1) ? 1.f / std::sqrt(float(active)) : 1.f;
        return out * norm * 0.5f;
    }
};

// ─────────────────────────────────────────────────────────────────────────────
//  KARPLUS-STRONG ENGINE  — FIX: excite writes at READ pointer for instant sound
//  color: 0=pluck, 1=sine, 2=triangle, 3=noise burst (drums)
// ─────────────────────────────────────────────────────────────────────────────
struct KarplusEngine {
    static constexpr int kMax = 4096;
    std::vector<float> buf;
    int   wp=0, rp=0, len=100;
    KarplusEngine() : buf(kMax, 0.f) {}
    float stretch=0.5f, body=0.5f, pick=0.25f;
    int   color=0;
    float dcState = 0.f;  // DC blocker state
    std::mt19937 rng{77};

    void setNote(float freq, double sr) {
        len = juce::jlimit(1, kMax-1, int(sr / freq));
    }

    void excite() {
        std::fill(buf.begin(), buf.end(), 0.f);
        dcState = 0.f;
        wp = 0; rp = 0;
        // Use fast algebraic shapes — NO sin() on audio thread
        for (int i = 0; i < len; ++i) {
            float t = float(i) / len;
            float v;
            float window = 4.f * t * (1.f - t);  // parabolic ≈ sin(πt), zero at edges
            int ps = std::max(1, int(pick * len));
            switch (color) {
                case 1: { // Sine-like — use parabolic approximation
                    float s = t * 2.f - 1.f;  // -1 to +1
                    v = s * (1.f - s * s * 0.33f);  // cubic ≈ sin(2πt) for one cycle
                } break;
                case 2: { // Triangle bipolar
                    v = (t < 0.5f ? 4*t - 1 : 3 - 4*t) * window;
                } break;
                case 3: { // Noise burst for drums
                    v = ((rng() & 0xFFFF) / 32768.f - 1.f) * window;
                } break;
                default: { // Pluck bipolar
                    float raw = i < ps ? float(i)/ps : 1.f - float(i-ps)/std::max(len-ps,1);
                    v = (raw * 2.f - 1.f) * window;
                } break;
            }
            buf[i] = v * 0.5f;
        }
        wp = len; rp = 0;
    }

    float tick() {
        float y1 = buf[rp % kMax], y2 = buf[(rp+1) % kMax];
        float out = y1;
        float a = 0.5f + (1.f - body) * 0.3f;
        float b = 1.f - a;
        float decay = 0.998f + stretch * 0.0019f;
        float filtered = (y1 * a + y2 * b) * decay;
        // One-pole DC blocker: removes any DC that accumulates in the loop
        float dcFree = filtered - dcState;
        dcState += 0.0005f * (filtered - dcState);  // very slow DC tracker
        buf[wp % kMax] = juce::jlimit(-1.f, 1.f, dcFree);
        rp = (rp+1)%kMax; wp = (wp+1)%kMax;
        return out;
    }
};

// ─────────────────────────────────────────────────────────────────────────────
//  WAVEGUIDE ENGINE — single delay line with 2-pole LP + allpass stiffness
//  Same topology as KS but richer filter for bowed/sustained string character
// ─────────────────────────────────────────────────────────────────────────────
struct WaveguideEngine {
    static constexpr int kMax = 4096;
    std::vector<float> buf;
    int   wp=0, rp=0, len=200;
    WaveguideEngine() : buf(kMax, 0.f) {}
    float damping=0.5f, stiffness=0.3f;
    float lp1=0, lp2=0, ap1=0, dcState=0;

    void setNote(float freq, double sr) {
        len = juce::jlimit(2, kMax-1, int(sr / freq));
    }

    void pluck() {
        std::fill(buf.begin(), buf.end(), 0.f);
        lp1=0; lp2=0; ap1=0; dcState=0;
        wp = 0; rp = 0;
        for (int i = 0; i < len; ++i) {
            float t = float(i) / len;
            // Fast bipolar excitation — no sin() on audio thread
            float s1 = t * 2.f - 1.f;  // -1 to +1
            float v1 = s1 * (1.f - s1 * s1 * 0.33f) * 0.7f;  // cubic ≈ sin(2πt)
            float s2 = t * 4.f - 1.f;  // double freq
            s2 = s2 - std::floor(s2 + 0.5f);  // wrap to [-0.5, 0.5]
            float v2 = s2 * (1.f - 4.f * s2 * s2) * 0.2f;  // fast sin approx
            buf[i] = v1 + v2;
        }
        wp = len; rp = 0;
    }

    float tick() {
        float y = buf[rp % kMax];
        float out = y;

        // Two-pole LP: damping controls warmth (0=bright, 1=dark)
        float fc = 0.2f + (1.f - damping) * 0.7f;  // LP coeff 0.2..0.9
        lp1 += fc * (y - lp1);
        lp2 += fc * (lp1 - lp2);

        // Allpass for stiffness/inharmonicity
        float apC = stiffness * 0.4f;
        float apOut = -apC * lp2 + ap1;
        ap1 = lp2 + apC * apOut;

        // DC blocker
        float dcFree = apOut - dcState;
        dcState += 0.0005f * (apOut - dcState);

        // Near-unity decay
        float decay = 0.9988f + (1.f - damping) * 0.0011f;  // 0.9988..0.9999
        buf[wp % kMax] = juce::jlimit(-1.f, 1.f, dcFree * decay);

        rp=(rp+1)%kMax; wp=(wp+1)%kMax;
        return out;
    }
};
