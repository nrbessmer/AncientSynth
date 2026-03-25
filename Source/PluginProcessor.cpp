// Ancient Synth — single translation unit
#define ANCIENT_DEBUG 0

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_dsp/juce_dsp.h>
#include "Engines.h"
#include "DSP.h"
#include "Presets.h"
#include "UI/UI.h"

using namespace juce;

// ─── Debug logger (disabled for production) ──────────────────────────────────
#if ANCIENT_DEBUG
#include <cstdio>
#include <mach/mach.h>
#include <pthread.h>
static FILE* g_logFile = nullptr;
static void ancientLog(const char* msg) {
    fprintf(stderr, "[AS] %s\n", msg); fflush(stderr);
    if (!g_logFile) g_logFile = fopen("/tmp/ancient_debug.log", "a");
    if (g_logFile) { fprintf(g_logFile, "[AS] %s\n", msg); fflush(g_logFile); }
}
#define ALOG(msg)       ancientLog(msg)
#define ALOGF(fmt, ...) do { char _b[512]; snprintf(_b,sizeof(_b),fmt,__VA_ARGS__); ancientLog(_b); } while(0)
static void logHeapStats(const char* label) {
    struct mach_task_basic_info info;
    mach_msg_type_number_t count = MACH_TASK_BASIC_INFO_COUNT;
    if (task_info(mach_task_self(), MACH_TASK_BASIC_INFO, (task_info_t)&info, &count) == KERN_SUCCESS)
        ALOGF("  HEAP [%s]: resident=%.1fMB", label, info.resident_size/(1024.0*1024.0));
}
static void logStackStats(const char* label) {
    pthread_t self = pthread_self();
    size_t stackSize = pthread_get_stacksize_np(self);
    volatile char marker;
    ptrdiff_t used = (char*)pthread_get_stackaddr_np(self) - (char*)&marker;
    ptrdiff_t remaining = (ptrdiff_t)stackSize - used;
    ALOGF("  STACK [%s]: free=%tdKB", label, remaining/1024);
}
static void logMem(const char* label) { logHeapStats(label); logStackStats(label); }
#else
#define ALOG(msg)         ((void)0)
#define ALOGF(fmt, ...)   ((void)0)
#define logHeapStats(l)   ((void)0)
#define logStackStats(l)  ((void)0)
#define logMem(l)         ((void)0)
#endif

class AncientSynthProcessor : public AudioProcessor,
                               public AudioProcessorValueTreeState::Listener
{
public:
    AncientSynthProcessor();
    ~AncientSynthProcessor() override;

    AudioProcessorValueTreeState& getAPVTS()   { return apvts_; }
    PresetManager&                getPresets() { return presets_; }
    MicrotonalScale&              getScale()   { return scale_; }
    const AudioBuffer<float>&     getOscBuf()  const { return oscBuf_; }
    int activeVoices() const {
        int n=0;
        for (int i=0;i<synth_.getNumVoices();++i)
            if (synth_.getVoice(i)->isVoiceActive()) ++n;
        return n;
    }

    void prepareToPlay(double sr, int block) override;
    void releaseResources() override {
        synth_.allNotesOff(0, false);
        fx_.reset(fx_.sr);
    }

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override {
        const auto& out = layouts.getMainOutputChannelSet();
        return out == AudioChannelSet::stereo() || out == AudioChannelSet::mono();
    }

    void processBlock(AudioBuffer<float>&, MidiBuffer&) override;
    AudioProcessorEditor* createEditor() override;
    bool hasEditor()        const override { return true; }
    const String getName()  const override { return "Ancient Synth"; }
    bool acceptsMidi()      const override { return true; }
    bool producesMidi()     const override { return false; }
    double getTailLengthSeconds() const override { return 2.0; }
    int  getNumPrograms()    override { return presets_.numPresets(); }
    int  getCurrentProgram() override { return presets_.currentIndex(); }
    void setCurrentProgram(int i) override { presets_.load(i); }
    const String getProgramName(int i) override { return kPresets[size_t(i)].name; }
    void changeProgramName(int, const String&) override {}

    void getStateInformation(MemoryBlock& d) override {
        copyXmlToBinary(*apvts_.copyState().createXml(), d);
    }
    void setStateInformation(const void* d, int s) override {
        auto xml = getXmlFromBinary(d,s);
        if (xml && xml->hasTagName(apvts_.state.getType()))
            apvts_.replaceState(ValueTree::fromXml(*xml));
    }
    void parameterChanged(const String& id, float v) override {
        if      (id=="scale")      scale_.scaleIdx = int(v);
        else if (id=="scale_root") scale_.rootNote = int(v);
        paramsDirty_.store(true);
    }

    void syncVoiceParams();
    std::atomic<bool> paramsDirty_ { true };

private:
    static AudioProcessorValueTreeState::ParameterLayout createParams();

    AudioProcessorValueTreeState apvts_;
    PresetManager                presets_;
    MicrotonalScale              scale_;
    Synthesiser                  synth_;
    Arpeggiator                  arp_;
    MasterFX                     fx_;
    AudioBuffer<float>           oscBuf_;
    int    monoNote_ = -1;
    String modSrcIds_[4], modDstIds_[4], modAmtIds_[4];

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AncientSynthProcessor)
};

