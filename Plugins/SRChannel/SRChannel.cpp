#include "SRChannel.h"
#include "IPlug_include_in_plug_src.h"
#include "IControls.h"
#include "../../Classes/Graphics/SRCustomKnob.h"
#include "../../Classes/Graphics/SRCustomLayout.h"
#include "../../Classes/Graphics/SRCustomGraph.h"

#define FREQUENCYRESPONSE 300


SRChannel::SRChannel(const InstanceInfo& info)
	: Plugin(info, MakeConfig(kNumParams, kNumPresets))
	, fGainIn(100)
	, fGainOut(100)
	, fGainOutLow(100)
	, fEqHp()
	, fEqLp()
	, fEqLfBoost()
	, fEqLfCut()
	, fEqHfBoost()
	, fEqHfCut()
	, fEqLmf()
	, fEqHmf()
	, fCompRms()
	, fCompPeak()
	, fMeterEnvelope()
	, mFreqMeterValues(new float[FREQUENCYRESPONSE])

{
	GetParam(kGainIn)->InitDouble("Input", 0., -120., 12., 0.01, "dB", 0, "Gain", IParam::ShapePowCurve(SR::Utils::SetShapeCentered(-120., 12., 0., .5)));
	GetParam(kGainOut)->InitDouble("Output", 0., -120., 12., 0.01, "dB", 0, "Gain", IParam::ShapePowCurve(SR::Utils::SetShapeCentered(-120., 12., 0., .5)));

	GetParam(kSaturationDrive)->InitDouble("Sat Drive", 0., 0., 24., 0.01, "dB", 0, "Sat");
	GetParam(kSaturationAmount)->InitDouble("Sat Amount", 0., 0., 100., 1., "%", 0, "Sat");

	GetParam(kStereoPan)->InitDouble("Pan", 0., -100., 100., 1., "%", 0, "Stereo");
	GetParam(kStereoWidth)->InitDouble("Width", 100., 0., 1000., 1., "%", 0, "Stereo", IParam::ShapePowCurve(SR::Utils::SetShapeCentered(0., 1000., 100., .5)));
	GetParam(kStereoWidthLow)->InitDouble("Bass Width", 100., 0., 100., 1., "%", 0, "Stereo", IParam::ShapePowCurve(SR::Utils::SetShapeCentered(0., 100., 50., .5)));
	GetParam(kStereoMonoFreq)->InitDouble("Split FQ", 20., 20., 1000., .01, "Hz", 0, "Stereo", IParam::ShapePowCurve(SR::Utils::SetShapeCentered(0., 1000., 100., .5)));

	GetParam(kEqHpFreq)->InitDouble("HP", 20., 20., 400., 10., "Hz", IParam::EFlags::kFlagStepped);
	GetParam(kEqLpFreq)->InitDouble("LP", 22000., 3000., 22000., 1000., "Hz", IParam::EFlags::kFlagStepped);

	GetParam(kEqHfBoost)->InitDouble("HF Boost", 0., -0., 10., 1., "dB", IParam::EFlags::kFlagStepped);
	GetParam(kEqHfCut)->InitDouble("HF Cut", 0., -0., 10., 1., "dB", IParam::EFlags::kFlagStepped);
	GetParam(kEqHfFreq)->InitDouble("HF Freq", 8000., 3000., 16000., 1000., "Hz", IParam::EFlags::kFlagStepped, "", IParam::ShapePowCurve(SR::Utils::SetShapeCentered(3000., 16000., 8000., .5)));
	GetParam(kEqHfDs)->InitDouble("HF DS", 0., -20., 0., .01, "dB");

	GetParam(kEqHmfGain)->InitDouble("HMF Gain", 0., -12., 12., 1., "dB", IParam::EFlags::kFlagStepped);
	GetParam(kEqHmfFreq)->InitDouble("HMF Freq", 3000., 600., 7000., 1., "Hz", 0, "", IParam::ShapePowCurve(SR::Utils::SetShapeCentered(600., 7000., 3000., .5)));
	GetParam(kEqHmfQ)->InitDouble("HMF Q", .707, 0.1, 10., 0.01, "", 0, "", IParam::ShapePowCurve(SR::Utils::SetShapeCentered(.1, 10., .707, .5)));
	GetParam(kEqHmfDs)->InitDouble("HMF DS", 0., -20., 0., .01, "dB");

	GetParam(kEqLmfGain)->InitDouble("LMF Gain", 0., -12., 12., 1., "dB", IParam::EFlags::kFlagStepped);
	GetParam(kEqLmfFreq)->InitDouble("LMF Freq", 1000., 200., 2500., 1., "Hz", 0, "", IParam::ShapePowCurve(SR::Utils::SetShapeCentered(200., 2500., 1000., .5)));
	GetParam(kEqLmfQ)->InitDouble("LMF Q", .707, 0.1, 10., 0.01, "", 0, "", IParam::ShapePowCurve(SR::Utils::SetShapeCentered(.1, 10., .707, .5)));
	GetParam(kEqLmfDs)->InitDouble("LMF DS", 0., -20., 0., .01, "dB");

	GetParam(kEqLfBoost)->InitDouble("LF Boost", 0., 0., 10., 1., "dB", IParam::EFlags::kFlagStepped);
	GetParam(kEqLfCut)->InitDouble("LF Cut", 0., 0., 10., 1., "dB", IParam::EFlags::kFlagStepped);
	GetParam(kEqLfFreq)->InitDouble("LF Freq", 100., 30., 300., 10., "Hz", IParam::EFlags::kFlagStepped, "", IParam::ShapePowCurve(SR::Utils::SetShapeCentered(30., 300., 100., .5)));
	GetParam(kEqLfDs)->InitDouble("LF DS", 0., -20., 0., .01, "dB");

	GetParam(kCompRmsThresh)->InitDouble("Level Thresh", 0., -40., 0., 0.1, "dB");
	GetParam(kCompRmsRatio)->InitDouble("Level Ratio", 2., 1., 6., .5, ":1", IParam::EFlags::kFlagStepped, "", IParam::ShapePowCurve(SR::Utils::SetShapeCentered(1., 6., 2., .5)));
	GetParam(kCompRmsAttack)->InitDouble("Level Attack", 15., 5., 50., 0.01, "ms");
	GetParam(kCompRmsRelease)->InitDouble("Level Release", 300., 100., 3000., 1., "ms");
	GetParam(kCompRmsMakeup)->InitDouble("Level Makeup", 0., -12., 12., 0.01, "dB");
	GetParam(kCompRmsMix)->InitDouble("Level Mix", 100., 0., 100., 1., "%");

	GetParam(kCompPeakThresh)->InitDouble("Peak Thresh", 0., -40., 0., 0.1, "dB");
	GetParam(kCompPeakRatio)->InitDouble("Peak Ratio", 5., 2., 20., 2., ":1", IParam::EFlags::kFlagStepped, "", IParam::ShapePowCurve(SR::Utils::SetShapeCentered(2., 20., 5., .5)));
	GetParam(kCompPeakAttack)->InitDouble("Peak Attack", 5., 0.02, 20., 0.01, "ms");
	GetParam(kCompPeakRelease)->InitDouble("Peak Release", 200., 20., 500., 0.01, "ms");
	GetParam(kCompPeakMakeup)->InitDouble("Peak Makeup", 0., -12., 12., 0.01, "dB");
	GetParam(kCompPeakMix)->InitDouble("Peak Mix", 100., 0., 100., 1., "%");

	OnReset();

#if IPLUG_EDITOR // http://bit.ly/2S64BDd
	mMakeGraphicsFunc = [&]() {
		return MakeGraphics(*this, PLUG_WIDTH, PLUG_HEIGHT, PLUG_FPS, GetScaleForScreen(PLUG_WIDTH, PLUG_HEIGHT));
	};

	mLayoutFunc = [&](IGraphics* pGraphics) {
		pGraphics->AttachCornerResizer(EUIResizerMode::Scale, false);
		pGraphics->AttachPanelBackground(SR::Graphics::Layout::SR_DEFAULT_COLOR_CUSTOM_PANEL_BG);
		pGraphics->LoadFont("Roboto-Regular", ROBOTO_FN);
		const IRECT b = pGraphics->GetBounds();
		const IRECT rectControls = b.GetPadded(-50.f, -20.f, -50.f, 0.f);
		const IRECT rectTitle = b.GetPadded(50.f, 0.f, -50.f, -700.f);
		const IRECT rectMeterGr = b.GetFromLeft(50.f);
		const IRECT rectMeterVu = b.GetFromRight(50.f);
		// Attach Controls
		// -- Title		
		pGraphics->AttachControl(new ITextControl(rectTitle, PLUG_MFR " " PLUG_NAME " " PLUG_VERSION_STR "-alpha", SR::Graphics::Layout::SR_DEFAULT_TEXT));
		// -- Gains		
		pGraphics->AttachControl(new SR::Graphics::Controls::Knob(rectControls.GetGridCell(4, 11, 6, 12).GetCentredInside(100.f), kGainIn, "Input", SR::Graphics::Layout::SR_DEFAULT_STYLE, true, false, -150.f, 150.f, -150.f, EDirection::Vertical, 4., 1.f), cGainIn);
		pGraphics->AttachControl(new SR::Graphics::Controls::Knob(rectControls.GetGridCell(5, 11, 6, 12).GetCentredInside(100.f), kGainOut, "Output", SR::Graphics::Layout::SR_DEFAULT_STYLE, true, false, -150.f, 150.f, -150.f, EDirection::Vertical, 4., 1.f), cGainOut);
		// -- Saturation
		pGraphics->AttachControl(new SR::Graphics::Controls::Knob(rectControls.GetGridCell(0, 0, 6, 12).GetCentredInside(100.f), kSaturationDrive, "Drive", SR::Graphics::Layout::SR_DEFAULT_STYLE, true, false, -150.f, 150.f, -150.f, EDirection::Vertical, 4., 1.f), cSaturationDrive);
		pGraphics->AttachControl(new SR::Graphics::Controls::Knob(rectControls.GetGridCell(1, 0, 6, 12).GetCentredInside(100.f), kSaturationAmount, "Amount", SR::Graphics::Layout::SR_DEFAULT_STYLE, true, false, -150.f, 150.f, -150.f, EDirection::Vertical, 4., 1.f), cSaturationAmount);
		// -- Filters
		pGraphics->AttachControl(new SR::Graphics::Controls::Knob(rectControls.GetGridCell(4, 0, 6, 12).GetCentredInside(100.f), kEqLpFreq, "LP", SR::Graphics::Layout::SR_DEFAULT_STYLE, true, false, -150.f, 150.f, -150.f, EDirection::Vertical, 4., 1.f), cEqLpFreq);
		pGraphics->AttachControl(new SR::Graphics::Controls::Knob(rectControls.GetGridCell(5, 0, 6, 12).GetCentredInside(100.f), kEqHpFreq, "HP", SR::Graphics::Layout::SR_DEFAULT_STYLE, true, false, -150.f, 150.f, -150.f, EDirection::Vertical, 4., 1.f), cEqHpFreq);
		// -- Freqency Response Meter
		//pGraphics->AttachControl(new PlaceHolder(rectControls.GetGridCell(0, 2, 6, 12).FracRectHorizontal(4.f).FracRectVertical(2.f, true), "Frequency Response"));
		pGraphics->AttachControl(new SR::Graphics::Controls::SRGraphBase(rectControls.GetGridCell(0, 2, 6, 12).FracRectHorizontal(4.f).FracRectVertical(2.f, true), FREQUENCYRESPONSE, mFreqMeterValues, SR::Graphics::Layout::SR_DEFAULT_STYLE), cMeterFreqResponse);
		// -- EQ
		pGraphics->AttachControl(new SR::Graphics::Controls::Knob(rectControls.GetGridCell(2, 2, 6, 12).GetCentredInside(100.f), kEqLfBoost, "Boost", SR::Graphics::Layout::SR_DEFAULT_STYLE, true, false, -150.f, 150.f, -150.f, EDirection::Vertical, 4., 1.f), cEqLfBoost);
		pGraphics->AttachControl(new SR::Graphics::Controls::Knob(rectControls.GetGridCell(2, 3, 6, 12).GetCentredInside(100.f), kEqLfCut, "Cut", SR::Graphics::Layout::SR_DEFAULT_STYLE, true, false, -150.f, 150.f, -150.f, EDirection::Vertical, 4., 1.f), cEqLfCut);
		pGraphics->AttachControl(new SR::Graphics::Controls::Knob(rectControls.GetGridCell(2, 4, 6, 12).GetCentredInside(100.f), kEqHfBoost, "Boost", SR::Graphics::Layout::SR_DEFAULT_STYLE, true, false, -150.f, 150.f, -150.f, EDirection::Vertical, 4., 1.f), cEqHfBoost);
		pGraphics->AttachControl(new SR::Graphics::Controls::Knob(rectControls.GetGridCell(2, 5, 6, 12).GetCentredInside(100.f), kEqHfCut, "Cut", SR::Graphics::Layout::SR_DEFAULT_STYLE, true, false, -150.f, 150.f, -150.f, EDirection::Vertical, 4., 1.f), cEqHfCut);
		pGraphics->AttachControl(new SR::Graphics::Controls::Knob(rectControls.GetGridCell(3, 2, 6, 12).GetCentredInside(80.f), kEqLfFreq, "Freq", SR::Graphics::Layout::SR_DEFAULT_STYLE, true, false, -150.f, 150.f, -150.f, EDirection::Vertical, 4., 1.f), cEqLfFreq);
		pGraphics->AttachControl(new SR::Graphics::Controls::Knob(rectControls.GetGridCell(3, 3, 6, 12).GetCentredInside(80.f), kEqLfDs, "DS", SR::Graphics::Layout::SR_DEFAULT_STYLE, true, false, -150.f, 150.f, -150.f, EDirection::Vertical, 4., 1.f), cEqLfDs);
		pGraphics->AttachControl(new SR::Graphics::Controls::Knob(rectControls.GetGridCell(3, 4, 6, 12).GetCentredInside(80.f), kEqHfFreq, "Freq", SR::Graphics::Layout::SR_DEFAULT_STYLE, true, false, -150.f, 150.f, -150.f, EDirection::Vertical, 4., 1.f), cEqHfFreq);
		pGraphics->AttachControl(new SR::Graphics::Controls::Knob(rectControls.GetGridCell(3, 5, 6, 12).GetCentredInside(80.f), kEqHfDs, "DS", SR::Graphics::Layout::SR_DEFAULT_STYLE, true, false, -150.f, 150.f, -150.f, EDirection::Vertical, 4., 1.f), cEqHfDs);
		pGraphics->AttachControl(new SR::Graphics::Controls::Knob(rectControls.GetGridCell(4, 2, 6, 12).GetCentredInside(100.f), kEqLmfGain, "Gain", SR::Graphics::Layout::SR_DEFAULT_STYLE, true, false, -150.f, 150.f, -150.f, EDirection::Vertical, 4., 1.f), cEqLmfGain);
		pGraphics->AttachControl(new SR::Graphics::Controls::Knob(rectControls.GetGridCell(4, 3, 6, 12).GetCentredInside(100.f), kEqLmfQ, "Q", SR::Graphics::Layout::SR_DEFAULT_STYLE, true, false, -150.f, 150.f, -150.f, EDirection::Vertical, 4., 1.f), cEqLmfQ);
		pGraphics->AttachControl(new SR::Graphics::Controls::Knob(rectControls.GetGridCell(4, 4, 6, 12).GetCentredInside(100.f), kEqHmfGain, "Gain", SR::Graphics::Layout::SR_DEFAULT_STYLE, true, false, -150.f, 150.f, -150.f, EDirection::Vertical, 4., 1.f), cEqHmfGain);
		pGraphics->AttachControl(new SR::Graphics::Controls::Knob(rectControls.GetGridCell(4, 5, 6, 12).GetCentredInside(100.f), kEqHmfQ, "Q", SR::Graphics::Layout::SR_DEFAULT_STYLE, true, false, -150.f, 150.f, -150.f, EDirection::Vertical, 4., 1.f), cEqHmfQ);
		pGraphics->AttachControl(new SR::Graphics::Controls::Knob(rectControls.GetGridCell(5, 2, 6, 12).GetCentredInside(80.f), kEqLmfFreq, "Freq", SR::Graphics::Layout::SR_DEFAULT_STYLE, true, false, -150.f, 150.f, -150.f, EDirection::Vertical, 4., 1.f), cEqLmfFreq);
		pGraphics->AttachControl(new SR::Graphics::Controls::Knob(rectControls.GetGridCell(5, 3, 6, 12).GetCentredInside(80.f), kEqLmfDs, "DS", SR::Graphics::Layout::SR_DEFAULT_STYLE, true, false, -150.f, 150.f, -150.f, EDirection::Vertical, 4., 1.f), cEqLmfDs);
		pGraphics->AttachControl(new SR::Graphics::Controls::Knob(rectControls.GetGridCell(5, 4, 6, 12).GetCentredInside(80.f), kEqHmfFreq, "Freq", SR::Graphics::Layout::SR_DEFAULT_STYLE, true, false, -150.f, 150.f, -150.f, EDirection::Vertical, 4., 1.f), cEqHmfFreq);
		pGraphics->AttachControl(new SR::Graphics::Controls::Knob(rectControls.GetGridCell(5, 5, 6, 12).GetCentredInside(80.f), kEqHmfDs, "DS", SR::Graphics::Layout::SR_DEFAULT_STYLE, true, false, -150.f, 150.f, -150.f, EDirection::Vertical, 4., 1.f), cEqHmfDs);
		// -- Compressors
		pGraphics->AttachControl(new SR::Graphics::Controls::Knob(rectControls.GetGridCell(0, 7, 6, 12).GetCentredInside(100.f), kCompRmsThresh, "Thresh", SR::Graphics::Layout::SR_DEFAULT_STYLE, true, false, -150.f, 150.f, -150.f, EDirection::Vertical, 4., 1.f), cCompRmsThresh);
		pGraphics->AttachControl(new SR::Graphics::Controls::Knob(rectControls.GetGridCell(0, 8, 6, 12).GetCentredInside(100.f), kCompRmsRatio, "Ratio", SR::Graphics::Layout::SR_DEFAULT_STYLE, true, false, -150.f, 150.f, -150.f, EDirection::Vertical, 4., 1.f), cCompRmsRatio);
		pGraphics->AttachControl(new SR::Graphics::Controls::Knob(rectControls.GetGridCell(1, 7, 6, 12).GetCentredInside(100.f), kCompRmsAttack, "Attack", SR::Graphics::Layout::SR_DEFAULT_STYLE, true, false, -150.f, 150.f, -150.f, EDirection::Vertical, 4., 1.f), cCompRmsAttack);
		pGraphics->AttachControl(new SR::Graphics::Controls::Knob(rectControls.GetGridCell(1, 8, 6, 12).GetCentredInside(100.f), kCompRmsRelease, "Release", SR::Graphics::Layout::SR_DEFAULT_STYLE, true, false, -150.f, 150.f, -150.f, EDirection::Vertical, 4., 1.f), cCompRmsRelease);
		pGraphics->AttachControl(new SR::Graphics::Controls::Knob(rectControls.GetGridCell(2, 7, 6, 12).GetCentredInside(100.f), kCompRmsMakeup, "Makeup", SR::Graphics::Layout::SR_DEFAULT_STYLE, true, false, -150.f, 150.f, -150.f, EDirection::Vertical, 4., 1.f), cCompRmsMakeup);
		pGraphics->AttachControl(new SR::Graphics::Controls::Knob(rectControls.GetGridCell(2, 8, 6, 12).GetCentredInside(100.f), kCompRmsMix, "Mix", SR::Graphics::Layout::SR_DEFAULT_STYLE, true, false, -150.f, 150.f, -150.f, EDirection::Vertical, 4., 1.f), cCompRmsMix);

		pGraphics->AttachControl(new SR::Graphics::Controls::Knob(rectControls.GetGridCell(3, 7, 6, 12).GetCentredInside(100.f), kCompPeakThresh, "Thresh", SR::Graphics::Layout::SR_DEFAULT_STYLE, true, false, -150.f, 150.f, -150.f, EDirection::Vertical, 4., 1.f), cCompPeakThresh);
		pGraphics->AttachControl(new SR::Graphics::Controls::Knob(rectControls.GetGridCell(3, 8, 6, 12).GetCentredInside(100.f), kCompPeakRatio, "Ratio", SR::Graphics::Layout::SR_DEFAULT_STYLE, true, false, -150.f, 150.f, -150.f, EDirection::Vertical, 4., 1.f), cCompPeakRatio);
		pGraphics->AttachControl(new SR::Graphics::Controls::Knob(rectControls.GetGridCell(4, 7, 6, 12).GetCentredInside(100.f), kCompPeakAttack, "Attack", SR::Graphics::Layout::SR_DEFAULT_STYLE, true, false, -150.f, 150.f, -150.f, EDirection::Vertical, 4., 1.f), cCompPeakAttack);
		pGraphics->AttachControl(new SR::Graphics::Controls::Knob(rectControls.GetGridCell(4, 8, 6, 12).GetCentredInside(100.f), kCompPeakRelease, "Release", SR::Graphics::Layout::SR_DEFAULT_STYLE, true, false, -150.f, 150.f, -150.f, EDirection::Vertical, 4., 1.f), cCompPeakRelease);
		pGraphics->AttachControl(new SR::Graphics::Controls::Knob(rectControls.GetGridCell(5, 7, 6, 12).GetCentredInside(100.f), kCompPeakMakeup, "Makeup", SR::Graphics::Layout::SR_DEFAULT_STYLE, true, false, -150.f, 150.f, -150.f, EDirection::Vertical, 4., 1.f), cCompPeakMakeup);
		pGraphics->AttachControl(new SR::Graphics::Controls::Knob(rectControls.GetGridCell(5, 8, 6, 12).GetCentredInside(100.f), kCompPeakMix, "Mix", SR::Graphics::Layout::SR_DEFAULT_STYLE, true, false, -150.f, 150.f, -150.f, EDirection::Vertical, 4., 1.f), cCompPeakMix);
		// -- Stereo Controls
		pGraphics->AttachControl(new SR::Graphics::Controls::Knob(rectControls.GetGridCell(0, 10, 6, 12).GetCentredInside(100.f), kStereoPan, "Pan", SR::Graphics::Layout::SR_DEFAULT_STYLE, true, false, -150.f, 150.f, -150.f, EDirection::Vertical, 4., 1.f), cStereoPan);
		pGraphics->AttachControl(new SR::Graphics::Controls::Knob(rectControls.GetGridCell(1, 10, 6, 12).GetCentredInside(100.f), kStereoWidth, "Width", SR::Graphics::Layout::SR_DEFAULT_STYLE, true, false, -150.f, 150.f, -150.f, EDirection::Vertical, 4., 1.f), cStereoWidth);
		pGraphics->AttachControl(new SR::Graphics::Controls::Knob(rectControls.GetGridCell(2, 10, 6, 12).GetCentredInside(100.f), kStereoWidthLow, "Bass Width", SR::Graphics::Layout::SR_DEFAULT_STYLE, true, false, -150.f, 150.f, -150.f, EDirection::Vertical, 4., 1.f), cStereoWidthLow);
		pGraphics->AttachControl(new SR::Graphics::Controls::Knob(rectControls.GetGridCell(3, 10, 6, 12).GetCentredInside(100.f), kStereoMonoFreq, "Split FQ", SR::Graphics::Layout::SR_DEFAULT_STYLE, true, false, -150.f, 150.f, -150.f, EDirection::Vertical, 4., 1.f), cStereoMonoFreq);
		// -- Meters
		// We might test this AVG Meter later
		//pGraphics->AttachControl(new IVPeakAvgMeterControl<2>(rectMeterVu.GetGridCell(0, 0, 1, 2), "In", SR::Graphics::Layout::SR_DEFAULT_STYLE, EDirection::Vertical, { "L","R" }, 0, -60.f, 12.f, { 0,-6,-12,-24,-48 }), cMeterIn);
		//pGraphics->AttachControl(new IVPeakAvgMeterControl<2>(rectMeterVu.GetGridCell(0, 1, 1, 2), "Out", SR::Graphics::Layout::SR_DEFAULT_STYLE, EDirection::Vertical, { "L","R" }, 0, -60.f, 12.f, { 0,-6,-12,-24,-48 }), cMeterOut);
		pGraphics->AttachControl(new IVMeterControl<2>(rectMeterVu.GetGridCell(0, 0, 1, 2), "In", SR::Graphics::Layout::SR_DEFAULT_STYLE, EDirection::Vertical, { "L", "R" }, 0, iplug::igraphics::IVMeterControl<2>::EResponse::Log, -60.f, 12.f, { 0, -6, -12, -24, -48 }), cMeterIn);
		pGraphics->AttachControl(new IVMeterControl<2>(rectMeterVu.GetGridCell(0, 1, 1, 2), "Out", SR::Graphics::Layout::SR_DEFAULT_STYLE, EDirection::Vertical, { "L", "R" }, 0, iplug::igraphics::IVMeterControl<2>::EResponse::Log, -60.f, 12.f, { 0, -6, -12, -24, -48 }), cMeterOut);
		pGraphics->AttachControl(new IVMeterControl<1>(rectMeterGr.GetGridCell(0, 0, 1, 2), "L", SR::Graphics::Layout::SR_DEFAULT_STYLE, EDirection::Vertical, { }, 0, iplug::igraphics::IVMeterControl<1>::EResponse::Log, -12.f, 0.f, { 0,-1,-2,-3,-4,-6,-9 }), cMeterGrRms);
		pGraphics->AttachControl(new IVMeterControl<1>(rectMeterGr.GetGridCell(0, 1, 1, 2), "P", SR::Graphics::Layout::SR_DEFAULT_STYLE, EDirection::Vertical, { }, 0, iplug::igraphics::IVMeterControl<1>::EResponse::Log, -12.f, 0.f, { 0,-1,-2,-3,-4,-6,-9 }), cMeterGrPeak);
		// -- Set GR meters displaying the other way round
		dynamic_cast<IVMeterControl<1>*>(pGraphics->GetControlWithTag(cMeterGrRms))->SetBaseValue(1.);
		dynamic_cast<IVMeterControl<1>*>(pGraphics->GetControlWithTag(cMeterGrPeak))->SetBaseValue(1.);

		// Disable Parameters with no function
		for (int ctrlTag = 0; ctrlTag < kNumCtrlTags; ctrlTag++) {
			switch (ctrlTag)
			{
			case cEqHfDs:
			case cEqHmfDs:
			case cEqLmfDs:
			case cEqLfDs:
				pGraphics->GetControlWithTag(ctrlTag)->SetDisabled(true);
				break;
			default:
				break;
			}
		}
	};
#endif
}

