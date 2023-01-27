#include "SRChannel.h"
#include "IPlug_include_in_plug_src.h"
#include "IControls.h"
#include "../../Classes/Graphics/SRCustomKnob.h"
#include "../../Classes/Graphics/SRCustomSwitch.h"
#include "../../Classes/Graphics/SRCustomLayout.h"
#include "../../Classes/Graphics/SRCustomGraph.h"

#define FREQRESP_NUMVALUES 300
#define FREQRESP_RANGEDB 12.
#define VU_ATTACK 4.
#define VU_RELEASE 750.

class MainMenu : public IControl
{
public:
	MainMenu(const IRECT& bounds)
		: IControl(bounds)
	{}

	void Draw(IGraphics& g) override {
		const IRECT c = mRECT.GetCentredInside(20.f);
		g.FillRect(COLOR_WHITE, c.GetGridCell(0, 0, 5, 1));
		g.FillRect(COLOR_WHITE, c.GetGridCell(2, 0, 5, 1));
		g.FillRect(COLOR_WHITE, c.GetGridCell(4, 0, 5, 1));
		g.DrawRoundRect(COLOR_WHITE, c.GetPadded(3.f), 3.f);
	}
	void OnMouseDown(float x, float y, const IMouseMod& mod) override {	
		GetUI()->CreatePopupMenu(*this, mMenu, x, y);
	}
	void OnPopupMenuSelection(IPopupMenu* pSelectedMenu, int valIdx) override {
		auto* delegate = static_cast<iplug::IPluginBase*>(GetDelegate());
		auto chosenItemIdx = pSelectedMenu->GetChosenItemIdx();
		switch (chosenItemIdx) {
		case 0: delegate->DumpMakePresetSrc("Preset.txt"); 
			break;
		//case 1: break;
		//case 2: break;
		default:
			break;
		}
	}
private:
	IPopupMenu mMenu{ "Menu", {"Dump Preset TXT"/*, "...", "..."*/} };
};

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
	, fEqBandSolo()
	, fCompRms()
	, fCompPeak()
	, fMeterEnvelope()
	, mFreqMeterValues(new float[FREQRESP_NUMVALUES])