// ─── Parameters ───────────────────────────────────────────────────────────────
AudioProcessorValueTreeState::ParameterLayout AncientSynthProcessor::createParams()
{
    std::vector<std::unique_ptr<RangedAudioParameter>> p;
    auto F=[&](const char* id,const char* nm,float lo,float hi,float def,float sk=1.f){
        NormalisableRange<float> r(lo,hi); r.skew=sk;
        p.push_back(std::make_unique<AudioParameterFloat>(ParameterID{id,1},nm,r,def));};
    auto C=[&](const char* id,const char* nm,StringArray ch,int def){
        p.push_back(std::make_unique<AudioParameterChoice>(ParameterID{id,1},nm,ch,def));};
    auto B=[&](const char* id,const char* nm,bool def){
        p.push_back(std::make_unique<AudioParameterBool>(ParameterID{id,1},nm,def));};

    C("engine","Engine",{"Wavetable","FM","Granular","Karplus","Waveguide"},0);
    C("wt_shape","WT Shape",{"Plucked","Kithara","Flute","Reed","Brass","Organ","Bell","Vocal"},0);
    F("wt_pos","WT Pos",0,1,0);
    F("wt_morph","WT Morph",0,1,0);
    C("fm_algo","FM Algo",{"1","2","3","4","5","6","7","8"},0);
    F("fm_feedback","FM FB",0,1,0.3f);
    F("gr_size","Gr Size",0.01f,0.5f,0.08f,0.4f);
    F("gr_scatter","Gr Scatter",0,1,0.3f);
    F("gr_density","Gr Density",0,1,0.6f);
    F("gr_pitch_rnd","Gr Pitch",0,1,0.1f);
    C("gr_source","Gr Source",{"Warm","Vocal","Hollow","Bell","Breathy","Drone","Crystal","Noise"},0);
    F("ks_body","KS Body",0,1,0.5f);
    F("ks_pick","KS Pick",0,1,0.25f);
    C("ks_color","KS Color",{"Pluck","Sine","Triangle","Noise"},0);
    F("ks_stretch","KS Sustain",0,1,0.5f);
    F("wg_stiff","WG Stiff",0,1,0.3f);
    F("wg_damp","WG Damp",0,1,0.4f);
    F("amp_attack", "Amp A",0.001f,10.f,0.01f,0.3f);
    F("amp_decay",  "Amp D",0.001f,10.f,0.20f,0.3f);
    F("amp_sustain","Amp S",0,1,0.8f);
    F("amp_release","Amp R",0.001f,10.f,0.50f,0.3f);
    C("filt_type","Filter",{"LP12","LP24","HP","BP","Notch","Comb","Ladder"},0);
    F("filt_cutoff","Cutoff",20.f,20000.f,8000.f,0.25f);
    F("filt_res","Reso",0,1,0.3f);
    F("filt_drive","Drive",1,8,1.f);
    F("filt_env_amt","Filt Env",-1,1,0.3f);
    F("filt_keytrack","Key Trk",0,1,0);
    F("filt_attack", "Filt A",0.001f,10.f,0.01f,0.3f);
    F("filt_decay",  "Filt D",0.001f,10.f,0.20f,0.3f);
    F("filt_sustain","Filt S",0,1,0.5f);
    F("filt_release","Filt R",0.001f,10.f,0.50f,0.3f);
    F("lfo1_rate","LFO1 Rate",0.01f,20.f,1.f,0.4f);
    F("lfo1_depth","LFO1 Depth",0,1,0);
    C("lfo1_shape","LFO1 Shape",{"Sine","Tri","Saw","Square","S&H"},0);
    C("lfo1_dest","LFO1 Dest",{"Pitch","Cutoff","FM Depth","Amp","Pan"},0);
    B("lfo1_sync","LFO1 Sync",false);
    F("lfo2_rate","LFO2 Rate",0.01f,20.f,2.f,0.4f);
    F("lfo2_depth","LFO2 Depth",0,1,0);
    C("lfo2_shape","LFO2 Shape",{"Sine","Tri","Saw","Square","S&H"},0);
    C("lfo2_dest","LFO2 Dest",{"Pitch","Cutoff","FM Depth","Amp","Pan"},1);
    B("lfo2_sync","LFO2 Sync",false);
    F("glide","Glide",0,2,0);
    F("detune","Detune",-50,50,0);
    C("unison","Unison",{"1","2","3","4","5","6","7","8"},0);
    F("unison_spread","U.Spread",0,1,0.3f);
    C("poly_mode","Poly",{"Poly","Mono","Legato"},0);
    C("scale","Scale",{MicrotonalScale::kNames[0],MicrotonalScale::kNames[1],
        MicrotonalScale::kNames[2],MicrotonalScale::kNames[3],MicrotonalScale::kNames[4],
        MicrotonalScale::kNames[5],MicrotonalScale::kNames[6],MicrotonalScale::kNames[7],
        MicrotonalScale::kNames[8],MicrotonalScale::kNames[9],MicrotonalScale::kNames[10],
        MicrotonalScale::kNames[11],MicrotonalScale::kNames[12]},0);
    C("scale_root","Root",{"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"},0);
    B("arp_on","Arp On",false);
    C("arp_mode","Arp Mode",{"Up","Down","UpDown","Random","Played"},0);
    C("arp_rate","Arp Rate",{"1/4","1/8","1/16","1/32"},1);
    F("arp_gate","Arp Gate",0.1f,1.f,0.8f);
    F("arp_swing","Arp Swing",0,0.5f,0);
    F("fx_rev_mix","Rev Mix",0,1,0.3f);
    F("fx_rev_size","Rev Size",0,1,0.7f);
    F("fx_rev_damp","Rev Damp",0,1,0.4f);
    F("fx_chr_mix","Chr Mix",0,1,0);
    F("fx_chr_rate","Chr Rate",0.1f,5.f,0.5f);
    F("fx_chr_depth","Chr Depth",0,1,0.3f);
    F("fx_dly_mix","Dly Mix",0,1,0);
    F("fx_dly_time","Dly Time",0.01f,2.f,0.3f);
    F("fx_dly_fb","Dly FB",0,0.95f,0.4f);
    B("fx_dly_sync","Dly Sync",false);
    F("fx_dist_amt","Dist Amt",0,1,0);
    F("fx_dist_tone","Dist Tone",0,1,0.5f);
    C("fx_dist_type","Dist Type",{"Hard","Warm","Tape","Bit","Fold"},0);
    F("fx_eq_lo","EQ Lo",-12,12,0);
    F("fx_eq_mid","EQ Mid",-12,12,0);
    F("fx_eq_hi","EQ Hi",-12,12,0);
    F("master_vol","Volume",0,1,0.8f);
    F("master_gain","Gain",-24,24,0);
    F("master_tune","Tune",-100,100,0);
    F("stereo_width","Width",0,2,1.f);
    C("tuning_system","Tuning",{
        HistoricalTuning::kNames[0],HistoricalTuning::kNames[1],
        HistoricalTuning::kNames[2],HistoricalTuning::kNames[3],
        HistoricalTuning::kNames[4],HistoricalTuning::kNames[5]},0);
    C("tuning_root","Tuning Root",{"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"},0);

    for (int i=0;i<4;++i) {
        juce::String si(i+1);
        C(("mod"+si+"_src").toRawUTF8(), ("Mod"+si+" Src").toRawUTF8(),
          {ModMatrix::kSrcNames[0],ModMatrix::kSrcNames[1],ModMatrix::kSrcNames[2],
           ModMatrix::kSrcNames[3],ModMatrix::kSrcNames[4],ModMatrix::kSrcNames[5],
           ModMatrix::kSrcNames[6],ModMatrix::kSrcNames[7]}, 0);
        C(("mod"+si+"_dst").toRawUTF8(), ("Mod"+si+" Dst").toRawUTF8(),
          {ModMatrix::kDstNames[0],ModMatrix::kDstNames[1],ModMatrix::kDstNames[2],
           ModMatrix::kDstNames[3],ModMatrix::kDstNames[4],ModMatrix::kDstNames[5],
           ModMatrix::kDstNames[6],ModMatrix::kDstNames[7]}, 0);
        F(("mod"+si+"_amt").toRawUTF8(), ("Mod"+si+" Amt").toRawUTF8(), -1,1,0);
    }
    return {std::make_move_iterator(p.begin()),std::make_move_iterator(p.end())};
}

