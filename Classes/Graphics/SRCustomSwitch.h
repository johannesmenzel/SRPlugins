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
				Switch(const IRECT& bounds, int paramIdx = kNoParameter, const char* label = "", const IVStyle& style = DEFAULT_STYLE, bool valueInButton = true)
					:IVSwitchControl(bounds, paramIdx, label, style, valueInButton) {
				};

				void DrawWidget(IGraphics& g) override {
					g.DrawRect(SR::Graphics::Layout::SR_DEFAULT_COLOR_FG, mWidgetBounds);
				};
			};

		}
	}
}