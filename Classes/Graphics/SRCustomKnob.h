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
					EDirection direction = EDirection::Vertical, double gearing = DEFAULT_GEARING,
					float trackSize = 2.f, float innerPointerFrac = .0f, float outerPointerFrac = .75f, float pointerThickness = 3.f);

				virtual ~Knob() {};

				void DrawWidget(IGraphics& g) override;
			};

			inline Knob::Knob(const IRECT& bounds, int paramIdx, const char* label, const IVStyle& style, bool valueIsEditable, bool valueInWidget, float a1, float a2, float aAnchor, EDirection direction, double gearing, float trackSize, float innerPointerFrac, float outerPointerFrac, float pointerThickness)
				: IVKnobControl(bounds, paramIdx, label, style, valueIsEditable, valueInWidget, a1, a2, aAnchor, direction, gearing, trackSize)
			{
				SetPointerThickness(pointerThickness);
				SetInnerPointerFrac(innerPointerFrac);
				SetOuterPointerFrac(outerPointerFrac);
			}

			inline void Knob::DrawWidget(IGraphics& g)
			{
				const float widgetRadius = mWidgetBounds.GetLengthOfShortestSide() * .5f - (mTrackSize * .5f); // The radius out to the indicator track arc
				const float innerWidgetRadius = widgetRadius * .92f;
				const float cx = mWidgetBounds.MW(), cy = mWidgetBounds.MH();
				IRECT knobHandleBounds = mWidgetBounds.GetCentredInside((widgetRadius - mTrackToHandleDistance) * 2.f);
				const float angle = mAngle1 + (static_cast<float>(GetValue()) * (mAngle2 - mAngle1));
				//DrawIndicatorTrack(g, angle, cx, cy, widgetRadius);
				if (mTrackSize > 0.f) {
					//g.DrawArc(GetColor(kX1), cx, cy, widgetRadius, angle >= mAnchorAngle ? mAnchorAngle : mAnchorAngle - (mAnchorAngle - angle), angle >= mAnchorAngle ? angle : mAnchorAngle, &mBlend, mTrackSize);
					g.DrawArc(SR::Graphics::Layout::SR_DEFAULT_COLOR_FG, cx, cy, innerWidgetRadius, angle - 15.f, angle + 15.f, &mBlend, mTrackSize);
					g.DrawRadialLine(SR::Graphics::Layout::SR_DEFAULT_COLOR_FG, cx, cy, angle + 15.f, innerWidgetRadius, widgetRadius, &mBlend, mTrackSize);
					g.DrawRadialLine(SR::Graphics::Layout::SR_DEFAULT_COLOR_FG, cx, cy, angle + 45.f, innerWidgetRadius, widgetRadius, &mBlend, mTrackSize);
					g.DrawRadialLine(SR::Graphics::Layout::SR_DEFAULT_COLOR_FG, cx, cy, angle + 75.f, innerWidgetRadius, widgetRadius, &mBlend, mTrackSize);
					g.DrawRadialLine(SR::Graphics::Layout::SR_DEFAULT_COLOR_FG, cx, cy, angle + 105.f, innerWidgetRadius, widgetRadius, &mBlend, mTrackSize);
					g.DrawRadialLine(SR::Graphics::Layout::SR_DEFAULT_COLOR_FG, cx, cy, angle + 135.f, innerWidgetRadius, widgetRadius, &mBlend, mTrackSize);
					g.DrawRadialLine(SR::Graphics::Layout::SR_DEFAULT_COLOR_FG, cx, cy, angle + 165.f, innerWidgetRadius, widgetRadius, &mBlend, mTrackSize);
					g.DrawRadialLine(SR::Graphics::Layout::SR_DEFAULT_COLOR_FG, cx, cy, angle - 165.f, innerWidgetRadius, widgetRadius, &mBlend, mTrackSize);
					g.DrawRadialLine(SR::Graphics::Layout::SR_DEFAULT_COLOR_FG, cx, cy, angle - 135.f, innerWidgetRadius, widgetRadius, &mBlend, mTrackSize);
					g.DrawRadialLine(SR::Graphics::Layout::SR_DEFAULT_COLOR_FG, cx, cy, angle - 105.f, innerWidgetRadius, widgetRadius, &mBlend, mTrackSize);
					g.DrawRadialLine(SR::Graphics::Layout::SR_DEFAULT_COLOR_FG, cx, cy, angle - 75.f, innerWidgetRadius, widgetRadius, &mBlend, mTrackSize);
					g.DrawRadialLine(SR::Graphics::Layout::SR_DEFAULT_COLOR_FG, cx, cy, angle - 45.f, innerWidgetRadius, widgetRadius, &mBlend, mTrackSize);
					g.DrawRadialLine(SR::Graphics::Layout::SR_DEFAULT_COLOR_FG, cx, cy, angle - 15.f, innerWidgetRadius, widgetRadius, &mBlend, mTrackSize);
					g.DrawArc(SR::Graphics::Layout::SR_DEFAULT_COLOR_FG, cx, cy, widgetRadius, angle + 15.f, angle + 45.f, &mBlend, mTrackSize);
					g.DrawArc(SR::Graphics::Layout::SR_DEFAULT_COLOR_FG, cx, cy, innerWidgetRadius, angle + 45.f, angle + 75.f, &mBlend, mTrackSize);
					g.DrawArc(SR::Graphics::Layout::SR_DEFAULT_COLOR_FG, cx, cy, widgetRadius, angle + 75.f, angle + 105.f, &mBlend, mTrackSize);
					g.DrawArc(SR::Graphics::Layout::SR_DEFAULT_COLOR_FG, cx, cy, innerWidgetRadius, angle + 105.f, angle + 135.f, &mBlend, mTrackSize);
					g.DrawArc(SR::Graphics::Layout::SR_DEFAULT_COLOR_FG, cx, cy, widgetRadius, angle + 135.f, angle + 165.f, &mBlend, mTrackSize);
					g.DrawArc(SR::Graphics::Layout::SR_DEFAULT_COLOR_FG, cx, cy, innerWidgetRadius, angle + 165.f, angle - 165.f, &mBlend, mTrackSize);
					g.DrawArc(SR::Graphics::Layout::SR_DEFAULT_COLOR_FG, cx, cy, widgetRadius, angle - 165.f, angle - 135.f, &mBlend, mTrackSize);
					g.DrawArc(SR::Graphics::Layout::SR_DEFAULT_COLOR_FG, cx, cy, innerWidgetRadius, angle - 135.f, angle - 105.f, &mBlend, mTrackSize);
					g.DrawArc(SR::Graphics::Layout::SR_DEFAULT_COLOR_FG, cx, cy, widgetRadius, angle - 105.f, angle - 75.f, &mBlend, mTrackSize);
					g.DrawArc(SR::Graphics::Layout::SR_DEFAULT_COLOR_FG, cx, cy, innerWidgetRadius, angle - 75.f, angle - 45.f, &mBlend, mTrackSize);
					g.DrawArc(SR::Graphics::Layout::SR_DEFAULT_COLOR_FG, cx, cy, widgetRadius, angle - 45.f, angle - 15.f, &mBlend, mTrackSize);
					g.DrawCircle(SR::Graphics::Layout::SR_DEFAULT_COLOR_FG, cx, cy, widgetRadius * mOuterPointerFrac, &mBlend, mTrackSize);
					g.DrawRadialLine(SR::Graphics::Layout::SR_DEFAULT_COLOR_FG, cx, cy, angle, mInnerPointerFrac * widgetRadius, mOuterPointerFrac * widgetRadius, &mBlend, mPointerThickness + 2.f * mTrackSize);
					g.DrawRadialLine(SR::Graphics::Layout::SR_DEFAULT_COLOR_CUSTOM_PANEL_BG, cx, cy, angle, mInnerPointerFrac * widgetRadius + mTrackSize, mOuterPointerFrac * widgetRadius + mTrackSize, &mBlend, mPointerThickness);
				}
				//DrawPressableShape(g, /*mShape*/ EVShape::Ellipse, knobHandleBounds, mMouseDown, mMouseIsOver, IsDisabled());
				//DrawPointer(g, angle, cx, cy, knobHandleBounds.W() / 2.f);
			}

		}
	}
}