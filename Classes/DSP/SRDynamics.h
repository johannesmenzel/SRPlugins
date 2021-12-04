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
        mTimeConstantMs = std::max(ms, 1. / mSampleRate);
        setCoef();
      }

      virtual ~SRDynamicsEnvelope() {}

      virtual double getTc(void) const { return mTimeConstantMs; }
      virtual double getSampleRate(void) const { return mSampleRate; }

      virtual void setTc(double ms) {
        assert(ms > 0.0);
        mTimeConstantMs = std::max(ms, 1. / mSampleRate);
        setCoef();
      }

      virtual void setSampleRate(double sampleRate) {
        assert(sampleRate > 0.0);
        mSampleRate = sampleRate;
        setCoef();
      }

      // Runtime method of Envelope detector
      INLINE void process(double in, double &state) {
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
      SRDynamicsDetector (double mAttackMs = 10.0, double mReleaseMs = 100.0, double sampleRate = 44100.0)
        : mEnvelopeAttack(mAttackMs, sampleRate)
        , mEnvelopeRelease(mReleaseMs, sampleRate)
      {
      }
      virtual ~SRDynamicsDetector() {}

      virtual double GetAttack(void) const { return mEnvelopeAttack.getTc(); }
      virtual double GetRelease(void) const { return mEnvelopeRelease.getTc(); }
      virtual double GetSampleRate(void) const { return mEnvelopeAttack.getSampleRate(); }

      virtual void SetAttack(double ms) { mEnvelopeAttack.setTc(ms); }
      virtual void SetRelease(double ms) { mEnvelopeRelease.setTc(ms); }
      virtual void SetSampleRate(double sampleRate) {
        mEnvelopeAttack.setSampleRate(sampleRate);
        mEnvelopeRelease.setSampleRate(sampleRate);
      }

      // RUNTIME
      INLINE void process(double in, double &state) {
        if (in > state)
          mEnvelopeAttack.process(in, state);
        else
          mEnvelopeRelease.process(in, state);
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

      // Constructor
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
        , fMakeup(100)
        , fAutoMakeup(100)
      {
      }

      // Destructor
      virtual ~SRDynamicsBase() {}

      // Sets dynamic processors threshold in dB
      virtual void SetThresh(double threshDb) {
        mThreshDb = threshDb;
        mThreshLin = SR::Utils::DBToAmp(threshDb);
        if (mIsAutoMakeup) AdjustAutoMakeup();
      }

      // Sets dynamic processors ratio
      virtual void SetRatio(double ratio) {
        assert(ratio >= 0.0);
        mRatio = ratio;
        if (mIsAutoMakeup) AdjustAutoMakeup();
      }

      // Sets dynamic processors makeup gain in dB
      virtual void SetMakeup(double makeupDb) {
        mMakeup = SR::Utils::DBToAmp(makeupDb);
        fMakeup.SetGain(mMakeup);
      }

       // Sets if dynamic processor compensates gain reduction automatically
      virtual void SetIsAutoMakeup(bool autoMakeup) {
        mIsAutoMakeup = autoMakeup;
        if (mIsAutoMakeup) AdjustAutoMakeup();
      }

      // Sets target loudness of the track
      virtual void SetReference(double referenceDb) {
        mReferenceDb = referenceDb;
        if (mIsAutoMakeup) AdjustAutoMakeup();
      }

      // Sets soft knee width in dB
      virtual void SetKnee(double kneeDb) {
        assert(kneeDb >= 0.0);
        mKneeWidthDb = kneeDb;
      }

      // Call before runtime, typically in OnReset() or similar
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
          fAutoMakeup.SetGain(mAutoMakeup);
      }

      SRGain fMakeup, fAutoMakeup;
      double mThreshDb;               // Dynamic processors threshold in dB
      double mThreshLin;              // Dynamic processors linear threshold
      double mRatio;                  // Dynamic processors ratio
      double mMakeup;                 // Dynamic processors makeup gain (linear voltage)
      double mKneeWidthDb;            // Dynamic processors soft knee width in dB
      double mGrLin;                  // Dynamic processors linear gain reduction (0..1)
      double mGrDb;                   // Dynamic processors logarithmic gain reduction
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
        : SRDynamicsDetector(10.0, 100.0)
        , SRDynamicsBase(0.0, 1.0)
        , mSidechainFc(0.0)
        , mTopologyFeedback(false)
        , mMaxGr(0.0)
        , sidechainSignal1(0.0)
        , sidechainSignal2(0.0)
      {
      }
      virtual ~SRCompressor() {}

      // parameters
      virtual void InitCompressor(double threshDb, double ratio, double attackMs, double releaseMs, double sidechainFc, double kneeDb, bool isFeedbackCompressor, bool autoMakeup, double referenceDb, double samplerate) {
        SRDynamicsDetector::SetSampleRate(samplerate);
        SRDynamicsBase::SetThresh(threshDb);
        SRDynamicsBase::SetRatio(ratio);
        SRDynamicsBase::SetIsAutoMakeup(autoMakeup);
        SRDynamicsBase::SetReference(referenceDb);
        SRDynamicsDetector::SetAttack(attackMs);
        SRDynamicsDetector::SetRelease(releaseMs);
        InitSidechainFilter(sidechainFc);
        SRDynamicsBase::SetKnee(kneeDb);
        SetTopologyFeedback(isFeedbackCompressor);
        SRDynamicsBase::Reset();
      }

      virtual void SetMaxGrDb(double maxGrDb, bool sigmoid = true) {
        if (!sigmoid)
          mMaxGr = maxGrDb;
        else {
          const double tempratio = 1. / mRatio;
          mMaxGr = (maxGrDb + (maxGrDb * 9.) / (maxGrDb * tempratio - maxGrDb - 9.)); // Simplified P4 sigmoid fitting with d+\frac{da}{dx-d-a} with f(1) = 0.0
        }
      }

      virtual void InitSidechainFilter(double sidechainFC) {
        mSidechainFc = sidechainFC;
        fSidechainFilter.SetFilter(SRFilterIIR<double, 2>::EFilterType::BiquadHighpass, sidechainFC, 0.7071, 0., SRDynamicsDetector::GetSampleRate());
      }

      virtual void SetSidechainFilterFreq(double sidechainFc) {
        mSidechainFc = sidechainFc;
        fSidechainFilter.SetFreq(mSidechainFc);
      }

      virtual void SetTopologyFeedback(bool isFeedbackCompressor) { mTopologyFeedback = isFeedbackCompressor; }

      void Process(double &in1, double &in2); // Compressor runtime process for internal sidechain 
      void Process(double &in1, double &in2, double &extSC1, double &extSC2); // Compressor runtime process for external sidechain
      void process(double &in1, double &in2, double sidechain);	// Compressor runtime process with stereo-linked key

    protected:
      SRFilterIIR<double, 2> fSidechainFilter; // Compressors stereo sidechain filter
      bool mTopologyFeedback; // True if its a feedback compressor, false for modern feedforward
      double sidechainSignal1, sidechainSignal2;      // Gain reduced signal to get used as new sidechain for feedback topology
      double mSidechainFc;  // Compressors stereo sidechain filters center frequency
      double mMaxGr;  // Maximum gain reduction for gain reduction limiting (no brickwall, just gets damped there)
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
        mEnvelopeAverager.setSampleRate(sampleRate);
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
        mEnvelopeAverager.setTc(rmsWindowMs);
        SRDynamicsBase::Reset();
      }

      // RMS window
      virtual void SetWindow(double ms) {
        mEnvelopeAverager.setTc(ms);
      }
      virtual double GetWindow(void) const { return mEnvelopeAverager.getTc(); }

      void Process(double &in1, double &in2, double &extSC1, double &extSC2);
      void Process(double &in1, double &in2);	// compressor runtime process

    private:
      SRDynamicsEnvelope mEnvelopeAverager;	// averager
    };



    //-------------------------------------------------------------
    // COMPRESSOR Inline Functions
    //-------------------------------------------------------------

    // Compressor runtime process for internal sidechain 
    INLINE void SRCompressor::Process(double &in1, double &in2) {
      double rectifiedInput1 = (!mTopologyFeedback) ? in1 : sidechainSignal1;
      double rectifiedInput2 = (!mTopologyFeedback) ? in2 : sidechainSignal2;
      if (mSidechainFc > 16. / GetSampleRate()) {
        rectifiedInput1 = fSidechainFilter.Process(rectifiedInput1, 0);
        rectifiedInput2 = fSidechainFilter.Process(rectifiedInput2, 1);
      }
      rectifiedInput1 = std::fabs(rectifiedInput1);	// rectify input
      rectifiedInput2 = std::fabs(rectifiedInput2);

      // If desired, one could use another EnvelopeDetector to smooth the rectified signal.

      double rectifiedInputMaxed = std::max(rectifiedInput1, rectifiedInput2);	// link channels with greater of 2
      process(in1, in2, rectifiedInputMaxed);	// rest of process
    }

    // Compressor runtime process for external sidechain
    INLINE void SRCompressor::Process(double &in1, double &in2, double &extSC1, double &extSC2) {
      double rectifiedInput1 = extSC1;
      double rectifiedInput2 = extSC2;
      if (mSidechainFc > 16. / GetSampleRate()) {
        rectifiedInput1 = fSidechainFilter.Process(rectifiedInput1, 0);
        rectifiedInput2 = fSidechainFilter.Process(rectifiedInput2, 1);
      }

      rectifiedInput1 = std::fabs(rectifiedInput1);	// rectify input
      rectifiedInput2 = std::fabs(rectifiedInput2);

      // If desired, one could use another EnvelopeDetector to smooth the rectified signal.

      double rectifiedInputMaxed = std::max(rectifiedInput1, rectifiedInput2);	// link channels with greater of 2
      process(in1, in2, rectifiedInputMaxed);	// rest of process
    }

    // Inline RMS Compressor Sidechain
    //-------------------------------------------------------------

    // RMS Compressor runtime process for internal sidechain 
    INLINE void SRCompressorRMS::Process(double &in1, double &in2) {
      double squaredInput1 = (!mTopologyFeedback) ? in1 * in1 : sidechainSignal1 * sidechainSignal1;	// square input
      double squaredInput2 = (!mTopologyFeedback) ? in2 * in2 : sidechainSignal2 * sidechainSignal2;
      double summedSquaredInput = squaredInput1 + squaredInput2;			// power summing
      summedSquaredInput += DC_OFFSET;					// DC offset, to prevent denormal
      mEnvelopeAverager.process(summedSquaredInput, mAverageOfSquares);		// average of squares
      double sidechainRms = sqrt(mAverageOfSquares);	// sidechainRms (sort of ...), See NOTE 2

      SRCompressor::process(in1, in2, sidechainRms);	// rest of process
    }

    // RMS Compressor runtime process for external sidechain 
    INLINE void SRCompressorRMS::Process(double &in1, double &in2, double &extSC1, double &extSC2) {
      double squaredInput1 = extSC1 * extSC1;	// square input
      double squaredInput2 = extSC2 * extSC2;
      double summedSquaredInput = squaredInput1 + squaredInput2;			// power summing
      summedSquaredInput += DC_OFFSET;					// DC offset, to prevent denormal
      mEnvelopeAverager.process(summedSquaredInput, mAverageOfSquares);		// average of squares
      double sidechainRms = sqrt(mAverageOfSquares);	// sidechainRms (sort of ...), see NOTE 2
      SRCompressor::process(in1, in2, sidechainRms);	// rest of process
    }

    // Inline all compressors process
    // This is the protected method all compressors input sidechain methods call
    INLINE void SRCompressor::process(double &in1, double &in2, double sidechain) {
      sidechain = fabs(sidechain);                          // rect (just in case)
      sidechain += DC_OFFSET;                               // add DC offset to avoid log( 0 )
      double sidechainDb = SR::Utils::AmpToDB(sidechain);   // linear -> dB conversion
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
      //if (mMaxGr != 0.0) {
      //  grlimit = grRaw / (mMaxGr * 0.5);
      //  grlimitsqrt = std::pow(grlimit, 0.8);

      //  grRaw = (1. - grlimitsqrt < 0.)
      //    ? grRaw + ((1. - grlimitsqrt) * (grRaw - (mMaxGr * 0.5))) / grlimit
      //    : grRaw;
      //}
      if (grRaw < mMaxGr && mMaxGr < 0.0) {
        //grRaw = mMaxGr + 0.5 * mMaxGr / (grRaw - mMaxGr - 1.);
        grRaw = mMaxGr - std::tanh(mMaxGr - grRaw);
      }

      mGrDb = grRaw; // Store logarithmic gain reduction
      grRaw = mGrLin = SR::Utils::DBToAmp(grRaw);// Logarithmic to linear conversion
      // Apply gain reduction to inputs:
      in1 *= grRaw;
      in2 *= grRaw;

      // for feedback topology set old processed inputs as new sidechain.
      sidechainSignal1 = in1;
      sidechainSignal2 = in2;

      // Apply makeup gain
      fMakeup.Process(in1, in2);
      if (mIsAutoMakeup) {
        fAutoMakeup.Process(in1, in2);
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
        unsigned int samp = int(0.001 * ms * mEnvelopeDetectorAttack.getSampleRate());

        assert(samp < BUFFER_SIZE);

        mPeakHoldSamples = samp;
        mEnvelopeDetectorAttack.setTc(ms);
      }
      virtual void SetRelease(double ms) {
        mEnvelopeDetectorRelease.setTc(ms);
      }
      virtual double GetAttack(void)  const { return mEnvelopeDetectorAttack.getTc(); }
      virtual double GetRelease(void) const { return mEnvelopeDetectorRelease.getTc(); }
      virtual void   SetSampleRate(double sampleRate) {
        mEnvelopeDetectorAttack.setSampleRate(sampleRate);
        mEnvelopeDetectorRelease.setSampleRate(sampleRate);
      }
      virtual double GetSampleRate(void) { return mEnvelopeDetectorAttack.getSampleRate(); }
      virtual const unsigned int GetLatency(void) const { return mPeakHoldSamples; }

      virtual void Reset(void) {
        mPeakHoldTimer = 0;
        mMaxPeak = mThreshLin;
        currentOvershootLin = mThreshLin;
        mCursor = 0;
        mOutputBuffer[0].assign(BUFFER_SIZE, 0.0);
        mOutputBuffer[1].assign(BUFFER_SIZE, 0.0);
      }			// call before runtime (in resume())
      void Process(double &in1, double &in2);	// limiter runtime process

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
    INLINE void SRLimiter::Process(double &in1, double &in2)
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
        mEnvelopeDetectorAttack.process(mMaxPeak, currentOvershootLin);		// process attack phase
      else
        mEnvelopeDetectorRelease.process(mMaxPeak, currentOvershootLin);		// process release phase

                                                                        // See NOTE 4

                        // gain reduction
      double grRaw = mThreshLin / currentOvershootLin;

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

      void Process(double &in1, double &in2);	// gate runtime process
      void Process(double &in1, double &in2, double keyLinked);	// with stereo-linked key in
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
        mEnvelopeAverager.setSampleRate(sampleRate);
      }

      // RMS window
      virtual void setWindow(double ms) {
        mEnvelopeAverager.setTc(ms);
      }
      virtual double getWindow(void) const { return mEnvelopeAverager.getTc(); }

      // runtime process
      void Process(double &in1, double &in2);	// gate runtime process

    private:

      SRDynamicsEnvelope mEnvelopeAverager;	// averager

    };






    //-------------------------------------------------------------
    // GATE Inline Functions
    //-------------------------------------------------------------

    // Inline Gate Sidechain
    //-------------------------------------------------------------
    INLINE void SRGate::Process(double &in1, double &in2)
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
    INLINE void SRGateRMS::Process(double &in1, double &in2)
    {
      // create sidechain

      double squaredInput1 = in1 * in1;	// square input
      double squaredInput2 = in2 * in2;

      double summedSquaredInput = squaredInput1 + squaredInput2;			// power summing
      summedSquaredInput += DC_OFFSET;					// DC offset, to prevent denormal
      mEnvelopeAverager.process(summedSquaredInput, mAverageOfSquares);		// average of squares
      double sidechainRms = sqrt(mAverageOfSquares);	// sidechainRms (sort of ...), See NOTE 2

      SRGate::Process(in1, in2, sidechainRms);	// rest of process
    }

    // Inline Gates Process
    //-------------------------------------------------------------
    INLINE void SRGate::Process(double &in1, double &in2, double sidechain)
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
        , fSidechainBandpass(SRFilterIIR<double, 2>(SRFilterIIR<double, 2>::EFilterType::BiquadBandpass, 0.5, 0.707, 0.0, 44100.0))
        , fDynamicEqFilter(SRFilterIIR<double, 2>(SRFilterIIR<double, 2>::EFilterType::BiquadPeak, 0.5, 0.707, 0.0, 44100.0))
      {
      }
      virtual ~SRDeesser() {}

      // parameters
      virtual void SetDeesser(double threshDb, double ratio, double attackMs, double releaseMs, double normalizedFreq, double q, double kneeDb, double samplerate) {
        SRDynamicsBase::SetThresh(threshDb);
        SRDynamicsBase::SetRatio(ratio);
        SRDynamicsDetector::SetAttack(attackMs);
        SRDynamicsDetector::SetRelease(releaseMs);
        SRDynamicsDetector::SetSampleRate(samplerate);
        InitFilter(normalizedFreq, q);
        SetKnee(kneeDb);
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
      virtual void InitFilter(double freq, double q) {
        mFilterFreq = freq;
        mFilterQ = q;
        fSidechainBandpass.SetFilter(SRFilterIIR<double, 2>::EFilterType::BiquadBandpass, mFilterFreq, mFilterQ, 0.0, GetSampleRate());
        fDynamicEqFilter.SetFilter(SRFilterIIR<double, 2>::EFilterType::BiquadPeak, mFilterFreq, mFilterQ, 0.0, GetSampleRate());
      }

      void Process(double &in1, double &in2); // compressor runtime process if internal sidechain 
      void process(double &in1, double &in2, double sidechain);	// with stereo-linked key in
      SRFilterIIR<double, 2> fSidechainBandpass, fDynamicEqFilter;

    private:
      // transfer function
      double mFilterFreq;
      double mFilterQ;
      double mFilterGain;
    };

    // Inline deesser runtime method
    INLINE void SRDeesser::Process(double &in1, double &in2) {
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
    INLINE void SRDeesser::process(double &in1, double &in2, double sidechain) {
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
      fDynamicEqFilter.SetPeakGain(grRaw);
      in1 = fDynamicEqFilter.Process(in1, 0);
      in2 = fDynamicEqFilter.Process(in2, 1);
    }






  }	// end namespace SRDynamics
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