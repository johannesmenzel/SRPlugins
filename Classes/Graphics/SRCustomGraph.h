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

            // TODO: Draw with PathConvexShape from ptr to member array updated from Updatefunction
            class SRGraphBase
                : public IControl
                , public IVectorBase
            {
            public:
                SRGraphBase(IRECT bounds, int numValues, float* values, const IVStyle& style = DEFAULT_STYLE)
                    : IControl(bounds, -1)
                    , IVectorBase(style)
                    , mValues(values)
                    , mNumValues(numValues)
                    , mX(new float[numValues])
                    , mY(new float[numValues])
                {
                    AttachIControl(this, ""); // TODO: Should hand label
                }

                ~SRGraphBase() {
                    //delete[] mX;
                    //delete[] mY;
                    //delete[] mValues;
                }

                void Draw(IGraphics& g) override {
                    // Fill Graph
                    g.PathClear();
                    g.PathMoveTo(mRECT.L, mRECT.MH());
                    for (int i = 0; i < mNumValues; i++) {
                        g.PathLineTo(mX[i], mY[i]);
                    }
                    g.PathLineTo(mRECT.R, mRECT.MH());
                    g.PathClose();
                    g.PathFill(GetColor(kHL));

                    // Draw Graph
                    g.PathClear();
                    g.PathMoveTo(mRECT.L, mRECT.MH());
                    for (int i = 0; i < mNumValues; i++) {
                        //if (mY[i] == mRECT.T && mY[i-1] == mRECT.T || mY[i] == mRECT.B && mY[i - 1] == mRECT.B)
                        //  g.PathMoveTo(mX[i], mY[i]);
                        //else
                        g.PathLineTo(mX[i], mY[i]);
                    }
                    g.PathLineTo(mRECT.R, mRECT.MH());
                    g.PathStroke(GetColor(kPR), mStyle.frameThickness);
                };

                void OnResize() override {
                    for (int i = 0; i < mNumValues; i++) {
                        mX[i] = mRECT.L + (mRECT.W() / (mNumValues - 1.f)) * float(i);
                        mY[i] = mRECT.MH() - (mRECT.H() * (float)mValues[i] * 0.5f);
                    }
                    SetDirty(false);
                }

                void Process(float* values) {
                    mValues = values;
                    for (int i = 0; i < mNumValues; i++) {
                        mValues[i] = Clip<float>(mValues[i], -1.f, 1.f);
                    }
                    OnResize();
                };
                //void OnMouseDown(float x, float y, const IMouseMod& mod) override;
            private:
                //WDL_String mDisp;
                float* mValues;
                int mNumValues;
                float* mX;
                float* mY;
            };


            // TODO: Draw with PathConvexShape from ptr to member array updated from Updatefunction
            class SRFrequencyResponseMeter
                : public IControl
                , public IVectorBase
            {
            public:
                SRFrequencyResponseMeter(IRECT bounds, int numValues, double* values, double shape = 1.0, const IVStyle& style = DEFAULT_STYLE)
                    : IControl(bounds, -1)
                    , IVectorBase(style)
                    , mValues(values)
                    , mNumValues(numValues)
                    , mShape(shape)
                    , mPatternFill(IPattern(EPatternType::Solid))
                    , mPatternStroke(IPattern(EPatternType::Solid))
                {
                    mStrokeOptions.mPreserve = true;
                    mFillOptions.mPreserve = true;
                    mPatternStroke = IPattern(GetColor(kFG));
                    mPatternFill = IPattern(GetColor(kHL));
                    AttachIControl(this, ""); // TODO: shoud hand label
                }

                ~SRFrequencyResponseMeter() {}

                void Draw(IGraphics& g) override {
                    g.PathClear();
                    g.PathMoveTo(mRECT.L, mRECT.MH());

                    for (int i = 0; i < mNumValues; i++) {
                        const float y = mRECT.MH() - ((float)mValues[i] * 0.5f * mRECT.H());
                        const float x = mRECT.L + ((float)i / ((float)mNumValues - 1.f)) * mRECT.W();
                        g.PathLineTo(x, y);
                    }

                    g.PathLineTo(mRECT.R, mRECT.MH());
                    //g.PathClose();
                    g.PathFill(mPatternFill, mFillOptions, 0);
                    g.PathStroke(mPatternStroke, 1.f, mStrokeOptions, 0);
                };

                void Process(double* values) {
                    mValues = values;
                    for (int i = 0; i < mNumValues; i++) {
                        mValues[i] = Clip<double>(mValues[i], -1, 1);
                    }
                    SetDirty(false);
                };
                //void OnMouseDown(float x, float y, const IMouseMod& mod) override;
            private:
                //WDL_String mDisp;
                double* mValues;
                int mNumValues;
                double mShape;
                IStrokeOptions mStrokeOptions;
                IFillOptions mFillOptions;
                IPattern mPatternFill;
                IPattern mPatternStroke;
            };

		}
	}
}