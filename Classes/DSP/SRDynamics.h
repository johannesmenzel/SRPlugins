/*
*	copyright 2019, Johannes Menzel, MIT
*
*	This is an ongoing modification of the code provided in the Simple Source
*	library by Bojan Markovic. Licence disclaimer is merged here, since the
*	classes and functions were merged to one .h and .cpp source file each.
*	See the original disclaimer below:
*
*	copyright 2006, ChunkWare Music Software, OPEN-SOURCE
*
*	Permission is hereby granted, free of charge, to any person obtaining a
*	copy of this software and associated documentation files (the "Software"),
*	to deal in the Software without restriction, including without limitation
*	the rights to use, copy, modify, merge, publish, distribute, sublicense,
*	and/or sell copies of the Software, and to permit persons to whom the
*	Software is furnished to do so, subject to the following conditions:
*
*	The above copyright notice and this permission notice shall be included in
*	all copies or substantial portions of the Software.
*
*	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
*	THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
*	FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
*	DEALINGS IN THE SOFTWARE.
*/

#pragma once

// These may move to SRClasses common header:
#if _MSC_VER > 1000				// MS Visual Studio
#define INLINE __forceinline	// forces inline
//#define NOMINMAX				// for standard library min(), max()
#else							// other IDE's
#define INLINE inline
#endif

#include <algorithm>			// for min(), max()
#include <cassert>				// for assert()
#include <cmath>
#define _USE_MATH_DEFINES		// for math constants
#include <vector>
#include "SRFilters.h"    // for deesser filters and compressor emphasis filters
#include "../Utils/SRHelpers.h"
#include "SRGain.h"


namespace SR {
	namespace DSP {
		/*
		DC offset (to prevent denormal); usage:
			1. init envelope state to DC_OFFSET before processing
			2. add to input before envelope runtime function
		*/
		static const double DC_OFFSET = 1.0e-25;



		// Envelope Detector class
		// This is basically a one-pole lowpass
		class SRDynamicsEnvelope {
		public:
			SRDynamicsEnvelope(double ms = 1.0, double sampleRate = 44100.0)
			{
				assert(sampleRate > 0.0);
				assert(ms > 0.0);
				mSampleRate = sampleRate;
				mTimeConstantMs = ms;
				setCoef();
			}

			virtual ~SRDynamicsEnvelope() {}

			virtual void SetTc(double ms) {
				assert(ms > 0.0);
				mTimeConstantMs = ms;
				setCoef();
			}

			virtual void SetSampleRate(double sampleRate) {
				assert(sampleRate > 0.0);
				mSampleRate = sampleRate;
				setCoef();
			}

			virtual double GetTc(void) const { return mTimeConstantMs; }
			virtual double GetSampleRate(void) const { return mSampleRate; }

			// Runtime method of Envelope detector
			INLINE void Process(double in, double& state) {
				state = in + mRuntimeCoeff * (state - in);
			}

		protected:
			double mSampleRate;           // Envelope detectors samplerate
			double mTimeConstantMs;       // Envelope detectors time constant in ms
			double mRuntimeCoeff;			    // Envelope detectors runtime coefficient

			virtual void setCoef(void) {
				mRuntimeCoeff = exp(-1000.0 / (mTimeConstantMs * mSampleRate));
			}	  // Envelope detectors coefficient calculation
		};


		// Attack/Release envelope class
		// Holds attack AND release parameters
		class SRDynamicsDetector {
		public:
			//-------------------------------------------------------------
			// Attack/release envelope
			//-------------------------------------------------------------
			SRDynamicsDetector(double attackMs = 10.0, double releaseMs = 100.0, double sampleRate = 44100.0)
				: mEnvelopeAttack(attackMs, sampleRate)
				, mEnvelopeRelease(releaseMs, sampleRate) {}
			virtual ~SRDynamicsDetector() {}

			virtual double GetAttack(void) const { return mEnvelopeAttack.GetTc(); }
			virtual double GetRelease(void) const { return mEnvelopeRelease.GetTc(); }
			virtual double GetSampleRate(void) const { return mEnvelopeAttack.GetSampleRate(); }

			virtual void ResetDetector(double attackMs, double releaseMs, double sampleRate) {
				SetAttack(attackMs);
				SetRelease(releaseMs);
				SetSampleRate(sampleRate);
			}
			virtual void SetAttack(double ms) { mEnvelopeAttack.SetTc(ms); }
			virtual void SetRelease(double ms) { mEnvelopeRelease.SetTc(ms); }
			virtual void SetSampleRate(double sampleRate) {
				mEnvelopeAttack.SetSampleRate(sampleRate);
				mEnvelopeRelease.SetSampleRate(sampleRate);
			}

			// RUNTIME
			INLINE void process(double in, double& state) {
				if (in > state)
					mEnvelopeAttack.Process(in, state);
				else
					mEnvelopeRelease.Process(in, state);
			}

		private:
			SRDynamicsEnvelope mEnvelopeAttack;
			SRDynamicsEnvelope mEnvelopeRelease;
		};

		//class SRDynamicsGainStage {

		//};

		//class SRDynamicsGainComputer {

		//};

		// Dynamics base class
		// Holds standard static members of any basic dynamic processor and can be inherited from
		class SRDynamicsBase
		{
		public:

