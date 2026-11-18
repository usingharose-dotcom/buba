#include "PluginEditor.h"
using namespace yl;
static const char* kSharp[12] = {"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};
static const char* kFlat[12]  = {"C","Db","D","Eb","E","F","Gb","G","Ab","A","Bb","B"};
static bool isBlack (int n) { return n==1||n==3||n==6||n==8||n==10; }
static juce::Font serif (float h, int style = juce::Font::plain) { return juce::FontOptions (juce::Font::getDefaultSerifFontName(), h, style); }
static const float PI = juce::MathConstants<float>::pi;

// small caps style heading / label text
static void caps (juce::Graphics& g, const juce::String& t, juce::Rectangle<float> r, float size, juce::Colour c, juce::Justification j = juce::Justification::centredLeft, float kern = .14f)
{
    juce::Font f (juce::FontOptions (size, juce::Font::bold)); f.setExtraKerningFactor (kern);
    g.setColour (c); g.setFont (f); g.drawText (t.toUpperCase(), r, j);
}

static void drawYin (juce::Graphics& g, float cx, float cy, float R, float a, float alpha, juce::Colour light, juce::Colour dark)
{
    juce::Graphics::ScopedSaveState ss (g);
    g.addTransform (juce::AffineTransform::rotation (a, cx, cy));
    g.beginTransparencyLayer (alpha);
    g.setColour (light); g.fillEllipse (cx - R, cy - R, 2 * R, 2 * R);
    juce::Path half; half.addPieSegment (cx - R, cy - R, 2 * R, 2 * R, 0.f, PI, 0.f);
    g.setColour (dark); g.fillPath (half);
    g.setColour (light); g.fillEllipse (cx - R / 2, cy - R, R, R);
    g.setColour (dark);  g.fillEllipse (cx - R / 2, cy, R, R);
    g.fillEllipse (cx - R * .12f, cy - R * .62f, R * .24f, R * .24f);
    g.setColour (light); g.fillEllipse (cx - R * .12f, cy + R * .38f, R * .24f, R * .24f);
    g.endTransparencyLayer();
}

YoloLook::YoloLook()
{
    setColour (juce::PopupMenu::backgroundColourId, panel); setColour (juce::PopupMenu::textColourId, pink);
    setColour (juce::PopupMenu::highlightedBackgroundColourId, plum); setColour (juce::PopupMenu::highlightedTextColourId, cream);
}

void YoloLook::drawComboBox (juce::Graphics& g, int w, int h, bool, int, int, int, int, juce::ComboBox&)
{
    auto r = juce::Rectangle<float> (0, 0, (float) w, (float) h).reduced (.5f);
    g.setColour (ink); g.fillRoundedRectangle (r, 4.f); g.setColour (line); g.drawRoundedRectangle (r, 4.f, 1.f);
}

void YoloLook::drawLabel (juce::Graphics& g, juce::Label& l)
{
    if (dynamic_cast<juce::Slider*> (l.getParentComponent()) != nullptr || l.getComponentID() == "pill")
    {
        auto r = l.getLocalBounds().toFloat().reduced (.5f);
        g.setColour (ink); g.fillRoundedRectangle (r, 4.f); g.setColour (line); g.drawRoundedRectangle (r, 4.f, 1.f);
        if (! l.isBeingEdited()) { g.setColour (cream); g.setFont (juce::FontOptions (11.5f, juce::Font::bold)); g.drawText (l.getText(), l.getLocalBounds(), juce::Justification::centred); }
        return;
    }
    juce::LookAndFeel_V4::drawLabel (g, l);
}

juce::Slider::SliderLayout YoloLook::getSliderLayout (juce::Slider& s)
{
    juce::Slider::SliderLayout L;
    auto b = s.getLocalBounds();
    if (s.getSliderStyle() == juce::Slider::RotaryHorizontalVerticalDrag || s.getSliderStyle() == juce::Slider::Rotary)
    {
        L.textBoxBounds = b.removeFromBottom (20).withSizeKeepingCentre (juce::jmin (62, b.getWidth()), 20);
        L.sliderBounds = b;
    }
    else { L.sliderBounds = b; }
    return L;
}

void YoloLook::drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h, float pos, float a0, float a1, juce::Slider& s)
{
    auto b = juce::Rectangle<float> ((float) x, (float) y, (float) w, (float) h);
    auto nameArea = b.removeFromBottom (15.f);
    const float R = juce::jmin (b.getWidth(), b.getHeight()) * .5f - 2.f; const auto c = b.getCentre();
    const float rt = R - 2.f;
    juce::Path track; track.addCentredArc (c.x, c.y, rt, rt, 0.f, a0, a1, true);
    g.setColour (line.brighter (.1f)); g.strokePath (track, juce::PathStrokeType (3.f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    const float ang = a0 + pos * (a1 - a0);
    if (pos > .003f)
    {
        juce::Path val; val.addCentredArc (c.x, c.y, rt, rt, 0.f, a0, ang, true);
        g.setColour (pink2.withAlpha (.22f)); g.strokePath (val, juce::PathStrokeType (6.f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        g.setGradientFill (juce::ColourGradient (pink2, c.x - rt, c.y + rt, pink, c.x + rt, c.y - rt, false));
        g.strokePath (val, juce::PathStrokeType (3.f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }
    const float rb = rt * .74f;
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff3a2b32), c.x, c.y - rb, juce::Colour (0xff130d10), c.x, c.y + rb, false));
    g.fillEllipse (c.x - rb, c.y - rb, 2 * rb, 2 * rb);
    g.setColour (juce::Colour (0xff4a3a42)); g.drawEllipse (c.x - rb, c.y - rb, 2 * rb, 2 * rb, 1.f);
    g.setColour (pink); juce::Path pt; pt.addRoundedRectangle (-1.2f, -rb * .86f, 2.4f, rb * .5f, 1.2f);
    g.fillPath (pt, juce::AffineTransform::rotation (ang).translated (c));
    caps (g, s.getName(), nameArea, 8.5f, mute, juce::Justification::centred, .16f);
}

void YoloLook::drawLinearSlider (juce::Graphics& g, int x, int y, int w, int h, float pos, float, float, juce::Slider::SliderStyle, juce::Slider&)
{
    const float ty = y + h * .5f;
    g.setColour (line); g.fillRoundedRectangle ((float) x + 3, ty - 1.5f, (float) w - 6, 3.f, 1.5f);
    g.setGradientFill (juce::ColourGradient (pink2, (float) x, ty, pink, (float) x + w, ty, false));
    g.fillRoundedRectangle ((float) x + 3, ty - 1.5f, juce::jmax (0.f, pos - x - 3), 3.f, 1.5f);
    g.setColour (cream); g.fillEllipse (pos - 6.f, ty - 6.f, 12, 12);
    g.setColour (pink2); g.drawEllipse (pos - 6.f, ty - 6.f, 12, 12, 1.4f);
}

void YoloLook::drawToggleButton (juce::Graphics& g, juce::ToggleButton& b, bool, bool)
{
    auto r = b.getLocalBounds().toFloat(); const bool on = b.getToggleState();
    if (b.getComponentID() == "hdr")
    {
        caps (g, b.getButtonText(), r, 8.5f, on ? cream : mute, juce::Justification::centredRight, .14f);
        g.setColour (on ? pink2 : line); g.fillRect (r.getRight() - 14.f, r.getBottom() - 4.f, 14.f, 1.5f);
        return;
    }
    if (b.getComponentID() == "byp")
    {
        auto p = r.reduced (.5f);
        g.setColour (on ? pink : ink); g.fillRoundedRectangle (p, p.getHeight() / 2); g.setColour (on ? pink2 : line); g.drawRoundedRectangle (p, p.getHeight() / 2, 1.2f);
        const juce::Point<float> c (p.getX() + 19.f, p.getCentreY());
        g.setColour (on ? ink : pink2); juce::Path a; a.addCentredArc (c.x, c.y + .5f, 5, 5, 0, .6f, 2 * PI - .6f, true); a.startNewSubPath (c.x, c.y - 6.f); a.lineTo (c.x, c.y);
        g.strokePath (a, juce::PathStrokeType (1.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        caps (g, "Bypass", p.withTrimmedLeft (34.f), 9.5f, on ? ink : cream, juce::Justification::centredLeft, .18f);
        return;
    }
    auto sw = juce::Rectangle<float> (r.getX() + 1, r.getCentreY() - 9, 34, 18);
    g.setColour (on ? pink : ink); g.fillRoundedRectangle (sw, 9.f); g.setColour (on ? pink2 : line); g.drawRoundedRectangle (sw, 9.f, 1.2f);
    g.setColour (on ? ink : mute); g.fillEllipse (on ? sw.getRight() - 15.f : sw.getX() + 3.f, sw.getY() + 3.f, 12, 12);
    const juce::String sub = b.getProperties()["sub"].toString();
    auto tr = r.withTrimmedLeft (44).toNearestInt();
    g.setColour (cream); g.setFont (juce::FontOptions (12.f, juce::Font::bold));
    auto top = tr.removeFromTop (sub.isEmpty() ? tr.getHeight() : tr.getHeight() / 2 + 1);
    g.drawText (b.getButtonText(), top, juce::Justification::centredLeft);
    if (sub.isNotEmpty()) { g.setColour (mute); g.setFont (juce::FontOptions (10.f, juce::Font::bold)); g.drawText (sub, tr, juce::Justification::topLeft); }
}

// ---------- small widgets ----------
juce::Rectangle<float> Seg::cell (int i) const
{
    const int n = labels.size(), rows = (n + cols - 1) / cols;
    const float cw = (float) getWidth() / cols, ch = (float) getHeight() / rows;
    return { (i % cols) * cw, (i / cols) * ch, cw, ch };
}
void Seg::paint (juce::Graphics& g)
{
    const int sel = get ? get() : -1;
    if (! chips) { auto all = getLocalBounds().toFloat().reduced (.5f); g.setColour (ink); g.fillRoundedRectangle (all, radius); g.setColour (line); g.drawRoundedRectangle (all, radius, 1.f); }
    for (int i = 0; i < labels.size(); ++i)
    {
        auto r = cell (i).reduced (chips ? 2.f : 2.f);
        if (i == sel) { g.setColour (pink); g.fillRoundedRectangle (r, radius); }
        else if (chips) { g.setColour (juce::Colour (0xff171014)); g.fillRoundedRectangle (r, radius); g.setColour (line.withAlpha (.7f)); g.drawRoundedRectangle (r, radius, 1.f); }
        g.setColour (i == sel ? ink : (chips ? cream.withAlpha (.55f) : cream.withAlpha (.85f))); g.setFont (juce::FontOptions (fs, juce::Font::bold));
        g.drawText (labels[i], r.toNearestInt(), juce::Justification::centred);
    }
}
void Seg::mouseDown (const juce::MouseEvent& e)
{
    for (int i = 0; i < labels.size(); ++i) if (cell (i).contains (e.position)) { if (set) set (i); repaint(); return; }
}

void IconBtn::paint (juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat().reduced (.5f);
    if (kind == Prev || kind == Next) { g.setColour (ink); g.fillRoundedRectangle (r, 4.f); g.setColour (line); g.drawRoundedRectangle (r, 4.f, 1.f); }
    g.setColour (enabledLook ? cream.withAlpha (.85f) : line.brighter (.2f));
    juce::Path p; const auto c = r.getCentre();
    if (kind == Prev) { p.addTriangle (c.x + 3.f, c.y - 4.5f, c.x + 3.f, c.y + 4.5f, c.x - 3.5f, c.y); g.fillPath (p); return; }
    if (kind == Next) { p.addTriangle (c.x - 3.f, c.y - 4.5f, c.x - 3.f, c.y + 4.5f, c.x + 3.5f, c.y); g.fillPath (p); return; }
    const bool u = kind == Undo; const float d = u ? 1.f : -1.f;
    p.addCentredArc (c.x, c.y + 1, 6.f, 5.5f, 0, u ? -1.2f : 1.2f, u ? 2.2f : -2.2f, true);
    g.strokePath (p, juce::PathStrokeType (1.6f)); juce::Path ah;
    ah.addTriangle (c.x - d * 7.f, c.y - 3.f, c.x - d * 2.f, c.y - 6.f, c.x - d * 2.f, c.y - .5f);
    g.fillPath (ah);
}

// ---------- pitch roll ----------
static void currentMask (YoloProcessor& p, bool out[12])
{
    bool cu[12]; for (int i = 0; i < 12; ++i) cu[i] = p.apvts.getRawParameterValue ("n" + juce::String (i))->load() > .5f;
    YoloProcessor::scaleMask ((int) p.apvts.getRawParameterValue ("key")->load(), (int) p.apvts.getRawParameterValue ("scale")->load(), cu, out);
}

void PitchRoll::timerCallback()
{
    for (int i = 0; i < N - 1; ++i) { inH[i] = inH[i + 1]; outH[i] = outH[i + 1]; }
    if (proc.engine.voiced.load()) { inH[N - 1] = proc.engine.detM.load(); outH[N - 1] = proc.engine.outM.load(); if (std::fabs (outH[N - 1] - (float) centreI) > 5.f) centreI = (int) std::lround (outH[N - 1]); }
    else { inH[N - 1] = -1.f; outH[N - 1] = -1.f; }
    repaint();
}
void PitchRoll::paint (juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat(); const float labW = 38.f; auto plot = r.withTrimmedLeft (labW);
    const int rows = 15; const float rh = r.getHeight() / rows; const int top = centreI + 7;
    bool al[12]; currentMask (proc, al);
    g.setColour (ink); g.fillRect (r);
    for (int i = 0; i < rows; ++i)
    {
        const int midi = top - i, n = ((midi % 12) + 12) % 12; const float y = i * rh;
        g.setColour (isBlack (n) ? juce::Colour (0xff140d10) : juce::Colour (0xff191216)); g.fillRect (plot.getX(), y, plot.getWidth(), rh);
        if (! al[n]) { g.setColour (juce::Colours::black.withAlpha (.28f)); g.fillRect (plot.getX(), y, plot.getWidth(), rh); }
        g.setColour (line.withAlpha (.45f)); g.fillRect (plot.getX(), y + rh - 1.f, plot.getWidth(), 1.f);
        g.setColour (isBlack (n) ? juce::Colour (0xff2a1f25) : juce::Colour (0xff342830)); g.fillRect (0.f, y, labW - 3.f, rh - 1.f);
        g.setColour (n == 0 ? cream : (al[n] ? cream.withAlpha (.8f) : mute.withAlpha (.55f))); g.setFont (juce::FontOptions (9.5f, juce::Font::bold));
        g.drawText ((flats ? kFlat[n] : kSharp[n]) + (n == 0 ? juce::String ((midi / 12) - 1) : juce::String()), juce::Rectangle<float> (0.f, y, labW - 6.f, rh).toNearestInt(), juce::Justification::centredLeft);
    }
    g.setColour (line); g.fillRect (labW - 3.f, 0.f, 3.f, r.getHeight());
    auto yOf = [&] (float m) { return (top + .5f - m) * rh; };
    auto stroke = [&] (const float* h, juce::Colour c, float wdt, bool glow)
    {
        juce::Path p; bool pen = false;
        for (int i = 0; i < N; ++i)
        {
            if (h[i] < 0.f) { pen = false; continue; }
            const float x = plot.getX() + plot.getWidth() * i / (float) (N - 1), y = juce::jlimit (0.f, r.getHeight(), yOf (h[i]));
            if (! pen) { p.startNewSubPath (x, y); pen = true; } else p.lineTo (x, y);
        }
        if (glow) { g.setColour (c.withAlpha (.22f)); g.strokePath (p, juce::PathStrokeType (wdt + 4.f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded)); }
        g.setColour (c); g.strokePath (p, juce::PathStrokeType (wdt, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    };
    if (showLine) stroke (inH, juce::Colour (0xffb9a9b0), 1.3f, false);
    stroke (outH, pink2, 2.2f, true);
    // legend
    g.setColour (mute); g.setFont (juce::FontOptions (9.5f, juce::Font::bold));
    const float ly = r.getBottom() - 15.f;
    g.setColour (juce::Colour (0xffb9a9b0)); g.fillRect (plot.getX() + 8.f, ly + 6.f, 12.f, 1.5f); g.setColour (mute); g.drawText ("Input", (int) plot.getX() + 24, (int) ly, 40, 13, juce::Justification::centredLeft);
    g.setColour (pink2); g.fillRect (plot.getX() + 70.f, ly + 6.f, 12.f, 1.5f); g.setColour (mute); g.drawText ("Corrected", (int) plot.getX() + 86, (int) ly, 60, 13, juce::Justification::centredLeft);
}

// ---------- piano ----------
static juce::Rectangle<float> keyRect (juce::Rectangle<float> b, int n)
{
    static const int wi[12] = {0,-1,1,-1,2,3,-1,4,-1,5,-1,6};
    const float ww = b.getWidth() / 7.f;
    if (! isBlack (n)) return { b.getX() + wi[n] * ww, b.getY(), ww, b.getHeight() };
    return { b.getX() + (wi[n - 1] + 1) * ww - ww * .30f, b.getY(), ww * .60f, b.getHeight() * .62f };
}
void Piano::timerCallback()
{
    for (int i = 0; i < 12; ++i) keyHist[i] *= .9995f;
    if (proc.engine.voiced.load()) keyHist[((int) std::lround (proc.engine.detM.load()) % 12 + 12) % 12] += 1.f;
    repaint();
}
int Piano::detectKey() const
{
    static const int maj[7] = {0,2,4,5,7,9,11}; float tot = 0; for (float v : keyHist) tot += v;
    if (tot < 8.f) return -1;
    int best = 0; float bs = -1;
    for (int k = 0; k < 12; ++k) { float s = 0; for (int d : maj) s += keyHist[(k + d) % 12]; if (s > bs) { bs = s; best = k; } }
    return best;
}
int Piano::keyAt (juce::Point<float> pt) const
{
    auto kb = getLocalBounds().toFloat();
    for (int n = 0; n < 12; ++n) if (isBlack (n) && keyRect (kb, n).contains (pt)) return n;
    for (int n = 0; n < 12; ++n) if (! isBlack (n) && keyRect (kb, n).contains (pt)) return n;
    return -1;
}
void Piano::mouseDown (const juce::MouseEvent& e)
{
    const int n = keyAt (e.position); if (n < 0) return;
    proc.toneNote.store (60 + n);
    bool al[12]; currentMask (proc, al);
    for (int i = 0; i < 12; ++i)
        if (auto* p = proc.apvts.getParameter ("n" + juce::String (i))) p->setValueNotifyingHost ((i == n ? ! al[i] : al[i]) ? 1.f : 0.f);
    if (auto* s = proc.apvts.getParameter ("scale")) s->setValueNotifyingHost (s->convertTo0to1 (3.f));
}
void Piano::paint (juce::Graphics& g)
{
    bool al[12]; currentMask (proc, al);
    const bool vc = proc.engine.voiced.load();
    const int act = ((int) std::lround (proc.engine.outM.load()) % 12 + 12) % 12;
    auto kb = getLocalBounds().toFloat();
    for (int pass = 0; pass < 2; ++pass)
        for (int n = 0; n < 12; ++n)
        {
            if (isBlack (n) != (pass == 1)) continue;
            auto r = keyRect (kb, n).reduced (1.f, 0);
            const bool hot = showDetected && vc && n == act, blk = isBlack (n);
            juce::Colour top = blk ? juce::Colour (0xffa4507a) : cream, bot = blk ? juce::Colour (0xff6b2a4a) : juce::Colour (0xffffc3da);
            if (! al[n]) { top = blk ? juce::Colour (0xff2a1c22) : juce::Colour (0xff4a3a41); bot = blk ? juce::Colour (0xff1a1216) : juce::Colour (0xff33262c); }
            if (hot) { top = pink; bot = pink2; }
            auto rr = r.withTrimmedTop (-6);
            g.setGradientFill (juce::ColourGradient (top, 0, r.getY(), bot, 0, r.getBottom(), false));
            g.fillRoundedRectangle (rr, 5.f);
            g.setColour (ink.withAlpha (.5f)); g.drawRoundedRectangle (rr, 5.f, .8f);
            g.setColour (blk ? cream.withAlpha (al[n] ? .9f : .4f) : (al[n] ? juce::Colour (0xff6a4655) : mute.withAlpha (.6f)));
            g.setFont (juce::FontOptions (9.5f, juce::Font::bold));
            g.drawText (flats ? kFlat[n] : kSharp[n], r.withTrimmedTop (r.getHeight() - 22.f).toNearestInt(), juce::Justification::centred);
        }
}

void Slots::paint (juce::Graphics& g)
{
    auto all = getLocalBounds().toFloat().reduced (.5f);
    g.setColour (ink); g.fillRoundedRectangle (all, 4.f); g.setColour (line); g.drawRoundedRectangle (all, 4.f, 1.f);
    const float w = getWidth() / 4.f;
    for (int i = 0; i < 4; ++i)
    {
        auto r = juce::Rectangle<float> (i * w, 0, w, (float) getHeight()).reduced (2.f);
        const bool sel = proc.currentSlot() == i;
        if (sel) { g.setColour (pink); g.fillRoundedRectangle (r, 3.f); }
        g.setColour (sel ? ink : (proc.slotUsed (i) ? pink2 : line.brighter (.3f))); g.fillEllipse (r.getX() + 8.f, r.getCentreY() - 2.f, 4.f, 4.f);
        g.setColour (sel ? ink : cream.withAlpha (.8f)); g.setFont (juce::FontOptions (11.5f, juce::Font::bold)); g.drawText (juce::String (i + 1), r.withTrimmedLeft (10.f).toNearestInt(), juce::Justification::centred);
    }
}

// ---------- credits (click the logo) ----------
struct LogoHit : juce::Component
{
    std::function<void()> onClick;
    LogoHit() { setMouseCursor (juce::MouseCursor::PointingHandCursor); setInterceptsMouseClicks (true, false); }
    void paint (juce::Graphics&) override {}           // fully transparent
    void mouseUp (const juce::MouseEvent& e) override { if (e.mouseWasClicked() || getLocalBounds().contains (e.getPosition())) { if (onClick) onClick(); } }
};
struct CreditsPanel : juce::Component
{
    CreditsPanel() { setVisible (false); }
    void paint (juce::Graphics& g) override
    {
        auto r = getLocalBounds().toFloat().reduced (1.f);
        g.setColour (juce::Colour (0xf0180f13)); g.fillRoundedRectangle (r, 10.f);
        g.setColour (pink2.withAlpha (.6f)); g.drawRoundedRectangle (r, 10.f, 1.2f);
        caps (g, "Made by", juce::Rectangle<float> (18, 12, 200, 14), 10.f, cream);
        const char* names[] = { "Kinok9", "Ohjokr", "Razee", "17daess" };
        for (int i = 0; i < 4; ++i)
        {
            const float y = 34.f + i * 26.f;
            g.setColour (pink); g.setFont (juce::FontOptions (16.f, juce::Font::bold)); g.drawText (names[i], juce::Rectangle<float> (18, y, 90, 22), juce::Justification::centredLeft);
            if (i == 3) { g.setColour (mute); g.setFont (juce::FontOptions (11.f, juce::Font::italic)); g.drawText ("even tho he ain't do shi", juce::Rectangle<float> (100, y, 200, 22), juce::Justification::centredLeft); }
        }
    }
    void mouseDown (const juce::MouseEvent&) override { setVisible (false); }
};

// ---------- yin yang: correction dial + Doubler drag ----------
void Scene::setDoubler (float v01)
{
    if (auto* p = proc.apvts.getParameter ("doubler")) p->setValueNotifyingHost (juce::jlimit (0.f, 1.f, v01));
}
void Scene::mouseDown (const juce::MouseEvent&)
{
    if (auto* p = proc.apvts.getParameter ("doubler")) { dragStart = p->getValue(); p->beginChangeGesture(); dragging = true; }
}
void Scene::mouseDrag (const juce::MouseEvent& e)
{
    if (! dragging) return;
    setDoubler (dragStart + (float) (-e.getDistanceFromDragStartY() + e.getDistanceFromDragStartX()) / 220.f);   // up or right = more
}
void Scene::mouseUp (const juce::MouseEvent&)
{
    if (dragging) { if (auto* p = proc.apvts.getParameter ("doubler")) p->endChangeGesture(); dragging = false; }
}
void Scene::mouseDoubleClick (const juce::MouseEvent&)
{
    if (auto* p = proc.apvts.getParameter ("doubler")) { p->beginChangeGesture(); p->setValueNotifyingHost (0.f); p->endChangeGesture(); }
}
void Scene::mouseWheelMove (const juce::MouseEvent&, const juce::MouseWheelDetails& w)
{
    if (auto* p = proc.apvts.getParameter ("doubler")) { p->beginChangeGesture(); p->setValueNotifyingHost (juce::jlimit (0.f, 1.f, p->getValue() + w.deltaY * .25f)); p->endChangeGesture(); }
}
Scene::Scene (YoloProcessor& p) : proc (p)
{
    setMouseCursor (juce::MouseCursor::UpDownResizeCursor);
    startTimerHz (30);
}
void Scene::timerCallback()
{
    level += (juce::jlimit (0.f, 1.f, proc.engine.peakIn.load() * 3.f) - level) * .25f;
    angle += .006f + level * .07f;
    const float target = proc.engine.voiced.load() ? juce::jlimit (-100.f, 100.f, (proc.engine.outM.load() - proc.engine.detM.load()) * 100.f) : 0.f;
    corr += (target - corr) * .25f;
    repaint();
}
void Scene::paint (juce::Graphics& g)
{
    auto b = getLocalBounds().toFloat(); const juce::Point<float> c (b.getCentreX(), b.getCentreY() + 4.f);
    const float Ro = juce::jmin (b.getWidth() * .5f, b.getHeight() * .5f) - 6.f;
    const float a0 = PI * 1.17f, a1 = PI * 2.83f;                       // dial sweep (-100 .. +100 cents)
    const float dbl = proc.apvts.getParameter ("doubler")->getValue();
    // glow behind the yin yang follows the input level
    g.setGradientFill (juce::ColourGradient (pink2.withAlpha (.10f + level * .22f + dbl * .08f), c.x, c.y, pink2.withAlpha (0.f), c.x + Ro, c.y, true));
    g.fillEllipse (c.x - Ro, c.y - Ro, 2 * Ro, 2 * Ro);
    // tick ring
    juce::Path track; track.addCentredArc (c.x, c.y, Ro - 3.f, Ro - 3.f, 0.f, a0, a1, true);
    g.setColour (line.brighter (.1f)); g.strokePath (track, juce::PathStrokeType (5.f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    const float cn = (corr + 100.f) / 200.f, am = a0 + cn * (a1 - a0), a50 = a0 + .5f * (a1 - a0);
    if (std::fabs (corr) > .5f)
    {
        juce::Path v; v.addCentredArc (c.x, c.y, Ro - 3.f, Ro - 3.f, 0.f, juce::jmin (a50, am), juce::jmax (a50, am), true);
        g.setColour (pink2.withAlpha (.25f)); g.strokePath (v, juce::PathStrokeType (10.f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        g.setColour (pink); g.strokePath (v, juce::PathStrokeType (5.f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }
    for (int i = 0; i <= 40; ++i)
    {
        const float t = (float) i / 40.f, a = a0 + t * (a1 - a0); const bool major = (i % 10) == 0 || i == 20;
        const float r0 = Ro - 13.f - (major ? 3.f : 0.f), r1 = Ro - 9.f;
        g.setColour (major ? cream.withAlpha (.7f) : mute.withAlpha (.45f));
        g.drawLine (c.x + std::sin (a) * r0, c.y - std::cos (a) * r0, c.x + std::sin (a) * r1, c.y - std::cos (a) * r1, major ? 1.4f : 1.f);
    }
    g.setColour (cream); g.fillEllipse (c.x + std::sin (am) * (Ro - 3.f) - 4.f, c.y - std::cos (am) * (Ro - 3.f) - 4.f, 8.f, 8.f);
    g.setColour (mute); g.setFont (juce::FontOptions (8.f, juce::Font::bold));
    g.drawText ("-100", (int) (c.x + std::sin (a0) * (Ro - 26.f)) - 16, (int) (c.y - std::cos (a0) * (Ro - 26.f)) - 6, 32, 12, juce::Justification::centred);
    g.drawText ("100", (int) (c.x + std::sin (a1) * (Ro - 26.f)) - 16, (int) (c.y - std::cos (a1) * (Ro - 26.f)) - 6, 32, 12, juce::Justification::centred);
    // yin yang
    const float R = Ro - 38.f;
    g.setColour (ink); g.fillEllipse (c.x - R - 5.f, c.y - R - 5.f, 2 * R + 10.f, 2 * R + 10.f);
    g.setColour (pink.withAlpha (.9f)); g.drawEllipse (c.x - R - 5.f, c.y - R - 5.f, 2 * R + 10.f, 2 * R + 10.f, 2.5f);
    if (dbl > 0.01f) { const float off = dbl * 16.f; drawYin (g, c.x - off, c.y, R, angle * .9f - .15f * dbl, .30f * dbl + .08f, cream, ink); drawYin (g, c.x + off, c.y, R, angle * 1.1f + .15f * dbl, .30f * dbl + .08f, cream, ink); }
    drawYin (g, c.x, c.y, R, angle, 1.f, cream, ink);
    if (hover || dragging || dbl > 0.001f)
    {
        auto lr = juce::Rectangle<float> (c.x - 48.f, b.getBottom() - 16.f, 96.f, 16.f);
        g.setColour (ink.withAlpha (.8f)); g.fillRoundedRectangle (lr, 8.f);
        g.setColour (dragging || hover ? cream : pink); g.setFont (juce::FontOptions (10.f, juce::Font::bold));
        g.drawText ("Doubler  " + juce::String ((int) std::lround (dbl * 100.f)) + "%", lr.toNearestInt(), juce::Justification::centred);
    }
}

// ---------- editor ----------
static constexpr int kW = 1264, kH = 676;

struct YoloEditor::Content : public juce::Component, private juce::Timer
{
    using SA = juce::AudioProcessorValueTreeState::SliderAttachment;
    using BA = juce::AudioProcessorValueTreeState::ButtonAttachment;

    explicit Content (YoloProcessor& p)
        : proc (p), scene (p), roll (p, flats), piano (p, flats), slots (p)
    {
        auto& a = proc.apvts;
        for (auto* c : std::initializer_list<juce::Component*> { &scene, &roll, &piano, &slots }) addAndMakeVisible (c);
        auto knob = [this] (juce::Slider& s, const juce::String& name)
        {
            s.setName (name); s.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
            s.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 62, 20);
            s.setRotaryParameters (PI * 1.2f, PI * 2.8f, true);
            s.setMouseCursor (juce::MouseCursor::UpDownResizeCursor);
            addAndMakeVisible (s);
        };
        for (auto* s : { &speed, &sustain, &amount, &inG, &outG, &flex, &hum, &dbl, &tone }) knob (*s, "");
        speed.setName ("Retune speed"); sustain.setName ("Sustain"); amount.setName ("Amount"); inG.setName ("Input"); outG.setName ("Output");
        flex.setName ("Flex-Tune"); hum.setName ("Humanize"); dbl.setName ("Doubler"); tone.setName ("Volume");
        ref.setSliderStyle (juce::Slider::LinearHorizontal); ref.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0); addAndMakeVisible (ref);
        aSp = std::make_unique<SA> (a, "speed", speed); aSu = std::make_unique<SA> (a, "sustain", sustain); aAm = std::make_unique<SA> (a, "amount", amount);
        aIn = std::make_unique<SA> (a, "inGain", inG); aOut = std::make_unique<SA> (a, "outGain", outG); aFl = std::make_unique<SA> (a, "flex", flex);
        aHu = std::make_unique<SA> (a, "humanize", hum); aDb = std::make_unique<SA> (a, "doubler", dbl); aTo = std::make_unique<SA> (a, "tone", tone); aRef = std::make_unique<SA> (a, "ref", ref);
        // value pills: whole numbers with units (click a pill to type a value)
        for (auto* s : { &speed, &sustain, &amount, &inG, &outG, &flex, &hum, &dbl, &tone }) s->setNumDecimalPlacesToDisplay (0);
        speed.setTextValueSuffix (" ms");
        for (auto* s : { &sustain, &amount, &inG, &outG, &flex, &hum, &dbl, &tone }) s->setTextValueSuffix ("%");

        auto par = [this] (const char* id) { return proc.apvts.getParameter (id); };
        auto idx = [par] (const char* id) { auto* p = par (id); return (int) std::lround (p->convertFrom0to1 (p->getValue())); };
        auto setIdx = [par] (const char* id, int i) { auto* p = par (id); p->setValueNotifyingHost (p->convertTo0to1 ((float) i)); };
        modeSeg = std::make_unique<Seg> (juce::StringArray {"Custom", "Major", "Minor"}, 3,
            [idx] { const int s = idx ("scale"); return s == 3 ? 0 : (s == 1 ? 1 : (s == 2 ? 2 : -1)); },
            [setIdx] (int i) { setIdx ("scale", i == 0 ? 3 : i); });
        modeSeg->fs = 11.5f;
        accSeg = std::make_unique<Seg> (juce::StringArray {juce::String (juce::CharPointer_UTF8 ("\xe2\x99\xaf")), juce::String (juce::CharPointer_UTF8 ("\xe2\x99\xad"))}, 2,
            [this] { return flats ? 1 : 0; }, [this] (int i) { flats = i == 1; repaintKeys(); });
        accSeg->fs = 12.f;
        stabSeg = std::make_unique<Seg> (juce::StringArray {"None", "Short", "Mid", "Long"}, 4, [idx] { return idx ("stab"); }, [setIdx] (int i) { setIdx ("stab", i); });
        stabSeg->fs = 11.f;
        keySeg = std::make_unique<Seg> (juce::StringArray (kSharp, 12), 4, [idx] { return idx ("key"); }, [setIdx] (int i) { setIdx ("key", i); });
        keySeg->chips = true; keySeg->fs = 11.f; keySeg->radius = 4.f;
        for (auto* s : { modeSeg.get(), accSeg.get(), stabSeg.get(), keySeg.get() }) addAndMakeVisible (*s);

        detectLink = std::make_unique<Link> ("Detect Key", [this, setIdx]
        {
            const int k = piano.detectKey(); if (k < 0) return;
            setIdx ("key", k); setIdx ("scale", 1);
        });
        chromLink = std::make_unique<Link> ("Chromatic", [setIdx] { setIdx ("scale", 0); });
        detectLink->box = chromLink->box = true;
        addAndMakeVisible (*detectLink); addAndMakeVisible (*chromLink);

        for (auto* t : { &inst, &lowlat, &lineT, &detT, &bypass }) addAndMakeVisible (t);
        inst.setButtonText ("Instrument"); lowlat.setButtonText ("Low Latency");
        lineT.setButtonText ("Input line"); detT.setButtonText ("Detected note"); bypass.setButtonText ("Bypass");
        lineT.setComponentID ("hdr"); detT.setComponentID ("hdr"); bypass.setComponentID ("byp");
        lineT.setToggleState (true, juce::dontSendNotification); detT.setToggleState (true, juce::dontSendNotification);
        lineT.onClick = [this] { roll.showLine = lineT.getToggleState(); }; detT.onClick = [this] { piano.showDetected = detT.getToggleState(); };
        aInst = std::make_unique<BA> (a, "inst", inst); aLow = std::make_unique<BA> (a, "lowlat", lowlat); aByp = std::make_unique<BA> (a, "bypass", bypass);

        for (int i = 0; i < proc.getNumPrograms(); ++i) preset.addItem (proc.getProgramName (i), i + 1);
        preset.setTextWhenNothingSelected ("No preset selected"); preset.setJustificationType (juce::Justification::centredLeft);
        preset.setColour (juce::ComboBox::textColourId, cream);
        preset.onChange = [this] { proc.setCurrentProgram (preset.getSelectedId() - 1); };
        addAndMakeVisible (preset);
        prevBtn = std::make_unique<IconBtn> (IconBtn::Prev, [this] { stepPreset (-1); });
        nextBtn = std::make_unique<IconBtn> (IconBtn::Next, [this] { stepPreset (1); });
        undoBtn = std::make_unique<IconBtn> (IconBtn::Undo, [this] { proc.undoManager.undo(); });
        redoBtn = std::make_unique<IconBtn> (IconBtn::Redo, [this] { proc.undoManager.redo(); });
        for (auto* c : { (juce::Component*) prevBtn.get(), (juce::Component*) nextBtn.get(), (juce::Component*) undoBtn.get(), (juce::Component*) redoBtn.get() }) addAndMakeVisible (c);
        abSeg = std::make_unique<Seg> (juce::StringArray {"A", juce::String (juce::CharPointer_UTF8 ("\xe2\x86\x92")), "B"}, 3, [this] { return proc.currentAB() == 0 ? 0 : 2; },
            [this] (int i) { if (i == 1) proc.copyAB(); else if ((i == 0 ? 0 : 1) != proc.currentAB()) proc.switchAB(); });
        abSeg->fs = 11.f; addAndMakeVisible (*abSeg);
        modeLink = std::make_unique<Link> ("Voice", [this] { if (auto* q = proc.apvts.getParameter ("inst")) q->setValueNotifyingHost (q->getValue() > .5f ? 0.f : 1.f); });
        modeLink->just = juce::Justification::centredRight; modeLink->col = mute; addAndMakeVisible (*modeLink);

        // pitch reference: cents + Hz, both click-to-type
        auto styleLabel = [this] (juce::Label& l)
        {
            l.setEditable (true, true, false); l.setJustificationType (juce::Justification::centred); l.setComponentID ("pill");
            l.setFont (juce::FontOptions (11.5f, juce::Font::bold)); l.setColour (juce::Label::textColourId, cream); l.setMouseCursor (juce::MouseCursor::IBeamCursor);
            l.setColour (juce::Label::backgroundWhenEditingColourId, ink); l.setColour (juce::Label::textWhenEditingColourId, cream); l.setColour (juce::Label::outlineWhenEditingColourId, pink2);
            addAndMakeVisible (l);
        };
        styleLabel (hzLabel); styleLabel (ctLabel);
        hzLabel.onEditorShow = [this] { if (auto* e = hzLabel.getCurrentTextEditor()) { e->setText (juce::String (440.0 * std::pow (2.0, ref.getValue() / 1200.0), 1), false); e->selectAll(); } };
        ctLabel.onEditorShow = [this] { if (auto* e = ctLabel.getCurrentTextEditor()) { e->setText (juce::String ((int) std::lround (ref.getValue())), false); e->selectAll(); } };
        hzLabel.onTextChange = [this]
        {
            const double hz = hzLabel.getText().retainCharacters ("0123456789.").getDoubleValue();
            if (hz >= 300.0 && hz <= 600.0) ref.setValue (juce::jlimit (-100.0, 100.0, 1200.0 * std::log2 (hz / 440.0)), juce::sendNotificationSync);
            refreshRef (true);
        };
        ctLabel.onTextChange = [this]
        {
            ref.setValue (juce::jlimit (-100.0, 100.0, ctLabel.getText().retainCharacters ("0123456789.-+").getDoubleValue()), juce::sendNotificationSync);
            refreshRef (true);
        };
        addAndMakeVisible (logoHit); addChildComponent (credits);
        logoHit.onClick = [this] { credits.setVisible (! credits.isVisible()); credits.toFront (false); };
        refreshRef (true);
        startTimerHz (20);
    }

    void stepPreset (int d)
    {
        const int n = proc.getNumPrograms(); const int cur = preset.getSelectedId() > 0 ? preset.getSelectedId() - 1 : (d > 0 ? -1 : n);
        const int nx = juce::jlimit (0, n - 1, cur + d); preset.setSelectedId (nx + 1, juce::sendNotificationSync);
    }
    void repaintKeys() { for (int i = 0; i < 12; ++i) keySeg->labels.set (i, flats ? kFlat[i] : kSharp[i]); keySeg->repaint(); piano.repaint(); }
    void refreshRef (bool force = false)
    {
        const double hz = 440.0 * std::pow (2.0, ref.getValue() / 1200.0); const int c = (int) std::lround (ref.getValue());
        if (force || ! ctLabel.isBeingEdited()) ctLabel.setText (juce::String (c) + " cts", juce::dontSendNotification);
        if (force || ! hzLabel.isBeingEdited()) hzLabel.setText (juce::String (hz, 1) + " Hz", juce::dontSendNotification);
    }
    void timerCallback() override
    {
        refreshRef();
        if ((++flushTick & 3) == 0) (void) proc.apvts.copyState();   // makes parameter edits reach the undo history promptly
        if (proc.undoManager.getNumActionsInCurrentTransaction() > 0 && ! juce::Desktop::getInstance().getMainMouseSource().isDragging()) proc.undoManager.beginNewTransaction();
        undoBtn->enabledLook = proc.undoManager.canUndo(); redoBtn->enabledLook = proc.undoManager.canRedo();
        const double ms = proc.getSampleRate() > 0 ? 1000.0 * proc.getLatencySamples() / proc.getSampleRate() : 0.0;
        lowlat.getProperties().set ("sub", juce::String (ms, 1) + " ms");
        inst.getProperties().set ("sub", inst.getToggleState() ? "instrument" : "voice");
        modeLink->text = inst.getToggleState() ? "Instrument" : "Voice";
        mIn = juce::jmax (proc.engine.peakIn.load(), mIn * .82f); mOut = juce::jmax (proc.engine.peakOut.load(), mOut * .82f);
        repaint();
    }

    void drawPanel (juce::Graphics& g, juce::Rectangle<float> r, const juce::String& title)
    {
        g.setColour (panel); g.fillRoundedRectangle (r, 8.f);
        g.setColour (panelHi); g.fillRoundedRectangle (r.withHeight (28.f), 8.f); g.fillRect (r.getX(), r.getY() + 14.f, r.getWidth(), 14.f);
        g.setColour (juce::Colour (0xff0d080a)); g.fillRect (r.getX(), r.getY() + 28.f, r.getWidth(), 1.f);
        g.setColour (line); g.drawRoundedRectangle (r.reduced (.5f), 8.f, 1.f);
        caps (g, title, r.withHeight (28.f).withTrimmedLeft (12.f), 9.f, mute);
    }
    void meter (juce::Graphics& g, juce::Rectangle<float> r, float peak)
    {
        g.setColour (ink); g.fillRoundedRectangle (r, 2.f); g.setColour (line); g.drawRoundedRectangle (r, 2.f, 1.f);
        const float db = peak > 1e-5f ? 20.f * std::log10 (peak) : -90.f, v = juce::jlimit (0.f, 1.f, (db + 54.f) / 54.f);
        const int segs = 18; const float sh = (r.getHeight() - 4.f) / segs;
        for (int i = 0; i < segs; ++i)
        {
            const bool on = (float) (i + 1) / segs <= v;
            g.setColour (on ? (i > 14 ? pink2 : pink) : line.withAlpha (.5f));
            g.fillRect (r.getX() + 2.f, r.getBottom() - 2.f - (i + 1) * sh + 1.f, r.getWidth() - 4.f, sh - 1.f);
        }
    }

    void paint (juce::Graphics& g) override
    {
        g.setGradientFill (juce::ColourGradient (juce::Colour (0xff1d1519), 0, 0, juce::Colour (0xff120c0f), 0, (float) kH, false)); g.fillAll();
        g.setColour (line); g.drawRect (getLocalBounds().toFloat(), 1.f);
        // header
        g.setColour (juce::Colour (0xff21181d)); g.fillRect (0, 0, kW, 56); g.setColour (ink); g.fillRect (0, 56, kW, 1);
        g.setGradientFill (juce::ColourGradient (pink, 20, 10, pink2, 56, 46, false)); g.fillRoundedRectangle (20, 10, 36, 36, 9.f);
        drawYin (g, 38.f, 28.f, 12.f, 0.f, 1.f, ink, cream);
        g.setColour (cream); g.setFont (serif (28, juce::Font::bold)); g.drawText ("Yolo", 66, 6, 80, 34, juce::Justification::centredLeft);
        caps (g, "Tune", juce::Rectangle<float> (67, 36, 60, 12), 7.5f, mute, juce::Justification::centredLeft, .3f);
        caps (g, "Snapshot", juce::Rectangle<float> (872, 18, 80, 20), 8.5f, mute, juce::Justification::centredRight, .2f);
        // main panels
        drawPanel (g, { 9, 65, 227, 421 }, "Key & Scale");
        drawPanel (g, { 244, 65, 704, 421 }, "Pitch");
        drawPanel (g, { 957, 65, 298, 421 }, "Correction");
        drawPanel (g, { 9, 495, 194, 143 }, "Levels");
        drawPanel (g, { 211, 495, 284, 143 }, "Character");
        drawPanel (g, { 504, 495, 117, 143 }, "Guide tone");
        drawPanel (g, { 630, 495, 318, 143 }, "Input");
        drawPanel (g, { 957, 495, 298, 143 }, "Pitch reference");
        // left panel labels
        caps (g, "Scale", { 20, 100, 100, 14 }, 8.5f, mute); caps (g, "Key", { 20, 158, 100, 14 }, 8.5f, mute);
        { auto cb = juce::Rectangle<float> (20, 296, 203, 32); g.setColour (ink); g.fillRoundedRectangle (cb, 4.f); g.setColour (line); g.drawRoundedRectangle (cb.reduced (.5f), 4.f, 1.f);
          caps (g, "Current", cb.withTrimmedLeft (12.f), 8.5f, mute);
          g.setColour (cream); g.setFont (juce::FontOptions (13.f, juce::Font::bold)); g.drawText (scaleName(), cb.withTrimmedRight (12.f).toNearestInt(), juce::Justification::centredRight); }
        g.setColour (mute); g.setFont (juce::FontOptions (10.5f)); g.drawText ("Click keyboard notes to allow / mute", 14, 372, 215, 14, juce::Justification::centred);
        g.setColour (juce::Colour (0xff2a1e24)); g.fillRect (10, 400, 225, 1);
        caps (g, "Note stabilizer", { 20, 410, 150, 14 }, 8.5f, mute);
        // correction readout under the yin yang
        { const int cts = (int) std::lround (scene.correctionCents());
          g.setColour (pink2); g.fillRect (1086.f, 311.f, 40.f, 2.f);
          g.setColour (cream); g.setFont (juce::FontOptions (12.f, juce::Font::bold));
          g.drawText (juce::String (cts > 0 ? "+" : "") + juce::String (cts) + juce::String (juce::CharPointer_UTF8 (" \xc2\xa2")) + " correction", 957, 318, 298, 18, juce::Justification::centred); }
        // level meters
        meter (g, { 88, 535, 9, 66 }, mIn); meter (g, { 186, 535, 9, 66 }, mOut);
        // status bar
        g.setColour (juce::Colour (0xff160f12)); g.fillRect (1, 646, kW - 2, 29);
        g.setColour (line); g.fillRect (1, 646, kW - 2, 1);
        const bool vc = proc.engine.voiced.load();
        g.setColour (vc ? pink2 : line.brighter (.3f)); g.fillEllipse (20, 657, 7, 7);
        g.setColour (cream); g.setFont (juce::FontOptions (11.f, juce::Font::bold)); g.drawText (vc ? "Live" : "Idle", 34, 646, 50, 29, juce::Justification::centredLeft);
        g.setColour (mute); g.setFont (juce::FontOptions (11.f));
        g.drawText (juce::String (proc.getSampleRate() / 1000.0, 1) + " kHz", 96, 646, 70, 29, juce::Justification::centredLeft);
        const double ms = proc.getSampleRate() > 0 ? 1000.0 * proc.getLatencySamples() / proc.getSampleRate() : 0.0;
        g.drawText (juce::String (ms, 1) + " ms", 176, 646, 70, 29, juce::Justification::centredLeft);
        g.drawText (inst.getToggleState() ? "Instrument" : "Voice", 256, 646, 90, 29, juce::Justification::centredLeft);
        juce::String det = "-- > --";
        if (vc) { auto nm = [this] (float m) { const int c = ((int) std::lround (m) % 12 + 12) % 12; return juce::String (flats ? kFlat[c] : kSharp[c]); }; det = nm (proc.engine.detM.load()) + " > " + nm (proc.engine.outM.load()); }
        g.setColour (pink2); g.drawText (det, 860, 646, 120, 29, juce::Justification::centredRight);
        g.setColour (mute); g.drawText (scaleName(), 760, 646, 90, 29, juce::Justification::centredRight);
        caps (g, "Yolo Tune 1.1", { 1050, 646, 200, 29 }, 9.f, mute, juce::Justification::centredRight, .12f);
    }
    juce::String scaleName() const { static const char* n[4] = {"Chromatic", "Major", "Minor", "Custom"}; return n[juce::jlimit (0, 3, (int) proc.apvts.getRawParameterValue ("scale")->load())]; }

    void resized() override
    {
        logoHit.setBounds (20, 10, 36, 36); credits.setBounds (20, 60, 270, 146);
        prevBtn->setBounds (142, 15, 28, 28); preset.setBounds (176, 15, 232, 28); nextBtn->setBounds (414, 15, 28, 28);
        undoBtn->setBounds (462, 15, 28, 28); redoBtn->setBounds (496, 15, 28, 28); abSeg->setBounds (544, 16, 100, 26);
        slots.setBounds (958, 16, 180, 26); bypass.setBounds (1150, 14, 100, 28);
        // key & scale
        accSeg->setBounds (182, 69, 46, 20); modeSeg->setBounds (20, 118, 203, 32); keySeg->setBounds (18, 176, 207, 110);
        chromLink->setBounds (20, 336, 99, 30); detectLink->setBounds (124, 336, 99, 30); stabSeg->setBounds (20, 430, 203, 30);
        // pitch
        lineT.setBounds (700, 69, 118, 20); detT.setBounds (826, 69, 118, 20);
        roll.setBounds (245, 94, 702, 230); piano.setBounds (254, 336, 684, 144);
        // correction
        modeLink->setBounds (1130, 69, 118, 20);
        scene.setBounds (962, 96, 288, 214);
        amount.setBounds (966, 346, 84, 102); speed.setBounds (1054, 336, 108, 114); sustain.setBounds (1166, 346, 84, 102);
        // bottom row
        inG.setBounds (18, 530, 64, 100); outG.setBounds (116, 530, 64, 100);
        hum.setBounds (222, 530, 82, 100); flex.setBounds (318, 530, 82, 100); dbl.setBounds (406, 530, 82, 100);
        tone.setBounds (520, 530, 84, 100);
        inst.setBounds (660, 540, 140, 42); lowlat.setBounds (800, 540, 140, 42);
        ref.setBounds (976, 538, 262, 20); ctLabel.setBounds (996, 582, 108, 28); hzLabel.setBounds (1112, 582, 120, 28);
    }

    YoloProcessor& proc; bool flats = false;
    Scene scene; PitchRoll roll; Piano piano; Slots slots; LogoHit logoHit; CreditsPanel credits; int flushTick = 0; float mIn = 0, mOut = 0;
    juce::Slider speed, sustain, amount, inG, outG, flex, hum, dbl, tone, ref;
    juce::ToggleButton inst, lowlat, lineT, detT, bypass;
    juce::ComboBox preset; juce::Label hzLabel, ctLabel;
    std::unique_ptr<Seg> modeSeg, accSeg, stabSeg, keySeg, abSeg;
    std::unique_ptr<Link> detectLink, chromLink, modeLink;
    std::unique_ptr<IconBtn> prevBtn, nextBtn, undoBtn, redoBtn;
    std::unique_ptr<SA> aSp, aSu, aAm, aIn, aOut, aFl, aHu, aDb, aTo, aRef;
    std::unique_ptr<BA> aInst, aLow, aByp;
};

YoloEditor::YoloEditor (YoloProcessor& p) : AudioProcessorEditor (&p)
{
    static YoloLook look; juce::ignoreUnused (look);
    content = std::make_unique<Content> (p);
    content->setLookAndFeel (&look);
    content->setSize (kW, kH);
    addAndMakeVisible (*content);
    constrainer.setFixedAspectRatio ((double) kW / kH);
    constrainer.setSizeLimits (kW / 2, kH / 2, kW * 3 / 2, kH * 3 / 2);
    setConstrainer (&constrainer);
    setResizable (true, true);
    setSize (kW * 3 / 4, kH * 3 / 4);          // opens compact; drag the corner to scale up
}
YoloEditor::~YoloEditor() { content->setLookAndFeel (nullptr); }
void YoloEditor::paint (juce::Graphics& g) { g.fillAll (juce::Colour (0xff120c0e)); }
void YoloEditor::resized()
{
    content->setTransform (juce::AffineTransform::scale ((float) getWidth() / (float) kW));
    content->setBounds (0, 0, kW, kH);
}
