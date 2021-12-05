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

			class Knob
				: public IVKnobControl {
			public:
				Knob(const IRECT& bounds, int paramIdx,
					const char* label = "",
					const IVStyle& style = SR::Graphics::Layout::SR_DEFAULT_STYLE,
					bool valueIsEditable = false, bool valueInWidget = false,
					float a1 = -135.f, float a2 = 135.f, float aAnchor = -135.f,
					EDirection direction = EDirection::Vertical, double gearing = DEFAULT_GEARING, float trackSize = 2.f);

				virtual ~Knob() {};

				void DrawWidget(IGraphics& g) override;
			};

			inline Knob::Knob(const IRECT& bounds, int paramIdx, const char* label, const IVStyle& style, bool valueIsEditable, bool valueInWidget, float a1, float a2, float aAnchor, EDirection direction, double gearing, float trackSize) 
				: IVKnobControl(bounds, paramIdx, label, style, valueIsEditable, valueInWidget, a1, a2, aAnchor, direction, gearing, trackSize) 
			{
				SetPointerThickness(1.f);
				SetInnerPointerFrac(0.f);
				SetOuterPointerFrac(.8f);
			}

			inline void Knob::DrawWidget(IGraphics& g)
			{
				float widgetRadius; // The radius out to the indicator track arc

				if (mWidgetBounds.W() > mWidgetBounds.H())
					widgetRadius = (mWidgetBounds.H() / 2.f);
				else
					widgetRadius = (mWidgetBounds.W() / 2.f);

				const float cx = mWidgetBounds.MW(), cy = mWidgetBounds.MH();

				widgetRadius -= (mTrackSize / 2.f);

				IRECT knobHandleBounds = mWidgetBounds.GetCentredInside((widgetRadius - mTrackToHandleDistance) * 2.f);
				const float angle = mAngle1 + (static_cast<float>(GetValue()) * (mAngle2 - mAngle1));
				//DrawIndicatorTrack(g, angle, cx, cy, widgetRadius);
				if (mTrackSize > 0.f)
				{
					//g.DrawArc(GetColor(kX1), cx, cy, widgetRadius, angle >= mAnchorAngle ? mAnchorAngle : mAnchorAngle - (mAnchorAngle - angle), angle >= mAnchorAngle ? angle : mAnchorAngle, &mBlend, mTrackSize);
					g.DrawArc(SR::Graphics::Layout::SR_DEFAULT_COLOR_FG, cx, cy, widgetRadius * .95f, angle - 15.f, angle + 15.f, &mBlend, mTrackSize);
					g.DrawArc(SR::Graphics::Layout::SR_DEFAULT_COLOR_FG, cx, cy, widgetRadius, angle + 15.f, angle + 45.f, &mBlend, mTrackSize);
					g.DrawArc(SR::Graphics::Layout::SR_DEFAULT_COLOR_FG, cx, cy, widgetRadius * .95f, angle + 45.f, angle + 75.f, &mBlend, mTrackSize);
					g.DrawArc(SR::Graphics::Layout::SR_DEFAULT_COLOR_FG, cx, cy, widgetRadius, angle + 75.f, angle + 105.f, &mBlend, mTrackSize);
					g.DrawArc(SR::Graphics::Layout::SR_DEFAULT_COLOR_FG, cx, cy, widgetRadius * .95f, angle + 105.f, angle + 135.f, &mBlend, mTrackSize);
					g.DrawArc(SR::Graphics::Layout::SR_DEFAULT_COLOR_FG, cx, cy, widgetRadius, angle + 135.f, angle + 165.f, &mBlend, mTrackSize);
					g.DrawArc(SR::Graphics::Layout::SR_DEFAULT_COLOR_FG, cx, cy, widgetRadius * .95f, angle + 165.f, angle - 165.f, &mBlend, mTrackSize);
					g.DrawArc(SR::Graphics::Layout::SR_DEFAULT_COLOR_FG, cx, cy, widgetRadius, angle - 165.f, angle - 135.f, &mBlend, mTrackSize);
					g.DrawArc(SR::Graphics::Layout::SR_DEFAULT_COLOR_FG, cx, cy, widgetRadius * .95f, angle - 135.f, angle - 105.f, &mBlend, mTrackSize);
					g.DrawArc(SR::Graphics::Layout::SR_DEFAULT_COLOR_FG, cx, cy, widgetRadius, angle - 105.f, angle - 75.f, &mBlend, mTrackSize);
					g.DrawArc(SR::Graphics::Layout::SR_DEFAULT_COLOR_FG, cx, cy, widgetRadius * .95f, angle - 75.f, angle - 45.f, &mBlend, mTrackSize);
					g.DrawArc(SR::Graphics::Layout::SR_DEFAULT_COLOR_FG, cx, cy, widgetRadius, angle - 45.f, angle - 15.f, &mBlend, mTrackSize);
					g.DrawCircle(SR::Graphics::Layout::SR_DEFAULT_COLOR_FG, cx, cy, widgetRadius * .8f, &mBlend, mTrackSize);
					g.DrawRadialLine(SR::Graphics::Layout::SR_DEFAULT_COLOR_FG, cx, cy, angle, mInnerPointerFrac * widgetRadius, mOuterPointerFrac * widgetRadius, &mBlend, mPointerThickness * 4.f);
					g.DrawRadialLine(SR::Graphics::Layout::SR_DEFAULT_COLOR_CUSTOM_PANEL_BG, cx, cy, angle, mInnerPointerFrac * widgetRadius + 2.f * mPointerThickness, mOuterPointerFrac * widgetRadius + 2.f * mPointerThickness, &mBlend, mPointerThickness * 2.f);

				}
				//DrawPressableShape(g, /*mShape*/ EVShape::Ellipse, knobHandleBounds, mMouseDown, mMouseIsOver, IsDisabled());
				//DrawPointer(g, angle, cx, cy, knobHandleBounds.W() / 2.f);

			}

		}
	}
}