#if IPLUG_DSP
void SRChannel::ProcessBlock(sample** inputs, sample** outputs, int nFrames)
{
	const int nChans = NOutChansConnected();
	for (int s = 0; s < nFrames; s++) {

		// Store input in output data, perform processing here later
		for (int c = 0; c < nChans; c++) {
			outputs[c][s] = inputs[c][s];
		}

		// Process input gain
		fGainIn.Process(outputs[0][s], outputs[1][s]);

		// Run input data through envelope filter to match VU like metering, then send to respective buffer. (After input gain) 
		fMeterEnvelope[0].process(abs(outputs[0][s]), meterIn1);
		fMeterEnvelope[1].process(abs(outputs[1][s]), meterIn2);
		mBufferInput.ProcessBuffer(meterIn1, 0, s);
		mBufferInput.ProcessBuffer(meterIn2, 1, s);

		// Process filters
		if (GetParam(kEqHpFreq)->Value() > 20.) {
			outputs[0][s] = fEqHp.Process(outputs[0][s], 0);
			outputs[1][s] = fEqHp.Process(outputs[1][s], 1);
		}
		if (GetParam(kEqLpFreq)->Value() < 22000.) {
			outputs[0][s] = fEqLp.Process(outputs[0][s], 0);
			outputs[1][s] = fEqLp.Process(outputs[1][s], 1);
		}

		// Process saturation
		outputs[0][s] = fSatInput[0].Process(outputs[0][s]);
		outputs[1][s] = fSatInput[0].Process(outputs[1][s]);

		// Process EQ
		outputs[0][s] = fEqHfBoost.Process(outputs[0][s], 0);
		outputs[1][s] = fEqHfBoost.Process(outputs[1][s], 1);

		outputs[0][s] = fEqHfCut.Process(outputs[0][s], 0);
		outputs[1][s] = fEqHfCut.Process(outputs[1][s], 1);

		outputs[0][s] = fEqHmf.Process(outputs[0][s], 0);
		outputs[1][s] = fEqHmf.Process(outputs[1][s], 1);

		outputs[0][s] = fEqLmf.Process(outputs[0][s], 0);
		outputs[1][s] = fEqLmf.Process(outputs[1][s], 1);

		outputs[0][s] = fEqLfBoost.Process(outputs[0][s], 0);
		outputs[1][s] = fEqLfBoost.Process(outputs[1][s], 1);

		outputs[0][s] = fEqLfCut.Process(outputs[0][s], 0);
		outputs[1][s] = fEqLfCut.Process(outputs[1][s], 1);

		// Process compressors
		fCompRms.Process(outputs[0][s], outputs[1][s]);
		fCompPeak.Process(outputs[0][s], outputs[1][s]);

		// Process output gain
		if (GetParam(kStereoMonoFreq)->Value() <= 20.) {
			fGainOut.Process(outputs[0][s], outputs[1][s]);
		}
		else {
			// Funny nested function which stores the output signal in buffer after processing of the Linkwitz-Riley-LP
			mBufferLowSignal.ProcessBuffer(fSplitLp.Process(outputs[0][s], 0), 0, s);
			mBufferLowSignal.ProcessBuffer(fSplitLp.Process(outputs[1][s], 1), 1, s);
			// Now apply the complementary LR-HP to the outputs itself
			outputs[0][s] = fSplitHp.Process(outputs[0][s], 0);
			outputs[1][s] = fSplitHp.Process(outputs[1][s], 1);
			// Process output gain (with width) of low signal
			fGainOutLow.Process(mBufferLowSignal.GetBuffer()[0][s], mBufferLowSignal.GetBuffer()[1][s]);
			// Process output gain (with width and pan) of high signal
			fGainOut.Process(outputs[0][s], outputs[1][s]);
			// Mix both signals, flip phase of the latter (allpass solution would be better)
			outputs[0][s] -= mBufferLowSignal.GetBuffer(0, s);
			outputs[1][s] -= mBufferLowSignal.GetBuffer(1, s);
		}

		// Store current gain reduction in respective buffer
		mBufferMeterGrRms.ProcessBuffer(fCompRms.GetGrLin(), 0, s);
		mBufferMeterGrPeak.ProcessBuffer(fCompPeak.GetGrLin(), 0, s);


		// Run input data through envelope filter to match VU like metering, then send to respective buffer.
		fMeterEnvelope[2].process(abs(outputs[0][s]), meterOut1);
		fMeterEnvelope[3].process(abs(outputs[1][s]), meterOut2);
		mBufferOutput.ProcessBuffer(meterOut1, 0, s);
		mBufferOutput.ProcessBuffer(meterOut2, 1, s);
	}
	//fEqHp.ProcessBlock(outputs, outputs, 2, nFrames);
	//fEqLp.ProcessBlock(outputs, outputs, 2, nFrames);
	mMeterSenderIn.ProcessBlock(mBufferInput.GetBuffer(), nFrames, cMeterIn, 2);
	mMeterSenderOut.ProcessBlock(mBufferOutput.GetBuffer(), nFrames, cMeterOut, 2);
	mMeterSenderGrRms.ProcessBlock(mBufferMeterGrRms.GetBuffer(), nFrames, cMeterGrRms);
	mMeterSenderGrPeak.ProcessBlock(mBufferMeterGrPeak.GetBuffer(), nFrames, cMeterGrPeak);


}

