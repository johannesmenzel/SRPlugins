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
					float trackSize = 2.f, float pointerThickness = 3.f)
					: IVKnobControl(bounds, paramIdx, label, style, valueIsEditable, valueInWidget, a1, a2, aAnchor, direction, gearing, trackSize)
				{
					SetPointerThickness(pointerThickness);
				};

				virtual ~Knob() {};

				void DrawWidget(IGraphics& g) override {
					const float widgetRadius = mWidgetBounds.GetLengthOfShortestSide() * .5f - (mTrackSize * .5f); // The radius out to the indicator track arc
					const float widgetRadius92 = widgetRadius * .92f;
					const float widgetRadius75 = widgetRadius * .75f;
					const float cx = mWidgetBounds.MW(), cy = mWidgetBounds.MH();
					const float angle = mAngle1 + (static_cast<float>(GetValue()) * (mAngle2 - mAngle1));
					const IColor color = (mMouseIsOver) ? SR::Graphics::Layout::SR_DEFAULT_COLOR_X1 : SR::Graphics::Layout::SR_DEFAULT_COLOR_FG;
					if (mTrackSize > 0.f) {
						g.DrawArc(color, cx, cy, widgetRadius92, angle - 15.f, angle + 15.f, &mBlend, mTrackSize);
						g.DrawRadialLine(color, cx, cy, angle + 15.f, widgetRadius92, widgetRadius, &mBlend, mTrackSize);
						g.DrawRadialLine(color, cx, cy, angle + 45.f, widgetRadius92, widgetRadius, &mBlend, mTrackSize);
						g.DrawRadialLine(color, cx, cy, angle + 75.f, widgetRadius92, widgetRadius, &mBlend, mTrackSize);
						g.DrawRadialLine(color, cx, cy, angle + 105.f, widgetRadius92, widgetRadius, &mBlend, mTrackSize);
						g.DrawRadialLine(color, cx, cy, angle + 135.f, widgetRadius92, widgetRadius, &mBlend, mTrackSize);
						g.DrawRadialLine(color, cx, cy, angle + 165.f, widgetRadius92, widgetRadius, &mBlend, mTrackSize);
						g.DrawRadialLine(color, cx, cy, angle - 165.f, widgetRadius92, widgetRadius, &mBlend, mTrackSize);
						g.DrawRadialLine(color, cx, cy, angle - 135.f, widgetRadius92, widgetRadius, &mBlend, mTrackSize);
						g.DrawRadialLine(color, cx, cy, angle - 105.f, widgetRadius92, widgetRadius, &mBlend, mTrackSize);
						g.DrawRadialLine(color, cx, cy, angle - 75.f, widgetRadius92, widgetRadius, &mBlend, mTrackSize);
						g.DrawRadialLine(color, cx, cy, angle - 45.f, widgetRadius92, widgetRadius, &mBlend, mTrackSize);
						g.DrawRadialLine(color, cx, cy, angle - 15.f, widgetRadius92, widgetRadius, &mBlend, mTrackSize);
						g.DrawArc(color, cx, cy, widgetRadius, angle + 15.f, angle + 45.f, &mBlend, mTrackSize);
						g.DrawArc(color, cx, cy, widgetRadius92, angle + 45.f, angle + 75.f, &mBlend, mTrackSize);
						g.DrawArc(color, cx, cy, widgetRadius, angle + 75.f, angle + 105.f, &mBlend, mTrackSize);
						g.DrawArc(color, cx, cy, widgetRadius92, angle + 105.f, angle + 135.f, &mBlend, mTrackSize);
						g.DrawArc(color, cx, cy, widgetRadius, angle + 135.f, angle + 165.f, &mBlend, mTrackSize);
						g.DrawArc(color, cx, cy, widgetRadius92, angle + 165.f, angle - 165.f, &mBlend, mTrackSize);
						g.DrawArc(color, cx, cy, widgetRadius, angle - 165.f, angle - 135.f, &mBlend, mTrackSize);
						g.DrawArc(color, cx, cy, widgetRadius92, angle - 135.f, angle - 105.f, &mBlend, mTrackSize);
						g.DrawArc(color, cx, cy, widgetRadius, angle - 105.f, angle - 75.f, &mBlend, mTrackSize);
						g.DrawArc(color, cx, cy, widgetRadius92, angle - 75.f, angle - 45.f, &mBlend, mTrackSize);
						g.DrawArc(color, cx, cy, widgetRadius, angle - 45.f, angle - 15.f, &mBlend, mTrackSize);
						g.DrawCircle(color, cx, cy, widgetRadius75, &mBlend, mTrackSize);
						g.DrawRadialLine(color, cx, cy, angle, 0.f, widgetRadius75, &mBlend, mPointerThickness + 2.f * mTrackSize);
						g.DrawRadialLine(SR::Graphics::Layout::SR_DEFAULT_COLOR_CUSTOM_PANEL_BG, cx, cy, angle, 0. + mTrackSize, widgetRadius75 + mTrackSize, &mBlend, mPointerThickness);
					}
				};
			};


			class KnobChicken : public Knob {
			public:
				KnobChicken(const IRECT& bounds, int paramIdx,
					const char* label = "",
					const IVStyle& style = SR::Graphics::Layout::SR_DEFAULT_STYLE,
					bool valueIsEditable = false, bool valueInWidget = false,
					float a1 = -135.f, float a2 = 135.f, float aAnchor = -135.f,
					EDirection direction = EDirection::Vertical, double gearing = DEFAULT_GEARING,
					float trackSize = 2.f, float pointerThickness = 3.f)
					: Knob(bounds, paramIdx, label, style, valueIsEditable, valueInWidget, a1, a2, aAnchor, direction, gearing, trackSize, pointerThickness)
				{
					SetPointerThickness(pointerThickness);
				};

				virtual ~KnobChicken() {};

				void DrawWidget(IGraphics& g) override {
					const float widgetRadius = mWidgetBounds.GetLengthOfShortestSide() * .5f - (mTrackSize * .5f); // The radius out to the indicator track arc
					const float widgetRadius50 = widgetRadius * .5f;
					const float widgetRadius55 = widgetRadius * .55f;
					const float widgetRadius75 = widgetRadius * .75f;
					const float cx = mWidgetBounds.MW(), cy = mWidgetBounds.MH();
					const float angle = mAngle1 + (static_cast<float>(GetValue()) * (mAngle2 - mAngle1));
					const IColor color = (mMouseIsOver) ? SR::Graphics::Layout::SR_DEFAULT_COLOR_X1 : SR::Graphics::Layout::SR_DEFAULT_COLOR_FG;
					if (mTrackSize > 0.f) {
						g.DrawRadialLine(color, cx, cy, angle + 75.f, widgetRadius50, widgetRadius55, &mBlend, mTrackSize);
						g.DrawRadialLine(color, cx, cy, angle + 105.f, widgetRadius50, widgetRadius55, &mBlend, mTrackSize);
						g.DrawRadialLine(color, cx, cy, angle + 135.f, widgetRadius50, widgetRadius55, &mBlend, mTrackSize);
						g.DrawRadialLine(color, cx, cy, angle + 150.f, widgetRadius50, widgetRadius, &mBlend, mTrackSize);
						g.DrawRadialLine(color, cx, cy, angle - 150.f, widgetRadius50, widgetRadius, &mBlend, mTrackSize);
						g.DrawRadialLine(color, cx, cy, angle - 135.f, widgetRadius50, widgetRadius55, &mBlend, mTrackSize);
						g.DrawRadialLine(color, cx, cy, angle - 105.f, widgetRadius50, widgetRadius55, &mBlend, mTrackSize);
						g.DrawRadialLine(color, cx, cy, angle - 75.f, widgetRadius50, widgetRadius55, &mBlend, mTrackSize);
						g.DrawRadialLine(color, cx, cy, angle - 45.f, widgetRadius50, widgetRadius55, &mBlend, mTrackSize);
						g.DrawArc(color, cx, cy, widgetRadius55, angle + 30.f, angle + 45.f, &mBlend, mTrackSize);
						g.DrawArc(color, cx, cy, widgetRadius50, angle + 45.f, angle + 75.f, &mBlend, mTrackSize);
						g.DrawArc(color, cx, cy, widgetRadius55, angle + 75.f, angle + 105.f, &mBlend, mTrackSize);
						g.DrawArc(color, cx, cy, widgetRadius50, angle + 105.f, angle + 135.f, &mBlend, mTrackSize);
						g.DrawArc(color, cx, cy, widgetRadius55, angle + 135.f, angle + 150.f, &mBlend, mTrackSize);
						g.DrawArc(color, cx, cy, widgetRadius55, angle - 150.f, angle - 135.f, &mBlend, mTrackSize);
						g.DrawArc(color, cx, cy, widgetRadius50, angle - 135.f, angle - 105.f, &mBlend, mTrackSize);
						g.DrawArc(color, cx, cy, widgetRadius55, angle - 105.f, angle - 75.f, &mBlend, mTrackSize);
						g.DrawArc(color, cx, cy, widgetRadius50, angle - 75.f, angle - 45.f, &mBlend, mTrackSize);
						g.DrawArc(color, cx, cy, widgetRadius55, angle - 45.f, angle - 30.f, &mBlend, mTrackSize);
						g.DrawArc(color, cx, cy, widgetRadius75, angle + 10.f, angle + 150.f, &mBlend, mTrackSize);
						g.DrawArc(color, cx, cy, widgetRadius75, angle - 150.f, angle - 10.f, &mBlend, mTrackSize);
						g.DrawArc(color, cx, cy, widgetRadius, angle + 150.f, angle - 150.f, &mBlend, mTrackSize);
						g.DrawLine(color,
							cx + widgetRadius50 * std::cos(DegToRad(angle - 90.f - 30.f)),
							cy + widgetRadius50 * std::sin(DegToRad(angle - 90.f - 30.f)),
							cx + widgetRadius * std::cos(DegToRad(angle - 90.f - 5.f)),
							cy + widgetRadius * std::sin(DegToRad(angle - 90.f - 5.f)),
							&mBlend, mTrackSize);
						g.DrawLine(color,
							cx + widgetRadius50 * std::cos(DegToRad(angle - 90.f + 30.f)),
							cy + widgetRadius50 * std::sin(DegToRad(angle - 90.f + 30.f)),
							cx + widgetRadius * std::cos(DegToRad(angle - 90.f + 5.f)),
							cy + widgetRadius * std::sin(DegToRad(angle - 90.f + 5.f)),
							&mBlend, mTrackSize);
						g.DrawRadialLine(color, cx, cy, angle, 0.f, widgetRadius, &mBlend, mPointerThickness + 2.f * mTrackSize);
						g.DrawRadialLine(SR::Graphics::Layout::SR_DEFAULT_COLOR_CUSTOM_PANEL_BG, cx, cy, angle, 0.f + mTrackSize, widgetRadius - mTrackSize, &mBlend, mPointerThickness);
						g.DrawArc(color, cx, cy, widgetRadius, angle - 5.f, angle + 5.f, &mBlend, mTrackSize);
					}
				};
			};

			class KnobPassive : public Knob {
			public:
				KnobPassive(const IRECT& bounds, int paramIdx,
					const char* label = "",
					const IVStyle& style = SR::Graphics::Layout::SR_DEFAULT_STYLE,
					bool valueIsEditable = false, bool valueInWidget = false,
					float a1 = -135.f, float a2 = 135.f, float aAnchor = -135.f,
					EDirection direction = EDirection::Vertical, double gearing = DEFAULT_GEARING,
					float trackSize = 2.f, float pointerThickness = 3.f)
					: Knob(bounds, paramIdx, label, style, valueIsEditable, valueInWidget, a1, a2, aAnchor, direction, gearing, trackSize, pointerThickness)
				{
					SetPointerThickness(pointerThickness);
				};

				virtual ~KnobPassive() {};

				void DrawWidget(IGraphics& g) override {
					const float widgetRadius = mWidgetBounds.GetLengthOfShortestSide() * .5f - (mTrackSize * .5f); // The radius out to the indicator track arc
					const float widgetRadius1 = widgetRadius * .75f;
					const float widgetRadius2 = widgetRadius * .67f;
					const float widgetRadius3 = widgetRadius * .50f;
					const float widgetRadius4 = widgetRadius * .40f;
					const float cx = mWidgetBounds.MW(), cy = mWidgetBounds.MH();
					const float angle = mAngle1 + (static_cast<float>(GetValue()) * (mAngle2 - mAngle1));
					const IColor color = (mMouseIsOver) ? SR::Graphics::Layout::SR_DEFAULT_COLOR_X1 : SR::Graphics::Layout::SR_DEFAULT_COLOR_FG;
					if (mTrackSize > 0.f) {
						g.DrawArc(color, cx, cy, widgetRadius2, angle - 15.f, angle + 15.f, &mBlend, mTrackSize);
						g.DrawRadialLine(color, cx, cy, angle + 15.f, widgetRadius2, widgetRadius1, &mBlend, mTrackSize);
						g.DrawRadialLine(color, cx, cy, angle + 45.f, widgetRadius2, widgetRadius1, &mBlend, mTrackSize);
						g.DrawRadialLine(color, cx, cy, angle + 75.f, widgetRadius2, widgetRadius1, &mBlend, mTrackSize);
						g.DrawRadialLine(color, cx, cy, angle + 105.f, widgetRadius2, widgetRadius1, &mBlend, mTrackSize);
						g.DrawRadialLine(color, cx, cy, angle + 135.f, widgetRadius2, widgetRadius1, &mBlend, mTrackSize);
						g.DrawRadialLine(color, cx, cy, angle + 165.f, widgetRadius2, widgetRadius1, &mBlend, mTrackSize);
						g.DrawRadialLine(color, cx, cy, angle - 165.f, widgetRadius2, widgetRadius1, &mBlend, mTrackSize);
						g.DrawRadialLine(color, cx, cy, angle - 135.f, widgetRadius2, widgetRadius1, &mBlend, mTrackSize);
						g.DrawRadialLine(color, cx, cy, angle - 105.f, widgetRadius2, widgetRadius1, &mBlend, mTrackSize);
						g.DrawRadialLine(color, cx, cy, angle - 75.f, widgetRadius2, widgetRadius1, &mBlend, mTrackSize);
						g.DrawRadialLine(color, cx, cy, angle - 45.f, widgetRadius2, widgetRadius1, &mBlend, mTrackSize);
						g.DrawRadialLine(color, cx, cy, angle - 15.f, widgetRadius2, widgetRadius1, &mBlend, mTrackSize);
						g.DrawArc(color, cx, cy, widgetRadius1, angle + 15.f, angle + 45.f, &mBlend, mTrackSize);
						g.DrawArc(color, cx, cy, widgetRadius2, angle + 45.f, angle + 75.f, &mBlend, mTrackSize);
						g.DrawArc(color, cx, cy, widgetRadius1, angle + 75.f, angle + 105.f, &mBlend, mTrackSize);
						g.DrawArc(color, cx, cy, widgetRadius2, angle + 105.f, angle + 135.f, &mBlend, mTrackSize);
						g.DrawArc(color, cx, cy, widgetRadius1, angle + 135.f, angle + 165.f, &mBlend, mTrackSize);
						g.DrawArc(color, cx, cy, widgetRadius2, angle + 165.f, angle - 165.f, &mBlend, mTrackSize);
						g.DrawArc(color, cx, cy, widgetRadius1, angle - 165.f, angle - 135.f, &mBlend, mTrackSize);
						g.DrawArc(color, cx, cy, widgetRadius2, angle - 135.f, angle - 105.f, &mBlend, mTrackSize);
						g.DrawArc(color, cx, cy, widgetRadius1, angle - 105.f, angle - 75.f, &mBlend, mTrackSize);
						g.DrawArc(color, cx, cy, widgetRadius2, angle - 75.f, angle - 45.f, &mBlend, mTrackSize);
						g.DrawArc(color, cx, cy, widgetRadius1, angle - 45.f, angle - 15.f, &mBlend, mTrackSize);
						g.DrawRadialLine(color, cx, cy, angle, widgetRadius2, widgetRadius, &mBlend, mPointerThickness + 2.f * mTrackSize);
						g.DrawRadialLine(SR::Graphics::Layout::SR_DEFAULT_COLOR_CUSTOM_PANEL_BG, cx, cy, angle, widgetRadius2 + mTrackSize, widgetRadius + mTrackSize, &mBlend, mPointerThickness);
						g.DrawCircle(color, cx, cy, widgetRadius3, &mBlend, mTrackSize);
						g.DrawCircle(color, cx, cy, widgetRadius4, &mBlend, mTrackSize);
						g.DrawCircle(color, cx, cy, widgetRadius, &mBlend, mTrackSize);
					}
				};
			};
		}
	}
}
