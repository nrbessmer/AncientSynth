#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_dsp/juce_dsp.h>
#include <vector>
#include <map>
#include <string>

struct Preset {
    const char* name;
    const char* category;
    std::map<std::string,float> params;
};
#define P(k,v) {k,v}

static const std::vector<Preset> kPresets = {

// ═══════════════════════════════════════════════════════════════════════
//  LEADS (6) — ornamental, vocal, reedy
// ═══════════════════════════════════════════════════════════════════════

// Gut-string lyre with slow ornamental pitch wobble + resonant body formant
{ "Sumerian Lyre","Lead",
  {P("engine",0),P("wt_shape",1),P("wt_pos",0.42f),P("wt_morph",0.15f),
   P("amp_attack",0.008f),P("amp_decay",0.7f),P("amp_sustain",0.42f),P("amp_release",0.5f),
   P("filt_type",6),P("filt_cutoff",2400),P("filt_res",0.35f),P("filt_drive",1.5f),P("filt_keytrack",0.55f),
   P("filt_env_amt",0.6f),P("filt_attack",0.003f),P("filt_decay",1.f),P("filt_sustain",0.15f),P("filt_release",0.4f),
   P("lfo1_rate",4.8f),P("lfo1_depth",0.15f),P("lfo1_dest",0),   // wide vibrato like oud
   P("lfo2_rate",0.12f),P("lfo2_depth",0.25f),P("lfo2_dest",1),  // slow formant drift
   P("detune",6),P("scale",1),P("tuning_system",3),  // pentatonic + Sumerian tuning
   P("fx_rev_mix",0.5f),P("fx_rev_size",0.8f),P("fx_rev_damp",0.18f),
   P("fx_chr_mix",0.18f),P("fx_chr_rate",0.22f),P("fx_chr_depth",0.5f),
   P("fx_dly_mix",0.14f),P("fx_dly_time",0.25f),P("fx_dly_fb",0.32f),
   P("master_vol",0.92f)}},

// Nasal double-reed: high FM feedback + narrow bandpass resonance
{ "Mizmar Reed","Lead",
  {P("engine",1),P("fm_algo",3),P("fm_feedback",0.58f),
   P("amp_attack",0.05f),P("amp_decay",0.3f),P("amp_sustain",0.6f),P("amp_release",0.3f),
   P("filt_type",3),P("filt_cutoff",2800),P("filt_res",0.45f),P("filt_drive",1.6f),P("filt_keytrack",0.5f),
   P("filt_env_amt",0.4f),P("filt_attack",0.03f),P("filt_decay",0.5f),P("filt_sustain",0.3f),P("filt_release",0.25f),
   P("lfo1_rate",6.f),P("lfo1_depth",0.08f),P("lfo1_dest",0),
   P("lfo2_rate",0.2f),P("lfo2_depth",0.2f),P("lfo2_dest",1),
   P("scale",11),P("tuning_system",5),  // Maqam Rast + Mesopotamian
   P("fx_dist_amt",0.15f),P("fx_dist_tone",0.5f),P("fx_dist_type",1),
   P("fx_rev_mix",0.35f),P("fx_rev_size",0.6f),P("fx_rev_damp",0.2f),
   P("fx_dly_mix",0.12f),P("fx_dly_time",0.2f),P("fx_dly_fb",0.28f),
   P("master_vol",0.9f)}},

// Breathy ney: airy WT + slow filter + wide Eastern vibrato
{ "Ney Flute","Lead",
  {P("engine",0),P("wt_shape",2),P("wt_pos",0.18f),
   P("amp_attack",0.12f),P("amp_decay",0.3f),P("amp_sustain",0.5f),P("amp_release",0.3f),
   P("filt_cutoff",3200),P("filt_res",0.12f),P("filt_keytrack",0.35f),
   P("filt_env_amt",0.2f),P("filt_attack",0.1f),P("filt_decay",0.4f),P("filt_sustain",0.3f),P("filt_release",0.25f),
   P("lfo1_rate",5.5f),P("lfo1_depth",0.25f),P("lfo1_dest",0),   // wide ornamental vibrato
   P("lfo2_rate",0.15f),P("lfo2_depth",0.28f),P("lfo2_dest",1),  // breath-like filter sway
   P("scale",9),P("tuning_system",4),  // Double Harmonic + Egyptian
   P("fx_rev_mix",0.55f),P("fx_rev_size",0.78f),P("fx_rev_damp",0.15f),
   P("fx_dly_mix",0.2f),P("fx_dly_time",0.32f),P("fx_dly_fb",0.38f),
   P("master_vol",0.88f)}},

// Aggressive ritual horn: saturated FM + resonant sweep
{ "Ziggurat Horn","Lead",
  {P("engine",1),P("fm_algo",1),P("fm_feedback",0.55f),
   P("amp_attack",0.002f),P("amp_decay",0.2f),P("amp_sustain",0.55f),P("amp_release",0.25f),
   P("filt_type",6),P("filt_cutoff",4000),P("filt_res",0.35f),P("filt_drive",1.6f),P("filt_keytrack",0.5f),
   P("filt_env_amt",0.65f),P("filt_attack",0.001f),P("filt_decay",0.2f),P("filt_sustain",0.1f),P("filt_release",0.2f),
   P("lfo1_rate",0.18f),P("lfo1_depth",0.12f),P("lfo1_dest",1),
   P("lfo2_rate",7.f),P("lfo2_depth",0.04f),P("lfo2_dest",0),
   P("scale",8),P("tuning_system",2),  // Phrygian Dom + Pythagorean
   P("fx_dist_amt",0.22f),P("fx_dist_tone",0.45f),P("fx_dist_type",1),
   P("fx_rev_mix",0.35f),P("fx_rev_size",0.55f),P("fx_rev_damp",0.22f),
   P("fx_dly_mix",0.1f),P("fx_dly_time",0.14f),P("fx_dly_fb",0.25f),
   P("master_vol",0.88f)}},

// Ishtar's vocal quality: formant-like filter sweep + slow morph
{ "Ishtar's Song","Lead",
  {P("engine",0),P("wt_shape",7),P("wt_pos",0.55f),P("wt_morph",0.2f),
   P("amp_attack",0.04f),P("amp_decay",0.6f),P("amp_sustain",0.5f),P("amp_release",0.4f),
   P("filt_type",6),P("filt_cutoff",2200),P("filt_res",0.4f),P("filt_drive",1.4f),P("filt_keytrack",0.45f),
   P("filt_env_amt",0.5f),P("filt_attack",0.03f),P("filt_decay",0.7f),P("filt_sustain",0.2f),P("filt_release",0.3f),
   P("lfo1_rate",5.f),P("lfo1_depth",0.16f),P("lfo1_dest",0),
   P("lfo2_rate",0.08f),P("lfo2_depth",0.3f),P("lfo2_dest",1),  // deep formant sweep
   P("scale",12),P("tuning_system",3),  // Maqam Nahawand + Sumerian
   P("fx_chr_mix",0.2f),P("fx_chr_rate",0.2f),P("fx_chr_depth",0.45f),
   P("fx_rev_mix",0.5f),P("fx_rev_size",0.75f),P("fx_rev_damp",0.15f),
   P("fx_dly_mix",0.1f),P("fx_dly_time",0.3f),P("fx_dly_fb",0.3f),
   P("master_vol",0.9f)}},

// Processional fanfare: bold FM + bright sweep + Egyptian modal
{ "Pharaoh's March","Lead",
  {P("engine",1),P("fm_algo",0),P("fm_feedback",0.48f),
   P("amp_attack",0.015f),P("amp_decay",0.35f),P("amp_sustain",0.65f),P("amp_release",0.3f),
   P("filt_type",6),P("filt_cutoff",3800),P("filt_res",0.3f),P("filt_drive",1.5f),P("filt_keytrack",0.4f),
   P("filt_env_amt",0.55f),P("filt_attack",0.01f),P("filt_decay",0.4f),P("filt_sustain",0.2f),P("filt_release",0.25f),
   P("lfo1_rate",6.5f),P("lfo1_depth",0.06f),P("lfo1_dest",0),
   P("lfo2_rate",0.06f),P("lfo2_depth",0.15f),P("lfo2_dest",1),
   P("scale",3),P("tuning_system",4),  // Phrygian + Egyptian
   P("fx_dist_amt",0.12f),P("fx_dist_tone",0.3f),P("fx_dist_type",1),
   P("fx_rev_mix",0.42f),P("fx_rev_size",0.68f),P("fx_rev_damp",0.15f),
   P("master_vol",0.9f)}},

// ═══════════════════════════════════════════════════════════════════════
//  BASS (3) — deep, ritual, resonant
// ═══════════════════════════════════════════════════════════════════════

// Thundering ritual bass: heavy ladder filter + Sumerian pentatonic
{ "Ur Bass","Bass",
  {P("engine",1),P("fm_algo",0),P("fm_feedback",0.32f),
   P("amp_attack",0.001f),P("amp_decay",0.28f),P("amp_sustain",0.5f),P("amp_release",0.18f),
   P("filt_type",6),P("filt_cutoff",900),P("filt_res",0.35f),P("filt_drive",1.6f),
   P("filt_env_amt",0.65f),P("filt_attack",0.001f),P("filt_decay",0.2f),P("filt_sustain",0.08f),P("filt_release",0.12f),
   P("lfo1_rate",0.06f),P("lfo1_depth",0.1f),P("lfo1_dest",1),
   P("scale",1),P("tuning_system",3),  // pentatonic + Sumerian
   P("fx_dist_amt",0.18f),P("fx_dist_tone",0.15f),P("fx_dist_type",1),
   P("fx_rev_mix",0.08f),P("fx_rev_size",0.3f),P("fx_rev_damp",0.5f),
   P("master_vol",0.95f)}},

// Waveguide low drone with slow filter breathing
{ "Temple Drone","Bass",
  {P("engine",4),P("wg_stiff",0.08f),P("wg_damp",0.18f),
   P("amp_attack",0.35f),P("amp_decay",0.5f),P("amp_sustain",0.82f),P("amp_release",1.2f),
   P("filt_cutoff",1500),P("filt_res",0.2f),P("filt_drive",1.3f),
   P("filt_env_amt",0.22f),P("filt_attack",0.3f),P("filt_decay",0.5f),P("filt_sustain",0.4f),P("filt_release",0.8f),
   P("lfo1_rate",0.04f),P("lfo1_depth",0.2f),P("lfo1_dest",1),  // very slow breathing
   P("lfo2_rate",0.02f),P("lfo2_depth",0.08f),P("lfo2_dest",0),
   P("scale",7),P("tuning_system",1),  // Harmonic Minor + Just
   P("fx_rev_mix",0.65f),P("fx_rev_size",0.92f),P("fx_rev_damp",0.25f),
   P("fx_chr_mix",0.12f),P("fx_chr_rate",0.08f),
   P("master_vol",0.9f)}},

// Buzzy double-reed sub: nasal formant + warm saturation
{ "Nile Reed","Bass",
  {P("engine",0),P("wt_shape",3),P("wt_pos",0.38f),
   P("amp_attack",0.002f),P("amp_decay",0.22f),P("amp_sustain",0.55f),P("amp_release",0.15f),
   P("filt_type",3),P("filt_cutoff",600),P("filt_res",0.3f),P("filt_drive",1.5f),
   P("filt_env_amt",0.5f),P("filt_attack",0.002f),P("filt_decay",0.15f),P("filt_sustain",0.12f),P("filt_release",0.1f),
   P("lfo1_rate",0.1f),P("lfo1_depth",0.12f),P("lfo1_dest",1),
   P("scale",5),P("tuning_system",4),  // Mixolydian + Egyptian
   P("fx_dist_amt",0.12f),P("fx_dist_tone",0.2f),P("fx_dist_type",1),
   P("fx_rev_mix",0.06f),P("fx_rev_size",0.2f),
   P("master_vol",0.95f)}},

// ═══════════════════════════════════════════════════════════════════════
//  KEYS (5) — plucked strings, bells, struck instruments
// ═══════════════════════════════════════════════════════════════════════

// Bright gut-string pluck with temple reverb
{ "Kithara Pluck","Keys",
  {P("engine",3),P("ks_color",1),P("ks_body",0.32f),P("ks_pick",0.42f),P("ks_stretch",0.82f),
   P("amp_attack",0.001f),P("amp_decay",1.2f),P("amp_sustain",0),P("amp_release",0.6f),
   P("filt_cutoff",9000),P("filt_res",0.1f),P("filt_keytrack",0.7f),
   P("filt_env_amt",0.35f),P("filt_attack",0.001f),P("filt_decay",1.f),P("filt_sustain",0),P("filt_release",0.5f),
   P("scale",1),P("tuning_system",3),  // pentatonic + Sumerian
   P("fx_rev_mix",0.5f),P("fx_rev_size",0.75f),P("fx_rev_damp",0.18f),
   P("fx_chr_mix",0.1f),P("fx_chr_rate",0.18f),
   P("master_vol",0.95f)}},

// Soft harp with Egyptian tuning + long shimmer
{ "Harp of Amun","Keys",
  {P("engine",3),P("ks_color",0),P("ks_body",0.5f),P("ks_pick",0.25f),P("ks_stretch",0.9f),
   P("amp_attack",0.001f),P("amp_decay",1.8f),P("amp_sustain",0),P("amp_release",1.2f),
   P("filt_cutoff",6500),P("filt_res",0.06f),P("filt_keytrack",0.75f),
   P("filt_env_amt",0.2f),P("filt_attack",0.001f),P("filt_decay",1.5f),P("filt_sustain",0),P("filt_release",1.f),
   P("scale",9),P("tuning_system",4),  // Double Harmonic + Egyptian
   P("fx_rev_mix",0.6f),P("fx_rev_size",0.85f),P("fx_rev_damp",0.1f),
   P("fx_chr_mix",0.14f),P("fx_chr_rate",0.15f),P("fx_chr_depth",0.42f),
   P("fx_dly_mix",0.08f),P("fx_dly_time",0.4f),P("fx_dly_fb",0.22f),
   P("master_vol",0.92f)}},

// Inharmonic bronze bell: FM with non-integer ratios + Pythagorean
{ "Bronze Bell","Keys",
  {P("engine",1),P("fm_algo",6),P("fm_feedback",0.24f),
   P("amp_attack",0.001f),P("amp_decay",1.8f),P("amp_sustain",0.01f),P("amp_release",1.2f),
   P("filt_cutoff",15000),P("filt_res",0.04f),
   P("filt_env_amt",0.08f),P("filt_attack",0.001f),P("filt_decay",1.8f),P("filt_sustain",0.02f),P("filt_release",1.f),
   P("scale",10),P("tuning_system",2),  // Hirajoshi + Pythagorean
   P("fx_rev_mix",0.7f),P("fx_rev_size",0.9f),P("fx_rev_damp",0.06f),
   P("fx_dly_mix",0.06f),P("fx_dly_time",0.5f),P("fx_dly_fb",0.18f),
   P("master_vol",0.88f)}},

// Stopped pipe organ with Just Intonation purity
{ "Temple Organ","Keys",
  {P("engine",0),P("wt_shape",5),P("wt_pos",0.5f),
   P("amp_attack",0.025f),P("amp_decay",0.04f),P("amp_sustain",0.88f),P("amp_release",0.1f),
   P("filt_cutoff",4000),P("filt_res",0.1f),P("filt_drive",1.2f),
   P("filt_env_amt",0.12f),P("filt_attack",0.02f),P("filt_decay",0.1f),P("filt_sustain",0.65f),
   P("lfo1_rate",6.8f),P("lfo1_depth",0.04f),P("lfo1_dest",0),    // fast organ vibrato
   P("lfo2_rate",0.08f),P("lfo2_depth",0.12f),P("lfo2_dest",1),   // slow leslie-like
   P("scale",4),P("tuning_system",1),  // Lydian + Just Intonation
   P("fx_chr_mix",0.45f),P("fx_chr_rate",0.22f),P("fx_chr_depth",0.6f),
   P("fx_rev_mix",0.5f),P("fx_rev_size",0.8f),P("fx_rev_damp",0.2f),
   P("master_vol",0.88f)}},

// Metallic waveguide struck idiophone
{ "Osiris Gate","Keys",
  {P("engine",4),P("wg_stiff",0.4f),P("wg_damp",0.1f),
   P("amp_attack",0.001f),P("amp_decay",1.2f),P("amp_sustain",0.02f),P("amp_release",0.8f),
   P("filt_cutoff",11000),P("filt_res",0.08f),P("filt_keytrack",0.55f),
   P("filt_env_amt",0.12f),P("filt_attack",0.001f),P("filt_decay",1.2f),P("filt_sustain",0),P("filt_release",0.6f),
   P("scale",10),P("tuning_system",4),  // Hirajoshi + Egyptian
   P("fx_rev_mix",0.6f),P("fx_rev_size",0.85f),P("fx_rev_damp",0.08f),
   P("fx_dly_mix",0.08f),P("fx_dly_time",0.35f),P("fx_dly_fb",0.22f),
   P("master_vol",0.9f)}},

// ═══════════════════════════════════════════════════════════════════════
//  PADS (5) — vast, ritualistic, otherworldly
// ═══════════════════════════════════════════════════════════════════════

// Vocal choir with wide detuned unison + formant filter sweep
{ "Anunnaki Choir","Pad",
  {P("engine",0),P("wt_shape",7),P("wt_pos",0.65f),P("wt_morph",0.18f),
   P("amp_attack",1.8f),P("amp_decay",0.8f),P("amp_sustain",0.72f),P("amp_release",1.2f),
   P("filt_type",6),P("filt_cutoff",2200),P("filt_res",0.2f),P("filt_drive",1.3f),
   P("filt_env_amt",0.25f),P("filt_attack",1.5f),P("filt_decay",0.7f),P("filt_sustain",0.35f),P("filt_release",0.8f),
   P("unison",5),P("unison_spread",0.55f),P("detune",20),
   P("lfo1_rate",0.04f),P("lfo1_depth",0.2f),P("lfo1_dest",1),   // glacial formant
   P("lfo2_rate",0.02f),P("lfo2_depth",0.08f),P("lfo2_dest",0),  // pitch drift
   P("scale",7),P("tuning_system",1),  // Harmonic Minor + Just
   P("fx_chr_mix",0.55f),P("fx_chr_rate",0.1f),P("fx_chr_depth",0.65f),
   P("fx_rev_mix",0.72f),P("fx_rev_size",0.95f),P("fx_rev_damp",0.18f),
   P("master_vol",0.8f)}},

// Deep organ drone with glacial movement
{ "Nile Drone","Pad",
  {P("engine",0),P("wt_shape",5),P("wt_pos",0.6f),
   P("amp_attack",2.f),P("amp_decay",0.8f),P("amp_sustain",0.85f),P("amp_release",1.2f),
   P("filt_cutoff",1000),P("filt_res",0.15f),P("filt_drive",1.2f),
   P("filt_env_amt",0.1f),P("filt_attack",1.8f),P("filt_decay",0.5f),P("filt_sustain",0.48f),P("filt_release",1.f),
   P("unison",2),P("unison_spread",0.55f),P("detune",16),
   P("lfo1_rate",0.03f),P("lfo1_depth",0.3f),P("lfo1_dest",1),
   P("lfo2_rate",0.015f),P("lfo2_depth",0.12f),P("lfo2_dest",0),
   P("scale",1),P("tuning_system",3),  // pentatonic + Sumerian
   P("fx_rev_mix",0.78f),P("fx_rev_size",0.96f),P("fx_rev_damp",0.25f),
   P("master_vol",0.82f)}},

// Pyramid meditation: evolving bell wavetable + Egyptian modal
{ "Pyramid Meditation","Pad",
  {P("engine",0),P("wt_shape",6),P("wt_pos",0.48f),P("wt_morph",0.25f),
   P("amp_attack",2.f),P("amp_decay",1.f),P("amp_sustain",0.68f),P("amp_release",1.2f),
   P("filt_cutoff",1800),P("filt_res",0.22f),P("filt_drive",1.3f),
   P("filt_env_amt",0.28f),P("filt_attack",1.8f),P("filt_decay",0.8f),P("filt_sustain",0.28f),P("filt_release",1.f),
   P("unison",4),P("unison_spread",0.48f),P("detune",12),
   P("lfo1_rate",0.025f),P("lfo1_depth",0.3f),P("lfo1_dest",1),
   P("lfo2_rate",0.012f),P("lfo2_depth",0.08f),P("lfo2_dest",0),
   P("scale",9),P("tuning_system",4),  // Double Harmonic + Egyptian
   P("fx_chr_mix",0.42f),P("fx_chr_rate",0.08f),P("fx_chr_depth",0.58f),
   P("fx_rev_mix",0.72f),P("fx_rev_size",0.94f),P("fx_rev_damp",0.18f),
   P("fx_dly_mix",0.05f),P("fx_dly_time",0.5f),P("fx_dly_fb",0.2f),
   P("master_vol",0.82f)}},

// Sphinx Riddle: dark pulsing pad with Phrygian tension
{ "Sphinx Riddle","Pad",
  {P("engine",0),P("wt_shape",4),P("wt_pos",0.55f),
   P("amp_attack",1.5f),P("amp_decay",0.8f),P("amp_sustain",0.65f),P("amp_release",1.2f),
   P("filt_cutoff",1500),P("filt_res",0.25f),P("filt_drive",1.4f),
   P("filt_env_amt",0.2f),P("filt_attack",1.2f),P("filt_decay",0.6f),P("filt_sustain",0.3f),P("filt_release",0.8f),
   P("unison",3),P("unison_spread",0.6f),P("detune",22),
   P("lfo1_rate",0.5f),P("lfo1_depth",0.15f),P("lfo1_dest",1),   // pulsing filter
   P("lfo2_rate",0.03f),P("lfo2_depth",0.06f),P("lfo2_dest",0),
   P("scale",3),P("tuning_system",2),  // Phrygian + Pythagorean
   P("fx_chr_mix",0.35f),P("fx_chr_rate",0.06f),
   P("fx_rev_mix",0.68f),P("fx_rev_size",0.9f),P("fx_rev_damp",0.3f),
   P("master_vol",0.82f)}},

// Desert Night: extremely dark with wide detuned shimmer
{ "Desert Night","Pad",
  {P("engine",0),P("wt_shape",5),P("wt_pos",0.72f),
   P("amp_attack",2.5f),P("amp_decay",1.f),P("amp_sustain",0.6f),P("amp_release",1.2f),
   P("filt_cutoff",700),P("filt_res",0.18f),P("filt_drive",1.3f),
   P("filt_env_amt",0.12f),P("filt_attack",2.f),P("filt_decay",0.8f),P("filt_sustain",0.3f),P("filt_release",1.f),
   P("unison",3),P("unison_spread",0.65f),P("detune",25),
   P("lfo1_rate",0.02f),P("lfo1_depth",0.35f),P("lfo1_dest",1),
   P("lfo2_rate",0.01f),P("lfo2_depth",0.12f),P("lfo2_dest",0),
   P("scale",8),P("tuning_system",5),  // Phrygian Dominant + Mesopotamian
   P("fx_chr_mix",0.3f),P("fx_chr_rate",0.05f),
   P("fx_rev_mix",0.82f),P("fx_rev_size",0.97f),P("fx_rev_damp",0.28f),
   P("master_vol",0.82f)}},

// ═══════════════════════════════════════════════════════════════════════
//  ATMOSPHERE (4) — granular textures, wind, spirit voices
// ═══════════════════════════════════════════════════════════════════════

// Howling wind with filter resonance creating whistle harmonics
{ "Sandstorm","Atmo",
  {P("engine",2),P("gr_source",4),P("gr_size",0.14f),P("gr_scatter",0.75f),P("gr_density",0.88f),P("gr_pitch_rnd",0.4f),
   P("amp_attack",1.2f),P("amp_decay",0.5f),P("amp_sustain",0.65f),P("amp_release",1.2f),
   P("filt_cutoff",2000),P("filt_res",0.35f),P("filt_drive",1.5f),
   P("filt_env_amt",0.42f),P("filt_attack",1.f),P("filt_decay",1.f),P("filt_sustain",0.2f),P("filt_release",1.f),
   P("lfo1_rate",0.05f),P("lfo1_depth",0.4f),P("lfo1_dest",1),   // deep howling filter
   P("lfo2_rate",0.025f),P("lfo2_depth",0.12f),P("lfo2_dest",0),
   P("scale",1),P("tuning_system",4),  // pentatonic + Egyptian
   P("fx_rev_mix",0.72f),P("fx_rev_size",0.92f),P("fx_rev_damp",0.28f),
   P("fx_dly_mix",0.12f),P("fx_dly_time",0.35f),P("fx_dly_fb",0.35f),
   P("master_vol",0.88f)}},

// Granular incantation: vocal formant grains + temple space
{ "Spirit Voice","Atmo",
  {P("engine",2),P("gr_source",1),P("gr_size",0.12f),P("gr_scatter",0.15f),P("gr_density",0.85f),P("gr_pitch_rnd",0.04f),
   P("amp_attack",0.8f),P("amp_decay",0.4f),P("amp_sustain",0.72f),P("amp_release",1.2f),
   P("filt_type",6),P("filt_cutoff",2800),P("filt_res",0.25f),P("filt_drive",1.3f),
   P("filt_env_amt",0.35f),P("filt_attack",0.6f),P("filt_decay",0.5f),P("filt_sustain",0.28f),P("filt_release",0.8f),
   P("lfo1_rate",0.12f),P("lfo1_depth",0.2f),P("lfo1_dest",1),
   P("lfo2_rate",0.04f),P("lfo2_depth",0.06f),P("lfo2_dest",0),
   P("scale",12),P("tuning_system",5),  // Maqam Nahawand + Mesopotamian
   P("fx_chr_mix",0.28f),P("fx_chr_rate",0.18f),P("fx_chr_depth",0.48f),
   P("fx_rev_mix",0.65f),P("fx_rev_size",0.85f),P("fx_rev_damp",0.2f),
   P("master_vol",0.88f)}},

// Vast ethereal field: sparse grains + maximum reverb
{ "Akashic Field","Atmo",
  {P("engine",2),P("gr_source",2),P("gr_size",0.22f),P("gr_scatter",0.65f),P("gr_density",0.48f),P("gr_pitch_rnd",0.1f),
   P("amp_attack",1.8f),P("amp_decay",1.f),P("amp_sustain",0.55f),P("amp_release",1.2f),
   P("filt_cutoff",1400),P("filt_res",0.12f),
   P("filt_env_amt",0.18f),P("filt_attack",1.5f),P("filt_decay",1.f),P("filt_sustain",0.3f),P("filt_release",1.f),
   P("detune",10),
   P("lfo1_rate",0.02f),P("lfo1_depth",0.28f),P("lfo1_dest",1),
   P("lfo2_rate",0.01f),P("lfo2_depth",0.1f),P("lfo2_dest",0),
   P("scale",10),P("tuning_system",1),  // Hirajoshi + Just
   P("fx_chr_mix",0.4f),P("fx_chr_rate",0.05f),P("fx_chr_depth",0.58f),
   P("fx_rev_mix",0.85f),P("fx_rev_size",0.97f),P("fx_rev_damp",0.2f),
   P("master_vol",0.85f)}},

// Crystal grain swirl with pitch scatter
{ "Dust Devils","Atmo",
  {P("engine",2),P("gr_source",6),P("gr_size",0.06f),P("gr_scatter",0.85f),P("gr_density",0.92f),P("gr_pitch_rnd",0.45f),
   P("amp_attack",1.f),P("amp_decay",0.5f),P("amp_sustain",0.6f),P("amp_release",1.2f),
   P("filt_cutoff",3500),P("filt_res",0.28f),P("filt_drive",1.4f),
   P("filt_env_amt",0.38f),P("filt_attack",0.6f),P("filt_decay",0.8f),P("filt_sustain",0.2f),P("filt_release",0.8f),
   P("lfo1_rate",0.08f),P("lfo1_depth",0.35f),P("lfo1_dest",1),
   P("lfo2_rate",0.04f),P("lfo2_depth",0.15f),P("lfo2_dest",0),
   P("scale",3),P("tuning_system",5),  // Phrygian + Mesopotamian
   P("fx_rev_mix",0.62f),P("fx_rev_size",0.88f),P("fx_rev_damp",0.22f),
   P("fx_dly_mix",0.15f),P("fx_dly_time",0.2f),P("fx_dly_fb",0.38f),
   P("master_vol",0.88f)}},

// ═══════════════════════════════════════════════════════════════════════
//  PERCUSSION (4) — ritual drums, metalwork, clay
// ═══════════════════════════════════════════════════════════════════════

// Deep frame drum: noise KS + heavy filter thump
{ "Balag Drum","Perc",
  {P("engine",3),P("ks_color",3),P("ks_body",0.88f),P("ks_pick",0.5f),P("ks_stretch",0.06f),
   P("amp_attack",0.001f),P("amp_decay",0.35f),P("amp_sustain",0),P("amp_release",0.2f),
   P("filt_cutoff",4000),P("filt_res",0.3f),P("filt_drive",1.5f),
   P("filt_env_amt",0.8f),P("filt_attack",0.001f),P("filt_decay",0.05f),P("filt_sustain",0),P("filt_release",0.1f),
   P("fx_rev_mix",0.28f),P("fx_rev_size",0.5f),P("fx_rev_damp",0.35f),
   P("fx_dist_amt",0.08f),P("fx_dist_tone",0.22f),P("fx_dist_type",1),
   P("master_vol",0.95f)}},

// Metallic rattle: FM inharmonic partials + bright reverb
{ "Bronze Sistrum","Perc",
  {P("engine",1),P("fm_algo",6),P("fm_feedback",0.2f),
   P("amp_attack",0.001f),P("amp_decay",0.48f),P("amp_sustain",0),P("amp_release",0.5f),
   P("filt_cutoff",14000),P("filt_res",0.06f),
   P("filt_env_amt",0.06f),P("filt_attack",0.001f),P("filt_decay",0.32f),P("filt_sustain",0),P("filt_release",0.35f),
   P("scale",10),P("tuning_system",2),  // Hirajoshi + Pythagorean
   P("fx_rev_mix",0.5f),P("fx_rev_size",0.65f),P("fx_rev_damp",0.1f),
   P("fx_dly_mix",0.1f),P("fx_dly_time",0.18f),P("fx_dly_fb",0.2f),
   P("master_vol",0.92f)}},

// Dry clay hit with earthy body
{ "Clay Tablet","Perc",
  {P("engine",3),P("ks_color",2),P("ks_body",0.72f),P("ks_pick",0.42f),P("ks_stretch",0.05f),
   P("amp_attack",0.001f),P("amp_decay",0.25f),P("amp_sustain",0),P("amp_release",0.12f),
   P("filt_cutoff",2200),P("filt_res",0.25f),P("filt_drive",1.4f),
   P("filt_env_amt",0.6f),P("filt_attack",0.001f),P("filt_decay",0.04f),P("filt_sustain",0),P("filt_release",0.06f),
   P("fx_rev_mix",0.2f),P("fx_rev_size",0.4f),P("fx_rev_damp",0.45f),
   P("master_vol",0.95f)}},

// High metallic chime with long shimmer tail
{ "Ankh Chime","Perc",
  {P("engine",1),P("fm_algo",4),P("fm_feedback",0.14f),
   P("amp_attack",0.001f),P("amp_decay",0.7f),P("amp_sustain",0),P("amp_release",0.8f),
   P("filt_cutoff",12000),P("filt_res",0.08f),
   P("filt_env_amt",0.08f),P("filt_attack",0.001f),P("filt_decay",0.55f),P("filt_sustain",0),P("filt_release",0.6f),
   P("scale",10),P("tuning_system",1),  // Hirajoshi + Just
   P("fx_rev_mix",0.6f),P("fx_rev_size",0.78f),P("fx_rev_damp",0.06f),
   P("fx_dly_mix",0.1f),P("fx_dly_time",0.28f),P("fx_dly_fb",0.22f),
   P("master_vol",0.9f)}},

// ═══════════════════════════════════════════════════════════════════════
//  WORLD (3) — culturally specific performance idioms
// ═══════════════════════════════════════════════════════════════════════

// Quarter-tone ornamental melody: wide vibrato + Maqam scale + delay
{ "Maqam Meditation","World",
  {P("engine",0),P("wt_shape",6),P("wt_pos",0.4f),
   P("amp_attack",0.04f),P("amp_decay",0.45f),P("amp_sustain",0.5f),P("amp_release",0.8f),
   P("filt_cutoff",4200),P("filt_res",0.3f),P("filt_drive",1.3f),P("filt_keytrack",0.4f),
   P("filt_env_amt",0.45f),P("filt_attack",0.03f),P("filt_decay",0.55f),P("filt_sustain",0.15f),P("filt_release",0.6f),
   P("lfo1_rate",0.5f),P("lfo1_depth",0.3f),P("lfo1_dest",0),   // ornamental quarter-tone wobble
   P("lfo2_rate",0.06f),P("lfo2_depth",0.22f),P("lfo2_dest",1),
   P("scale",11),P("tuning_system",5),  // Maqam Rast + Mesopotamian
   P("fx_rev_mix",0.55f),P("fx_rev_size",0.75f),P("fx_rev_damp",0.15f),
   P("fx_dly_mix",0.22f),P("fx_dly_time",0.26f),P("fx_dly_fb",0.4f),
   P("master_vol",0.9f)}},

// Breathy granular ritual chant
{ "Sacred Breath","World",
  {P("engine",2),P("gr_source",4),P("gr_size",0.11f),P("gr_scatter",0.15f),P("gr_density",0.9f),P("gr_pitch_rnd",0.03f),
   P("amp_attack",0.45f),P("amp_decay",0.3f),P("amp_sustain",0.68f),P("amp_release",1.f),
   P("filt_cutoff",3200),P("filt_res",0.15f),P("filt_drive",1.2f),
   P("filt_env_amt",0.28f),P("filt_attack",0.3f),P("filt_decay",0.3f),P("filt_sustain",0.35f),P("filt_release",0.7f),
   P("lfo1_rate",0.3f),P("lfo1_depth",0.16f),P("lfo1_dest",1),
   P("lfo2_rate",0.08f),P("lfo2_depth",0.06f),P("lfo2_dest",0),
   P("scale",7),P("tuning_system",3),  // Harmonic Minor + Sumerian
   P("fx_rev_mix",0.6f),P("fx_rev_size",0.82f),P("fx_rev_damp",0.18f),
   P("fx_chr_mix",0.22f),P("fx_chr_rate",0.15f),P("fx_chr_depth",0.4f),
   P("master_vol",0.9f)}},

// Rhythmic FM pluck with Mesopotamian groove
{ "Tigris Dance","World",
  {P("engine",1),P("fm_algo",2),P("fm_feedback",0.4f),
   P("amp_attack",0.002f),P("amp_decay",0.35f),P("amp_sustain",0.1f),P("amp_release",0.2f),
   P("filt_type",6),P("filt_cutoff",4800),P("filt_res",0.28f),P("filt_drive",1.5f),P("filt_keytrack",0.45f),
   P("filt_env_amt",0.55f),P("filt_attack",0.001f),P("filt_decay",0.25f),P("filt_sustain",0.06f),P("filt_release",0.15f),
   P("lfo2_rate",0.12f),P("lfo2_depth",0.1f),P("lfo2_dest",1),
   P("scale",8),P("tuning_system",5),  // Phrygian Dominant + Mesopotamian
   P("fx_rev_mix",0.35f),P("fx_rev_size",0.6f),P("fx_rev_damp",0.2f),
   P("fx_dly_mix",0.2f),P("fx_dly_time",0.18f),P("fx_dly_fb",0.35f),
   P("master_vol",0.92f)}},
};
#undef P