			SRDynamicsBase(double threshDb = 0.0, double ratio = 1.0, bool autoMakeup = false)
				: mThreshDb(threshDb)
				, mThreshLin(Utils::DBToAmp(threshDb))
				, mRatio(ratio)
				, mMakeup(1.0)
				, mKneeWidthDb(0.0)
				, mGrDb(0.0)
				, mGrLin(1.0)
				, currentOvershootDb(DC_OFFSET)
				, currentOvershootLin(SR::Utils::DBToAmp(DC_OFFSET))
				, mAverageOfSquares(DC_OFFSET)
				, mIsAutoMakeup(autoMakeup)
				, mAutoMakeup(1.0)
				, mReferenceDb(0.0)
				, mMix(1.0)
				, fMakeup(100)
				, fAutoMakeup(100)
			{
			}

			virtual ~SRDynamicsBase() {}

			/** Sets dynamic processors threshold in dB (typically negative values)
			* @param threshDb Sets dynamic processors threshold in dB (typically negative values) */
			virtual void SetThresh(double threshDb) {
				mThreshDb = threshDb;
				mThreshLin = SR::Utils::DBToAmp(threshDb);
				if (mIsAutoMakeup) AdjustAutoMakeup();
			}

			/** Set dynamics processors ratio, for ration n:1 enter 1./n
			* @param ratio Set dynamics processors ratio, for ration n:1 enter 1./n */
			virtual void SetRatio(double ratio) {
				assert(ratio >= 0.0);
				mRatio = ratio;
				if (mIsAutoMakeup) AdjustAutoMakeup();
			}

			/** Sets dynamic processors makeup gain in dB
			* @param makeupDb Sets dynamic processors makeup gain in dB */
			virtual void SetMakeup(double makeupDb) {
				mMakeup = SR::Utils::DBToAmp(makeupDb);
				fMakeup.SetGainLin(mMakeup);
			}

			/** Sets if dynamic processor compensates gain reduction automatically
			* @param autoMakeup Sets if dynamic processor compensates gain reduction automatically*/
			virtual void SetIsAutoMakeup(bool autoMakeup) {
				mIsAutoMakeup = autoMakeup;
				if (mIsAutoMakeup) AdjustAutoMakeup();
			}

			/** Sets target loudness of the track in dB
			* @param referenceDb Sets target loudness of the track in dB */
			virtual void SetReference(double referenceDb) {
				mReferenceDb = referenceDb;
				if (mIsAutoMakeup) AdjustAutoMakeup();
			}

			/** Sets soft knee width in dB
			* @param kneeDb Sets soft knee width in dB*/
			virtual void SetKnee(double kneeDb) {
				assert(kneeDb >= 0.0);
				mKneeWidthDb = kneeDb;
			}

			/** Sets mix behavoir dry/wet, range: 0.-1.
			* @param mix Sets mix behavoir dry/wet, range: 0.-1. */
			virtual void SetMix(double mix) {
				assert(mix <= 1.);
				assert(mix >= 0.);
				mMix = mix;
			}

			/** Call before runtime, typically in OnReset() or similar. This resets current overshoot and average of squares to DC_OFFSET */
			virtual void Reset(void) {
				currentOvershootDb = DC_OFFSET;
				currentOvershootLin = SR::Utils::DBToAmp(DC_OFFSET);
				mAverageOfSquares = DC_OFFSET;
			}

			virtual double GetThresh(void) const { return mThreshDb; }      // Returns dynamic processors logarithmic threshold
			virtual double GetThreshLin(void) const { return mThreshLin; }  // Returns dynamic processors linear threshold
			virtual double GetRatio(void) const { return mRatio; }          // Returns dynamic processors ratio
			virtual double GetKneeDb(void) const { return mKneeWidthDb; }   // Returns dynamic processors soft knee width in dB
			virtual double GetGrLin(void) { return mGrLin; }                // Returns dynamic processors linear gain reduction (0..1)
			virtual double GetGrDb(void) { return mGrDb; }                  // Returns dynamic processors logarithmic gain reduction
			virtual double GetMix(void) { return mMix; }					// Returns dynamic processors mix percentage (0..1)

		protected:

			// Advanced compressor auto makeup calculation with time constants
			virtual double AutoMakeup(double threshDb, double ratio, double referenceDb, double attackMs, double releaseMs)
			{
				return	1. + (1. / (Utils::DBToAmp(((ratio - 1.) * -threshDb) / 2.)) - 1.) * sqrt(threshDb / referenceDb) * (sqrt(30.) / sqrt(attackMs)) * (sqrt(releaseMs) / sqrt(5000.));
			}

			// Simple compressor auto makeup calculation based on threshold, ratio and reference loudness
			virtual double AutoMakeup(double threshDb, double ratio, double referenceDb)
			{
				if (referenceDb > threshDb)
					return Utils::DBToAmp((ratio - 1.) * (threshDb - referenceDb));
				else
					return 1.;
			}

			// Simple compressor auto makeup calculation based on threshold and ratio
			virtual double AutoMakeup(double threshDb, double ratio)
			{
				return Utils::DBToAmp((ratio - 1.) * threshDb);
			}

			virtual void AdjustAutoMakeup() {
				mAutoMakeup = AutoMakeup(mThreshDb, mRatio, mReferenceDb);
				fAutoMakeup.SetGainLin(mAutoMakeup);
			}

