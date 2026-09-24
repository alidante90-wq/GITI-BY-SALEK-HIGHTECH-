// SALEK HIGHTECH — optimized parallax + 3D light + Iranian geometric motifs
void SalekHightechAudioProcessorEditor::paint (juce::Graphics& g)
{
    const int themeId = themeBox.getSelectedId();
    const float W = (float) getWidth();
    const float H = (float) getHeight();
    const float t = animPhase;
    const float peak  = processor.getOutputPeak();
    const float pulse = juce::jlimit (0.0f, 1.0f, peak * 5.0f);
    const auto mouse = getMouseXYRelative();
    const float mx = W > 1.f ? ((float) mouse.x / W - 0.5f) : 0.f;
    const float my = H > 1.f ? ((float) mouse.y / H - 0.5f) : 0.f;
    const float morph = 0.5f + 0.5f * std::sin (t * 0.35f);

    auto lerpCol = [] (juce::Colour a, juce::Colour b, float x) {
        return a.interpolatedWith (b, juce::jlimit (0.f, 1.f, x));
    };

    juce::Colour cyan, magenta, gold, purple, lime;
    if (themeId == 2) {
        cyan = lerpCol (juce::Colour (0xffb8ff00), juce::Colour (0xff39ff14), morph);
        magenta = lerpCol (juce::Colour (0xffff6b00), juce::Colour (0xffffaa00), morph);
        gold = juce::Colour (0xffffee00); purple = juce::Colour (0xff88ff00); lime = juce::Colour (0xff39ff14);
    } else if (themeId == 3) {
        cyan = lerpCol (juce::Colour (0xffff00dd), juce::Colour (0xffff66ff), morph);
        magenta = lerpCol (juce::Colour (0xff00ffff), juce::Colour (0xff7c4dff), morph);
        gold = juce::Colour (0xffff66ff); purple = juce::Colour (0xffaa00ff); lime = juce::Colour (0xffff2d9b);
    } else if (themeId == 4) {
        cyan = lerpCol (juce::Colour (0xff7ef9e0), juce::Colour (0xffffb0d0), morph);
        magenta = lerpCol (juce::Colour (0xffff2d9b), juce::Colour (0xffff80c0), 1.f - morph);
        gold = juce::Colour (0xffff66aa); purple = juce::Colour (0xffff80c0); lime = juce::Colour (0xffb8ff40);
    } else {
        // Mockup cyber: hot pink + electric cyan (SALEK reference art)
        cyan = lerpCol (juce::Colour (0xff00f0ff), juce::Colour (0xff7cf0ff), morph);
        magenta = lerpCol (juce::Colour (0xffff2ec8), juce::Colour (0xffff66e0), morph);
        gold = juce::Colour (0xffffd700); purple = juce::Colour (0xffc44dff); lime = juce::Colour (0xff39ff14);
    }

    // Light source follows mouse (3D lighting key)
    const float lx = W * (0.5f + mx * 0.6f);
    const float ly = H * (0.35f + my * 0.4f);

    // ---- L0 far gradient ----
    {
        juce::Colour c0, c1;
        if (themeId == 4)      { c0 = juce::Colour (0xff1a0c18); c1 = juce::Colour (0xff3a1830); }
        else if (themeId == 2) { c0 = juce::Colour (0xff040a02); c1 = juce::Colour (0xff1a2e08); }
        else if (themeId == 3) { c0 = juce::Colour (0xff0c0210); c1 = juce::Colour (0xff2a0838); }
        else                   { c0 = juce::Colour (0xff0a0218); c1 = juce::Colour (0xff2a0a3a); } // deep cyber night
        float gx = lx * 0.3f + W * 0.3f, gy = ly * 0.3f + H * 0.2f;
        juce::Colour mid = c1.interpolatedWith (purple.darker (0.2f), morph * 0.35f + pulse * 0.15f);
        juce::ColourGradient bg (c0, gx - W * 0.3f, gy - H * 0.3f, mid, gx + W * 0.5f, gy + H * 0.5f, true);
        g.setGradientFill (bg);
        g.fillAll();
    }

    // girih / stars disabled for performance
    juce::ignoreUnused (t, pulse, mx, my, morph, gold, purple, lime, lx, ly);

    // ---- background art (CACHED scaled image — rebuild only on resize/theme) ----
    {
        const int iw = (int) W, ih = (int) H;
        if (heroImg.isValid() && (cachedBgW != iw || cachedBgH != ih || cachedBgTheme != themeId || ! cachedBg.isValid()))
        {
            cachedBg = juce::Image (juce::Image::ARGB, juce::jmax (1, iw), juce::jmax (1, ih), true);
            juce::Graphics cg (cachedBg);
            cg.drawImage (heroImg, juce::Rectangle<float> (0, 0, (float) iw, (float) ih),
                          juce::RectanglePlacement::fillDestination);
            // dark veil baked into cache
            juce::ColourGradient veil (juce::Colours::black.withAlpha (0.35f), iw * 0.5f, (float) ih,
                                      juce::Colours::transparentBlack, iw * 0.5f, ih * 0.28f, false);
            cg.setGradientFill (veil);
            cg.fillAll();
            cachedBgW = iw; cachedBgH = ih; cachedBgTheme = themeId;
        }
        if (cachedBg.isValid())
            g.drawImageAt (cachedBg, 0, 0);
    }


    // Lightweight 3D bezel
    {
        g.setColour (cyan.withAlpha (0.28f));
        g.drawRoundedRectangle (3.f, 3.f, W - 6.f, H - 6.f, 12.f, 2.f);
        g.setColour (magenta.withAlpha (0.16f));
        g.drawRoundedRectangle (6.f, 6.f, W - 12.f, H - 12.f, 10.f, 1.f);
        juce::ColourGradient top (juce::Colours::white.withAlpha (0.07f), W * 0.5f, 0.f,
                                  juce::Colours::transparentWhite, W * 0.5f, 28.f, false);
        g.setGradientFill (top);
        g.fillRect (0.f, 0.f, W, 28.f);
    }


// BG thumbnail panel REMOVED (user red-X). Use top-right BG button only.
        
    
    // Bottom model strip backdrop (above keyboard)
    if (modelStripBounds.getWidth() > 20)
    {
        auto r = modelStripBounds.toFloat();
        g.setColour (juce::Colour (0xff06040e).withAlpha (0.82f));
        g.fillRoundedRectangle (r, 8.f);
        g.setColour (juce::Colour (0xff00e8ff).withAlpha (0.25f));
        g.drawRoundedRectangle (r, 8.f, 1.f);
    }

    if (armedModSource >= 0)
    {
        const juce::Colour cols[3] = {
            juce::Colour (0xff00e8ff), juce::Colour (0xffff2d9b), juce::Colour (0xff39ff14)
        };
        g.setColour (cols[juce::jlimit (0, 2, armedModSource)].withAlpha (0.9f));
        g.setFont (juce::FontOptions (12.f, juce::Font::bold));
        g.drawText ("LFO" + juce::String (armedModSource + 1) + " ARMED — click knob",
                    8, 80, (int) W - 16, 16, juce::Justification::centred);
    }

    // FX underlay — solid dark (never pink / garbage)
    {
        const int ti = tabs.getCurrentTabIndex();
        const bool isFx = (ti >= 0 && ti < tabs.getNumTabs()
                           && tabs.getTabNames()[ti] == "FX");
        if (isFx)
        {
            auto r = getLocalArea (&fxTab, fxTab.getLocalBounds()).toFloat();
            if (r.getWidth() > 10.f)
            {
                g.setColour (juce::Colour (0xff08060f));
                g.fillRoundedRectangle (r.reduced (1.f), 10.f);
                g.setColour (juce::Colour (0xff00e8ff).withAlpha (0.2f));
                g.drawRoundedRectangle (r.reduced (1.f), 10.f, 1.2f);
            }
        }
    }

// Logo drawn by logoOverlay component (always on top)


    // ---- Alt logo (Designer) bottom-right ----
    if (logoAltImg.isValid())
    {
        const float aw = juce::jmin (120.f, W * 0.11f);
        const float ah = aw;
        // top-left under brand triangle (not covering keyboard / filter)
        auto area = juce::Rectangle<float> (10.f, 92.f, aw, ah);
        g.setOpacity (0.92f);
        g.drawImage (logoAltImg, area, juce::RectanglePlacement::centred | juce::RectanglePlacement::onlyReduceInSize);
        g.setOpacity (1.f);
    }

    // ---- Character PNGs LAST = top z-order (cached images, no disk IO) ----
    // Character model — ONLY in left slot under presets (does NOT change background)
    if (charImgL.isValid() && charSlotBounds.getWidth() > 20 && charSlotBounds.getHeight() > 40)
    {
        auto slot = charSlotBounds.toFloat();
        // dark panel
        g.setColour (juce::Colour (0xff06040e).withAlpha (0.72f));
        g.fillRoundedRectangle (slot, 12.f);
        // multi-layer neon frame (cyan + magenta)
        g.setColour (juce::Colour (0xff00e8ff).withAlpha (0.25f));
        g.drawRoundedRectangle (slot.expanded (2.f), 14.f, 3.5f);
        g.setColour (juce::Colour (0xffff2d9b).withAlpha (0.55f));
        g.drawRoundedRectangle (slot, 12.f, 2.0f);
        g.setColour (juce::Colour (0xff00e8ff).withAlpha (0.85f));
        g.drawRoundedRectangle (slot.reduced (2.f), 10.f, 1.2f);
        // corner accents
        const float c = 14.f;
        g.setColour (juce::Colour (0xffff2d9b).withAlpha (0.9f));
        g.drawLine (slot.getX(), slot.getY() + c, slot.getX(), slot.getY(), 2.f);
        g.drawLine (slot.getX(), slot.getY(), slot.getX() + c, slot.getY(), 2.f);
        g.drawLine (slot.getRight(), slot.getY() + c, slot.getRight(), slot.getY(), 2.f);
        g.drawLine (slot.getRight() - c, slot.getY(), slot.getRight(), slot.getY(), 2.f);
        g.drawLine (slot.getX(), slot.getBottom() - c, slot.getX(), slot.getBottom(), 2.f);
        g.drawLine (slot.getX(), slot.getBottom(), slot.getX() + c, slot.getBottom(), 2.f);
        g.drawLine (slot.getRight(), slot.getBottom() - c, slot.getRight(), slot.getBottom(), 2.f);
        g.drawLine (slot.getRight() - c, slot.getBottom(), slot.getRight(), slot.getBottom(), 2.f);
        // BOTTOM-aligned portrait (fixes knee-up / full-body mismatch)
        g.setOpacity (0.98f);
        g.drawImage (charImgL, slot.reduced (6.f, 8.f),
                     juce::RectanglePlacement::xMid | juce::RectanglePlacement::yBottom
                     | juce::RectanglePlacement::onlyReduceInSize);
        g.setOpacity (1.f);
    }
    g.setOpacity (1.f);

    // SALEK icon badges (paint fallback — always visible if atlas loads)
    {
        if (tabs.getCurrentTabIndex() == 0)
        {
            auto drawIcon = [&] (int idx, juce::Rectangle<int> dest)
            {
                if (dest.getWidth() < 8) return;
                idx = juce::jlimit (0, 49, idx);
                auto sub = SalekAssets::loadIcon (idx);
                if (sub.isValid())
                {
                    g.setOpacity (1.f);
                    g.drawImage (sub, dest.toFloat(),
                                 juce::RectanglePlacement::centred | juce::RectanglePlacement::onlyReduceInSize);
                }
                else
                {
                    // neon fallback so icons never "missing"
                    auto d = dest.toFloat().reduced (2.f);
                    g.setColour (juce::Colour (0xff00e8ff).withAlpha (0.85f));
                    g.drawEllipse (d, 1.6f);
                    g.setColour (juce::Colour (0xffff2d9b).withAlpha (0.7f));
                    g.drawLine (d.getCentreX(), d.getY() + 4, d.getCentreX(), d.getBottom() - 4, 1.4f);
                    g.drawLine (d.getX() + 4, d.getCentreY(), d.getRight() - 4, d.getCentreY(), 1.4f);
                }
            };
            // OSC1/2/3 headers from osc monitors if present
            if (oscMon1 != nullptr)
            {
                auto r = getLocalArea (oscMon1.get(), oscMon1->getLocalBounds());
                drawIcon (0, juce::Rectangle<int> (r.getX() + 2, r.getY() + 1, 32, 32));
            }
            if (oscMon2 != nullptr)
            {
                auto r = getLocalArea (oscMon2.get(), oscMon2->getLocalBounds());
                drawIcon (1, juce::Rectangle<int> (r.getX() + 2, r.getY() + 1, 32, 32));
            }
            if (oscMon3 != nullptr)
            {
                auto r = getLocalArea (oscMon3.get(), oscMon3->getLocalBounds());
                drawIcon (2, juce::Rectangle<int> (r.getX() + 2, r.getY() + 1, 32, 32));
            }
            if (filterTab.isShowing())
            {
                auto r = getLocalArea (&filterTab, filterTab.getLocalBounds());
                drawIcon (11, juce::Rectangle<int> (r.getX() + 4, r.getY() + 4, 26, 26));
            }
        }
        else if (atlas.isValid() && tabs.getCurrentTabIndex() == 2) // LFO
        {
            auto r = getLocalArea (&lfoTab, lfoTab.getLocalBounds());
            auto drawIcon = [&] (int idx, int x)
            {
                auto sub = atlas.getClippedImage ({ (idx%10)*64, (idx/10)*64, 64, 64 });
                g.drawImage (sub, juce::Rectangle<float> ((float) x, (float) r.getY() + 4, 28.f, 28.f),
                             juce::RectanglePlacement::centred | juce::RectanglePlacement::onlyReduceInSize);
            };
            drawIcon (17, r.getX() + 8);
            drawIcon (18, r.getX() + 40);
            drawIcon (19, r.getX() + 72);
        }
        else if (atlas.isValid() && tabs.getCurrentTabIndex() == 1) // MOD
        {
            auto r = getLocalArea (&modTab, modTab.getLocalBounds());
            auto drawIcon = [&] (int idx, int x)
            {
                auto sub = atlas.getClippedImage ({ (idx%10)*64, (idx/10)*64, 64, 64 });
                g.drawImage (sub, juce::Rectangle<float> ((float) x, (float) r.getY() + 4, 28.f, 28.f),
                             juce::RectanglePlacement::centred | juce::RectanglePlacement::onlyReduceInSize);
            };
            drawIcon (20, r.getX() + 8);
            drawIcon (21, r.getX() + 40);
            drawIcon (5,  r.getX() + 72);
        }
        else if (atlas.isValid() && tabs.getCurrentTabIndex() == 3) // FX
        {
            auto r = getLocalArea (&fxTab, fxTab.getLocalBounds());
            const int idxs[] = { 37, 36, 35, 9, 41, 40, 39, 32 };
            for (int i = 0; i < 8; ++i)
            {
                int idx = idxs[i];
                auto sub = atlas.getClippedImage ({ (idx%10)*64, (idx/10)*64, 64, 64 });
                float x = (float) r.getX() + 10.f + (float) (i % 2) * (r.getWidth() * 0.5f);
                float y = (float) r.getY() + 8.f + (float) (i / 2) * (r.getHeight() * 0.22f);
                g.drawImage (sub, juce::Rectangle<float> (x, y, 26.f, 26.f),
                             juce::RectanglePlacement::centred | juce::RectanglePlacement::onlyReduceInSize);
            }
        }

    }

    // ---- left panel: only show branding card when preset sidebar is collapsed ----
    if (presetCollapsed)
    {
        auto left = juce::Rectangle<float> (8.f, 48.f, 28.f, 120.f);
        g.setColour (juce::Colours::black.withAlpha (0.35f));
        g.fillRoundedRectangle (left.translated (2.f, 3.f), 8.f);
        g.setColour (juce::Colour (0xff0a0614).withAlpha (0.45f));
        g.fillRoundedRectangle (left, 8.f);
        g.setColour (cyan.withAlpha (0.5f));
        g.drawRoundedRectangle (left, 8.f, 1.2f);
        g.setColour (magenta);
        g.setFont (juce::FontOptions (10.0f, juce::Font::bold));
        g.drawText ("S", left, juce::Justification::centred);
    }
    // when preset is open, no solid card — background art stays fully visible behind the list


        // Serum-like section frames on MAIN (osc / filter / env)
        if (tabs.getCurrentTabIndex() == 0)
        {
            auto frame = [] (juce::Graphics& g, juce::Component& c, juce::Colour col, const char* title)
            {
                if (! c.isVisible() || c.getWidth() < 8) return;
                auto b = c.getBounds().toFloat();
                // convert to editor coords
                auto topLeft = c.getParentComponent()->getLocalBounds(); // wrong
                juce::ignoreUnused (topLeft);
            };
            juce::ignoreUnused (frame);
            auto drawFrame = [&] (juce::Component& c, juce::Colour col, const juce::String& title)
            {
                if (c.getWidth() < 10) return;
                auto r = getLocalArea (&c, c.getLocalBounds()).toFloat().reduced (2.f);
                g.setColour (juce::Colour (0xff0a0614).withAlpha (0.22f));
                g.fillRoundedRectangle (r, 10.f);
                g.setColour (col.withAlpha (0.45f + pulse * 0.15f));
                g.drawRoundedRectangle (r, 10.f, 1.4f);
                g.setColour (col.withAlpha (0.85f));
                g.setFont (juce::FontOptions (10.f, juce::Font::bold));
                g.drawText (title, r.removeFromTop (14).reduced (8, 0), juce::Justification::centredLeft);
            };
            drawFrame (oscTab, cyan, "OSCILLATORS");
            drawFrame (filterTab, magenta, "FILTER");
            drawFrame (envTab, gold, "AMP ENV");
            // Vertical separators between OSC1 | OSC2 | OSC3 (user red lines)
            {
                auto orc = getLocalArea (&oscTab, oscTab.getLocalBounds()).toFloat();
                const float gap = 12.f;
                const float colW = (orc.getWidth() - gap * 2.f) / 3.f;
                g.setColour (cyan.withAlpha (0.55f));
                for (int s = 1; s <= 2; ++s)
                {
                    const float x = orc.getX() + s * (colW + gap) - gap * 0.5f;
                    g.drawLine (x, orc.getY() + 8.f, x, orc.getBottom() - 8.f, 1.6f);
                }
            }
        }

}
