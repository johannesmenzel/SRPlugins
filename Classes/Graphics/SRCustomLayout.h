#pragma once
using namespace iplug;
using namespace igraphics;
// SRPLUGIN CONSTANTS

namespace SR {
    namespace Graphics {
        namespace Layout {
            // Colors
            const IColor SR_DEFAULT_COLOR = COLOR_LIGHT_GRAY; // oder COLOR_MID_GRAY?
            const IColor SR_DEFAULT_COLOR_BG = COLOR_TRANSPARENT;
            const IColor SR_DEFAULT_COLOR_FG = SR_DEFAULT_COLOR;
            const IColor SR_DEFAULT_COLOR_PR = IColor(255, 234, 158, 19);
            const IColor SR_DEFAULT_COLOR_FR = IColor(255, 150, 150, 150);
            const IColor SR_DEFAULT_COLOR_HL = IColor(30, 255, 255, 255);
            const IColor SR_DEFAULT_COLOR_SH = IColor(100, 0, 0, 0);
            const IColor SR_DEFAULT_COLOR_X1 = IColor(255, 234, 158, 19);
            const IColor SR_DEFAULT_COLOR_X2 = IColor(255, 48, 166, 186);
            const IColor SR_DEFAULT_COLOR_X3 = IColor(255, 249, 206, 34);

            const IColor SR_DEFAULT_COLOR_CUSTOM_PLUGIN_BG = IColor(255, 12, 17, 23);
            const IColor SR_DEFAULT_COLOR_CUSTOM_PANEL_BG = IColor(255, 50, 75, 95);
            const IColor SR_DEFAULT_COLOR_CUSTOM_BLUE = IColor(255, 62, 100, 121);
            const IColor SR_DEFAULT_COLOR_CUSTOM_RED = IColor(255, 131, 18, 18);
            const IColor SR_DEFAULT_COLOR_CUSTOM_GREEN = IColor(255, 103, 141, 52);
            const IColor SR_DEFAULT_COLOR_CUSTOM_ORANGE = IColor(255, 234, 158, 19);
            const IColor SR_DEFAULT_COLOR_CUSTOM_YELLOW = IColor(255, 219, 181, 30);
            const IColor SR_DEFAULT_COLOR_CUSTOM_BLACK = IColor(255, 23, 23, 23);
            const IColor SR_DEFAULT_COLOR_CUSTOM_WHITE = IColor(255, 243, 243, 243);

            // Texts
            const float SR_DEFAULT_TEXT_SIZE = 20.f;

            const IText SR_DEFAULT_TEXT = IText(20.f, SR_DEFAULT_COLOR, nullptr, EAlign::Center, EVAlign::Top);
            const IText SR_DEFAULT_TEXT_KNOB_LABEL = IText(20.f, SR_DEFAULT_COLOR, nullptr, EAlign::Center, EVAlign::Top);
            const IText SR_DEFAULT_TEXT_KNOB_VALUE = IText(20.f, COLOR_MID_GRAY, nullptr, EAlign::Center, EVAlign::Bottom);
            const IText SR_DEFAULT_TEXT_BUTTON_LABEL = IText(20.f, SR_DEFAULT_COLOR, nullptr, EAlign::Center, EVAlign::Bottom);
            const IText SR_DEFAULT_TEXT_BUTTON_VALUE = IText(14.f, COLOR_MID_GRAY, nullptr, EAlign::Center, EVAlign::Bottom);;
            const IText SR_DEFAULT_TEXT_VERSIONSTRING = IText(14.f, SR_DEFAULT_COLOR, nullptr, EAlign::Near, EVAlign::Middle);
            const IText SR_DEFAULT_TEXT_PRESETMENU = IText(30.f, SR_DEFAULT_COLOR, nullptr, EAlign::Center, EVAlign::Middle);

            // Color Specs
            const IVColorSpec SR_DEFAULT_COLOR_SPEC = IVColorSpec(
                { SR_DEFAULT_COLOR_BG
                , SR_DEFAULT_COLOR_FG
                , SR_DEFAULT_COLOR_PR
                , SR_DEFAULT_COLOR_FR
                , SR_DEFAULT_COLOR_HL
                , SR_DEFAULT_COLOR_SH
                , SR_DEFAULT_COLOR_X1
                , SR_DEFAULT_COLOR_X2
                , SR_DEFAULT_COLOR_X3
                }
            );

            // Styles
            const IVStyle SR_DEFAULT_STYLE = IVStyle(
                true,
                true,
                SR_DEFAULT_COLOR_SPEC,
                SR_DEFAULT_TEXT_KNOB_LABEL,
                SR_DEFAULT_TEXT_KNOB_VALUE,
                true,
                false,
                false,
                false,
                0.05f, // roundness
                2.f, // frame-thick, def: 1.f
                3.f, // shadow-off
                DEFAULT_WIDGET_FRAC,
                DEFAULT_WIDGET_ANGLE
            );

            const IVStyle SR_DEFAULT_STYLE_KNOB = SR_DEFAULT_STYLE;
            const IVStyle SR_DEFAULT_STYLE_BUTTON = SR_DEFAULT_STYLE
                .WithLabelText(SR_DEFAULT_TEXT_BUTTON_LABEL)
                .WithValueText(SR_DEFAULT_TEXT_BUTTON_VALUE)
                .WithShowLabel(false);
            const IVStyle SR_DEFAULT_STYLE_FADER = SR_DEFAULT_STYLE
                .WithColor(EVColor::kFG, IColor(255, 30, 50, 70))
                .WithDrawFrame(false);
            const IVStyle SR_DEFAULT_STYLE_METER = SR_DEFAULT_STYLE_FADER;
            const IVStyle SR_DEFAULT_STYLE_GRAPH = SR_DEFAULT_STYLE_FADER;

            // END SRPLUGIN CONSTANTS
        }
    }
}