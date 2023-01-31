//  SRGain.h

// Implementation:
//   Note that you might need a class for each channel
//   (so 2 for stereo processing.)
//
// Header:
//   private:
//     Impementation as object:
//       SRPan name;
//     Implementation as pointer:
//       SRPan *name = new SRPan();
//
// Class: Constructor, Reset()
//     Implementation as object:
//       name.setClass(pType, pVar1, pVar2, pVar3);
//     Implementation as pointer:
//       name->setClass(pType, pVar1, pVar2, pVar3);
//
// Class: ProcessDoubleReplacing()
//   Per sample and channel:
//     Implementation as object:
//       *out1 = name.process(*in1);
//     Implementation as pointer:
//       *out1 = name->process(*in1);

#pragma once

// #include this and that
#include "../Utils/SRHelpers.h" // optional
#include "../Utils/SRParam.h" // param smoother
#include <cassert>

namespace SR
{
	namespace DSP
	{


		class SRGain
		{
		public:
			enum PanType
			{
				kLinear = 0,
				kSquareroot,
				kSinusodial,
				kTanh,
				// ...
				kNumTypes
			};

			// Constructors & Destructor
			SRGain(
				int pRampNumSamples = 0,
				PanType pType = kLinear,
				double pPanNormalized = 0.5,
				bool pLinearMiddlePosition = true,
				double pWidthNormalized = 1.0,
				bool pBypassed = false
			)
				: mGainLin(1.0)
				, mRampNumSamples(1)
				, mBypassed(false)
				, mPanType(kLinear)
				, mPanNormalized(0.5)
				, mLinearMiddlePosition(true)
				, mWidthNormalized(1.0)
				, mMidCoeff(0.5)
				, mSideCoeff(0.5)
			{
				InitGain(pRampNumSamples, pType, pPanNormalized, pLinearMiddlePosition, pWidthNormalized, pBypassed);
			}
			~SRGain() {}

			void InitGain(int pRampNumSamples = 0
				, PanType pType = kLinear
				, double pPanNormalized = 0.5
				, bool pLinearMiddlePosition = true
				, double pWidthNormalized = 1.0
				, bool pBypassed = false) 
			{
				mGainLin = 1.0;
				mRampNumSamples = (pRampNumSamples < 1) ? 1 : pRampNumSamples;
				mBypassed = pBypassed;
				mPanType = pType;
				mPanNormalized = pPanNormalized;
				mWidthNormalized = pWidthNormalized;
				mLinearMiddlePosition = pLinearMiddlePosition;
				mGainRamp[0].SetStrict(mGainLin);
				mGainRamp[1].SetStrict(mGainLin);
				mGainRamp[0].SetNumSmoothSamples(mRampNumSamples);
				mGainRamp[1].SetNumSmoothSamples(mRampNumSamples);
				update();
			}

			// Set normalized gain with linear voltage value (>= 0.0; 1.0 = unity)
			void SetGainLin(double pGainLin) { mGainLin = pGainLin; update(); }
			// Set gain with dB value
			void SetGainDb(double pGainDb) { SetGainLin(SR::Utils::DBToAmp(pGainDb)); /* don't update because SetGain does */ }
			// Set algorithm for panning
			void SetPanType(PanType pType) { mPanType = pType; update(); }
			// If true, gain is at unity in center position and may differ in other positions
			void SetPanLinearMiddlePosition(bool pLinearMiddlePosition) { mLinearMiddlePosition = pLinearMiddlePosition; update(); }
			// Set normalized panning position 0 .. 1, while .5 is middle position
			void SetPanPosition(double pPanNormalized) { mPanNormalized = pPanNormalized; update(); }
			// Set normalized stereo width (0. = mono; 1. = stereo; > 1 = wider than stereo
			void SetWidth(double pWidthNormalized) {
				mWidthNormalized = pWidthNormalized;
				const double tmp = 1. / std::max(1. + mWidthNormalized, 2.);
				mMidCoeff = 1. * tmp;
				mSideCoeff = mWidthNormalized * tmp;
				update();
			}
			// Set gain computer bypassed
			void SetBypassed(bool pBypassed) { mBypassed = pBypassed; update(); }
			// Set number of samples until gain smoothing ramp reaches target value
			void SetRamp(int pRampNumSamples) { mRampNumSamples = (pRampNumSamples < 1) ? 1 : pRampNumSamples; update(); }


