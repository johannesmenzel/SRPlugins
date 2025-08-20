//  SRGain.h

#pragma once

// #include this and that
#include "../Utils/SRHelpers.h" // optional
#include "../Utils/SRParam.h" // param smoother
#include <cassert>

namespace SR {
	namespace DSP {

		/** A gain class with panning, stereo width and mix */
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

			/** Construct gain class
			* @param pRampNumSamples Parameter smoothing ramp in number of samples
			* @param pPanType Panning algorithm
			* @param pLinearInMiddlePosition Adjust panning algorithm for unity gain in middle position	*/
			SRGain(int pRampNumSamples = 100,
				PanType pPanType = kLinear,
				bool pLinearInMiddlePosition = true
			)
				: mRampNumSamples(pRampNumSamples)
				, mPanType(pPanType)
				, mLinearInMiddlePosition(pLinearInMiddlePosition)
				, mBypassed(false)
				, mWidthNormalized(1.)
				, mGainLin(1.)
				, mPanNormalized(.5)
				, mMidCoeff(.5)
				, mSideCoeff(.5)
			{
				Reset(mGainLin, mPanNormalized, mWidthNormalized, mBypassed, mRampNumSamples, mPanType, mLinearInMiddlePosition);
			}
			~SRGain() {}

			/** Reset Gain class, typically in OnReset() to set strict values to skip parameter smoothing on reset */
			void Reset() {
				mGainRamp[0].SetStrict(mGainLin);
				mGainRamp[1].SetStrict(mGainLin);
				update();
			};
			/** Reset gain class, typically in OnReset() to set strict values to skip parameter smoothing on reset
			* @param pPanNormalized Panning in normalized value (0.-1.), while .5 is centered 
			* @param pWidthNormalized Stereo width in normalized value (<= 0.), while 0. = mono; 1. = stereo; >1. wider
			* @param pBypassed Disable processing
			* @param pRampNumSamples Parameter smoothing ramp in number of samples
			* @param pPanType Panning algorithm
			* @param pLinearInMiddlePosition Adjust panning algorithm for unity gain in middle position */
			void Reset(double pGainLin
				, double pPanNormalized
				, double pWidthNormalized
				, bool pBypassed
				, int pRampNumSamples
				, PanType pPanType
				, bool pLinearInMiddlePosition)
			{
				mGainLin = pGainLin;
				mPanNormalized = pPanNormalized;
				mWidthNormalized = pWidthNormalized;
				SetWidth(mWidthNormalized);
				mRampNumSamples = (pRampNumSamples < 1) ? 1 : pRampNumSamples;
				mBypassed = pBypassed;
				mPanType = pPanType;
				mLinearInMiddlePosition = pLinearInMiddlePosition;
				mGainRamp[0].SetNumSmoothSamples(mRampNumSamples);
				mGainRamp[1].SetNumSmoothSamples(mRampNumSamples);
				Reset(); // Also holds necessary update()
			}

			/** Set normalized gain with linear voltage value (>= 0.0; 1.0 = unity) */
			void SetGainLin(double pGainLin) { mGainLin = pGainLin; update(); }
			/** Set gain with dB value */
			void SetGainDb(double pGainDb) { SetGainLin(SR::Utils::DBToAmp(pGainDb)); /* don't update because SetGainLin does */ }
			/** Set algorithm for panning */
			void SetPanType(PanType pType) { mPanType = pType; update(); }
			/** If true, gain is at unity in center position and may differ in other positions */
			void SetPanLinearMiddlePosition(bool pLinearMiddlePosition) { mLinearInMiddlePosition = pLinearMiddlePosition; update(); }
			/** Set normalized panning position 0 .. 1, while .5 is middle position */
			void SetPanPosition(double pPanNormalized) { mPanNormalized = pPanNormalized; update(); }
			/** Set normalized stereo width (0. = mono; 1. = stereo; > 1 = wider than stereo */
			void SetWidth(double pWidthNormalized) {
				mWidthNormalized = pWidthNormalized;
				const double tmp = 1. / std::max(1. + mWidthNormalized, 2.);
				mMidCoeff = 1. * tmp;
				mSideCoeff = mWidthNormalized * tmp;
			}
			/** Set gain computer bypassed */
			void SetBypassed(bool pBypassed) { mBypassed = pBypassed; }
			/** Set number of samples until gain smoothing ramp reaches target value */
			void SetRamp(int pRampNumSamples) {
				mRampNumSamples = (pRampNumSamples < 1) ? 1 : pRampNumSamples;
				mGainRamp[0].SetNumSmoothSamples(mRampNumSamples);
				mGainRamp[1].SetNumSmoothSamples(mRampNumSamples);
			}
			/** Get normalized gain with linear voltage value (>= 0.0; 1.0 = unity) */
			double GetGainLin() const { return mGainLin; }
			/** Get gain with dB value */
			double GetGainDb() const { return SR::Utils::AmpToDB(mGainLin); }
			/** Get current linear gain voltage at channel [0] or [1] */
			double GetCurrentGainLin(int channel) { return mGainRamp[channel].Get(); }
			/** Get algorithm for panning */
			PanType GetPanType() const { return mPanType; }
			/** Get normalized panning position 0 .. 1, while .5 is middle position */
			double GetPanPosition() const { return mPanNormalized; }
			/** Get normalized stereo width (0. = mono; 1. = stereo; > 1 = wider than stereo */
			double GetWidth() const { return mWidthNormalized; }
			/** Get if gain computer is bypassed */
			bool GetBypassed() const { return mBypassed; }
			/** Get number of samples until gain smoothing ramp reaches target value */
			int GetRamp() const { return mRampNumSamples; }
			/** Runtime method, call per sample, overwrites data in sample** directly */
			void Process(double& in1, double& in2); // for stereo
		protected:
			/** Update gain values after values changed: gain, pan, pan type */
			void update(void) {
				double gain1;
				double gain2;

				// Pan position calculation
				if (mPanNormalized != 0.5) {

					switch (mPanType) {
					case kLinear:
						if (mLinearInMiddlePosition == true) {
							gain1 = (mPanNormalized > 0.5) ? (1. - mPanNormalized) * 2 : 1.0;
							gain2 = (mPanNormalized < 0.5) ? mPanNormalized * 2 : 1.0;
						}
						else {
							gain1 = (1. - mPanNormalized);
							gain2 = (mPanNormalized);
						}
						break;
					case kSquareroot:
						if (mLinearInMiddlePosition == true) {
							gain1 = (mPanNormalized > 0.5) ? sqrt(mPanNormalized) * sqrt(2.) : 1.0;
							gain2 = (mPanNormalized < 0.5) ? sqrt(1 - mPanNormalized) * sqrt(2.) : 1.0;
						}
						else {
							gain1 = sqrt(1. - mPanNormalized);
							gain2 = sqrt(mPanNormalized);
						}
						break;
					case kSinusodial:
						if (mLinearInMiddlePosition == true) {
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
				else {
					// Center position, no panning
					gain1 = 1.0;
					gain2 = 1.0;
				}

				gain1 *= mGainLin;
				gain2 *= mGainLin;

				mGainRamp[0].Set(gain1);
				mGainRamp[1].Set(gain2);
			}
			double mGainLin;
			PanType mPanType;
			double mPanNormalized; // Enter normalized pan between 0.0 (left) and 1.0 (right)
			bool mLinearInMiddlePosition; // Center volume doens't change if true
			double mWidthNormalized; // Normalized width where 0 = mono, 1 = normal stereo, > 1 = wider
			double mMidCoeff, mSideCoeff;
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

	} // namespace !DSP
} // namespace !SR