// ─── Constructor ──────────────────────────────────────────────────────────────
AncientSynthProcessor::AncientSynthProcessor()
    : AudioProcessor(BusesProperties().withOutput("Output",AudioChannelSet::stereo(),true)),
      apvts_(*this,nullptr,"AncientSynth",createParams()),
      presets_(apvts_)
{
    synth_.addSound(new AncientSound());
    for (int i=0;i<16;++i) synth_.addVoice(new AncientVoice());

    for (auto* p : getParameters())
        if (auto* rp = dynamic_cast<RangedAudioParameter*>(p))
            apvts_.addParameterListener(rp->getParameterID(), this);

    for (int i = 0; i < 4; ++i) {
        juce::String si(i+1);
        modSrcIds_[i] = "mod" + si + "_src";
        modDstIds_[i] = "mod" + si + "_dst";
        modAmtIds_[i] = "mod" + si + "_amt";
    }

    synth_.setCurrentPlaybackSampleRate(48000.0);
    fx_.prepare(48000.0);
    oscBuf_.setSize(2, 4096, false, true, true);
    syncVoiceParams();
}

AncientSynthProcessor::~AncientSynthProcessor() {}

// ─── Prepare ──────────────────────────────────────────────────────────────────
void AncientSynthProcessor::prepareToPlay(double sr, int block) {
    if (sr <= 0.0) sr = 48000.0;
    if (block <= 0) block = 512;
    setRateAndBufferSizeDetails(sr, block);
    synth_.setCurrentPlaybackSampleRate(sr);
    fx_.prepare(sr);
    oscBuf_.setSize(2, std::max(block, 4096), false, true, true);
    syncVoiceParams();
}