{

	GetParam(kGainIn)->InitDouble("Input", 0., -120., 12., 0.01, "dB", 0, "Gain", IParam::ShapePowCurve(SR::Utils::SetShapeCentered(-120., 12., 0., .5)));
	GetParam(kGainOut)->InitDouble("Output", 0., -120., 12., 0.01, "dB", 0, "Gain", IParam::ShapePowCurve(SR::Utils::SetShapeCentered(-120., 12., 0., .5)));

	GetParam(kSaturationDrive)->InitDouble("Sat Drive", 0., 0., 24., 0.01, "dB", 0, "Sat");
	GetParam(kSaturationAmount)->InitDouble("Sat Amount", 0., 0., 100., 1., "%", 0, "Sat");

	GetParam(kStereoPan)->InitDouble("Pan", 0., -100., 100., 1., "%", 0, "Stereo");
	GetParam(kStereoWidth)->InitDouble("Width", 100., 0., 1000., 1., "%", 0, "Stereo", IParam::ShapePowCurve(SR::Utils::SetShapeCentered(0., 1000., 100., .5)));
	GetParam(kStereoWidthLow)->InitDouble("Bass Width", 100., 0., 100., 1., "%", 0, "Stereo", IParam::ShapePowCurve(SR::Utils::SetShapeCentered(0., 100., 50., .5)));
	GetParam(kStereoMonoFreq)->InitDouble("Split FQ", 20., 20., 1000., .01, "Hz", 0, "Stereo", IParam::ShapePowCurve(SR::Utils::SetShapeCentered(0., 1000., 100., .5)));

	GetParam(kEqHpFreq)->InitDouble("HP", 0., 0., 400., 10., "Hz", IParam::EFlags::kFlagStepped, "Filter");
	GetParam(kEqLpFreq)->InitDouble("LP", 22000., 3000., 22000., 1000., "Hz", IParam::EFlags::kFlagStepped, "Filter");

	GetParam(kEqHfBoost)->InitDouble("HF Boost", 0., -0., 10., 1., "dB", IParam::EFlags::kFlagStepped, "EQ");
	GetParam(kEqHfCut)->InitDouble("HF Cut", 0., -0., 10., 1., "dB", IParam::EFlags::kFlagStepped, "EQ");
	GetParam(kEqHfFreq)->InitDouble("HF Freq", 8000., 3000., 16000., 1000., "Hz", IParam::EFlags::kFlagStepped, "EQ", IParam::ShapePowCurve(SR::Utils::SetShapeCentered(3000., 16000., 8000., .5)));
	GetParam(kEqHfDs)->InitDouble("HF DS", 0., -20., 0., .01, "dB", 0, "EQ");

	GetParam(kEqHmfGain)->InitDouble("HMF Gain", 0., -12., 12., 1., "dB", IParam::EFlags::kFlagStepped, "EQ");
	GetParam(kEqHmfFreq)->InitDouble("HMF Freq", 3000., 600., 15000., 1., "Hz", 0, "EQ", IParam::ShapePowCurve(SR::Utils::SetShapeCentered(600., 15000., 3000., .5)));
	GetParam(kEqHmfQ)->InitDouble("HMF Q", .707, 0.1, 10., 0.01, "", 0, "EQ", IParam::ShapePowCurve(SR::Utils::SetShapeCentered(.1, 10., .707, .5)));
	GetParam(kEqHmfDs)->InitDouble("HMF DS", 0., -50., 0., .01, "dB", 0, "EQ");

	GetParam(kEqLmfGain)->InitDouble("LMF Gain", 0., -12., 12., 1., "dB", IParam::EFlags::kFlagStepped, "EQ");
	GetParam(kEqLmfFreq)->InitDouble("LMF Freq", 1000., 20., 2500., 1., "Hz", 0, "EQ", IParam::ShapePowCurve(SR::Utils::SetShapeCentered(20., 2500., 1000., .5)));
	GetParam(kEqLmfQ)->InitDouble("LMF Q", .707, 0.1, 10., 0.01, "", 0, "EQ", IParam::ShapePowCurve(SR::Utils::SetShapeCentered(.1, 10., .707, .5)));
	GetParam(kEqLmfDs)->InitDouble("LMF DS", 0., -50., 0., .01, "dB", 0, "EQ");

	GetParam(kEqLfBoost)->InitDouble("LF Boost", 0., 0., 10., 1., "dB", IParam::EFlags::kFlagStepped, "EQ");
	GetParam(kEqLfCut)->InitDouble("LF Cut", 0., 0., 10., 1., "dB", IParam::EFlags::kFlagStepped, "EQ");
	GetParam(kEqLfFreq)->InitDouble("LF Freq", 100., 30., 300., 10., "Hz", IParam::EFlags::kFlagStepped, "EQ", IParam::ShapePowCurve(SR::Utils::SetShapeCentered(30., 300., 100., .5)));
	GetParam(kEqLfDs)->InitDouble("LF DS", 0., -20., 0., .01, "dB", 0, "EQ");

	GetParam(kCompRmsThresh)->InitDouble("Level Thresh", 0., -40., 0., 0.1, "dB", 0, "Comp");
	GetParam(kCompRmsRatio)->InitDouble("Level Ratio", 2.5, 1., 6., .5, ":1", IParam::EFlags::kFlagStepped, "Comp", IParam::ShapePowCurve(SR::Utils::SetShapeCentered(1., 6., 2.5, .5)));
	GetParam(kCompRmsAttack)->InitDouble("Level Attack", 20., 4., 50., 1., "ms", 0, "Comp", IParam::ShapePowCurve(SR::Utils::SetShapeCentered(4., 50., 20., .5)));
	GetParam(kCompRmsRelease)->InitDouble("Level Release", 300., 100., 3000., 10., "ms", IParam::EFlags::kFlagStepped, "Comp", IParam::ShapePowCurve(SR::Utils::SetShapeCentered(100., 3000., 300., .5)));
	GetParam(kCompRmsMakeup)->InitDouble("Level Makeup", 0., -12., 12., 0.01, "dB", 0, "Comp");
	GetParam(kCompRmsMix)->InitDouble("Level Mix", 100., 0., 100., 1., "%", 0, "Comp");

	GetParam(kCompPeakThresh)->InitDouble("Peak Thresh", 0., -30., 0., 0.1, "dB", 0, "Comp");
	GetParam(kCompPeakRatio)->InitDouble("Peak Ratio", 8., 2., 20., 2., ":1", IParam::EFlags::kFlagStepped, "Comp", IParam::ShapePowCurve(SR::Utils::SetShapeCentered(2., 20., 8., .5)));
	GetParam(kCompPeakAttack)->InitDouble("Peak Attack", 4., 0.02, 20., 0.01, "ms", 0, "Comp", IParam::ShapePowCurve(SR::Utils::SetShapeCentered(.02, 20., 4., .5)));
	GetParam(kCompPeakRelease)->InitDouble("Peak Release", 120., 20., 500., 10., "ms", IParam::EFlags::kFlagStepped, "Comp", IParam::ShapePowCurve(SR::Utils::SetShapeCentered(20., 500., 120., .5)));
	GetParam(kCompPeakMakeup)->InitDouble("Peak Makeup", 0., -12., 12., 0.01, "dB", 0, "Comp");
	GetParam(kCompPeakMix)->InitDouble("Peak Mix", 100., 0., 100., 1., "%", 0, "Comp");

	GetParam(kBypass)->InitBool("Bypass", false, "Bypass", 0, "Global", "OFF", "ON");
	GetParam(kEqHmfIsShelf)->InitBool("Hmf Shelf", false, "Hmf Shelf", 0, "EQ", "PEAK", "HS");
	GetParam(kEqLmfIsShelf)->InitBool("Lmf Shelf", false, "Lmf Shelf", 0, "EQ", "PEAK", "LS");

	GetParam(kEqBandSolo)->InitEnum("Band Solo", 0, { "Off", "HP", "LP", "Hmf", "Lmf", "Hf", "Lf" }, 0, "EQ");

	OnReset();

	MakeDefaultPreset("Init", 1);
	MakePreset("VC De-ess", 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 22000.000000, 0.000000, 0.000000, 8000.000000, 0.000000, 0.000000, 0.000000, 8192.059190, 4.432813, -26.370989, false, 0.000000, 1000.000000, 0.707000, 0.000000, false, 0.000000, 0.000000, 100.000000, 0.000000, 0.000000, 0, 0.000000, 0.000000, 2.500000, 20.000000, 300.000000, 0.000000, 0.000000, 100.000000, 0.000000, 0.000000, 0.000000, 8.000000, 4.000000, 120.000000, 0.000000, 0.000000, 100.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 100.000000, 100.000000, 0.000000, 20.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, false);
	MakePreset("MS Mastering", 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 30.000000, 21000.000000, 0.000000, 0.000000, 8000.000000, 0.000000, 0.000000, 2.000000, 11165.008820, 0.707000, -26.612924, true, 2.000000, 123.792685, 0.707000, -21.290339, true, 0.000000, 0.000000, 100.000000, 0.000000, 0.000000, 0, 0.000000, -20.812508, 2.000000, 10.434966, 120.000000, 0.000000, 1.237500, 100.000000, 0.000000, 0.000000, -14.203114, 8.000000, 2.281333, 80.000000, 0.000000, 0.450000, 100.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 110.000000, 0.000000, 0.000000, 100.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, false);


#if IPLUG_EDITOR // http://bit.ly/2S64BDd
	mMakeGraphicsFunc = [&]() {
		return MakeGraphics(*this, PLUG_WIDTH, PLUG_HEIGHT, PLUG_FPS, GetScaleForScreen(PLUG_WIDTH, PLUG_HEIGHT));
	};

	mLayoutFunc = [&](IGraphics* pGraphics) {
		pGraphics->AttachCornerResizer(EUIResizerMode::Scale, false);
		pGraphics->AttachPanelBackground(SR::Graphics::Layout::SR_DEFAULT_COLOR_CUSTOM_PANEL_BG);
		pGraphics->LoadFont("Roboto-Regular", ROBOTO_FN);
		const IRECT b = pGraphics->GetBounds();
		const IRECT rectTitle = b.GetPadded(-50.f, 0.f, -50.f, -700.f);
		const IRECT rectMeterGr = b.GetFromLeft(50.f);
		const IRECT rectMeterVu = b.GetFromRight(50.f);
		const IRECT rectControls = b.GetPadded(-50.f, -60.f, -50.f, 0.f);
		const IRECT rectControlsSat = rectControls.GetGridCell(0, 0, 6, 8).FracRectVertical(4.f, true).GetPadded(-5.f);
		const IRECT rectControlsFilter = rectControls.GetGridCell(4, 0, 6, 8).FracRectVertical(2.f, true).GetPadded(-5.f);
		const IRECT rectControlsFreqResponse = rectControls.GetGridCell(0, 1, 6, 8).FracRectVertical(2.f, true).FracRectHorizontal(4.f).GetPadded(-5.f);
		const IRECT rectControlsEqParametric = rectControls.GetGridCell(2, 1, 6, 8).FracRectVertical(2.f, true).FracRectHorizontal(4.f).GetPadded(-5.f);
		const IRECT rectControlsEqPassive = rectControls.GetGridCell(4, 1, 6, 8).FracRectVertical(2.f, true).FracRectHorizontal(4.f).GetPadded(-5.f);
		const IRECT rectControlsCompLevel = rectControls.GetGridCell(0, 5, 6, 8).FracRectVertical(3.f, true).FracRectHorizontal(2.f).GetPadded(-5.f);
		const IRECT rectControlsCompPeak = rectControls.GetGridCell(3, 5, 6, 8).FracRectVertical(3.f, true).FracRectHorizontal(2.f).GetPadded(-5.f);
		const IRECT rectControlsStereo = rectControls.GetGridCell(0, 7, 6, 8).FracRectVertical(4.f, true).GetPadded(-5.f);
		const IRECT rectControlsGain = rectControls.GetGridCell(4, 7, 6, 8).FracRectVertical(2.f, true).GetPadded(-5.f);

		// Attach Controls
		// -- Title		
		pGraphics->AttachControl(new ITextControl(rectTitle, PLUG_MFR " " PLUG_NAME " " PLUG_VERSION_STR "-alpha", SR::Graphics::Layout::SR_DEFAULT_TEXT));
		pGraphics->AttachControl(new SR::Graphics::Controls::Switch(rectTitle.GetGridCell(0, 0, 1, 8).GetCentredInside(20.f), kBypass, "Byp", SR::Graphics::Layout::SR_DEFAULT_STYLE_BUTTON, true), cBypass, "Global");
		pGraphics->AttachControl(new SR::Graphics::Controls::Switch(rectTitle.GetGridCell(0, 1, 1, 8).GetCentredInside(20.f), kEqBandSolo, "Solo", SR::Graphics::Layout::SR_DEFAULT_STYLE_BUTTON, true), cEqBandSolo, "Global");
		pGraphics->AttachControl(new IVBakedPresetManagerControl(rectTitle.GetGridCell(1, 2, 2, 8).FracRectHorizontal(5.f, false), SR::Graphics::Layout::SR_DEFAULT_STYLE_METER));
		//pGraphics->AttachControl(new IVDiskPresetManagerControl(rectTitle.GetGridCell(1, 2, 2, 8).FracRectHorizontal(6.f, false), ".", "vstpreset", false, DEFAULT_STYLE));
		pGraphics->AttachControl(new MainMenu(rectTitle.GetGridCell(1, 7, 2, 8)));
		// -- Gains		
		pGraphics->AttachControl(new SR::Graphics::Controls::Knob(rectControlsGain.GetGridCell(0, 0, 2, 1).GetReducedFromTop(20.f), kGainIn, "Input", SR::Graphics::Layout::SR_DEFAULT_STYLE, true, false, -150.f, 150.f, -150.f, EDirection::Vertical, 4., 1.f), cGainIn, "Gain");
		pGraphics->AttachControl(new SR::Graphics::Controls::Knob(rectControlsGain.GetGridCell(1, 0, 2, 1).GetReducedFromTop(20.f), kGainOut, "Output", SR::Graphics::Layout::SR_DEFAULT_STYLE, true, false, -150.f, 150.f, -150.f, EDirection::Vertical, 4., 1.f), cGainOut, "Gain");
		// -- Saturation
		pGraphics->AttachControl(new SR::Graphics::Controls::Knob(rectControlsSat.GetGridCell(0, 0, 4, 1).GetReducedFromTop(20.f), kSaturationAmount, "Amount", SR::Graphics::Layout::SR_DEFAULT_STYLE, true, false, -150.f, 150.f, -150.f, EDirection::Vertical, 4., 1.f), cSaturationAmount, "Sat");
		pGraphics->AttachControl(new SR::Graphics::Controls::Knob(rectControlsSat.GetGridCell(1, 0, 4, 1).GetReducedFromTop(20.f), kSaturationDrive, "Drive", SR::Graphics::Layout::SR_DEFAULT_STYLE, true, false, -150.f, 150.f, -150.f, EDirection::Vertical, 4., 1.f), cSaturationDrive, "Sat");
		// -- Filters
		pGraphics->AttachControl(new SR::Graphics::Controls::Knob(rectControlsFilter.GetGridCell(0, 0, 2, 1).GetReducedFromTop(20.f), kEqLpFreq, "LP", SR::Graphics::Layout::SR_DEFAULT_STYLE, true, false, -150.f, 150.f, -150.f, EDirection::Vertical, 4., 1.f), cEqLpFreq, "Filter");
		pGraphics->AttachControl(new SR::Graphics::Controls::Knob(rectControlsFilter.GetGridCell(1, 0, 2, 1).GetReducedFromTop(20.f), kEqHpFreq, "HP", SR::Graphics::Layout::SR_DEFAULT_STYLE, true, false, -150.f, 150.f, -150.f, EDirection::Vertical, 4., 1.f), cEqHpFreq, "Filter");
		// -- Freqency Response Meter
		pGraphics->AttachControl(new SR::Graphics::Controls::SRGraphBase(rectControlsFreqResponse.GetReducedFromTop(20.f), FREQRESP_NUMVALUES, mFreqMeterValues, .5f, SR::Graphics::Layout::SR_DEFAULT_STYLE), cMeterFreqResponse, "Response");
		// -- EQ
		pGraphics->AttachControl(new SR::Graphics::Controls::Knob(rectControlsEqPassive.GetGridCell(0, 0, 2, 4).GetReducedFromTop(20.f), kEqLfBoost, "Boost", SR::Graphics::Layout::SR_DEFAULT_STYLE, true, false, -150.f, 150.f, -150.f, EDirection::Vertical, 4., 1.f), cEqLfBoost, "Passive EQ");
		pGraphics->AttachControl(new SR::Graphics::Controls::Knob(rectControlsEqPassive.GetGridCell(0, 1, 2, 4).GetReducedFromTop(20.f), kEqLfCut, "Cut", SR::Graphics::Layout::SR_DEFAULT_STYLE, true, false, -150.f, 150.f, -150.f, EDirection::Vertical, 4., 1.f), cEqLfCut, "Passive EQ");
		pGraphics->AttachControl(new SR::Graphics::Controls::Knob(rectControlsEqPassive.GetGridCell(0, 2, 2, 4).GetReducedFromTop(20.f), kEqHfBoost, "Boost", SR::Graphics::Layout::SR_DEFAULT_STYLE, true, false, -150.f, 150.f, -150.f, EDirection::Vertical, 4., 1.f), cEqHfBoost, "Passive EQ");
		pGraphics->AttachControl(new SR::Graphics::Controls::Knob(rectControlsEqPassive.GetGridCell(0, 3, 2, 4).GetReducedFromTop(20.f), kEqHfCut, "Cut", SR::Graphics::Layout::SR_DEFAULT_STYLE, true, false, -150.f, 150.f, -150.f, EDirection::Vertical, 4., 1.f), cEqHfCut, "Passive EQ");
		pGraphics->AttachControl(new SR::Graphics::Controls::Knob(rectControlsEqPassive.GetGridCell(1, 0, 2, 4).GetReducedFromTop(20.f), kEqLfFreq, "Freq", SR::Graphics::Layout::SR_DEFAULT_STYLE, true, false, -150.f, 150.f, -150.f, EDirection::Vertical, 4., 1.f), cEqLfFreq, "Passive EQ");
		pGraphics->AttachControl(new SR::Graphics::Controls::Knob(rectControlsEqPassive.GetGridCell(1, 1, 2, 4).GetReducedFromTop(20.f), kEqLfDs, "DS", SR::Graphics::Layout::SR_DEFAULT_STYLE, true, false, -150.f, 150.f, -150.f, EDirection::Vertical, 4., 1.f), cEqLfDs, "Passive EQ");
		pGraphics->AttachControl(new SR::Graphics::Controls::Knob(rectControlsEqPassive.GetGridCell(1, 2, 2, 4).GetReducedFromTop(20.f), kEqHfFreq, "Freq", SR::Graphics::Layout::SR_DEFAULT_STYLE, true, false, -150.f, 150.f, -150.f, EDirection::Vertical, 4., 1.f), cEqHfFreq, "Passive EQ");
		pGraphics->AttachControl(new SR::Graphics::Controls::Knob(rectControlsEqPassive.GetGridCell(1, 3, 2, 4).GetReducedFromTop(20.f), kEqHfDs, "DS", SR::Graphics::Layout::SR_DEFAULT_STYLE, true, false, -150.f, 150.f, -150.f, EDirection::Vertical, 4., 1.f), cEqHfDs, "Passive EQ");
		pGraphics->AttachControl(new SR::Graphics::Controls::Knob(rectControlsEqParametric.GetGridCell(0, 0, 2, 4).GetReducedFromTop(20.f), kEqLmfGain, "Gain", SR::Graphics::Layout::SR_DEFAULT_STYLE, true, false, -150.f, 150.f, -150.f, EDirection::Vertical, 4., 1.f), cEqLmfGain, "Parametric EQ");
		pGraphics->AttachControl(new SR::Graphics::Controls::Knob(rectControlsEqParametric.GetGridCell(0, 1, 2, 4).GetReducedFromTop(20.f), kEqLmfQ, "Q", SR::Graphics::Layout::SR_DEFAULT_STYLE, true, false, -150.f, 150.f, -150.f, EDirection::Vertical, 4., 1.f), cEqLmfQ, "Parametric EQ");
		pGraphics->AttachControl(new SR::Graphics::Controls::Knob(rectControlsEqParametric.GetGridCell(0, 2, 2, 4).GetReducedFromTop(20.f), kEqHmfGain, "Gain", SR::Graphics::Layout::SR_DEFAULT_STYLE, true, false, -150.f, 150.f, -150.f, EDirection::Vertical, 4., 1.f), cEqHmfGain, "Parametric EQ");
		pGraphics->AttachControl(new SR::Graphics::Controls::Knob(rectControlsEqParametric.GetGridCell(0, 3, 2, 4).GetReducedFromTop(20.f), kEqHmfQ, "Q", SR::Graphics::Layout::SR_DEFAULT_STYLE, true, false, -150.f, 150.f, -150.f, EDirection::Vertical, 4., 1.f), cEqHmfQ, "Parametric EQ");
		pGraphics->AttachControl(new SR::Graphics::Controls::Knob(rectControlsEqParametric.GetGridCell(1, 0, 2, 4).GetReducedFromTop(20.f), kEqLmfFreq, "Freq", SR::Graphics::Layout::SR_DEFAULT_STYLE, true, false, -150.f, 150.f, -150.f, EDirection::Vertical, 4., 1.f), cEqLmfFreq, "Parametric EQ");
		pGraphics->AttachControl(new SR::Graphics::Controls::Knob(rectControlsEqParametric.GetGridCell(1, 1, 2, 4).GetReducedFromTop(20.f), kEqLmfDs, "DS", SR::Graphics::Layout::SR_DEFAULT_STYLE, true, false, -150.f, 150.f, -150.f, EDirection::Vertical, 4., 1.f), cEqLmfDs, "Parametric EQ");
		pGraphics->AttachControl(new SR::Graphics::Controls::Knob(rectControlsEqParametric.GetGridCell(1, 2, 2, 4).GetReducedFromTop(20.f), kEqHmfFreq, "Freq", SR::Graphics::Layout::SR_DEFAULT_STYLE, true, false, -150.f, 150.f, -150.f, EDirection::Vertical, 4., 1.f), cEqHmfFreq, "Parametric EQ");
		pGraphics->AttachControl(new SR::Graphics::Controls::Knob(rectControlsEqParametric.GetGridCell(1, 3, 2, 4).GetReducedFromTop(20.f), kEqHmfDs, "DS", SR::Graphics::Layout::SR_DEFAULT_STYLE, true, false, -150.f, 150.f, -150.f, EDirection::Vertical, 4., 1.f), cEqHmfDs, "Parametric EQ");
		pGraphics->AttachControl(new SR::Graphics::Controls::Switch(rectControlsEqParametric.GetGridCell(0, 0, 1, 2).GetCentredInside(20.f), kEqLmfIsShelf, "Shelf", SR::Graphics::Layout::SR_DEFAULT_STYLE_BUTTON, true), cEqLmfIsShelf, "Parametric EQ");
		pGraphics->AttachControl(new SR::Graphics::Controls::Switch(rectControlsEqParametric.GetGridCell(0, 1, 1, 2).GetCentredInside(20.f), kEqHmfIsShelf, "Shelf", SR::Graphics::Layout::SR_DEFAULT_STYLE_BUTTON, true), cEqHmfIsShelf, "Parametric EQ");
		// -- Compressors
		pGraphics->AttachControl(new SR::Graphics::Controls::Knob(rectControlsCompLevel.GetGridCell(0, 0, 3, 2).GetReducedFromTop(20.f), kCompRmsThresh, "Thresh", SR::Graphics::Layout::SR_DEFAULT_STYLE, true, false, -150.f, 150.f, -150.f, EDirection::Vertical, 4., 1.f), cCompRmsThresh, "Comp Level");
		pGraphics->AttachControl(new SR::Graphics::Controls::Knob(rectControlsCompLevel.GetGridCell(0, 1, 3, 2).GetReducedFromTop(20.f), kCompRmsRatio, "Ratio", SR::Graphics::Layout::SR_DEFAULT_STYLE, true, false, -150.f, 150.f, -150.f, EDirection::Vertical, 4., 1.f), cCompRmsRatio, "Comp Level");
		pGraphics->AttachControl(new SR::Graphics::Controls::Knob(rectControlsCompLevel.GetGridCell(1, 0, 3, 2).GetReducedFromTop(20.f), kCompRmsAttack, "Attack", SR::Graphics::Layout::SR_DEFAULT_STYLE, true, false, -150.f, 150.f, -150.f, EDirection::Vertical, 4., 1.f), cCompRmsAttack, "Comp Level");
		pGraphics->AttachControl(new SR::Graphics::Controls::Knob(rectControlsCompLevel.GetGridCell(1, 1, 3, 2).GetReducedFromTop(20.f), kCompRmsRelease, "Release", SR::Graphics::Layout::SR_DEFAULT_STYLE, true, false, -150.f, 150.f, -150.f, EDirection::Vertical, 4., 1.f), cCompRmsRelease, "Comp Level");
		pGraphics->AttachControl(new SR::Graphics::Controls::Knob(rectControlsCompLevel.GetGridCell(2, 0, 3, 2).GetReducedFromTop(20.f), kCompRmsMakeup, "Makeup", SR::Graphics::Layout::SR_DEFAULT_STYLE, true, false, -150.f, 150.f, -150.f, EDirection::Vertical, 4., 1.f), cCompRmsMakeup, "Comp Level");
		pGraphics->AttachControl(new SR::Graphics::Controls::Knob(rectControlsCompLevel.GetGridCell(2, 1, 3, 2).GetReducedFromTop(20.f), kCompRmsMix, "Mix", SR::Graphics::Layout::SR_DEFAULT_STYLE, true, false, -150.f, 150.f, -150.f, EDirection::Vertical, 4., 1.f), cCompRmsMix, "Comp Level");

		pGraphics->AttachControl(new SR::Graphics::Controls::Knob(rectControlsCompPeak.GetGridCell(0, 0, 3, 2).GetReducedFromTop(20.f), kCompPeakThresh, "Thresh", SR::Graphics::Layout::SR_DEFAULT_STYLE, true, false, -150.f, 150.f, -150.f, EDirection::Vertical, 4., 1.f), cCompPeakThresh, "Comp Peak");
		pGraphics->AttachControl(new SR::Graphics::Controls::Knob(rectControlsCompPeak.GetGridCell(0, 1, 3, 2).GetReducedFromTop(20.f), kCompPeakRatio, "Ratio", SR::Graphics::Layout::SR_DEFAULT_STYLE, true, false, -150.f, 150.f, -150.f, EDirection::Vertical, 4., 1.f), cCompPeakRatio, "Comp Peak");
		pGraphics->AttachControl(new SR::Graphics::Controls::Knob(rectControlsCompPeak.GetGridCell(1, 0, 3, 2).GetReducedFromTop(20.f), kCompPeakAttack, "Attack", SR::Graphics::Layout::SR_DEFAULT_STYLE, true, false, -150.f, 150.f, -150.f, EDirection::Vertical, 4., 1.f), cCompPeakAttack, "Comp Peak");
		pGraphics->AttachControl(new SR::Graphics::Controls::Knob(rectControlsCompPeak.GetGridCell(1, 1, 3, 2).GetReducedFromTop(20.f), kCompPeakRelease, "Release", SR::Graphics::Layout::SR_DEFAULT_STYLE, true, false, -150.f, 150.f, -150.f, EDirection::Vertical, 4., 1.f), cCompPeakRelease, "Comp Peak");
		pGraphics->AttachControl(new SR::Graphics::Controls::Knob(rectControlsCompPeak.GetGridCell(2, 0, 3, 2).GetReducedFromTop(20.f), kCompPeakMakeup, "Makeup", SR::Graphics::Layout::SR_DEFAULT_STYLE, true, false, -150.f, 150.f, -150.f, EDirection::Vertical, 4., 1.f), cCompPeakMakeup, "Comp Peak");
		pGraphics->AttachControl(new SR::Graphics::Controls::Knob(rectControlsCompPeak.GetGridCell(2, 1, 3, 2).GetReducedFromTop(20.f), kCompPeakMix, "Mix", SR::Graphics::Layout::SR_DEFAULT_STYLE, true, false, -150.f, 150.f, -150.f, EDirection::Vertical, 4., 1.f), cCompPeakMix, "Comp Peak");
		// -- Stereo Controls
		pGraphics->AttachControl(new SR::Graphics::Controls::Knob(rectControlsStereo.GetGridCell(0, 0, 4, 1).GetReducedFromTop(20.f), kStereoPan, "Pan", SR::Graphics::Layout::SR_DEFAULT_STYLE, true, false, -150.f, 150.f, -150.f, EDirection::Vertical, 4., 1.f), cStereoPan, "Stereo");
		pGraphics->AttachControl(new SR::Graphics::Controls::Knob(rectControlsStereo.GetGridCell(1, 0, 4, 1).GetReducedFromTop(20.f), kStereoWidth, "Width", SR::Graphics::Layout::SR_DEFAULT_STYLE, true, false, -150.f, 150.f, -150.f, EDirection::Vertical, 4., 1.f), cStereoWidth, "Stereo");
		pGraphics->AttachControl(new SR::Graphics::Controls::Knob(rectControlsStereo.GetGridCell(2, 0, 4, 1).GetReducedFromTop(20.f), kStereoWidthLow, "Bass Width", SR::Graphics::Layout::SR_DEFAULT_STYLE, true, false, -150.f, 150.f, -150.f, EDirection::Vertical, 4., 1.f), cStereoWidthLow, "Stereo");
		pGraphics->AttachControl(new SR::Graphics::Controls::Knob(rectControlsStereo.GetGridCell(3, 0, 4, 1).GetReducedFromTop(20.f), kStereoMonoFreq, "Split FQ", SR::Graphics::Layout::SR_DEFAULT_STYLE, true, false, -150.f, 150.f, -150.f, EDirection::Vertical, 4., 1.f), cStereoMonoFreq, "Stereo");
		// -- Meters
		pGraphics->AttachControl(new IVMeterControl<4>(rectMeterVu, "In Out", SR::Graphics::Layout::SR_DEFAULT_STYLE_METER, EDirection::Vertical, { "", "", "", "" }, 0, iplug::igraphics::IVMeterControl<4>::EResponse::Log, -60.f, 12.f, { 0, -6, -12, -24, -48 }), cMeterVu, "VU");
		pGraphics->AttachControl(new IVMeterControl<2>(rectMeterGr, "GR", SR::Graphics::Layout::SR_DEFAULT_STYLE_METER, EDirection::Vertical, { "", "" }, 0, iplug::igraphics::IVMeterControl<2>::EResponse::Log, -12.f, 0.f, { 0,-1,-2,-3,-4,-6,-9 }), cMeterGr, "GR");
		// -- Set GR meter displaying the other way round
		dynamic_cast<IVMeterControl<2>*>(pGraphics->GetControlWithTag(cMeterGr))->SetBaseValue(1.);

		// Disable Parameters with no function
		for (int ctrlTag = 0; ctrlTag < kNumCtrlTags; ctrlTag++) {
			switch (ctrlTag)
			{
			case cEqHfDs:
			case cEqLfDs:
				pGraphics->GetControlWithTag(ctrlTag)->SetDisabled(true);
				break;
			default:
				break;
			}
		}
		pGraphics->AttachControl(new IVGroupControl(rectControlsGain, "Gain", 0.f, SR::Graphics::Layout::SR_DEFAULT_STYLE));
		pGraphics->AttachControl(new IVGroupControl(rectControlsFilter, "Filter", 0.f, SR::Graphics::Layout::SR_DEFAULT_STYLE));
		pGraphics->AttachControl(new IVGroupControl(rectControlsSat, "Sat", 0.f, SR::Graphics::Layout::SR_DEFAULT_STYLE));
		pGraphics->AttachControl(new IVGroupControl(rectControlsEqParametric, "Parametric EQ", 0.f, SR::Graphics::Layout::SR_DEFAULT_STYLE));
		pGraphics->AttachControl(new IVGroupControl(rectControlsEqPassive, "Passive EQ", 0.f, SR::Graphics::Layout::SR_DEFAULT_STYLE));
		pGraphics->AttachControl(new IVGroupControl(rectControlsCompLevel, "Comp Level", 0.f, SR::Graphics::Layout::SR_DEFAULT_STYLE));
		pGraphics->AttachControl(new IVGroupControl(rectControlsCompPeak, "Comp Peak", 0.f, SR::Graphics::Layout::SR_DEFAULT_STYLE));
		pGraphics->AttachControl(new IVGroupControl(rectControlsStereo, "Stereo", 0.f, SR::Graphics::Layout::SR_DEFAULT_STYLE));
		pGraphics->AttachControl(new IVGroupControl(rectControlsFreqResponse, "Response", 0.f, SR::Graphics::Layout::SR_DEFAULT_STYLE));

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
		if (!GetParam(kBypass)->Bool())
			fGainIn.Process(outputs[0][s], outputs[1][s]);

		// Run input data through envelope filter to match VU like metering, then send to respective buffer. (After input gain) 
		fMeterEnvelope[0].process(abs(outputs[0][s]), meterIn1);
		fMeterEnvelope[1].process(abs(outputs[1][s]), meterIn2);


		mBufferVu.ProcessBuffer(meterIn1, 0, s);
		mBufferVu.ProcessBuffer(meterIn2, 1, s);

		if (!GetParam(kBypass)->Bool()) {
			// Process filters
			if (GetParam(kEqHpFreq)->Value() > 0.) {
				outputs[0][s] = fEqHp.Process(outputs[0][s], 0);
				outputs[1][s] = fEqHp.Process(outputs[1][s], 1);
			}
			if (GetParam(kEqLpFreq)->Value() < 22000.) {
				outputs[0][s] = fEqLp.Process(outputs[0][s], 0);
				outputs[1][s] = fEqLp.Process(outputs[1][s], 1);
			}

			// Process parametric dynamic eq
			fEqHmf.Process(outputs[0][s], outputs[1][s]);
			fEqLmf.Process(outputs[0][s], outputs[1][s]);

			// Process saturation
			outputs[0][s] = fSatInput[0].Process(outputs[0][s]);
			outputs[1][s] = fSatInput[0].Process(outputs[1][s]);

			// Process compressors
			fCompRms.Process(outputs[0][s], outputs[1][s]);
			fCompPeak.Process(outputs[0][s], outputs[1][s]);

			// Process "passive" EQ
#if FLT == 1
			outputs[0][s] = fEqLfBoost.Process(outputs[0][s], 0);
			outputs[1][s] = fEqLfBoost.Process(outputs[1][s], 1);

			outputs[0][s] = fEqLfCut.Process(outputs[0][s], 0);
			outputs[1][s] = fEqLfCut.Process(outputs[1][s], 1);

			outputs[0][s] = fEqHfBoost.Process(outputs[0][s], 0);
			outputs[1][s] = fEqHfBoost.Process(outputs[1][s], 1);

			outputs[0][s] = fEqHfCut.Process(outputs[0][s], 0);
			outputs[1][s] = fEqHfCut.Process(outputs[1][s], 1);
#elif FLT == 3
			outputs[0][s] = fEqLfBoost[0].filter(outputs[0][s]);
			outputs[1][s] = fEqLfBoost[1].filter(outputs[1][s]);

			outputs[0][s] = fEqLfCut[0].filter(outputs[0][s]);
			outputs[1][s] = fEqLfCut[1].filter(outputs[1][s]);

			outputs[0][s] = fEqHfBoost[0].filter(outputs[0][s]);
			outputs[1][s] = fEqHfBoost[1].filter(outputs[1][s]);

			outputs[0][s] = fEqHfCut[0].filter(outputs[0][s]);
			outputs[1][s] = fEqHfCut[1].filter(outputs[1][s]);
#endif // !FLT

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

			if (GetParam(kEqBandSolo)->Int()) {
				outputs[0][s] = fEqBandSolo.Process(outputs[0][s], 0);
				outputs[1][s] = fEqBandSolo.Process(outputs[1][s], 1);
			}
		}

		// Store current gain reduction in respective buffer
		if (!GetParam(kBypass)->Bool()) {
			mBufferMeterGr.ProcessBuffer(fCompRms.GetGrLin(), 0, s);
			mBufferMeterGr.ProcessBuffer(fCompPeak.GetGrLin(), 1, s);
		}
		else {
			// Prevent freezing when bypassed, just set to no gain reduction
			mBufferMeterGr.ProcessBuffer(1., 0, s);
			mBufferMeterGr.ProcessBuffer(1., 1, s);
		}

		// Run input data through envelope filter to match VU like metering, then send to respective buffer.
		fMeterEnvelope[2].process(abs(outputs[0][s]), meterOut1);
		fMeterEnvelope[3].process(abs(outputs[1][s]), meterOut2);
		mBufferVu.ProcessBuffer(meterOut1, 2, s);
		mBufferVu.ProcessBuffer(meterOut2, 3, s);
	}
	mMeterSender.ProcessBlock(mBufferVu.GetBuffer(), nFrames, cMeterVu, 4);
	mMeterSenderGr.ProcessBlock(mBufferMeterGr.GetBuffer(), nFrames, cMeterGr, 2);
}