void SRChannel::OnReset()
{
	// Here we set our complete range of filters and other effects
	// ToDo: This 1.5 Ratio in The Cut Freq is still arbitrary
	// ToDo: Match boost and cut behaviour to normal passive equalizing
	// ToDo: Match all above ToDos in OnParamChange and think of procedure to prevent double code
	const double samplerate = GetSampleRate();

	fGainIn.InitGain(100, SR::DSP::SRGain::kSinusodial);
	fGainOut.InitGain(100, SR::DSP::SRGain::kSinusodial);

	fSatInput[0].SetSaturation(SR::DSP::SRSaturation::kSoftSat, GetParam(kSaturationDrive)->Value(), GetParam(kSaturationAmount)->Value(), 1., true, 0., 1., samplerate);
	fSatInput[1].SetSaturation(SR::DSP::SRSaturation::kSoftSat, GetParam(kSaturationDrive)->Value(), GetParam(kSaturationAmount)->Value(), 1., true, 0., 1., samplerate);

	fEqHp.SetFilter(SR::DSP::SRFilterIIR<sample, 2>::BiquadHighpass, GetParam(kEqHpFreq)->Value() / samplerate, 0.707, 0., samplerate);
	fEqLp.SetFilter(SR::DSP::SRFilterIIR<sample, 2>::BiquadLowpass, GetParam(kEqLpFreq)->Value() / samplerate, 0.707, 0., samplerate);
	fEqHfBoost.SetFilter(SR::DSP::SRFilterIIR<sample, 2>::BiquadPeak, GetParam(kEqHfFreq)->Value() / samplerate, 0.707, GetParam(kEqHfBoost)->Value(), samplerate);
	fEqHfCut.SetFilter(SR::DSP::SRFilterIIR<sample, 2>::BiquadHighshelf, fmin(1.5 * GetParam(kEqHfFreq)->Value() / samplerate, .499), 0.707, -GetParam(kEqHfCut)->Value(), samplerate);
	fEqHmf.SetFilter(SR::DSP::SRFilterIIR<sample, 2>::BiquadPeak, GetParam(kEqHmfFreq)->Value() / samplerate, GetParam(kEqHmfQ)->Value(), GetParam(kEqHmfGain)->Value(), samplerate);
	fEqLmf.SetFilter(SR::DSP::SRFilterIIR<sample, 2>::BiquadPeak, GetParam(kEqLmfFreq)->Value() / samplerate, GetParam(kEqLmfQ)->Value(), GetParam(kEqLmfGain)->Value(), samplerate);
	fEqLfBoost.SetFilter(SR::DSP::SRFilterIIR<sample, 2>::BiquadLowshelf, GetParam(kEqLfFreq)->Value() / samplerate, 0.707, GetParam(kEqLfBoost)->Value(), samplerate);
	fEqLfCut.SetFilter(SR::DSP::SRFilterIIR<sample, 2>::BiquadLowshelf, 1.5 * GetParam(kEqLfFreq)->Value() / samplerate, 0.707, -GetParam(kEqLfCut)->Value(), samplerate);
	fSplitHp.SetFilter(SR::DSP::SRFilterIIR < sample, 2>::BiquadLinkwitzHighpass, GetParam(kStereoMonoFreq)->Value() / samplerate, 0., 0., samplerate);
	fSplitLp.SetFilter(SR::DSP::SRFilterIIR < sample, 2>::BiquadLinkwitzLowpass, GetParam(kStereoMonoFreq)->Value() / samplerate, 0., 0., samplerate);

	fCompRms.Reset();
	fCompRms.ResetCompressor(
		GetParam(kCompRmsThresh)->Value(),
		GetParam(1. / kCompRmsRatio)->Value(),
		GetParam(kCompRmsAttack)->Value(),
		GetParam(kCompRmsRelease)->Value(),
		16. / samplerate, // sidechain HP
		5., // knee dB
		true, // feedback
		true, // automake
		-18., // reference
		samplerate);

	fCompPeak.Reset();
	fCompPeak.ResetCompressor(
		GetParam(kCompPeakThresh)->Value(),
		GetParam(1. / kCompPeakRatio)->Value(),
		GetParam(kCompPeakAttack)->Value(),
		GetParam(kCompPeakRelease)->Value(),
		16. / samplerate, // sidechain HP
		2., // knee dB
		true, // feedback
		true, // automake
		-18., // reference
		samplerate);

	fMeterEnvelope[0].SetAttack(4.);
	fMeterEnvelope[1].SetAttack(4.);
	fMeterEnvelope[2].SetAttack(4.);
	fMeterEnvelope[3].SetAttack(4.);
	fMeterEnvelope[0].SetRelease(750.);
	fMeterEnvelope[1].SetRelease(750.);
	fMeterEnvelope[2].SetRelease(750.);
	fMeterEnvelope[3].SetRelease(750.);
	fMeterEnvelope[0].SetSampleRate(samplerate);
	fMeterEnvelope[1].SetSampleRate(samplerate);
	fMeterEnvelope[2].SetSampleRate(samplerate);
	fMeterEnvelope[3].SetSampleRate(samplerate);

	// Only needed fpr PeakAVGMeterSender
	//mMeterSenderIn.Reset(samplerate);
	//mMeterSenderOut.Reset(samplerate);
}
void SRChannel::OnIdle()
{
	mMeterSenderIn.TransmitData(*this);
	mMeterSenderOut.TransmitData(*this);
	mMeterSenderGrRms.TransmitData(*this);
	mMeterSenderGrPeak.TransmitData(*this);
	SetFreqMeterValues();
}
void SRChannel::OnParamChange(int paramIdx)
{
	const double samplerate = GetSampleRate();
	switch (paramIdx)
	{
	case kGainIn:
		fGainIn.SetGain(DBToAmp(GetParam(paramIdx)->Value()));
		break;
	case kGainOut:
		fGainOut.SetGain(DBToAmp(GetParam(kGainOut)->Value()));
		fGainOutLow.SetGain(DBToAmp(GetParam(kGainOut)->Value()));
		break;
	case kSaturationDrive:
		fSatInput[0].SetDrive(GetParam(kSaturationDrive)->Value());
		fSatInput[1].SetDrive(GetParam(kSaturationDrive)->Value());
		break;
	case kSaturationAmount:
		fSatInput[0].SetAmount(GetParam(kSaturationAmount)->Value() * .01);
		fSatInput[1].SetAmount(GetParam(kSaturationAmount)->Value() * .01);
		break;
	case kStereoPan:
		fGainOut.SetPanPosition((GetParam(kStereoPan)->Value() + 100.) / 200.);
		break;
	case kStereoWidth:
		fGainOut.SetWidth(GetParam(kStereoWidth)->Value() * 0.01);
		break;
	case kStereoWidthLow:
		fGainOutLow.SetWidth(GetParam(kStereoWidthLow)->Value() * 0.01);
		break;
	case kStereoMonoFreq:
		fSplitHp.SetFreq(GetParam(kStereoMonoFreq)->Value() / samplerate);
		fSplitLp.SetFreq(GetParam(kStereoMonoFreq)->Value() / samplerate);
		break;
	case kEqHpFreq:
		fEqHp.SetFreq(GetParam(kEqHpFreq)->Value() / samplerate);
		break;
	case kEqLpFreq:
		fEqLp.SetFreq(GetParam(kEqLpFreq)->Value() / samplerate);
		break;
	case kEqHfBoost:
	case kEqHfCut:
	case kEqHfFreq:
		fEqHfBoost.SetFilter(SR::DSP::SRFilterIIR<sample, 2>::BiquadPeak, GetParam(kEqHfFreq)->Value() / samplerate, 0.707, GetParam(kEqHfBoost)->Value(), samplerate);
		fEqHfCut.SetFilter(SR::DSP::SRFilterIIR<sample, 2>::BiquadHighshelf, fmin(1.5 * GetParam(kEqHfFreq)->Value() / samplerate, .4999), 0.707, -GetParam(kEqHfCut)->Value(), samplerate);
		break;
	case kEqHmfFreq:
	case kEqHmfGain:
	case kEqHmfQ:
		fEqHmf.SetFilter(SR::DSP::SRFilterIIR<sample, 2>::BiquadPeak, GetParam(kEqHmfFreq)->Value() / samplerate, GetParam(kEqHmfQ)->Value(), GetParam(kEqHmfGain)->Value(), samplerate);
		break;
	case kEqLmfFreq:
	case kEqLmfGain:
	case kEqLmfQ:
		fEqLmf.SetFilter(SR::DSP::SRFilterIIR<sample, 2>::BiquadPeak, GetParam(kEqLmfFreq)->Value() / samplerate, GetParam(kEqLmfQ)->Value(), GetParam(kEqLmfGain)->Value(), samplerate);
		break;
	case kEqLfBoost:
	case kEqLfCut:
	case kEqLfFreq:
		fEqLfBoost.SetFilter(SR::DSP::SRFilterIIR<sample, 2>::BiquadLowshelf, GetParam(kEqLfFreq)->Value() / samplerate, 0.707, GetParam(kEqLfBoost)->Value(), samplerate);
		fEqLfCut.SetFilter(SR::DSP::SRFilterIIR<sample, 2>::BiquadLowshelf, 1.5 * GetParam(kEqLfFreq)->Value() / samplerate, 0.707, -GetParam(kEqLfCut)->Value(), samplerate);
		break;

	case kCompRmsThresh:
		fCompRms.SetThresh(GetParam(kCompRmsThresh)->Value());
		break;
	case kCompRmsRatio:
		fCompRms.SetRatio(1. / GetParam(kCompRmsRatio)->Value());
		break;
	case kCompRmsAttack:
		fCompRms.SetAttack(GetParam(kCompRmsAttack)->Value());
		break;
	case kCompRmsRelease:
		fCompRms.SetRelease(GetParam(kCompRmsRelease)->Value());
		break;
	case kCompRmsMakeup:
		break;
	case kCompRmsMix:
		break;


	case kCompPeakThresh:
		fCompPeak.SetThresh(GetParam(kCompPeakThresh)->Value());
		break;
	case kCompPeakRatio:
		fCompPeak.SetRatio(1. / GetParam(kCompPeakRatio)->Value());
		break;
	case kCompPeakAttack:
		fCompPeak.SetAttack(GetParam(kCompPeakAttack)->Value());
		break;
	case kCompPeakRelease:
		fCompPeak.SetRelease(GetParam(kCompPeakRelease)->Value());
		break;
	case kCompPeakMakeup:
		break;
	case kCompPeakMix:
		break;

	}
}
void SRChannel::SetFreqMeterValues()
{
	const double samplerate = GetSampleRate();
	const double shape = log10(samplerate);
	for (int i = 0; i < FREQUENCYRESPONSE; i++) {
		// If linear shape
		//double freq = 0.5 * samplerate * double(i) / double(FREQUENCYRESPONSE);
		// If pow shape
		double freq = 0.5 * samplerate * std::pow((double(i) / double(FREQUENCYRESPONSE)), shape);
		mFreqMeterValues[i] = 0.;
		if (GetParam(kEqHpFreq)->Value() > 20.) mFreqMeterValues[i] += fEqHp.GetFrequencyResponse(freq / samplerate, 12., false);
		if (GetParam(kEqLpFreq)->Value() < 22000.) mFreqMeterValues[i] += fEqLp.GetFrequencyResponse(freq / samplerate, 12., false);
		if (GetParam(kEqHfBoost)->Value() != 0.0) mFreqMeterValues[i] += fEqHfBoost.GetFrequencyResponse(freq / samplerate, 12., false);
		if (GetParam(kEqHfCut)->Value() != 0.0) mFreqMeterValues[i] += fEqHfCut.GetFrequencyResponse(freq / samplerate, 12., false);
		if (GetParam(kEqHmfGain)->Value() != 0.0) mFreqMeterValues[i] += fEqHmf.GetFrequencyResponse(freq / samplerate, 12., false);
		if (GetParam(kEqLmfGain)->Value() != 0.0) mFreqMeterValues[i] += fEqLmf.GetFrequencyResponse(freq / samplerate, 12., false);
		if (GetParam(kEqLfBoost)->Value() != 0.0) mFreqMeterValues[i] += fEqLfBoost.GetFrequencyResponse(freq / samplerate, 12., false);
		if (GetParam(kEqLfCut)->Value() != 0.0) mFreqMeterValues[i] += fEqLfCut.GetFrequencyResponse(freq / samplerate, 12., false);
	}
	if (GetUI() && mFreqMeterValues != 0) dynamic_cast<SR::Graphics::Controls::SRGraphBase*>(GetUI()->GetControlWithTag(cMeterFreqResponse))->Process(mFreqMeterValues);
}
#endif