			SRGain fMakeup, fAutoMakeup;
			double mThreshDb;               // Dynamic processors threshold in dB
			double mThreshLin;              // Dynamic processors linear threshold
			double mRatio;                  // Dynamic processors ratio
			double mMakeup;                 // Dynamic processors makeup gain (linear voltage)
			double mKneeWidthDb;            // Dynamic processors soft knee width in dB
			double mGrLin;                  // Dynamic processors linear gain reduction (0..1)
			double mGrDb;                   // Dynamic processors logarithmic gain reduction
			double mMix;										// Dynamic Processors mix behaviour
			double currentOvershootDb;      // Logarithmic over-threshold envelope
			double currentOvershootLin;     // Linear over-threshold envelope
			double mAverageOfSquares;       // Dynamic processors gain detectors average of squares
			double mReferenceDb;
			bool mIsAutoMakeup;
			double mAutoMakeup;
		};


		//-------------------------------------------------------------
		// COMPRESSOR Class
		//-------------------------------------------------------------
		class SRCompressor
			: public SRDynamicsDetector
			, public SRDynamicsBase
		{
		public:
			//-------------------------------------------------------------
			// SRCompressor methods
			//-------------------------------------------------------------
			SRCompressor()
				: SRDynamicsDetector(10., 100.)
				, SRDynamicsBase(0., 1.)
				, mSidechainFc(0.)
				, mTopologyFeedback(false)
				, mMaxGr(0.)
				, mMaxGrWidth(0.)
				// Needed for feedback topology
				, mSidechainSignal1(0.)
				, mSidechainSignal2(0.)
			{
			}
			virtual ~SRCompressor() {}

			// parameters
			virtual void InitCompressor(double threshDb, double ratio, double attackMs, double releaseMs, double sidechainFc, double kneeDb, bool isFeedbackCompressor, bool autoMakeup, double referenceDb, double mix, double samplerate) {
				SRDynamicsDetector::SetSampleRate(samplerate);
				SRDynamicsBase::SetThresh(threshDb);
				SRDynamicsBase::SetRatio(ratio);
				SRDynamicsBase::SetIsAutoMakeup(autoMakeup);
				SRDynamicsBase::SetReference(referenceDb);
				SRDynamicsBase::SetMix(mix);
				SRDynamicsBase::SetKnee(kneeDb);
				SRDynamicsDetector::SetAttack(attackMs);
				SRDynamicsDetector::SetRelease(releaseMs);
				InitSidechainFilter(sidechainFc);
				SetTopologyFeedback(isFeedbackCompressor);
				SRDynamicsBase::Reset();
			}

			virtual void ResetCompressor(double threshDb, double ratio, double attackMs, double releaseMs, double sidechainFc, double kneeDb, bool isFeedbackCompressor, bool autoMakeup, double referenceDb, double mix, double samplerate) {
				SRDynamicsDetector::SetSampleRate(samplerate);
				SRDynamicsBase::SetThresh(threshDb);
				SRDynamicsBase::SetRatio(ratio);
				SRDynamicsBase::SetIsAutoMakeup(autoMakeup);
				SRDynamicsBase::SetReference(referenceDb);
				SRDynamicsBase::SetMix(mix);
				SRDynamicsBase::SetKnee(kneeDb);
				SRDynamicsDetector::SetAttack(attackMs);
				SRDynamicsDetector::SetRelease(releaseMs);
				InitSidechainFilter(sidechainFc);
				SetTopologyFeedback(isFeedbackCompressor);
			}
			/** Set maximum gain reduction, awaits negative values
			* @param maxGrDb Set maximum gain reduction in dB, set as negative value
			* @param transitionWidth Set width of transition between direct gain reduction and full limit, like knee. range >= 0.,
			* should be below maxGrDb / 2. (Is in fact limited internally) */
			virtual void SetMaxGrDb(double maxGrDb, double transitionWidth) {
				mMaxGr = maxGrDb;
				mMaxGrWidth = std::min(transitionWidth, abs(mMaxGr) * .5);
			}

			virtual void InitSidechainFilter(double sidechainFC) {
				mSidechainFc = sidechainFC;
				fSidechainFilter.SetFilter(EFilterType::BiquadHighpass, sidechainFC, 0.7071, 0., SRDynamicsDetector::GetSampleRate());
			}

			virtual void SetSidechainFilterFreq(double sidechainFc) {
				mSidechainFc = sidechainFc;
				fSidechainFilter.SetFreq(mSidechainFc);
			}

			virtual void SetTopologyFeedback(bool isFeedbackCompressor) { mTopologyFeedback = isFeedbackCompressor; }

			void Process(double& in1, double& in2); // Compressor runtime process for internal sidechain 
			void Process(double& in1, double& in2, double& extSC1, double& extSC2); // Compressor runtime process for external sidechain

		protected:
			void process(double& in1, double& in2, double sidechain);	// Compressor runtime process with stereo-linked key
			SRFilterIIR<double, 2> fSidechainFilter; // Compressors stereo sidechain filter
			bool mTopologyFeedback; // True if its a feedback compressor, false for modern feedforward
			double mSidechainSignal1, mSidechainSignal2;      // Gain reduced signal to get used as new sidechain for feedback topology
			double mSidechainFc;  // Compressors stereo sidechain filters center frequency
			double mMaxGr;  // Maximum gain reduction for gain reduction limiting (no brickwall, just gets damped there)
			double mMaxGrWidth; // Width of transition
		};