void SRChannel::OnReset()
{
	// Here we set our complete range of filters and other effects
	// ToDo: Match boost and cut behaviour to normal passive equalizing
	// ToDo: Match all above ToDos in OnParamChange and think of procedure to prevent double code
	const double samplerate = GetSampleRate();

	fGainIn.InitGain(100, SR::DSP::SRGain::kSinusodial);
	fGainOut.InitGain(100, SR::DSP::SRGain::kSinusodial);

	fSatInput[0].SetSaturation(SR::DSP::SRSaturation::kSoftSat, GetParam(kSaturationDrive)->Value(), GetParam(kSaturationAmount)->Value(), 1., true, 0., 1., samplerate);
	fSatInput[1].SetSaturation(SR::DSP::SRSaturation::kSoftSat, GetParam(kSaturationDrive)->Value(), GetParam(kSaturationAmount)->Value(), 1., true, 0., 1., samplerate);

	fEqHp.SetFilter(SR::DSP::BiquadHighpass, GetParam(kEqHpFreq)->Value() / samplerate, 0.707, 0., samplerate);
	fEqLp.SetFilter(SR::DSP::BiquadLowpass, GetParam(kEqLpFreq)->Value() / samplerate, 0.707, 0., samplerate);
	fEqHmf.Reset(GetParam(kEqHmfDs)->Value(), .25, 3., 50., GetParam(kEqHmfFreq)->Value() / samplerate, GetParam(kEqHmfQ)->Value(), 0., samplerate, (GetParam(kEqHmfIsShelf)->Bool()) ? SR::DSP::BiquadHighshelf : SR::DSP::BiquadPeak);
	fEqLmf.Reset(GetParam(kEqLmfDs)->Value(), .25, 7., 200., GetParam(kEqLmfFreq)->Value() / samplerate, GetParam(kEqLmfQ)->Value(), 0., samplerate, (GetParam(kEqLmfIsShelf)->Bool()) ? SR::DSP::BiquadLowshelf : SR::DSP::BiquadPeak);
#if FLT == 1
	fEqLfBoost.SetFilter(SR::DSP::BiquadPeak, GetParam(kEqLfFreq)->Value() / samplerate, .12, GetParam(kEqLfBoost)->Value(), samplerate);
	fEqLfCut.SetFilter(SR::DSP::BiquadPeak, GetParam(kEqLfFreq)->Value() / samplerate, .08, -GetParam(kEqLfCut)->Value(), samplerate);
	fEqHfBoost.SetFilter(SR::DSP::BiquadPeak, GetParam(kEqHfFreq)->Value() / samplerate, 1., GetParam(kEqHfBoost)->Value(), samplerate);
	fEqHfCut.SetFilter(SR::DSP::BiquadPeak, GetParam(kEqHfFreq)->Value() / samplerate, .1, -GetParam(kEqHfCut)->Value(), samplerate);
#elif FLT == 3
	fEqLfBoost[0].setup(samplerate, GetParam(kEqLfFreq)->Value(), GetParam(kEqLfBoost)->Value(), .12);
	fEqLfBoost[1].setup(samplerate, GetParam(kEqLfFreq)->Value(), GetParam(kEqLfBoost)->Value(), .12);
	fEqLfCut[0].setup(samplerate, GetParam(kEqLfFreq)->Value(), -GetParam(kEqLfCut)->Value(), .08);
	fEqLfCut[1].setup(samplerate, GetParam(kEqLfFreq)->Value(), -GetParam(kEqLfCut)->Value(), .08);
	fEqHfBoost[0].setup(samplerate, GetParam(kEqHfFreq)->Value(), GetParam(kEqHfBoost)->Value(), 1.);
	fEqHfBoost[1].setup(samplerate, GetParam(kEqHfFreq)->Value(), GetParam(kEqHfBoost)->Value(), 1.);
	fEqHfCut[0].setup(samplerate, GetParam(kEqHfFreq)->Value(), -GetParam(kEqHfCut)->Value(), .1);
	fEqHfCut[1].setup(samplerate, GetParam(kEqHfFreq)->Value(), -GetParam(kEqHfCut)->Value(), .1);
#endif // !FLT
	fSplitHp.SetFilter(SR::DSP::BiquadLinkwitzHighpass, GetParam(kStereoMonoFreq)->Value() / samplerate, 0., 0., samplerate);
	fSplitLp.SetFilter(SR::DSP::BiquadLinkwitzLowpass, GetParam(kStereoMonoFreq)->Value() / samplerate, 0., 0., samplerate);

	AdjustBandSolo();

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
		GetParam(kCompRmsMix)->Value() * .01, // mix
		samplerate);
	fCompRms.SetWindow(10.);

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
		GetParam(kCompPeakMix)->Value() * .01, // mix
		samplerate);

	for (int e = 0; e < 4; e++) {
		fMeterEnvelope[e].SetAttack(VU_ATTACK);
		fMeterEnvelope[e].SetRelease(VU_RELEASE);
		fMeterEnvelope[e].SetSampleRate(samplerate);
	}
}

