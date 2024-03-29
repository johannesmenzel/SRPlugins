#pragma once
//#include "IPlug_include_in_plug_hdr.h"
//#include "IPlug_include_in_plug_src.h"
#include "IControl.h"
#include "IControls.h"
#include "SRCustomLayout.h"
using namespace iplug;
using namespace igraphics;

namespace SR {
	namespace Graphics {
		namespace Controls {

			class Switch
				: public IVSwitchControl {
			public:
				Switch(const IRECT& bounds, int paramIdx = kNoParameter, const char* label = "", const IVStyle& style = DEFAULT_STYLE, bool valueInButton = true, float tracksize = 1.f)
					:IVSwitchControl(bounds, paramIdx, label, style, valueInButton) 
				{
					mTrackSize = tracksize;
				};


				void DrawWidget(IGraphics& g) override {
					const IColor color = (mMouseIsOver) ? SR::Graphics::Layout::SR_DEFAULT_COLOR_X1 : SR::Graphics::Layout::SR_DEFAULT_COLOR_FG;
					// Decided for fixed 3.f roundness, maybe align in other places
					g.DrawRoundRect(color, mWidgetBounds, 3.f, 0, mTrackSize);
				};

				virtual void DrawValue(IGraphics& g, bool mouseOver) override {
					const IRECT bounds = mValueBounds.GetCentredInside(mValueBounds.GetLengthOfShortestSide() * .8f);
					const float cx = bounds.MW(), cy = bounds.MH(), r = bounds.H() * .5f;

					// Draw on/off symbol not highlighted
					if (strcmp(mValueStr.Get(), "OFF") == 0) {
						g.DrawArc(SR::Graphics::Layout::SR_DEFAULT_COLOR_FG, cx, cy, r, 25.f, -25.f, 0, mTrackSize * 2.f);
						g.DrawRadialLine(SR::Graphics::Layout::SR_DEFAULT_COLOR_FG, cx, cy, 0.f, .0f, r, 0, mTrackSize * 2.f);
					}
					// Draw on/off symbol highlighted
					else if (strcmp(mValueStr.Get(), "ON") == 0) {
						g.DrawArc(SR::Graphics::Layout::SR_DEFAULT_COLOR_PR, cx, cy, r, 25.f, -25.f, 0, mTrackSize * 2.f);
						g.DrawRadialLine(SR::Graphics::Layout::SR_DEFAULT_COLOR_PR, cx, cy, 0.f, .0f, r, 0, mTrackSize * 2.f);
					}
					// Draw peak filter symbol
					else if (strcmp(mValueStr.Get(), "PEAK") == 0) {
						g.PathClear();
						g.PathMoveTo(bounds.L, bounds.T + .8f * bounds.H());
						g.PathLineTo(bounds.L + .2f * bounds.W(), bounds.T + .8f * bounds.H());
						g.PathLineTo(bounds.L + .4f * bounds.W(), bounds.T + .2f * bounds.H());
						g.PathLineTo(bounds.L + .6f * bounds.W(), bounds.T + .2f * bounds.H());
						g.PathLineTo(bounds.L + .8f * bounds.W(), bounds.T + .8f * bounds.H());
						g.PathLineTo(bounds.R, bounds.T + .8f * bounds.H());
						g.PathStroke(GetColor(kFG), mTrackSize);
					}
					// Draw lowshelf filter symbol
					else if (strcmp(mValueStr.Get(), "LS") == 0) {
						g.PathClear();
						g.PathMoveTo(bounds.L, bounds.T + .2f * bounds.H());
						g.PathLineTo(bounds.L + .2f * bounds.W(), bounds.T + .2f * bounds.H());
						g.PathLineTo(bounds.L + .4f * bounds.W(), bounds.T + .8f * bounds.H());
						g.PathLineTo(bounds.R, bounds.T + .8f * bounds.H());
						g.PathStroke(GetColor(kFG), mTrackSize);
					}
					// Draw highshelf filter symbol
					else if (strcmp(mValueStr.Get(), "HS") == 0) {
						g.PathClear();
						g.PathMoveTo(bounds.L, bounds.T + .8f * bounds.H());
						g.PathLineTo(bounds.L + .6f * bounds.W(), bounds.T + .8f * bounds.H());
						g.PathLineTo(bounds.L + .8f * bounds.W(), bounds.T + .2f * bounds.H());
						g.PathLineTo(bounds.R, bounds.T + .2f * bounds.H());
						g.PathStroke(GetColor(kFG), mTrackSize);
					}
					// Draw parallel processing symbol (unfinised)
					else if (strcmp(mValueStr.Get(), "PARALLEL") == 0) {
						g.PathClear();
						g.PathMoveTo(bounds.L, bounds.T + .3f * bounds.H());
						g.PathLineTo(bounds.L + .2f * bounds.W(), bounds.T + .3f * bounds.H());
						g.PathMoveTo(bounds.L, bounds.T + .7f * bounds.H());
						g.PathLineTo(bounds.L + .2f * bounds.W(), bounds.T + .7f * bounds.H());
						g.PathMoveTo(bounds.L + .8f * bounds.W(), bounds.T + .3f * bounds.H());
						g.PathLineTo(bounds.R, bounds.T + .3f * bounds.H());
						g.PathMoveTo(bounds.L + .8f * bounds.W(), bounds.T + .7f * bounds.H());
						g.PathLineTo(bounds.R, bounds.T + .7f * bounds.H());
						g.PathRect(IRECT(bounds.L + .2f * bounds.W(), bounds.T + .2f * bounds.H(), bounds.L + .8f * bounds.W(), bounds.T + .4f * bounds.H()));
						g.PathRect(IRECT(bounds.L + .2f * bounds.W(), bounds.T + .6f * bounds.H(), bounds.L + .8f * bounds.W(), bounds.T + .8f * bounds.H()));
						g.PathStroke(GetColor(kFG), mTrackSize);
					}
					else if (strcmp(mValueStr.Get(), "SERIAL") == 0) {
						g.PathClear();
						g.PathMoveTo(bounds.L, bounds.T + .5f * bounds.H());
						g.PathLineTo(bounds.L + .2f * bounds.W(), bounds.T + .5f * bounds.H());
						g.PathMoveTo(bounds.L + .4f * bounds.W(), bounds.T + .5f * bounds.H());
						g.PathLineTo(bounds.L + .6f * bounds.W(), bounds.T + .5f * bounds.H());
						g.PathMoveTo(bounds.L + .8f * bounds.W(), bounds.T + .5f * bounds.H());
						g.PathLineTo(bounds.R, bounds.T + .5f * bounds.H());
						g.PathRect(IRECT(bounds.L + .2f * bounds.W(), bounds.T + .2f * bounds.H(), bounds.L + .4f * bounds.W(), bounds.T + .8f * bounds.H()));
						g.PathRect(IRECT(bounds.L + .6f * bounds.W(), bounds.T + .2f * bounds.H(), bounds.L + .8f * bounds.W(), bounds.T + .8f * bounds.H()));
						g.PathStroke(GetColor(kFG), mTrackSize);
					}
					// if not, draw text
					else
					{
						if (mouseOver)
							g.FillRect(COLOR_TRANSLUCENT, mValueBounds);

						if (mStyle.showValue)
						{
							IBlend blend = mControl->GetBlend();
							g.DrawText(mStyle.valueText, mValueStr.Get(), mValueBounds, &blend);
						}
					}
				}
			};

		}
	}
}