		// RMS COMPRESSOR Class
		//-------------------------------------------------------------
		class SRCompressorRMS
			: public SRCompressor
		{
		public:
			// SRCompressorRMS methods
			//-------------------------------------------------------------
			SRCompressorRMS()
				: mEnvelopeAverager(5.0)
			{
			}
			virtual ~SRCompressorRMS() {}

			// sample rate
			virtual void SetSampleRate(double sampleRate) {
				SRCompressor::SetSampleRate(sampleRate);
				mEnvelopeAverager.SetSampleRate(sampleRate);
			}
			virtual void InitCompressor(double dB, double ratio, double attackMs, double releaseMs, double sidechainFc, double kneeDb, double rmsWindowMs, bool isFeedbackCompressor, bool autoMakeup, double samplerate) {
				SetSampleRate(samplerate);
				SRDynamicsBase::SetThresh(dB);
				SRDynamicsBase::SetRatio(ratio);
				SRDynamicsBase::SetIsAutoMakeup(autoMakeup);
				SRDynamicsDetector::SetAttack(attackMs);
				SRDynamicsDetector::SetRelease(releaseMs);
				SRCompressor::InitSidechainFilter(sidechainFc);
				SRDynamicsBase::SetKnee(kneeDb);
				SRCompressor::SetTopologyFeedback(isFeedbackCompressor);
				mEnvelopeAverager.SetTc(rmsWindowMs);
				SRDynamicsBase::Reset();
			}

			// RMS window
			virtual void SetWindow(double ms) {
				mEnvelopeAverager.SetTc(ms);
			}
			virtual double GetWindow(void) const { return mEnvelopeAverager.GetTc(); }

			void Process(double& in1, double& in2, double& extSC1, double& extSC2);
			void Process(double& in1, double& in2);	// compressor runtime process

		private:
			SRDynamicsEnvelope mEnvelopeAverager;	// averager
		};



		//-------------------------------------------------------------
		// COMPRESSOR Inline Functions
		//-------------------------------------------------------------

		// Compressor runtime process for external sidechain
		INLINE void SRCompressor::Process(double& in1, double& in2, double& extSC1, double& extSC2) {
			double sidechainL = extSC1;
			double sidechainR = extSC2;
			if (mSidechainFc > 16. / GetSampleRate()) {
				sidechainL = fSidechainFilter.Process(sidechainL, 0);
				sidechainR = fSidechainFilter.Process(sidechainR, 1);
			}
			// Rectify sidechain
			sidechainL = std::fabs(sidechainL);
			sidechainR = std::fabs(sidechainR);
			// Choose bigger of L and R
			double sidechainMaxOfLR = std::max(sidechainL, sidechainR);	// link channels with greater of 2
			process(in1, in2, sidechainMaxOfLR);	// rest of process
		}

		// Compressor runtime process for internal sidechain 
		INLINE void SRCompressor::Process(double& in1, double& in2) {
			double sidechainL = (!mTopologyFeedback) ? in1 : mSidechainSignal1;
			double sidechainR = (!mTopologyFeedback) ? in2 : mSidechainSignal2;
			if (mSidechainFc > 16. / GetSampleRate()) {
				sidechainL = fSidechainFilter.Process(sidechainL, 0);
				sidechainR = fSidechainFilter.Process(sidechainR, 1);
			}
			// Rectify sidechain
			sidechainL = std::fabs(sidechainL);
			sidechainR = std::fabs(sidechainR);
			// Choose bigger of L and R
			double sidechainMaxOfLR = std::max(sidechainL, sidechainR);	// link channels with greater of 2
			process(in1, in2, sidechainMaxOfLR);	// rest of process
		}

		// Inline RMS Compressor Sidechain
		//-------------------------------------------------------------

		// RMS Compressor runtime process for external sidechain 
		INLINE void SRCompressorRMS::Process(double& in1, double& in2, double& extSC1, double& extSC2) {
			double squaredInput1 = extSC1 * extSC1;	// square input
			double squaredInput2 = extSC2 * extSC2;
			double summedSquaredInput = squaredInput1 + squaredInput2;			// power summing
			summedSquaredInput += DC_OFFSET;					// DC offset, to prevent denormal
			mEnvelopeAverager.Process(summedSquaredInput, mAverageOfSquares);		// average of squares
			double sidechainRms = sqrt(mAverageOfSquares);	// sidechainRms (sort of ...), see NOTE 2
			SRCompressor::process(in1, in2, sidechainRms);	// rest of process
		}