// ─── Sync voice params (called from processBlock when dirty) ─────────────────
void AncientSynthProcessor::syncVoiceParams() {
    auto g=[&](const char* id){ return apvts_.getRawParameterValue(id)->load(); };
    float masterTune = g("master_tune");
    int   uniCount   = int(g("unison")) + 1;
    float uniSpread  = g("unison_spread");
    int   tunSys     = int(g("tuning_system"));
    int   tunRoot    = int(g("tuning_root"));

    ModMatrix mm;
    for (int i=0;i<4;++i) {
        mm.slots[i].src = int(g(modSrcIds_[i].toRawUTF8()));
        mm.slots[i].dst = int(g(modDstIds_[i].toRawUTF8()));
        mm.slots[i].amt = g(modAmtIds_[i].toRawUTF8());
    }

    for (int i=0;i<synth_.getNumVoices();++i) {
        auto* v=dynamic_cast<AncientVoice*>(synth_.getVoice(i));
        if (!v) continue;
        auto& p=v->p;
        float uniDetune = 0.f;
        if (uniCount > 1) {
            float slotF = float(i % uniCount);
            uniDetune = (slotF/(uniCount-1) - 0.5f) * 2.f * uniSpread * 100.f;
        }
        p.uniDetuneCents = uniDetune;
        p.engine=int(g("engine")); p.wtShape=int(g("wt_shape")); p.wtPos=g("wt_pos");
        p.fmAlgo=int(g("fm_algo")); p.fmFb=g("fm_feedback");
        p.grSource=int(g("gr_source"));
        p.grSz=g("gr_size"); p.grSc=g("gr_scatter"); p.grDen=g("gr_density"); p.grPr=g("gr_pitch_rnd");
        p.ksBody=g("ks_body"); p.ksPick=g("ks_pick"); p.ksColor=int(g("ks_color")); p.ksStretch=g("ks_stretch");
        p.wgStiff=g("wg_stiff"); p.wgDamp=g("wg_damp");
        p.filtType=int(g("filt_type"));
        p.ampA=g("amp_attack"); p.ampD=g("amp_decay"); p.ampS=g("amp_sustain"); p.ampR=g("amp_release");
        p.fCut=g("filt_cutoff"); p.fRes=g("filt_res"); p.fDrv=g("filt_drive");
        p.fEnvAmt=g("filt_env_amt"); p.fKeyTrack=g("filt_keytrack");
        p.fA=g("filt_attack"); p.fD=g("filt_decay"); p.fS=g("filt_sustain"); p.fR=g("filt_release");
        p.l1Rate=g("lfo1_rate"); p.l1Depth=g("lfo1_depth");
        p.l1Shape=g("lfo1_shape"); p.l1Dest=g("lfo1_dest");
        p.l2Rate=g("lfo2_rate"); p.l2Depth=g("lfo2_depth");
        p.l2Shape=g("lfo2_shape"); p.l2Dest=g("lfo2_dest");
        p.glide=g("glide"); p.detune=g("detune");
        p.uniSpread=uniSpread; p.masterTune=masterTune;
        p.tuningSystem=tunSys; p.tuningRoot=tunRoot;
        p.modMatrix=mm;
    }
}

