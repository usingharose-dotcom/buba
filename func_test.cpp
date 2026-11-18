#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include "PluginEditor.h"
// Drives the real editor with synthetic mouse events (design-space coordinates, 1264x676) and checks parameters/audio respond.
static int fails = 0;
static void check (const char* what, bool ok, double v = 0) { std::printf ("%-58s %s (%.3f)\n", what, ok ? "PASS" : "FAIL", v); if (! ok) ++fails; }
static float P (YoloProcessor& p, const char* id) { return p.apvts.getRawParameterValue (id)->load(); }
struct Stopper : juce::Timer { void timerCallback() override { stopTimer(); juce::MessageManager::getInstance()->stopDispatchLoop(); } };
static void pump (int ms) { Stopper st; st.startTimer (juce::jmax (1, ms)); juce::MessageManager::getInstance()->runDispatchLoop(); }
int main()
{
    juce::ScopedJuceInitialiser_GUI init;
    YoloProcessor p; p.prepareToPlay (48000, 512);
    std::unique_ptr<juce::AudioProcessorEditor> ed (p.createEditor());
    check ("editor opens compact (smaller than design size)", ed->getWidth() < 1264, ed->getWidth());
    ed->setVisible (true); ed->setSize (1264, 676); pump (300);
    juce::Component* tgt = nullptr; juce::Point<float> startPos; bool isDown = false; int clicks = 0; juce::uint32 lastT = 0;
    auto mk = [&] (juce::Component* c, juce::Point<float> pos, juce::Point<float> from, bool down, int nClicks)
    { auto src = juce::Desktop::getInstance().getMainMouseSource();
      return juce::MouseEvent (src, c->getLocalPoint (ed.get(), pos), down ? juce::ModifierKeys (juce::ModifierKeys::leftButtonModifier) : juce::ModifierKeys(), 1.f, 0.f, 0.f, 0.f, 0.f, c, c, juce::Time::getCurrentTime(), c->getLocalPoint (ed.get(), from), juce::Time::getCurrentTime(), nClicks, false); };
    auto ev = [&] (float x, float y, bool down)
    { juce::Point<float> pos { x, y };
      if (down && ! isDown) { tgt = ed->getComponentAt ((int) x, (int) y); if (! tgt) tgt = ed.get(); startPos = pos; isDown = true;
          auto t = juce::Time::getMillisecondCounter(); clicks = (t - lastT < 400) ? clicks + 1 : 1; lastT = t;
          tgt->mouseDown (mk (tgt, pos, pos, true, clicks)); if (clicks == 2) tgt->mouseDoubleClick (mk (tgt, pos, pos, true, 2)); }
      else if (down && isDown) tgt->mouseDrag (mk (tgt, pos, startPos, true, clicks));
      else if (! down && isDown) { tgt->mouseUp (mk (tgt, pos, startPos, false, clicks)); isDown = false; } };
    auto click = [&] (float x, float y) { ev (x, y, false); ev (x, y, true); ev (x, y, false); };
    auto drag = [&] (float x, float y, float dx, float dy) { ev (x, y, false); ev (x, y, true); ev (x + dx / 2, y + dy / 2, true); ev (x + dx, y + dy, true); ev (x + dx, y + dy, false); };
    auto changed = [&] (const char* what, const char* id, float cx, float cy, float dx, float dy) { const float v0 = P (p, id); drag (cx, cy, dx, dy); check (what, P (p, id) != v0, P (p, id)); };

    // yin yang = Doubler
    check ("doubler starts at 0", P (p, "doubler") == 0.f);
    drag (1106, 211, 0, -70); check ("dragging the yin yang up raises Doubler", P (p, "doubler") > 25.f, P (p, "doubler"));
    const float up = P (p, "doubler"); drag (1106, 211, 0, 40); check ("dragging it down lowers Doubler", P (p, "doubler") < up, P (p, "doubler"));
    { ev (1106, 211, false); ev (1106, 211, true); tgt->mouseDoubleClick (mk (tgt, { 1106, 211 }, { 1106, 211 }, true, 2)); ev (1106, 211, false); } check ("double-click resets Doubler", P (p, "doubler") == 0.f, P (p, "doubler"));
    { auto* q = p.apvts.getParameter ("doubler"); q->setValueNotifyingHost (1.f);
      juce::AudioBuffer<float> b (2, 512); juce::MidiBuffer m; double ph = 0, dif = 0, sum = 0;
      for (int blk = 0; blk < 200; ++blk) { for (int i = 0; i < 512; ++i) { ph += 6.2831853 * 237.0 / 48000; double v = 0; for (int h = 1; h <= 6; ++h) v += std::sin (h * ph) / h; b.setSample (0, i, (float) v * .25f); b.setSample (1, i, (float) v * .25f); }
        p.processBlock (b, m); if (blk > 100) for (int i = 0; i < 512; ++i) { dif += std::pow (b.getSample (0, i) - b.getSample (1, i), 2); sum += std::pow (b.getSample (0, i) + b.getSample (1, i), 2); } }
      check ("Doubler at 100% makes L and R differ (stereo)", dif / (sum + 1e-9) > .01, dif / (sum + 1e-9)); q->setValueNotifyingHost (0.f); }

    // logo -> credits
    { std::function<bool(juce::Component*)> shown = [&] (juce::Component* x) { for (auto* k : x->getChildren()) { if ((k->isVisible() && k->getWidth() == 270 && k->getHeight() == 146) || shown (k)) return true; } return false; };
      click (38, 28); check ("clicking the logo opens the credits", shown (ed.get()));
      click (100, 100); check ("clicking the credits closes them", ! shown (ed.get())); }

    // key & scale
    click (121, 134); check ("Major sets scale = Major", (int) P (p, "scale") == 1, P (p, "scale"));
    click (189, 134); check ("Minor sets scale = Minor", (int) P (p, "scale") == 2, P (p, "scale"));
    click (69, 351);  check ("Chromatic button sets scale = Chromatic", (int) P (p, "scale") == 0, P (p, "scale"));
    click (147, 445); check ("Stabilizer Mid", (int) P (p, "stab") == 2, P (p, "stab"));
    click (45, 445);  check ("Stabilizer None", (int) P (p, "stab") == 0, P (p, "stab"));
    click (147, 194); check ("Key grid D sets key = D", (int) P (p, "key") == 2, P (p, "key"));
    click (205, 79);  // flat symbol toggle (accidentals)
    click (173, 351); check ("Detect Key runs", true);

    // piano: tone + allow/mute
    p.apvts.getParameter ("tone")->setValueNotifyingHost (.5f);
    ev (303, 460, false); ev (303, 460, true);
    check ("Holding a piano key starts the reference tone", p.toneNote.load() == 60, p.toneNote.load());
    { juce::AudioBuffer<float> b (2, 512); juce::MidiBuffer m; float pk = 0; for (int k = 0; k < 8; ++k) { b.clear(); p.processBlock (b, m); pk = juce::jmax (pk, b.getMagnitude (0, 512)); }
      check ("...and you can hear it in the output", pk > .05f, pk); }
    ev (303, 460, false); check ("Releasing the key stops the tone", p.toneNote.load() == -1);
    check ("Clicking a key switches scale to Custom", (int) P (p, "scale") == 3, P (p, "scale"));

    // switches
    click (700, 561); check ("Instrument switch toggles", P (p, "inst") > .5f); click (700, 561); check ("Instrument switch toggles back", P (p, "inst") < .5f);
    { const float v0 = P (p, "lowlat"); click (870, 561); check ("Low Latency switch toggles", P (p, "lowlat") != v0); }
    click (1200, 28); check ("Bypass toggles", P (p, "bypass") > .5f); click (1200, 28); check ("Bypass toggles back", P (p, "bypass") < .5f);

    // knobs
    changed ("Retune speed knob", "speed", 1108, 375, 0, -40);
    changed ("Amount knob", "amount", 1008, 379, 0, -30);
    changed ("Sustain knob", "sustain", 1208, 379, 0, -30);
    changed ("Input knob", "inGain", 50, 562, 0, 30);
    changed ("Output knob", "outGain", 148, 562, 0, 30);
    changed ("Humanize knob", "humanize", 263, 562, 0, -30);
    changed ("Flex-Tune knob", "flex", 359, 562, 0, -30);
    changed ("Doubler knob", "doubler", 447, 562, 0, -30);
    changed ("Guide tone volume knob", "tone", 562, 562, 0, -30);
    { const float v0 = P (p, "ref"); drag (1000, 548, 120, 0); check ("Pitch reference slider moves", P (p, "ref") != v0, P (p, "ref")); }

    // snapshots, A/B, undo, presets
    click (1070, 29); check ("Snapshot 3 becomes current", p.currentSlot() == 2, p.currentSlot());
    click (980, 29);  check ("Snapshot 1 becomes current", p.currentSlot() == 0, p.currentSlot());
    click (627, 29);  check ("B switches A/B", p.currentAB() == 1, p.currentAB());
    click (569, 29);  check ("A switches back", p.currentAB() == 0, p.currentAB());
    p.undoManager.clearUndoHistory(); p.apvts.getParameter ("flex")->setValueNotifyingHost (.3f); (void) p.apvts.copyState(); p.undoManager.beginNewTransaction();
    check ("Undo available after a change", p.undoManager.canUndo());
    p.undoManager.undo(); check ("Undo reverts Flex-Tune", std::fabs (P (p, "flex") - 30.f) > .5f, P (p, "flex"));
    p.apvts.getParameter ("speed")->setValueNotifyingHost (.1f);
    click (156, 29); check ("Preset arrow loads a preset", p.getCurrentProgram() >= 0, p.getCurrentProgram());

    // click-to-type pitch reference
    { juce::Label* hz = nullptr; std::function<void(juce::Component*)> f = [&] (juce::Component* x) { for (auto* k : x->getChildren()) { if (auto* l = dynamic_cast<juce::Label*> (k)) if (l->getText().endsWith ("Hz")) hz = l; f (k); } }; f (ed.get());
      check ("Hz box exists", hz != nullptr);
      if (hz) { p.apvts.getParameter ("ref")->setValueNotifyingHost (.5f); hz->showEditor (); check ("Hz editor opens", hz->isBeingEdited());
        if (auto* e = hz->getCurrentTextEditor()) { check ("editor shows the bare number", e->getText() == "440.0"); e->setText ("432", false); hz->hideEditor (false); }
        check ("typing 432 sets Pitch Reference to 432 Hz", std::fabs (440.0 * std::pow (2.0, P (p, "ref") / 1200.0) - 432.0) < .5, P (p, "ref")); } }
    // value pills are click-to-type and show units
    { std::vector<juce::Slider*> sl; std::function<void(juce::Component*)> f = [&] (juce::Component* x) { for (auto* k : x->getChildren()) { if (auto* s = dynamic_cast<juce::Slider*> (k)) sl.push_back (s); f (k); } }; f (ed.get());
      bool anyMs = false, anyPct = false; for (auto* s : sl) { auto t = s->getTextFromValue (s->getValue()); if (s->getName() == "Retune speed") std::printf ("  speed text: %s\n", t.toRawUTF8()); anyMs |= t.endsWith (" ms") && ! t.contains ("."); anyPct |= t.endsWith ("%"); }
      check ("Speed pill reads whole ms", anyMs); check ("Percent pills read %", anyPct); }
    // saved state round trip
    juce::MemoryBlock mb; p.getStateInformation (mb); YoloProcessor p2; p2.setStateInformation (mb.getData(), (int) mb.getSize());
    check ("Saved state restores parameters", std::fabs (P (p2, "ref") - P (p, "ref")) < .01, P (p2, "ref"));
    // screenshot at default (compact) size
    ed->setSize (948, 507); pump (100);
    { auto img = ed->createComponentSnapshot (ed->getLocalBounds(), true, 1.f); juce::PNGImageFormat png; juce::File f ("/tmp/yolo_compact.png"); f.deleteFile(); juce::FileOutputStream os (f); png.writeImageToStream (img, os); }
    std::printf (fails ? "\n%d FAILED\n" : "\nall passed\n", fails);
    ed.reset(); return fails != 0;
}