		// RMS Compressor runtime process for internal sidechain 
		INLINE void SRCompressorRMS::Process(double& in1, double& in2) {
			double squaredInput1 = (!mTopologyFeedback) ? in1 * in1 : mSidechainSignal1 * mSidechainSignal1;	// square input
			double squaredInput2 = (!mTopologyFeedback) ? in2 * in2 : mSidechainSignal2 * mSidechainSignal2;
			double summedSquaredInput = squaredInput1 + squaredInput2;			// power summing
			summedSquaredInput += DC_OFFSET;					// DC offset, to prevent denormal
			mEnvelopeAverager.Process(summedSquaredInput, mAverageOfSquares);		// average of squares
			double sidechainRms = sqrt(mAverageOfSquares);	// sidechainRms (sort of ...), See NOTE 2
			SRCompressor::process(in1, in2, sidechainRms);	// rest of process
		}
		// Inline all compressors process
		// This is the protected method all compressors input sidechain methods call
		INLINE void SRCompressor::process(double& in1, double& in2, double sidechain) {
			double sidechainDb = fabs(sidechain);                 // rect (just in case)
			sidechainDb += DC_OFFSET;                             // add DC offset to avoid log( 0 )
			sidechainDb = SR::Utils::AmpToDB(sidechainDb);				// linear -> dB conversion
			double sampleOvershootDb = sidechainDb - mThreshDb;   // delta over threshold
			sampleOvershootDb += DC_OFFSET;                       // add DC offset to avoid denormal (why twice?); See NOTE 1
			SRDynamicsDetector::process(sampleOvershootDb, currentOvershootDb);	// process attack/release envelope
			sampleOvershootDb = currentOvershootDb - DC_OFFSET;   // subtract DC offset to avoid denormal

			// Calculate gain reduction with knee
			double grRaw/*, grlimit, grlimitsqrt*/;
			// above knee
			if (sampleOvershootDb > mKneeWidthDb * 0.5)
				grRaw = (mRatio - 1.) * (sampleOvershootDb);
			// within knee
			else if (fabs(sampleOvershootDb) <= mKneeWidthDb * 0.5)
				grRaw = ((mRatio - 1.) * std::pow(sampleOvershootDb + mKneeWidthDb * 0.5, 2.)) / (2. * mKneeWidthDb);
			// below knee
			else
				grRaw = 0.;

			//// Gain reducion limiter
			if (mMaxGr < 0.0) {
				if (grRaw <= mMaxGr - mMaxGrWidth * .5) {
					grRaw = mMaxGr;
				}
				else if (grRaw < mMaxGr + mMaxGrWidth * .5) {
					// The function f(grRaw) = ((grRaw - mMaxGr + (mMaxGrWidth / 2.)) ^ 2 / ( 2 * mMaxGrWidth )) + mMaxGr
					// is performed in single steps for faster calculation
					grRaw -= mMaxGr; // upper calculation
					grRaw += mMaxGrWidth * .5;
					grRaw *= grRaw; // instead pow (^2)
					grRaw /= mMaxGrWidth + mMaxGrWidth; // lower calculation
					grRaw += mMaxGr; // add mMaxGr
				}
				// else	grRaw just stays the same
			}

			mGrDb = grRaw; // Store logarithmic gain reduction
			grRaw = mGrLin = SR::Utils::DBToAmp(grRaw);// Logarithmic to linear conversion

			const double drySignal1 = in1;
			const double drySignal2 = in2;

			// Apply gain reduction to inputs:
			in1 *= grRaw;
			in2 *= grRaw;

			// for feedback topology set old processed inputs as new sidechain.
			mSidechainSignal1 = in1;
			mSidechainSignal2 = in2;

			// Apply makeup gain
			fMakeup.Process(in1, in2);
			if (mIsAutoMakeup) {
				fAutoMakeup.Process(in1, in2);
			}

			// Apply mix parameter:
			if (mMix != 1.) {
				in1 = mMix * in1 + drySignal1 * (1. - mMix);
				in2 = mMix * in2 + drySignal2 * (1. - mMix);
			}

		}


		//-------------------------------------------------------------
		// LIMITER Class
		//-------------------------------------------------------------
		class SRLimiter
			: public SRDynamicsBase
		{
		public:
			//-------------------------------------------------------------
			// SRLimiter Methods
			//-------------------------------------------------------------
			SRLimiter()
				: SRDynamicsBase(0.0, 1.0)
				, mPeakHoldSamples(0)
				, mPeakHoldTimer(0)
				, mMaxPeak(1.0)
				, mEnvelopeDetectorAttack(1.0)
				, mEnvelopeDetectorRelease(10.0)
				, mBufferMask(BUFFER_SIZE - 1)
				, mCursor(0)
			{
				SetAttack(1.0);
				mOutputBuffer[0].resize(BUFFER_SIZE, 0.0);
				mOutputBuffer[1].resize(BUFFER_SIZE, 0.0);
			}
			virtual ~SRLimiter() {}

			virtual void SetAttack(double ms) {
				unsigned int samp = int(0.001 * ms * mEnvelopeDetectorAttack.GetSampleRate());

				assert(samp < BUFFER_SIZE);

				mPeakHoldSamples = samp;
				mEnvelopeDetectorAttack.SetTc(ms);
			}
			virtual void SetRelease(double ms) {
				mEnvelopeDetectorRelease.SetTc(ms);
			}
			virtual double GetAttack(void)  const { return mEnvelopeDetectorAttack.GetTc(); }
			virtual double GetRelease(void) const { return mEnvelopeDetectorRelease.GetTc(); }
			virtual void   SetSampleRate(double sampleRate) {
				mEnvelopeDetectorAttack.SetSampleRate(sampleRate);
				mEnvelopeDetectorRelease.SetSampleRate(sampleRate);
			}
			virtual double GetSampleRate(void) { return mEnvelopeDetectorAttack.GetSampleRate(); }
			virtual const unsigned int GetLatency(void) const { return mPeakHoldSamples; }

			virtual void Reset(void) {
				mPeakHoldTimer = 0;
				mMaxPeak = mThreshLin;
				currentOvershootLin = mThreshLin;
				mCursor = 0;
				mOutputBuffer[0].assign(BUFFER_SIZE, 0.0);
				mOutputBuffer[1].assign(BUFFER_SIZE, 0.0);
			}			// call before runtime (in resume())
			void Process(double& in1, double& in2);	// limiter runtime process

		protected:
			// class for faster attack/release
			class FastEnvelope : public SRDynamicsEnvelope
			{
			public:
				FastEnvelope(double ms = 1.0, double sampleRate = 44100.0)
					: SRDynamicsEnvelope(ms, sampleRate)
				{}
				virtual ~FastEnvelope() {}

			protected:
				// override setCoef() - coefficient calculation
				virtual void setCoef(void) {
					mRuntimeCoeff = pow(0.01, (1000.0 / (mTimeConstantMs * mSampleRate))); // rises to 99% of in value over duration of time constant
				}
			};

		private:
			// max peak
			unsigned int mPeakHoldSamples;		// peak hold (samples)
			unsigned int mPeakHoldTimer;	// peak hold timer
			double mMaxPeak;			// max peak

			// attack/release envelope
			FastEnvelope mEnvelopeDetectorAttack;			// attack
			FastEnvelope mEnvelopeDetectorRelease;			// release

			// buffer
			// BUFFER_SIZE default can handle up to ~10ms at 96kHz
			// change this if you require more
			static const int BUFFER_SIZE = 1024;	// buffer size (always a power of 2!)
			unsigned int mBufferMask;						// buffer mask
			unsigned int mCursor;						// cursor
			std::vector< double > mOutputBuffer[2];	// output buffer

		};