// ─── Process block ────────────────────────────────────────────────────────────
void AncientSynthProcessor::processBlock(AudioBuffer<float>& buf, MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    const double currentSR = getSampleRate();

    if (currentSR <= 0.0 || buf.getNumSamples() == 0 || buf.getNumChannels() == 0)
        return;

    // Sample rate change — lightweight reset, no heap allocation
    if (currentSR != fx_.sr) {
        synth_.setCurrentPlaybackSampleRate(currentSR);
        fx_.reset(currentSR);
    }

    // Preset change: force-kill all voices + clear FX buffers
    if (presets_.consumePresetDirty()) {
        for (int vi = 0; vi < synth_.getNumVoices(); ++vi) {
            auto* v = dynamic_cast<AncientVoice*>(synth_.getVoice(vi));
            if (v && v->isVoiceActive())
                v->stopNote(0.f, false);
        }
        synth_.allNotesOff(0, false);
        fx_.reset(currentSR);
        arp_.held.clear();
        paramsDirty_.store(true);
    }

    // Sync voice params when any parameter changed
    if (paramsDirty_.exchange(false))
        syncVoiceParams();

    buf.clear();

    // ── MIDI + arp ───────────────────────────────────────────────────────────
    const int   polyMode = int(*apvts_.getRawParameterValue("poly_mode"));
    const int   uniCount = int(*apvts_.getRawParameterValue("unison")) + 1;
    const bool  arpOn    = *apvts_.getRawParameterValue("arp_on") > 0.5f;

    MidiBuffer quantised;
    for (auto msg : midi) {
        auto m = msg.getMessage();
        const int sp = msg.samplePosition;
        if (m.isNoteOn()) {
            const int qn = scale_.quantize(m.getNoteNumber());
            if (arpOn) {
                arp_.noteOn(qn);
            } else if (polyMode == 0) {
                for (int u=0; u<uniCount; ++u)
                    quantised.addEvent(MidiMessage::noteOn(m.getChannel(),qn,m.getVelocity()), sp);
            } else {
                if (polyMode == 1)
                    quantised.addEvent(MidiMessage::allNotesOff(m.getChannel()), sp);
                monoNote_ = qn;
                quantised.addEvent(MidiMessage::noteOn(m.getChannel(),qn,m.getVelocity()), sp);
            }
        } else if (m.isNoteOff()) {
            const int qnOff = scale_.quantize(m.getNoteNumber());
            if (arpOn) arp_.noteOff(qnOff);
            else quantised.addEvent(MidiMessage::noteOff(m.getChannel(), qnOff, (uint8)0), sp);
        } else {
            quantised.addEvent(m, sp);
        }
    }

    if (arpOn) {
        arp_.mode    = int(*apvts_.getRawParameterValue("arp_mode"));
        arp_.gate    = *apvts_.getRawParameterValue("arp_gate");
        arp_.rateDiv = [](int i){ return i==0?4:i==1?8:i==2?16:32; }
                       (int(*apvts_.getRawParameterValue("arp_rate")));
        double bpm = 120.0;
        if (auto* ph = getPlayHead())
            if (auto pos = ph->getPosition())
                if (auto b = pos->getBpm()) bpm = *b;
        const int note = arp_.tick(currentSR, bpm);
        if (note >= 0) {
            quantised.addEvent(MidiMessage::noteOn(1, note, (uint8)100), 0);
            quantised.addEvent(MidiMessage::noteOff(1, note),
                               int(buf.getNumSamples() * arp_.gate));
        }
    }

    // ── Render voices ────────────────────────────────────────────────────────
    synth_.renderNextBlock(buf, quantised, 0, buf.getNumSamples());

    // ── FX ───────────────────────────────────────────────────────────────────
    fx_.revMix      = apvts_.getRawParameterValue("fx_rev_mix")->load();
    fx_.revSize     = apvts_.getRawParameterValue("fx_rev_size")->load();
    fx_.revDamp     = apvts_.getRawParameterValue("fx_rev_damp")->load();
    fx_.chrMix      = apvts_.getRawParameterValue("fx_chr_mix")->load();
    fx_.chrRate     = apvts_.getRawParameterValue("fx_chr_rate")->load();
    fx_.chrDepth    = apvts_.getRawParameterValue("fx_chr_depth")->load();
    fx_.dlyMix      = apvts_.getRawParameterValue("fx_dly_mix")->load();
    fx_.dlyTime     = apvts_.getRawParameterValue("fx_dly_time")->load();
    fx_.dlyFB       = apvts_.getRawParameterValue("fx_dly_fb")->load();
    fx_.distAmt     = apvts_.getRawParameterValue("fx_dist_amt")->load();
    fx_.distTone    = apvts_.getRawParameterValue("fx_dist_tone")->load();
    fx_.distType    = int(apvts_.getRawParameterValue("fx_dist_type")->load());
    fx_.masterVol   = apvts_.getRawParameterValue("master_vol")->load();
    fx_.gain        = std::pow(10.f, apvts_.getRawParameterValue("master_gain")->load() / 20.f);
    fx_.stereoWidth = apvts_.getRawParameterValue("stereo_width")->load();

    {
        const float lo  = apvts_.getRawParameterValue("fx_eq_lo")->load();
        const float mid = apvts_.getRawParameterValue("fx_eq_mid")->load();
        const float hi  = apvts_.getRawParameterValue("fx_eq_hi")->load();
        if (lo != fx_.eqLo || mid != fx_.eqMid || hi != fx_.eqHi) {
            fx_.eqLo = lo; fx_.eqMid = mid; fx_.eqHi = hi;
            fx_.rebuildEQ(currentSR);
        }
    }

    for (int s = 0; s < buf.getNumSamples(); ++s) {
        float L = buf.getSample(0, s);
        float R = buf.getNumChannels() > 1 ? buf.getSample(1, s) : L;
        auto [oL, oR] = fx_.process(L, R);
        buf.setSample(0, s, jlimit(-1.f, 1.f, std::isfinite(oL) ? oL : 0.f));
        if (buf.getNumChannels() > 1)
            buf.setSample(1, s, jlimit(-1.f, 1.f, std::isfinite(oR) ? oR : 0.f));
    }

    if (oscBuf_.getNumSamples() >= buf.getNumSamples())
        oscBuf_.makeCopyOf(buf);
}