class PresetManager {
public:
    explicit PresetManager(juce::AudioProcessorValueTreeState& apvts) : apvts_(apvts) {}
    int  count()        const { return int(kPresets.size()); }
    int  numPresets()   const { return int(kPresets.size()); }
    int  index()        const { return idx_; }
    int  currentIndex() const { return idx_; }
    const char* name()     const { return kPresets[size_t(idx_)].name; }
    const char* category() const { return kPresets[size_t(idx_)].category; }
    bool consumePresetDirty() { return presetDirty_.exchange(false); }
    void load(int i) {
        if (i<0||i>=count()) return; idx_=i;
        static const std::map<std::string,float> kDefaults = {
            {"engine",0},{"wt_shape",0},{"wt_pos",0},{"wt_morph",0},
            {"fm_algo",0},{"fm_feedback",0.3f},
            {"gr_size",0.08f},{"gr_scatter",0.3f},{"gr_density",0.6f},{"gr_pitch_rnd",0.1f},{"gr_source",0},
            {"ks_body",0.5f},{"ks_pick",0.25f},{"ks_color",0},{"ks_stretch",0.5f},
            {"wg_stiff",0.3f},{"wg_damp",0.4f},
            {"amp_attack",0.01f},{"amp_decay",0.2f},{"amp_sustain",0.8f},{"amp_release",0.5f},
            {"filt_type",0},{"filt_cutoff",8000},{"filt_res",0.0f},{"filt_drive",1},
            {"filt_env_amt",0},{"filt_keytrack",0},
            {"filt_attack",0.01f},{"filt_decay",0.2f},{"filt_sustain",0.5f},{"filt_release",0.5f},
            {"lfo1_rate",1},{"lfo1_depth",0},{"lfo1_shape",0},{"lfo1_dest",0},
            {"lfo2_rate",2},{"lfo2_depth",0},{"lfo2_shape",0},{"lfo2_dest",1},
            {"glide",0},{"detune",0},{"unison",0},{"unison_spread",0},
            {"poly_mode",0},{"scale",0},{"scale_root",0},
            {"arp_on",0},{"arp_mode",0},{"arp_rate",1},{"arp_gate",0.8f},{"arp_swing",0},
            {"fx_rev_mix",0},{"fx_rev_size",0.5f},{"fx_rev_damp",0.4f},
            {"fx_chr_mix",0},{"fx_chr_rate",0.5f},{"fx_chr_depth",0.3f},
            {"fx_dly_mix",0},{"fx_dly_time",0.3f},{"fx_dly_fb",0.4f},{"fx_dly_sync",0},
            {"fx_dist_amt",0},{"fx_dist_tone",0.5f},{"fx_dist_type",0},
            {"fx_eq_lo",0},{"fx_eq_mid",0},{"fx_eq_hi",0},
            {"master_vol",0.9f},{"master_gain",0},{"master_tune",0},{"stereo_width",1},
            {"tuning_system",0},{"tuning_root",0}
        };
        for (auto& [k,v] : kDefaults)
            if (auto* p=apvts_.getParameter(k)) p->setValueNotifyingHost(p->convertTo0to1(v));
        for (auto& [k,v] : kPresets[size_t(i)].params)
            if (auto* p=apvts_.getParameter(k)) p->setValueNotifyingHost(p->convertTo0to1(v));
        presetDirty_.store(true);
    }
    void next() { load((idx_+1)%count()); }
    void prev() { load((idx_-1+count())%count()); }
private:
    juce::AudioProcessorValueTreeState& apvts_;
    int idx_=0;
    std::atomic<bool> presetDirty_{false};
};
