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
				/** SRGraphBase Constructor
				* @param bounds The rectangular area that the control occupies
				* @param numValues Number of values in the value array given to the control 
				* @param values A pointer type float which will hold the values
				* @param baseline value on Y axis considered as the base value between 0.f and 1.f 
				* @param style Style used for drawing that control */
				SRGraphBase(IRECT bounds, int numValues, float* values, float baseline = .5f, const IVStyle& style = DEFAULT_STYLE)
					: IControl(bounds, -1)
					, IVectorBase(style)
					, mBaseline(baseline)
					, mNumValues(numValues)
					, mValues(numValues, .0)
					, mX(numValues, .0)
					, mY(numValues, .0)
					, mPatternFill(EPatternType::Linear)
				{
					mPatternFill = IPattern::CreateLinearGradient(bounds, EDirection::Vertical, {IColorStop(GetColor(kX1), 0.f), IColorStop(GetColor(kHL), 1.f)});
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
					g.PathMoveTo(mRECT.L, (mRECT.B - mRECT.H() * mBaseline));
					for (int i = 0; i < mNumValues; i++) {
						g.PathLineTo(mX[i], mY[i]);
					}
					g.PathLineTo(mRECT.R, (mRECT.B - mRECT.H() * mBaseline));
					g.PathClose();
					// Added gradient, still unsure if I like it. Otherwise delete all lines containing mPatternFill and revert to just PathFill(kHL) like below
					//g.PathFill(GetColor(kHL));
					g.PathFill(mPatternFill);

					// Draw Graph
					g.PathClear();
					// -- For stroke move to first value and create path only if values are within bounds to prevent drawing at the edges
					g.PathMoveTo(mX[0], mY[0]);
					for (int i = 1; i < mNumValues; i++) {
						if (mY[i] >= mRECT.B || mY[i] <= mRECT.T) {
							g.PathMoveTo(mX[i], mY[i]);
						}
						else {
							g.PathLineTo(mX[i], mY[i]);
						}
					}
					g.PathStroke(GetColor(kFG), mStyle.frameThickness);
				};

				void OnResize() override {
					for (int i = 0; i < mNumValues; i++) {
						mX[i] = mRECT.L + (mRECT.W() / (mNumValues - 1.f)) * float(i);
						mY[i] = mRECT.MH() - (mRECT.H() * (float)mValues[i] * 0.5f);
					}
					SetDirty(false);
				}

				void Process(float* values) {
					for (int i = 0; i < mNumValues; i++)
						mValues.at(i) = Clip<float>(values[i], -1.f, 1.f);
					OnResize();
				};

				//void OnMouseDown(float x, float y, const IMouseMod& mod) override;

			private:
				//WDL_String mDisp;
				int mNumValues;
				float mBaseline;
				IPattern mPatternFill;
				std::vector<float> mValues;
				std::vector<float> mX;
				std::vector<float> mY;
				//float* mValues;
				//float* mX;
				//float* mY;
			};


			//// TODO: Draw with PathConvexShape from ptr to member array updated from Updatefunction
			//class SRFrequencyResponseMeter
			//	: public IControl
			//	, public IVectorBase
			//{
			//public:
			//	SRFrequencyResponseMeter(IRECT bounds, int numValues, double* values, double shape = 1.0, const IVStyle& style = DEFAULT_STYLE)
			//		: IControl(bounds, -1)
			//		, IVectorBase(style)
			//		, mValues(values)
			//		, mNumValues(numValues)
			//		, mShape(shape)
			//		, mPatternFill(IPattern(EPatternType::Solid))
			//		, mPatternStroke(IPattern(EPatternType::Solid))
			//	{
			//		mStrokeOptions.mPreserve = true;
			//		mFillOptions.mPreserve = true;
			//		mPatternStroke = IPattern(GetColor(kFG));
			//		mPatternFill = IPattern(GetColor(kHL));
			//		AttachIControl(this, ""); // TODO: shoud hand label
			//	}

			//	~SRFrequencyResponseMeter() {}

			//	void Draw(IGraphics& g) override {
			//		g.PathClear();
			//		g.PathMoveTo(mRECT.L, mRECT.MH());

			//		for (int i = 0; i < mNumValues; i++) {
			//			const float y = mRECT.MH() - ((float)mValues[i] * 0.5f * mRECT.H());
			//			const float x = mRECT.L + ((float)i / ((float)mNumValues - 1.f)) * mRECT.W();
			//			g.PathLineTo(x, y);
			//		}

			//		g.PathLineTo(mRECT.R, mRECT.MH());
			//		//g.PathClose();
			//		g.PathFill(mPatternFill, mFillOptions, 0);
			//		g.PathStroke(mPatternStroke, 1.f, mStrokeOptions, 0);
			//	};

			//	void Process(double* values) {
			//		mValues = values;
			//		for (int i = 0; i < mNumValues; i++) {
			//			mValues[i] = Clip<double>(mValues[i], -1.f, 1.f);
			//		}
			//		SetDirty(false);
			//	};
			//	//void OnMouseDown(float x, float y, const IMouseMod& mod) override;
			//private:
			//	//WDL_String mDisp;
			//	double* mValues;
			//	int mNumValues;
			//	double mShape;
			//	IStrokeOptions mStrokeOptions;
			//	IFillOptions mFillOptions;
			//	IPattern mPatternFill;
			//	IPattern mPatternStroke;
			//};

		}
	}
}