// ─── Editor ───────────────────────────────────────────────────────────────────
AudioProcessorEditor* AncientSynthProcessor::createEditor() {
    return new AncientSynthEditor(*this);
}

AncientSynthEditor::AncientSynthEditor(AncientSynthProcessor& p)
    : AudioProcessorEditor(&p), proc_(p),
      browser_(p.getPresets()),
      synthP_(p.getAPVTS()), modP_(p.getAPVTS()), fxP_(p.getAPVTS()),
      modMatrixP_(p.getAPVTS())
{
    setLookAndFeel(&laf_);
    setSize(kW, kH);
    setResizable(false, false);
    setOpaque(true);

    title_.setText("ANCIENT SYNTH", dontSendNotification);
    title_.setFont(Fonts::display(18.f));
    title_.setColour(Label::textColourId, Pal::Amber);
    addAndMakeVisible(title_);

    pName_.setFont(Fonts::body(13.f));
    pCat_.setFont(Fonts::label(9.f));
    vLabel_.setFont(Fonts::mono(9.f));
    for (auto* l:{&pName_,&pCat_,&vLabel_}) addAndMakeVisible(l);

    prev_.onClick=[this]{ proc_.getPresets().prev(); updatePatch(); };
    next_.onClick=[this]{ proc_.getPresets().next(); updatePatch(); };
    addAndMakeVisible(prev_); addAndMakeVisible(next_);

    for (const char* s : MicrotonalScale::kNames) scCombo_.addItem(s, scCombo_.getNumItems()+1);
    for (const char* n : {"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"})
        rtCombo_.addItem(n, rtCombo_.getNumItems()+1);
    addAndMakeVisible(scCombo_); addAndMakeVisible(rtCombo_);
    scA_ = std::make_unique<CA>(p.getAPVTS(), "scale",      scCombo_);
    rtA_ = std::make_unique<CA>(p.getAPVTS(), "scale_root", rtCombo_);

    browser_.onSelect = [this](int i){ proc_.getPresets().load(i); updatePatch(); };
    addAndMakeVisible(browser_);

    tabs_.addTab("OSCILLATOR",   Pal::Void, &synthP_,     false);
    tabs_.addTab("MOD / FILTER", Pal::Void, &modP_,       false);
    tabs_.addTab("EFFECTS",      Pal::Void, &fxP_,        false);
    tabs_.addTab("MOD MATRIX",   Pal::Void, &modMatrixP_, false);
    tabs_.setTabBarDepth(28); tabs_.setOutline(0);
    addAndMakeVisible(tabs_);
    modP_.setScale(&proc_.getScale());

    xyPad_.bind(p.getAPVTS(), "filt_cutoff", "filt_res");
    addAndMakeVisible(scope_); addAndMakeVisible(xyPad_);

    footer_.setFont(Fonts::mono(10.f));
    footer_.setColour(juce::Label::textColourId, Pal::TextDim);
    footer_.setText("Ancient Synth", dontSendNotification);
    addAndMakeVisible(footer_);

    updatePatch();
    startTimerHz(24);
}

