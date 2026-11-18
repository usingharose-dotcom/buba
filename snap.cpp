#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include "PluginEditor.h"
int main()
{
    juce::ScopedJuceInitialiser_GUI init;
    YoloProcessor p; p.prepareToPlay (48000, 512);
    p.setCurrentProgram (2);
    // feed a tone so the meters/yin yang react
    juce::AudioBuffer<float> b (2, 512); juce::MidiBuffer m;
    for (int blk = 0; blk < 40; ++blk) { for (int i = 0; i < 512; ++i) { float v = 0.3f * std::sin (6.2831853f * 237.f * (float) (blk * 512 + i) / 48000.f); b.setSample (0, i, v); b.setSample (1, i, v); } p.processBlock (b, m); }
    std::unique_ptr<juce::AudioProcessorEditor> ed (p.createEditor());
    ed->setBounds (0, 0, ed->getWidth(), ed->getHeight());
    struct Q : juce::Timer { void timerCallback() override { stopTimer(); juce::MessageManager::getInstance()->stopDispatchLoop(); } } q; q.startTimer (1500);
    juce::MessageManager::getInstance()->runDispatchLoop();
    auto img = ed->createComponentSnapshot (ed->getLocalBounds(), true, 1.5f);
    juce::PNGImageFormat png; juce::File f ("/tmp/yolo_editor.png"); f.deleteFile();
    juce::FileOutputStream os (f); png.writeImageToStream (img, os);
    return 0;
}