		//-------------------------------------------------------------
		// LIMITER Inline Functions
		//-------------------------------------------------------------
		INLINE void SRLimiter::Process(double& in1, double& in2)
		{
			// create sidechain

			double rectifiedInput1 = fabs(in1);	// rectify input
			double rectifiedInput2 = fabs(in2);

			double keyLink = std::max(rectifiedInput1, rectifiedInput2);	// link channels with greater of 2

			// threshold
			// we always want to feed the sidechain AT LEATS the threshold value
			if (keyLink < mThreshLin)
				keyLink = mThreshLin;

			// test:
			// a) whether peak timer has "expired"
			// b) whether new peak is greater than previous max peak
			if ((++mPeakHoldTimer >= mPeakHoldSamples) || (keyLink > mMaxPeak)) {
				// if either condition is met:
				mPeakHoldTimer = 0;		// reset peak timer
				mMaxPeak = keyLink;	// assign new peak to max peak
			}

			// See NOTE 5
					// attack/release
			if (mMaxPeak > currentOvershootLin)
				mEnvelopeDetectorAttack.Process(mMaxPeak, currentOvershootLin);		// process attack phase
			else
				mEnvelopeDetectorRelease.Process(mMaxPeak, currentOvershootLin);		// process release phase

			// See NOTE 4

// gain reduction
			double grRaw = mGrLin = mThreshLin / currentOvershootLin;

			// unload current buffer index
			// ( mCursor - delay ) & mBufferMask gets sample from [delay] samples ago
			// mBufferMask variable wraps index
			unsigned int delayIndex = (mCursor - mPeakHoldSamples) & mBufferMask;
			double delay1 = mOutputBuffer[0][delayIndex];
			double delay2 = mOutputBuffer[1][delayIndex];

			// load current buffer index and advance current index
			// mBufferMask wraps mCursor index
			mOutputBuffer[0][mCursor] = in1;
			mOutputBuffer[1][mCursor] = in2;
			++mCursor &= mBufferMask;

			// output gain
			in1 = delay1 * grRaw;	// apply gain reduction to input
			in2 = delay2 * grRaw;

			// See NOTE 3
		}






		//-------------------------------------------------------------
		// GATE Class
		//-------------------------------------------------------------
		class SRGate
			: public SRDynamicsDetector
			, public SRDynamicsBase
		{
		public:
			SRGate()
				: SRDynamicsDetector(1.0, 100.0)
				, SRDynamicsBase(0.0, 1.0)
			{
			}
			virtual ~SRGate() {}

			void Process(double& in1, double& in2);	// gate runtime process
			void Process(double& in1, double& in2, double keyLinked);	// with stereo-linked key in
		};






		//-------------------------------------------------------------
		// GATE RMS Class
		//-------------------------------------------------------------
		class SRGateRMS : public SRGate
		{
		public:
			SRGateRMS()
				: mEnvelopeAverager(5.0)
			{
			}
			virtual ~SRGateRMS() {}

			// sample rate
			virtual void setSampleRate(double sampleRate) {
				SRGate::SetSampleRate(sampleRate);
				mEnvelopeAverager.SetSampleRate(sampleRate);
			}

			// RMS window
			virtual void setWindow(double ms) {
				mEnvelopeAverager.SetTc(ms);
			}
			virtual double getWindow(void) const { return mEnvelopeAverager.GetTc(); }

			// runtime process
			void Process(double& in1, double& in2);	// gate runtime process

		private:

			SRDynamicsEnvelope mEnvelopeAverager;	// averager

		};






		//-------------------------------------------------------------
		// GATE Inline Functions
		//-------------------------------------------------------------

		// Inline Gate Sidechain
		//-------------------------------------------------------------
		INLINE void SRGate::Process(double& in1, double& in2)
		{
			// create sidechain

			double rectifiedInput1 = fabs(in1);	// rectify input
			double rectifiedInput2 = fabs(in2);

			/* if desired, one could use another EnvelopeDetector to smooth
			* the rectified signal.
			*/

			double rectifiedInputMaxed = std::max(rectifiedInput1, rectifiedInput2);	// link channels with greater of 2

			Process(in1, in2, rectifiedInputMaxed);	// rest of process
		}