void AncientSynthEditor::timerCallback() {
    scope_.pushBuffer(proc_.getOscBuf()); scope_.repaint();
    const int v = proc_.activeVoices();
    vLabel_.setText(String(v) + (v==1?" voice":" voices"), dontSendNotification);
    // Footer stats: voices, sample rate, buffer size, estimated memory
    double sr = proc_.getSampleRate();
    int block = proc_.getBlockSize();
    int totalVoices = proc_.getTotalNumOutputChannels() > 0 ? 24 : 0;
    // Rough memory estimate: each voice ~200KB (wavetable + KS + WG + granular buffers)
    int memKB = 24 * 200 + 512;  // voices + FX buffers
    String stats;
    stats << "Voices: " << v << "/" << totalVoices
          << "  |  SR: " << int(sr) << " Hz"
          << "  |  Buffer: " << block
          << "  |  Mem: ~" << (memKB / 1024) << "." << ((memKB % 1024) / 100) << " MB";
    footer_.setText(stats, dontSendNotification);
}

void AncientSynthEditor::updatePatch() {
    auto& pm = proc_.getPresets();
    pName_.setText(pm.name(),     dontSendNotification);
    pCat_ .setText(pm.category(), dontSendNotification);
    browser_.selectPreset(pm.currentIndex());
}

