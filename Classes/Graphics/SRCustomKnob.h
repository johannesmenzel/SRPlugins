#pragma once
#include "IPlug_include_in_plug_hdr.h"
#include "IPlug_include_in_plug_src.h"
#include "IControl.h"
#include "IControls.h"
using namespace iplug;
using namespace igraphics;



class Knob 
	: public IVKnobControl {
public:
	Knob(const IRECT& bounds, int paramIdx)
		: IVKnobControl(bounds, paramIdx) {

	};
	virtual ~Knob(){};

	void DrawWidget(IGraphics& g) override;
};

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
		g.DrawArc(GetColor(kFG), cx, cy, widgetRadius * .95f, angle - 15.f, angle + 15.f, &mBlend, mTrackSize);
		g.DrawArc(GetColor(kFG), cx, cy, widgetRadius, angle + 15.f, angle + 45.f, &mBlend, mTrackSize);
		g.DrawArc(GetColor(kFG), cx, cy, widgetRadius * .95f, angle + 45.f, angle + 75.f, &mBlend, mTrackSize);
		g.DrawArc(GetColor(kFG), cx, cy, widgetRadius, angle + 75.f, angle + 105.f, &mBlend, mTrackSize);
		g.DrawArc(GetColor(kFG), cx, cy, widgetRadius * .95f, angle + 105.f, angle + 135.f, &mBlend, mTrackSize);
		g.DrawArc(GetColor(kFG), cx, cy, widgetRadius, angle + 135.f, angle + 165.f, &mBlend, mTrackSize);
		g.DrawArc(GetColor(kFG), cx, cy, widgetRadius * .95f, angle + 165.f, angle -165.f, &mBlend, mTrackSize);
		g.DrawArc(GetColor(kFG), cx, cy, widgetRadius, angle - 165.f, angle -135.f, &mBlend, mTrackSize);
		g.DrawArc(GetColor(kFG), cx, cy, widgetRadius * .95f, angle - 135.f, angle - 105.f, &mBlend, mTrackSize);
		g.DrawArc(GetColor(kFG), cx, cy, widgetRadius, angle - 105.f, angle - 75.f, &mBlend, mTrackSize);
		g.DrawArc(GetColor(kFG), cx, cy, widgetRadius * .95f, angle - 75.f, angle - 45.f, &mBlend, mTrackSize);
		g.DrawArc(GetColor(kFG), cx, cy, widgetRadius, angle - 45.f, angle - 15.f, &mBlend, mTrackSize);
		g.DrawCircle(GetColor(kFG), cx, cy, widgetRadius * .8f, &mBlend, mTrackSize);
		g.DrawRadialLine(GetColor(kFG), cx, cy, angle, 0.f, mOuterPointerFrac * widgetRadius * .8f, &mBlend, mPointerThickness * 3.f);
		g.DrawRadialLine(GetColor(kX1), cx, cy, angle, mInnerPointerFrac * widgetRadius, mOuterPointerFrac * widgetRadius * .8f, &mBlend, mPointerThickness * 1.f);

	}
	//DrawPressableShape(g, /*mShape*/ EVShape::Ellipse, knobHandleBounds, mMouseDown, mMouseIsOver, IsDisabled());
	//DrawPointer(g, angle, cx, cy, knobHandleBounds.W() / 2.f);

}