		// Inline RMS Gate Sidechain
		//-------------------------------------------------------------
		INLINE void SRGateRMS::Process(double& in1, double& in2)
		{
			// create sidechain

			double squaredInput1 = in1 * in1;	// square input
			double squaredInput2 = in2 * in2;

			double summedSquaredInput = squaredInput1 + squaredInput2;			// power summing
			summedSquaredInput += DC_OFFSET;					// DC offset, to prevent denormal
			mEnvelopeAverager.Process(summedSquaredInput, mAverageOfSquares);		// average of squares
			double sidechainRms = sqrt(mAverageOfSquares);	// sidechainRms (sort of ...), See NOTE 2

			SRGate::Process(in1, in2, sidechainRms);	// rest of process
		}

		// Inline Gates Process
		//-------------------------------------------------------------
		INLINE void SRGate::Process(double& in1, double& in2, double sidechain)
		{
			sidechain = fabs(sidechain);	// rectify (just in case)

			// threshold
			// key over threshold ( 0.0 or 1.0 )
			double gateGainApply = double(sidechain > mThreshLin);

			// attack/release
			gateGainApply += DC_OFFSET;					// add DC offset to avoid denormal
			SRDynamicsDetector::process(gateGainApply, currentOvershootLin);	// process attack/release
			gateGainApply = currentOvershootLin - DC_OFFSET;			// subtract DC offset, See NOTE 1
			in1 *= gateGainApply;	// apply gain reduction to input
			in2 *= gateGainApply;
		}



		//-------------------------------------------------------------
		// DEESSER Class
		//-------------------------------------------------------------
		class SRDeesser
			: public SRDynamicsDetector
			, public SRDynamicsBase
		{
		public:
			SRDeesser()
				: SRDynamicsDetector(10.0, 100.0)
				, SRDynamicsBase(0.0, 1.0)
				, mFilterFreq(0.5)
				, mFilterQ(0.707)
				, mFilterGain(0.0)
				, fSidechainBandpass(SRFilterIIR<double, 2>(EFilterType::BiquadBandpass, 0.5, 0.707, 0.0, 44100.0))
				, fDynamicEqFilter(SRFilterIIR<double, 2>(EFilterType::BiquadPeak, 0.5, 0.707, 0.0, 44100.0))
			{
			}
			virtual ~SRDeesser() {}

			// parameters
			virtual void Reset(double threshDb, double ratio, double attackMs, double releaseMs, double normalizedFreq, double q, double kneeDb, double samplerate, SR::DSP::EFilterType type) {
				SRDynamicsBase::SetThresh(threshDb);
				SRDynamicsBase::SetRatio(ratio);
				SRDynamicsBase::SetKnee(kneeDb);
				SRDynamicsDetector::SetAttack(attackMs);
				SRDynamicsDetector::SetRelease(releaseMs);
				SRDynamicsDetector::SetSampleRate(samplerate);
				mFilterFreq = normalizedFreq;
				mFilterQ = q;
				fSidechainBandpass.SetFilter(EFilterType::BiquadBandpass, mFilterFreq, mFilterQ, 0.0, samplerate);
				fDynamicEqFilter.SetFilter(type, mFilterFreq, mFilterQ, mFilterGain, samplerate);
				SetType(type);
			}

			virtual void SetType(SR::DSP::EFilterType type) {
				switch (type)
				{
				case EFilterType::BiquadPeak:
					fDynamicEqFilter.SetType(SR::DSP::BiquadPeak);
					fSidechainBandpass.SetType(SR::DSP::BiquadBandpass);
					break;
				case EFilterType::BiquadLowshelf:
					fDynamicEqFilter.SetType(SR::DSP::BiquadLowshelf);
					fSidechainBandpass.SetType(SR::DSP::BiquadLowpass);
					break;
				case EFilterType::BiquadHighshelf:
					fDynamicEqFilter.SetType(SR::DSP::BiquadHighshelf);
					fSidechainBandpass.SetType(SR::DSP::BiquadHighpass);
					break;
				default:
					break;
				}
			}

			virtual void SetFrequency(double freq) {
				mFilterFreq = freq;
				fSidechainBandpass.SetFreq(mFilterFreq);
				fDynamicEqFilter.SetFreq(mFilterFreq);
			}
			virtual void SetQ(double q) {
				mFilterQ = q;
				fSidechainBandpass.SetQ(q);
				fDynamicEqFilter.SetQ(q);
			}

			virtual void SetGain(double gainDb) {
				mFilterGain = gainDb;
				fDynamicEqFilter.SetPeakGain(gainDb);
			}

			void Process(double& in1, double& in2); // compressor runtime process if internal sidechain 

			SRFilterIIR<double, 2> fSidechainBandpass, fDynamicEqFilter;
		private:
			void process(double& in1, double& in2, double sidechain);	// with stereo-linked key in
			// transfer function
			double mFilterFreq;
			double mFilterQ;
			double mFilterGain;
		};