void AncientSynthEditor::paint(Graphics& g) {
    g.fillAll(Pal::Void);
    AncientLAF::bg(g, getLocalBounds());
    g.setColour(Pal::Surface); g.fillRect(getLocalBounds().removeFromTop(52));
    g.setColour(Pal::Border);
    g.fillRect(0, 52, getWidth(), 1);
    g.fillRect(0, getHeight()-21, getWidth(), 1);
    g.fillRect(206, 53, 1, getHeight()-74);
    g.fillRect(getWidth()-207, 53, 1, getHeight()-74);
}

void AncientSynthEditor::resized() {
    auto b   = getLocalBounds();
    auto hdr = b.removeFromTop(52);
    title_.setBounds(hdr.removeFromLeft(145).reduced(12,12));
    auto nav = hdr.removeFromLeft(280);
    prev_.setBounds(nav.removeFromLeft(28).reduced(2,10));
    next_.setBounds(nav.removeFromRight(28).reduced(2,10));
    pName_.setBounds(nav.removeFromTop(28).reduced(0,4));
    pCat_ .setBounds(nav.reduced(0,2));
    auto sc = hdr.removeFromLeft(230); sc.reduce(8,12);
    scCombo_.setBounds(sc.removeFromLeft(155).reduced(0,2));
    sc.removeFromLeft(4); rtCombo_.setBounds(sc.reduced(0,2));
    vLabel_.setBounds(hdr.reduced(8,14));
    b.removeFromTop(1);
    footer_.setBounds(b.removeFromBottom(20).reduced(8,2));
    auto left  = b.removeFromLeft(200);  b.removeFromLeft(1);
    auto right = b.removeFromRight(200); b.removeFromRight(1);
    browser_.setBounds(left.reduced(5,4));
    scope_.setBounds(right.removeFromTop(145).reduced(5,5));
    right.removeFromTop(4); xyPad_.setBounds(right.reduced(5,4));
    tabs_.setBounds(b.reduced(4,0));
}

AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new AncientSynthProcessor();
}
