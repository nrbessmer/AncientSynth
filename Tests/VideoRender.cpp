// ASVideo: Ancient Synth demo rendered offline with the real synth + editor.
// Six presets, two bars each at 92 BPM, each with its own phrase (Hijaz on D).
// Writes fNNNNN.png + synth.wav + captions.txt. Usage: ASVideo outDir [scale]
#include <sstream>
#include <fstream>
#include <map>
#include <atomic>
#include <array>
#include <random>
#include <functional>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_dsp/juce_dsp.h>
#define private public
#define protected public
#include "../Source/PluginProcessor.cpp"
#undef private
#undef protected

int main (int argc, char** argv)
{
    juce::ScopedJuceInitialiser_GUI init;
    const double SR = 48000.0; const int FPS = 30, spf = 1600, B = 400;
    const juce::File outDir (argv[1]);
    const float scale = argc > 2 ? (float) atof (argv[2]) : 1.25f;
    outDir.createDirectory();

    AncientSynthProcessor proc;
    proc.setPlayConfigDetails (0, 2, SR, B);
    proc.prepareToPlay (SR, B);
    std::unique_ptr<juce::AudioProcessorEditor> ed (proc.createEditor());
    auto* e = static_cast<AncientSynthEditor*> (ed.get());
    if (argc > 3 && juce::String (argv[3]) == "tabs")   // layout check: one PNG per tab
    {
        for (int i = 0; i < e->tabs_.getNumTabs(); ++i)
        {
            e->tabs_.setCurrentTabIndex (i);
            auto img = ed->createComponentSnapshot (ed->getLocalBounds(), true, 1.0f);
            juce::File f = outDir.getChildFile ("tab" + juce::String (i) + ".png"); f.deleteFile();
            juce::FileOutputStream fos (f); juce::PNGImageFormat().writeImageToStream (img, fos);
        }
        ed = nullptr; return 0;
    }

    struct Part { const char* preset; char kind; };
    const Part parts[] = { { "Sumerian Lyre", 'M' }, { "Harp of Amun", 'A' }, { "Anunnaki Choir", 'C' },
                           { "Ziggurat Horn", 'H' }, { "Tigris Dance", 'D' }, { "Bronze Bell", 'B' } };
    const int nParts = 6;
    const double beat = 60.0 / 92.0, partLen = 8 * beat, tEnd = nParts * partLen + 1.5;
    struct Ev { double t; int note; int vel; bool on; };
    std::vector<Ev> ev;
    auto note = [&] (double t, double len, int n, int v = 100) { ev.push_back ({ t, n, v, true }); ev.push_back ({ t + len, n, 0, false }); };
    // Hijaz on D: D Eb F# G A Bb C
    const int D = 62, Eb = 63, Fs = 66, G = 67, A = 69, Bb = 70, C = 72, D5 = 74;
    for (int p = 0; p < nParts; ++p)
    {
        const double t0 = p * partLen; const double e8 = beat / 2;
        switch (parts[p].kind)
        {
            case 'M': { const int m[] = { D, Eb, Fs, G, A, G, Fs, Eb, D, Eb, Fs, A, G, Fs, Eb, D }; for (int i = 0; i < 16; ++i) note (t0 + i * e8, e8 * (i == 15 ? 1.9 : 0.9), m[i], 95 + (i % 4 == 0 ? 15 : 0)); } break;
            case 'A': { const int m[] = { D - 12, A - 12, D, Fs, A, D5, A, Fs }; for (int i = 0; i < 16; ++i) note (t0 + i * e8, e8 * 1.6, m[i % 8] + (i >= 8 ? -2 : 0) * 0, 90); } break;
            case 'C': note (t0, 4 * beat * 0.98, D - 12, 90); note (t0, 4 * beat * 0.98, A - 12, 85); note (t0, 4 * beat * 0.98, D, 85); note (t0, 4 * beat * 0.98, Fs, 80);
                      note (t0 + 4 * beat, 4 * beat * 0.98, G - 12, 90); note (t0 + 4 * beat, 4 * beat * 0.98, D, 85); note (t0 + 4 * beat, 4 * beat * 0.98, G, 85); note (t0 + 4 * beat, 4 * beat * 0.98, Bb, 80); break;
            case 'H': note (t0, 1.5 * beat, D, 110); note (t0 + 1.5 * beat, 0.5 * beat, Eb, 100); note (t0 + 2 * beat, 2 * beat, Fs, 110);
                      note (t0 + 4 * beat, 1.5 * beat, A, 110); note (t0 + 5.5 * beat, 0.5 * beat, G, 100); note (t0 + 6 * beat, 1.9 * beat, Fs, 105); break;
            case 'D': { const int m[] = { A, Bb, A, G, Fs, G, A, 0, A, Bb, C, Bb, A, G, Fs, Eb };
                        for (int i = 0; i < 16; ++i) if (m[i]) note (t0 + i * e8 + (i % 2 ? 0.04 : 0.0), e8 * 0.8, m[i], 100 + (i % 2 ? -12 : 10)); } break;
            case 'B': { const int m[] = { D5, A, Fs, D, Eb, Fs, A, D5 }; for (int i = 0; i < 8; ++i) note (t0 + i * beat, beat * 0.9, m[i], 95); } break;
        }
    }
    std::sort (ev.begin(), ev.end(), [] (const Ev& a, const Ev& b) { return a.t < b.t || (a.t == b.t && ! a.on && b.on); });

    auto& pm = proc.getPresets();
    auto findPreset = [&] (const char* n) { for (int i = 0; i < pm.count(); ++i) if (juce::String (kPresets[(size_t) i].name) == n) return i; return -1; };

    const int nFrames = (int) (tEnd * FPS);
    juce::AudioBuffer<float> out (2, nFrames * spf); out.clear();
    juce::AudioBuffer<float> blk (2, B);
    size_t ei = 0; int cur = -1; juce::String caps;
    for (int fr = 0; fr < nFrames; ++fr)
    {
        const double t = (double) fr / FPS;
        const int part = juce::jmin (nParts - 1, (int) (t / partLen));
        if (part != cur)
        {
            cur = part;
            const int idx = findPreset (parts[part].preset);
            if (idx >= 0) e->browser_.onSelect (idx);
            caps << juce::String (part * partLen, 3) << "\t" << juce::String (part + 1 < nParts ? (part + 1) * partLen : tEnd, 3) << "\t"
                 << parts[part].preset << "  -  " << (idx >= 0 ? kPresets[(size_t) idx].category : "?") << "\n";
            printf ("%5.2f %s (%d)\n", t, parts[part].preset, idx);
        }
        for (int k = 0; k < spf; k += B)
        {
            const double bt0 = t + k / SR, bt1 = bt0 + B / SR;
            juce::MidiBuffer midi;
            while (ei < ev.size() && ev[ei].t < bt1)
            {
                const int pos = juce::jlimit (0, B - 1, (int) ((ev[ei].t - bt0) * SR));
                midi.addEvent (ev[ei].on ? juce::MidiMessage::noteOn (1, ev[ei].note, (juce::uint8) ev[ei].vel) : juce::MidiMessage::noteOff (1, ev[ei].note), pos);
                ++ei;
            }
            blk.clear();
            proc.processBlock (blk, midi);
            for (int ch = 0; ch < 2; ++ch) out.copyFrom (ch, fr * spf + k, blk, ch, 0, B);
        }
        e->timerCallback();
        auto img = ed->createComponentSnapshot (ed->getLocalBounds(), true, scale);
        juce::File f = outDir.getChildFile (juce::String::formatted ("f%05d.png", fr)); f.deleteFile();
        juce::FileOutputStream fos (f); juce::PNGImageFormat().writeImageToStream (img, fos);
    }
    juce::WavAudioFormat wav; auto wf = outDir.getChildFile ("synth.wav"); wf.deleteFile();
    std::unique_ptr<juce::AudioFormatWriter> w (wav.createWriterFor (new juce::FileOutputStream (wf), SR, 2, 24, {}, 0));
    w->writeFromAudioSampleBuffer (out, 0, out.getNumSamples());
    outDir.getChildFile ("captions.txt").replaceWithText (caps);
    printf ("frames %d peak %.3f\n", nFrames, out.getMagnitude (0, out.getNumSamples()));
    ed = nullptr;
}