		// Inline deesser runtime method
		INLINE void SRDeesser::Process(double& in1, double& in2) {
			// create sidechain
			double rectifiedInput1 = in1;
			double rectifiedInput2 = in2;
			rectifiedInput1 = fSidechainBandpass.Process(rectifiedInput1, 0);
			rectifiedInput2 = fSidechainBandpass.Process(rectifiedInput2, 1);
			// rectify input
			rectifiedInput1 = fabs(rectifiedInput1);
			rectifiedInput2 = fabs(rectifiedInput2);
			// If desired, one could use another EnvelopeDetector to smooth the rectified signal.
			// link channels with greater of 2
			double rectifiedInputMaxed = std::max(rectifiedInput1, rectifiedInput2);
			process(in1, in2, rectifiedInputMaxed);	// rest of process
		}

		// Internal deesser runtime method
		INLINE void SRDeesser::process(double& in1, double& in2, double sidechain) {
			sidechain = fabs(sidechain);		// rectify (just in case)
			sidechain += DC_OFFSET;				  // add DC offset to avoid log( 0 )
			double sidechainDb = SR::Utils::AmpToDB(sidechain);	// convert linear -> dB
			double sampleOvershootDb = sidechainDb - mThreshDb;	// delta over threshold
			sampleOvershootDb += DC_OFFSET;					// add DC offset to avoid denormal, see NOTE 1
			SRDynamicsDetector::process(sampleOvershootDb, currentOvershootDb);	// process attack/release envelope
			sampleOvershootDb = currentOvershootDb - DC_OFFSET; // subtract DC offset
			double grRaw;

			if (sampleOvershootDb > mKneeWidthDb * 0.5) {
				grRaw = (mRatio - 1.) * (sampleOvershootDb);											// For linear gain reduction above knee range
			}
			else if (fabs(sampleOvershootDb) <= mKneeWidthDb * 0.5) {
				grRaw = ((mRatio - 1.) * std::pow(sampleOvershootDb + mKneeWidthDb * 0.5, 2.)) / (2. * mKneeWidthDb); // For smoothed gain reduction within knee range
			}
			else {
				grRaw = 0.;																			// For no gain reduction below knee range
			}


			mGrDb = grRaw;
			mGrLin = SR::Utils::DBToAmp(grRaw);
			fDynamicEqFilter.SetPeakGain(mFilterGain + grRaw);
			in1 = fDynamicEqFilter.Process(in1, 0);
			in2 = fDynamicEqFilter.Process(in2, 1);
		}






	}	// end namespace DSP
}	// end namespace SR


/*
	* -----------------------------------------------------------------------------
	* NOTE 1:
	* REGARDING THE DC OFFSET: In this case, since the offset is added before
	* the attack/release processes, the envelope will never fall below the offset,
	* thereby avoiding denormals. However, to prevent the offset from causing
	* constant gain reduction, we must subtract it from the envelope, yielding
	* a minimum value of 0dB.
	* -----------------------------------------------------------------------------
	*
	* NOTE 2:
	* REGARDING THE RMS AVERAGER: Ok, so this isn't a REAL RMS
	* calculation. A true RMS is an FIR moving average. This
	* approximation is a 1-pole IIR. Nonetheless, in practice,
	* and in the interest of simplicity, this method will suffice,
	* giving comparable results.
	* -----------------------------------------------------------------------------
	*
	* NOTE 3:
	* REGARDING THE GAIN REDUCTION: Due to the logarithmic nature
	* of the attack phase, the sidechain will never achieve "full"
	* attack. (Actually, it is only guaranteed to achieve 99% of
	* the input value over the given time constant.) As such, the
	* limiter cannot achieve "brick-wall" limiting. There are 2
	* workarounds:
	*
	* 1) Set the threshold slightly lower than the desired threshold.
	*		i.e. 0.0dB -> -0.1dB or even -0.5dB
	*
	* 2) Clip the output at the threshold, as such:
	*
	*		if ( in1 > mThreshLin )		in1 = mThreshLin;
	*		else if ( in1 < -mThreshLin )	in1 = -mThreshLin;
	*
	*		if ( in2 > mThreshLin )		in2 = mThreshLin;
	*		else if ( in2 < -mThreshLin )	in2 = -mThreshLin;
	*
	*		(... or replace with your favorite branchless clipper ...)
	* -----------------------------------------------------------------------------
	*
	* NOTE 4:
	* REGARDING THE ATTACK: This limiter achieves "look-ahead" detection
	* by allowing the envelope follower to attack the max peak, which is
	* held for the duration of the attack phase -- unless a new, higher
	* peak is detected. The output signal is buffered so that the gain
	* reduction is applied in advance of the "offending" sample.
	*
	* NOTE: a DC offset is not necessary for the envelope follower,
	* as neither the max peak nor envelope should fall below the
	* threshold (which is assumed to be around 1.0 linear).
	* -----------------------------------------------------------------------------
	*
	* NOTE 5:
	* REGARDING THE MAX PEAK: This method assumes that the only important
	* sample in a look-ahead buffer would be the highest peak. As such,
	* instead of storing all samples in a look-ahead buffer, it only stores
	* the max peak, and compares all incoming samples to that one.
	* The max peak has a hold time equal to what the look-ahead buffer
	* would have been, which is tracked by a timer (counter). When this
	* timer expires, the sample would have exited from the buffer. Therefore,
	* a new sample must be assigned to the max peak. We assume that the next
	* highest sample in our theoretical buffer is the current input sample.
	* In reality, we know this is probably NOT the case, and that there has
	* been another sample, slightly lower than the one before it, that has
	* passed the input. If we do not account for this possibility, our gain
	* reduction could be insufficient, resulting in an "over" at the output.
	* To remedy this, we simply apply a suitably long release stage in the
	* envelope follower.

*/