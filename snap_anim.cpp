#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include "PluginEditor.h"
// Renders an animated preview: feeds a gliding "vocal" through the plugin and saves PNG frames.
struct Rec : juce::Timer
{
    YoloProcessor& p; juce::AudioProcessorEditor& ed; int frame = 0; double ph = 0, t = 0;
    Rec (YoloProcessor& pp, juce::AudioProcessorEditor& e) : p (pp), ed (e) {}
    void timerCallback() override
    {
        // 1/15 s of audio per frame: a note sliding between a few pitches, with vibrato; louder at the start of each note
        const int n = 3200; juce::AudioBuffer<float> b (2, 512); juce::MidiBuffer m;
        for (int done = 0; done < n; done += 512)
        {
            for (int i = 0; i < 512; ++i)
            {
                const double tt = t + (done + i) / 48000.0, seg = std::fmod (tt, 1.6);
                const double notes[4] = {230.0, 262.0, 294.0, 247.0}; const int k = ((int) (tt / 1.6)) % 4;
                const double f = notes[k] * (1.0 + 0.012 * std::sin (6.2832 * 5.5 * tt));
                ph += 6.2831853 * f / 48000.0; double v = 0; for (int h = 1; h <= 8; ++h) v += std::sin (h * ph) / h;
                const float env = (float) (seg < 1.3 ? 0.35 : 0.04);
                b.setSample (0, i, (float) v * .25f * env); b.setSample (1, i, (float) v * .25f * env);
            }
            p.processBlock (b, m);
        }
        t += n / 48000.0;
        auto img = ed.createComponentSnapshot (ed.getLocalBounds(), true, 1.f);
        juce::PNGImageFormat png; juce::File f (juce::String ("/tmp/frames/f") + juce::String (frame).paddedLeft ('0', 4) + ".png");
        juce::FileOutputStream os (f); png.writeImageToStream (img, os);
        if (++frame >= 120) { stopTimer(); juce::MessageManager::getInstance()->stopDispatchLoop(); }
    }
};
int main()
{
    juce::ScopedJuceInitialiser_GUI init;
    juce::File ("/tmp/frames").createDirectory();
    YoloProcessor p; p.prepareToPlay (48000, 512);
    std::unique_ptr<juce::AudioProcessorEditor> ed (p.createEditor());
    ed->setBounds (0, 0, ed->getWidth(), ed->getHeight());
    Rec r (p, *ed); r.startTimer (66);
    juce::MessageManager::getInstance()->runDispatchLoop();
    return 0;
}