void SRChannel::OnIdle()
{
	mMeterSender.TransmitData(*this);
	mMeterSenderGr.TransmitData(*this);
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
		AdjustBandSolo();
		break;
	case kEqLpFreq:
		AdjustBandSolo();
		fEqLp.SetFreq(GetParam(kEqLpFreq)->Value() / samplerate);
		break;
	case kEqLmfFreq:
		fEqLmf.SetFrequency(GetParam(kEqLmfFreq)->Value() / samplerate);
		AdjustBandSolo();
		break;
	case kEqLmfGain:
		fEqLmf.SetGain(GetParam(kEqLmfGain)->Value());
		AdjustBandSolo();
		break;
	case kEqLmfQ:
		fEqLmf.SetQ(GetParam(kEqLmfQ)->Value());
		AdjustBandSolo();
		break;
	case kEqLmfDs:
		fEqLmf.SetThresh(GetParam(kEqLmfDs)->Value());
		break;
	case kEqLmfIsShelf:
		if (GetParam(kEqLmfIsShelf)->Bool())
			fEqLmf.SetType(SR::DSP::BiquadLowshelf);
		else
			fEqLmf.SetType(SR::DSP::BiquadPeak);
		AdjustBandSolo();
		break;
	case kEqHmfFreq:
		fEqHmf.SetFrequency(GetParam(kEqHmfFreq)->Value() / samplerate);
		AdjustBandSolo();
		break;
	case kEqHmfGain:
		fEqHmf.SetGain(GetParam(kEqHmfGain)->Value());
		AdjustBandSolo();
		break;
	case kEqHmfQ:
		fEqHmf.SetQ(GetParam(kEqHmfQ)->Value());
		AdjustBandSolo();
		break;
	case kEqHmfDs:
		fEqHmf.SetThresh(GetParam(kEqHmfDs)->Value());
		break;
	case kEqHmfIsShelf:
		if (GetParam(kEqHmfIsShelf)->Bool())
			fEqHmf.SetType(SR::DSP::BiquadHighshelf);
		else
			fEqHmf.SetType(SR::DSP::BiquadPeak);
		AdjustBandSolo();
		break;
#if FLT == 1
	case kEqLfBoost:
		fEqLfBoost.SetPeakGain(GetParam(kEqLfBoost)->Value());
		AdjustBandSolo();
		break;
	case kEqLfCut:
		fEqLfCut.SetPeakGain(-GetParam(kEqLfCut)->Value());
		break;
	case kEqLfFreq:
		fEqLfBoost.SetFreq(GetParam(kEqLfFreq)->Value() / samplerate);
		fEqLfCut.SetFreq(GetParam(kEqLfFreq)->Value() / samplerate);
		AdjustBandSolo();
		break;
	case kEqHfBoost:
		fEqHfBoost.SetPeakGain(GetParam(kEqHfBoost)->Value());
		AdjustBandSolo();
		break;
	case kEqHfCut:
		fEqHfCut.SetPeakGain(-GetParam(kEqHfCut)->Value());
		break;
	case kEqHfFreq:
		fEqHfBoost.SetFreq(GetParam(kEqHfFreq)->Value() / samplerate);
		fEqHfCut.SetFreq(GetParam(kEqHfFreq)->Value() / samplerate);
		AdjustBandSolo();
		break;
#elif FLT == 3
	case kEqLfBoost:
	case kEqLfCut:
	case kEqLfFreq:
		fEqLfBoost[0].setup(samplerate, GetParam(kEqLfFreq)->Value(), GetParam(kEqLfBoost)->Value(), .12);
		fEqLfBoost[1].setup(samplerate, GetParam(kEqLfFreq)->Value(), GetParam(kEqLfBoost)->Value(), .12);
		fEqLfCut[0].setup(samplerate, GetParam(kEqLfFreq)->Value(), -GetParam(kEqLfCut)->Value(), .08);
		fEqLfCut[1].setup(samplerate, GetParam(kEqLfFreq)->Value(), -GetParam(kEqLfCut)->Value(), .08);
		AdjustBandSolo();
		break;
	case kEqHfBoost:
	case kEqHfCut:
	case kEqHfFreq:
		fEqHfBoost[0].setup(samplerate, GetParam(kEqHfFreq)->Value(), GetParam(kEqHfBoost)->Value(), 1.);
		fEqHfBoost[1].setup(samplerate, GetParam(kEqHfFreq)->Value(), GetParam(kEqHfBoost)->Value(), 1.);
		fEqHfCut[0].setup(samplerate, GetParam(kEqHfFreq)->Value(), -GetParam(kEqHfCut)->Value(), .1);
		fEqHfCut[1].setup(samplerate, GetParam(kEqHfFreq)->Value(), -GetParam(kEqHfCut)->Value(), .1);
		AdjustBandSolo();
		break;
#endif // !FLT
	case kEqBandSolo:
		AdjustBandSolo();
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
		fCompRms.SetMakeup(GetParam(kCompRmsMakeup)->Value());
		break;
	case kCompRmsMix:
		fCompRms.SetMix(GetParam(kCompRmsMix)->Value() * .01);
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
		fCompPeak.SetMakeup(GetParam(kCompPeakMakeup)->Value());
		break;
	case kCompPeakMix:
		fCompPeak.SetMix(GetParam(kCompPeakMix)->Value() * .01);
		break;
	default:
		break;
	}
}
void SRChannel::AdjustBandSolo() {
	const double samplerate = GetSampleRate();
	switch (GetParam(kEqBandSolo)->Int())
	{
	case 0: // Off
		fEqBandSolo.SetFilter(SR::DSP::BiquadPeak, 1000. / samplerate, .707, 0., samplerate); // Dummy
		break;
	case 1: // HP
		fEqBandSolo.SetFilter(SR::DSP::BiquadLowpass, fmax(GetParam(kEqHpFreq)->Value(), 10.) / samplerate, .707, 0., samplerate);
		break;
	case 2: // LP
		fEqBandSolo.SetFilter(SR::DSP::BiquadHighpass, GetParam(kEqLpFreq)->Value() / samplerate, .707, 0., samplerate);
		break;
	case 3: // Hmf
		fEqBandSolo.SetFilter((GetParam(kEqHmfIsShelf)->Bool()) ? SR::DSP::BiquadHighpass : SR::DSP::BiquadBandpass, GetParam(kEqHmfFreq)->Value() / samplerate, GetParam(kEqHmfQ)->Value(), GetParam(kEqHmfGain)->Value(), samplerate);
		break;
	case 4: // Lmf
		fEqBandSolo.SetFilter((GetParam(kEqLmfIsShelf)->Bool()) ? SR::DSP::BiquadLowpass : SR::DSP::BiquadBandpass, GetParam(kEqLmfFreq)->Value() / samplerate, GetParam(kEqLmfQ)->Value(), GetParam(kEqLmfGain)->Value(), samplerate);
		break;
	case 5: // Hf
		fEqBandSolo.SetFilter(SR::DSP::BiquadBandpass, GetParam(kEqHfFreq)->Value() / samplerate, 1., GetParam(kEqHfBoost)->Value(), samplerate);
		break;
	case 6: // Lf
		fEqBandSolo.SetFilter(SR::DSP::BiquadBandpass, GetParam(kEqLfFreq)->Value() / samplerate, .12, GetParam(kEqLfBoost)->Value(), samplerate);
		break;

	default:
		break;
	}
}
void SRChannel::SetFreqMeterValues()
{
	const double samplerate = GetSampleRate();
	//const double shape = log10(samplerate);
	const double shape = log10(22000.);
	for (int i = 0; i < FREQRESP_NUMVALUES; i++) {
		// If linear shape
		//double freq = 0.5 * samplerate * double(i) / double(FREQRESP_NUMVALUES);
		// If pow shape
		// -- By adding 60 to the counter and the numValues we just spread the values leaving the first 60 out, which is the range 0 - 20 Hz in log(10)
		double freq = 22000. * std::pow((double(i + 60) / double(FREQRESP_NUMVALUES + 60)), shape);
		//double freq = 0.5 * samplerate * std::pow((double(i) / double(FREQRESP_NUMVALUES)), shape);
		mFreqMeterValues[i] = 0.;
		// We want a nice flat line if plugin is bypassed
		if (!GetParam(kBypass)->Bool()) {
			if (GetParam(kEqHpFreq)->Value() > 0.) mFreqMeterValues[i] += fEqHp.GetFrequencyResponse(freq / samplerate, FREQRESP_RANGEDB, false);
			if (GetParam(kEqLpFreq)->Value() < 22000.) mFreqMeterValues[i] += fEqLp.GetFrequencyResponse(freq / samplerate, FREQRESP_RANGEDB, false);
			mFreqMeterValues[i] += fEqHmf.fDynamicEqFilter.GetFrequencyResponse(freq / samplerate, FREQRESP_RANGEDB, false);
			mFreqMeterValues[i] += fEqLmf.fDynamicEqFilter.GetFrequencyResponse(freq / samplerate, FREQRESP_RANGEDB, false);
#if FLT == 1
			if (GetParam(kEqHfBoost)->Value() != 0.0) mFreqMeterValues[i] += fEqHfBoost.GetFrequencyResponse(freq / samplerate, FREQRESP_RANGEDB, false);
			if (GetParam(kEqHfCut)->Value() != 0.0) mFreqMeterValues[i] += fEqHfCut.GetFrequencyResponse(freq / samplerate, FREQRESP_RANGEDB, false);
			if (GetParam(kEqLfBoost)->Value() != 0.0) mFreqMeterValues[i] += fEqLfBoost.GetFrequencyResponse(freq / samplerate, FREQRESP_RANGEDB, false);
			if (GetParam(kEqLfCut)->Value() != 0.0) mFreqMeterValues[i] += fEqLfCut.GetFrequencyResponse(freq / samplerate, FREQRESP_RANGEDB, false);
#elif FLT == 3
			// Like above, but since response function gets complex_t, we just have to use abs() for magnitude or arg() for phase, then convert to dB and normalize
			if (GetParam(kEqLfBoost)->Value() != 0.0) mFreqMeterValues[i] += AmpToDB(abs(fEqLfBoost[0].response(freq / samplerate))) / FREQRESP_RANGEDB;
			if (GetParam(kEqLfCut)->Value() != 0.0) mFreqMeterValues[i] += AmpToDB(abs(fEqLfCut[0].response(freq / samplerate))) / FREQRESP_RANGEDB;
			if (GetParam(kEqHfBoost)->Value() != 0.0) mFreqMeterValues[i] += AmpToDB(abs(fEqHfBoost[0].response(freq / samplerate))) / FREQRESP_RANGEDB;
			if (GetParam(kEqHfCut)->Value() != 0.0) mFreqMeterValues[i] += AmpToDB(abs(fEqHfCut[0].response(freq / samplerate))) / FREQRESP_RANGEDB;;
#endif // !FLT
		}
	}
	if (GetUI() && mFreqMeterValues != 0) dynamic_cast<SR::Graphics::Controls::SRGraphBase*>(GetUI()->GetControlWithTag(cMeterFreqResponse))->Process(mFreqMeterValues);
}
#endif
