//
//	SRChannel.h
//	Copyright Johannes Menzel 2018 (MIT)
//
//	External libraries used or modified: 
//	-----------------------------------------------------------------------------------------------------------------------------------
//	Compressor Algorithm by Bojan Markovic (https://github.com/music-dsp-collection/chunkware-simple-dynamics/tree/master/simpleSource)
//	Biquad Source by Nigel Redmon (http://www.earlevel.com/main/2012/11/26/biquad-c-source-code/)
//	...
//	
//	For more information see specific source


#pragma warning(disable: 4312)
#pragma warning(disable: 4311)
#pragma warning(disable: 4302)

#ifndef __SRCHANNEL__
#define __SRCHANNEL__

#include "IPlug_include_in_plug_hdr.h"
#include "../SRClasses/SRGain.h"
#include "../SRClasses/SRFilters.h"
#include "../SRClasses/SRDynamics.h"
#include "../SRClasses/SRSaturation.h"
#include "../SRClasses/SRHelpers.h"
#include "../SRClasses/SRControls.h"
#include "../SRClasses/Oversampler.h"
#include "resample.h"
#include "denormal.h"
#include <string>
#define _USE_MATH_DEFINES
#include <math.h>
#define _USE_MATH_DEFINES
#include <cmath>
#include <algorithm>
#include <vector>
const int circularBufferLenght = 65536;



	class SRChannel : public IPlug
	{
	public:
		SRChannel(IPlugInstanceInfo instanceInfo);
		~SRChannel();

		void Reset();
		void OnParamChange(int paramIdx);
		void ProcessDoubleReplacing(double** inputs, double** outputs, int nFrames);
		// CIRCULAR BUFFER
		double circularBufferInL[circularBufferLenght], circularBufferInR[circularBufferLenght], circularBufferOutL[circularBufferLenght], circularBufferOutR[circularBufferLenght];

	private:
		double mSampleRate,
			// GAIN + PAN
			mInputGain, mOutputGain, mInputDrive,
			mPan, mSafePanFreq,
			vSafePanLowSignal1, vSafePanHighSignal1, vSafePanLowSignal2, vSafePanHighSignal2,

			// CLIP + LIMIT
			mClipperThreshold, mSaturationAmount, mSaturationHarmonics, mLimiterThresh, mSaturationSkew,

			//mSatLfGain, mSatLfFreq, mSatMfGain, mSatMfFreq, mSatHfGain, mSatHfFreq, mSatHpFreq, mSatLpFreq,

			// EQ
			mEqHpFreq, mEqLpFreq,
			mEqLfGain, mEqLfFreq, mEqLfQ,
			mEqLmfGain, mEqLmfFreq, mEqLmfQ,
			mEqHmfGain, mEqHmfFreq, mEqHmfQ,
			mEqHfGain, mEqHfFreq, mEqHfQ,

			// COMP
			mCompPeakThresh, mCompPeakRatio, mCompPeakAttack, mCompPeakRelease,
			mCompRmsThresh, mCompRmsRatio, mCompRmsAttack, mCompRmsRelease,
			mCompRmsMakeup, mCompPeakMakeup, mCompRmsAutoMakeup, mCompPeakAutoMakeup,
			mCompPeakRmsRatio, mCompDryWet,
			mCompPeakSidechainFilterFreq,
			mCompPeakKneeWidthDb, mCompRmsKneeWidthDb,

			//vCompDry1, vCompDry2,
			//vCompRmsIn1, vCompPeakIn1, vCompRmsIn2, vCompPeakIn2,

			// DEESSER
			mDeesserFreq, mDeesserQ,
			mDeesserThresh, mDeesserRatio, mDeesserAttack, mDeesserRelease, mDeesserMakeup,

		


			// TESTPARAM
			mTestParam1, mTestParam2, mTestParam3, mTestParam4, mTestParam5,

			// METERS
			mInputPeakMeterValue1, mInputPeakMeterValue2, mOutputPeakMeterValue1, mOutputPeakMeterValue2,
			mRmsGrMeterValue, mPeakGrMeterValue, mDeesserMeterValue,

			mInputPeakMeterPreviousValue1, mInputPeakMeterPreviousValue2, mOutputPeakMeterPreviousValue1, mOutputPeakMeterPreviousValue2,
			mInputPeakMeterTimeConst1, mInputPeakMeterTimeConst2, mOutputPeakMeterTimeConst1, mOutputPeakMeterTimeConst2,

			mOutputVuMeterValue1, mOutputVuMeterValue2, mOutputVuMeterSos1, mOutputVuMeterSos2;

		// FUNCT
		//SetShapeCentered(char csetshapeparam, double cCenteredValue, double cControlPosition),

		// FREQRESP
		//meterFreqRespValues[64];


	// BOOL VARS
		bool mEqLfIsBell, mEqHfIsBell, mCompIsParallel, mCompPeakIsExtSc, mCompRmsIsExtSc, mCompPeakIsFeedback, mCompRmsIsFeedback, mEqBypass, mCompBypass, mInputBypass, mOutputBypass, mBypass, mIsPanMonoLow,
			mAgc;

		// INT VARS
		int mEqHpOrder, mSaturationType,

			// METERS
			cInputPeakMeter1, cInputPeakMeter2, cOutputPeakMeter1, cOutputPeakMeter2,
			cRmsGrMeter, cPeakGrMeter, cDeesserMeter, /*cFreqRespGraph, */cOutputVuMeter1, cOutputVuMeter2;


		// CIRCULAR BUFFER
		unsigned short int circularBufferPointer;
		
		std::vector<int> cControlMatrix;

		// FUNCTIONS
		void CreatePresets(),
			CreateParams(),
			CreateGraphics(),
			InitGUI(),
			InitGain(),
			InitBiquad(),
			InitCompPeak(),
			InitCompRms(),
			InitExtSidechain(),
			InitDeesser(),
			InitLimiter(),
			InitSaturation(),
			InitSafePan(),
			InitMeter(),
			GrayOutControls();
		//CalculateFreqResp(),

	// FILTERS
		// Gain Filters
		SRPlugins::SRGain::SRGain fInputGain, fOutputGain;
		SRPlugins::SRGain::SRPan fPan;

		// Spectral Filters
		SRPlugins::SRFilters::SRFiltersTwoPole fEqHpFilter1L, fEqHpFilter2L, fEqHpFilter3L, fEqHpFilter4L, fEqHpFilter5L, fEqHpFilter6L, fEqHpFilter7L, fEqHpFilter8L, fEqHpFilter9L, fEqHpFilter10L,
			fEqLpFilter1L,
			fEqLfFilterL, fEqLmfFilterL, fEqHmfFilterL, fEqHfFilterL,
			fEqHpFilter1R, fEqHpFilter2R, fEqHpFilter3R, fEqHpFilter4R, fEqHpFilter5R, fEqHpFilter6R, fEqHpFilter7R, fEqHpFilter8R, fEqHpFilter9R, fEqHpFilter10R,
			fEqLpFilter1R,
			fEqLfFilterR, fEqLmfFilterR, fEqHmfFilterR, fEqHfFilterR,
			// Safe-Pan
			fSafePanHpL, fSafePanLpL, fSafePanHpR, fSafePanLpR,

			// Deesser
			fDeesserSidechainBandpassFilterL, fDeesserSidechainBandpassFilterR, fDeesserReductionPeakFilterL, fDeesserReductionPeakFilterR;

		SRPlugins::SRFilters::SRFiltersOnePole fDcBlockerL, fDcBlockerR, fEqHpFilterOnepoleL, fEqHpFilterOnepoleR, fEqLpFilterOnepoleL, fEqLpFilterOnepoleR;

		// Dynamic Filters
		SRPlugins::SRDynamics::SRCompressor fCompressorPeak;
		SRPlugins::SRDynamics::SRCompressorRMS fCompressorRms;
		SRPlugins::SRDynamics::SRLimiter fLimiter;
		SRPlugins::SRDynamics::SRDeesser fDeesser;

		SRPlugins::SRDynamics::EnvelopeDetector fOutputVuMeterEnvelopeDetector1, fOutputVuMeterEnvelopeDetector2;
		SRPlugins::SRDynamics::AttRelEnvelope fOutputVuMeterEnvelope1, fOutputVuMeterEnvelope2;

		// Saturation
		SRPlugins::SRSaturation::SRSaturation fInputSaturation;

		// TESTVARS
		double sumIn;
		double sumOut;
		double aveIn;
		double aveOut;
		double diffInOut;

	};

#endif // #ifndef __SRCHANNEL__