			// Get normalized gain with linear voltage value (>= 0.0; 1.0 = unity)
			double GetGainLin() { return mGainLin; }
			// Get gain with dB value
			double GetGainDb() { return SR::Utils::AmpToDB(mGainLin); } // Get current gain value (decibels)
			// Get current linear gain voltage at channel [0] or [1]
			double GetCurrentGainLin(int channel) { return mGainRamp[channel].Get(); }
			// Get algorithm for panning
			PanType GetPanType() { return mPanType; }
			// Get normalized panning position 0 .. 1, while .5 is middle position
			double GetPanPosition() { return mPanNormalized; }
			// Get normalized stereo width (0. = mono; 1. = stereo; > 1 = wider than stereo
			double GetWidth() { return mWidthNormalized; }
			// Get if gain computer is bypassed
			bool GetBypassed() { return mBypassed; } // Get if currently bypassed
			// Get number of samples until gain smoothing ramp reaches target value
			int GetRamp() { return mRampNumSamples; } // Get number of samples until smoother reaches target value

			// Runtime method
			void Process(double& in1, double& in2); // for stereo

		protected:
			// Update gain values after any value changed
			void update(void) {
				double gain1 = mGainLin;
				double gain2 = mGainLin;

				// Pan position calculation
				if (mPanNormalized != 0.5) {

					switch (mPanType) {
					case kLinear:
						if (mLinearMiddlePosition == true) {
							gain1 = (mPanNormalized > 0.5) ? (1. - mPanNormalized) * 2 : 1.0;
							gain2 = (mPanNormalized < 0.5) ? mPanNormalized * 2 : 1.0;
						}
						else {
							gain1 = (1. - mPanNormalized);
							gain2 = (mPanNormalized);
						}
						break;
					case kSquareroot:
						if (mLinearMiddlePosition == true) {
							gain1 = (mPanNormalized > 0.5) ? sqrt(mPanNormalized) * sqrt(2.) : 1.0;
							gain2 = (mPanNormalized < 0.5) ? sqrt(1 - mPanNormalized) * sqrt(2.) : 1.0;
						}
						else {
							gain1 = sqrt(1. - mPanNormalized);
							gain2 = sqrt(mPanNormalized);
						}
						break;
					case kSinusodial:
						if (mLinearMiddlePosition == true) {
							gain1 = (mPanNormalized > 0.5) ? sin(2. * mPanNormalized * M_PI_2) : 1.0;
							gain2 = (mPanNormalized < 0.5) ? cos((1. - 2. * mPanNormalized) * M_PI_2) : 1.0;
						}
						else {
							gain1 = sin(mPanNormalized * M_PI_2);
							gain2 = cos(mPanNormalized * M_PI_2);
						}
						break;
					case kTanh:
						gain1 = (mPanNormalized > 0.5) ? tanh(4. * (1 - mPanNormalized)) / tanh(2.) : 1.0; // you can scale it by replacing 4. and 2. by 2n and n (10. and 5.)
						gain2 = (mPanNormalized < 0.5) ? tanh(4. * mPanNormalized) / tanh(2.) : 1.0;
						break;
					default:
						break;
					}
				}

				mGainRamp[0].Set(gain1);
				mGainRamp[1].Set(gain2);
			}

			// Gain members
			double mGainLin;

			// Pan members
			PanType mPanType;
			double mPanNormalized; // Enter normalized pan between 0.0 (left) and 1.0 (right)
			bool mLinearMiddlePosition; // Center volume doens't change if true

			// Width members
			double mWidthNormalized; // Normalized width where 0 = mono, 1 = normal stereo, > 1 = wider
			double mMidCoeff, mSideCoeff;

			// Global members
			SRParamSmoothRamp mGainRamp[2]; // Holds gain smoothing functionality
			int mRampNumSamples; // Enter lenght of gain ramp in samples
			bool mBypassed; // bool is gain bypassed
		};



		inline void SRGain::Process(double& in1, double& in2)
		{
			if (!mBypassed) {
				// Update values when smoothing
				mGainRamp[0].Process();
				mGainRamp[1].Process();

				// width per sample
				double mid = (in1 + in2) * mMidCoeff;
				double side = (in2 - in1) * mSideCoeff;

				// Get processed signal from mid-side
				in1 = mid - side;
				in2 = mid + side;

				// Apply final gain
				in1 *= mGainRamp[0].Get();
				in2 *= mGainRamp[1].Get();
			}
		}

	} // namespace SRGain
} // namespace SR