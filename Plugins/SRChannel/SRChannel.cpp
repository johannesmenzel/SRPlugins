#include "SRChannel.h"
#include "IPlug_include_in_plug_src.h"
#include "IControls.h"
#include "../../Classes/Graphics/SRCustomKnob.h"
#include "../../Classes/Graphics/SRCustomLayout.h"

SRChannel::SRChannel(const InstanceInfo& info)
	: Plugin(info, MakeConfig(kNumParams, kNumPresets))
	, fGainIn(100)
	, fGainOut(100)
	, fEqHp()
	, fEqLp()
	, fEqLfBoost()
	, fEqLfCut()
	, fEqHfBoost()
	, fEqHfCut()
	, fEqLmf()
	, fEqHmf()
{
	GetParam(kGainIn)->InitDouble("Input", 0., -120., 12., 0.01, "dB", 0, "", IParam::ShapePowCurve(SR::Utils::SetShapeCentered(-120., 12., 0., .5)));
	GetParam(kGainOut)->InitDouble("Output", 0., -120., 12., 0.01, "dB", 0, "", IParam::ShapePowCurve(SR::Utils::SetShapeCentered(-120., 12., 0., .5)));

	GetParam(kStereoPan)->InitDouble("Pan", 0., -100., 100., 1., "%");
	GetParam(kStereoWidth)->InitDouble("Width", 100., 0., 1000., 1., "%", 0, "", IParam::ShapePowCurve(SR::Utils::SetShapeCentered(0., 1000., 100., .5)));


	GetParam(kEqHpFreq)->InitDouble("HP", 16., 16., 350., 1., "Hz");
	GetParam(kEqLpFreq)->InitDouble("LP", 22000., 3000., 22000., 1., "Hz");


	GetParam(kEqHfBoost)->InitDouble("HF Boost", 0., -0., 10., 1., "dB", IParam::EFlags::kFlagStepped);
	GetParam(kEqHfCut)->InitDouble("HF Cut", 0., -0., 10., 1., "dB", IParam::EFlags::kFlagStepped);
	GetParam(kEqHfFreq)->InitDouble("HF Freq", 8000., 3000., 16000., 1000., "Hz", IParam::EFlags::kFlagStepped, "", IParam::ShapePowCurve(SR::Utils::SetShapeCentered(3000., 16000., 8000., .5)));
	GetParam(kEqHfDs)->InitDouble("HF DS", 0., -20., 0., .01, "dB");

	GetParam(kEqHmfGain)->InitDouble("HMF Gain", 0., -12., 12., 0.01, "dB");
	GetParam(kEqHmfFreq)->InitDouble("HMF Freq", 3000., 600., 7000., 1., "Hz", 0, "", IParam::ShapePowCurve(SR::Utils::SetShapeCentered(600., 7000., 3000., .5)));
	GetParam(kEqHmfQ)->InitDouble("HMF Q", .707, 0.1, 10., 0.01, "", 0, "", IParam::ShapePowCurve(SR::Utils::SetShapeCentered(.1, 10., .707, .5)));
	GetParam(kEqHmfDs)->InitDouble("HMF DS", 0., -20., 0., .01, "dB");

	GetParam(kEqLmfGain)->InitDouble("LMF Gain", 0., -12., 12., 0.01, "dB");
	GetParam(kEqLmfFreq)->InitDouble("LMF Freq", 1000., 200., 2500., 1., "Hz", 0, "", IParam::ShapePowCurve(SR::Utils::SetShapeCentered(200., 2500., 1000., .5)));
	GetParam(kEqLmfQ)->InitDouble("LMF Q", .707, 0.1, 10., 0.01, "", 0, "", IParam::ShapePowCurve(SR::Utils::SetShapeCentered(.1, 10., .707, .5)));
	GetParam(kEqLmfDs)->InitDouble("LMF DS", 0., -20., 0., .01, "dB");

	GetParam(kEqLfBoost)->InitDouble("LF Boost", 0., -0., 10., 1., "dB", IParam::EFlags::kFlagStepped);
	GetParam(kEqLfCut)->InitDouble("LF Cut", 0., -0., 10., 1., "dB", IParam::EFlags::kFlagStepped);
	GetParam(kEqLfFreq)->InitDouble("LF Freq", 100., 30., 300., 20., "Hz", IParam::EFlags::kFlagStepped, "", IParam::ShapePowCurve(SR::Utils::SetShapeCentered(30., 300., 100., .5)));
	GetParam(kEqLfDs)->InitDouble("LF DS", 0., -20., 0., .01, "dB");

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
		const IRECT rectControls = b.GetPadded(0.f, -20.f, -50.f, 0.f);
		const IRECT rectTitle = b.GetPadded(0.f, 0.f, -50.f, -700.f);
		const IRECT rectMeterVu = b.GetFromRight(50.f);
		pGraphics->AttachControl(new ITextControl(rectTitle, PLUG_MFR " " PLUG_NAME " " PLUG_VERSION_STR "-alpha", SR::Graphics::Layout::SR_DEFAULT_TEXT));

		pGraphics->AttachControl(new SR::Graphics::Controls::Knob(rectControls.GetGridCell(4, 11, 6, 12).GetCentredInside(100.f), kGainIn, "Input", SR::Graphics::Layout::SR_DEFAULT_STYLE, true, false, -150.f, 150.f, -150.f, EDirection::Vertical, 4., 1.f), cGainIn);
		pGraphics->AttachControl(new SR::Graphics::Controls::Knob(rectControls.GetGridCell(5, 11, 6, 12).GetCentredInside(100.f), kGainOut, "Output", SR::Graphics::Layout::SR_DEFAULT_STYLE, true, false, -150.f, 150.f, -150.f, EDirection::Vertical, 4., 1.f), cGainOut);

		pGraphics->AttachControl(new SR::Graphics::Controls::Knob(rectControls.GetGridCell(4, 0, 6, 12).GetCentredInside(100.f), kEqLpFreq, "LP", SR::Graphics::Layout::SR_DEFAULT_STYLE, true, false, -150.f, 150.f, -150.f, EDirection::Vertical, 4., 1.f), cEqLpFreq);
		pGraphics->AttachControl(new SR::Graphics::Controls::Knob(rectControls.GetGridCell(5, 0, 6, 12).GetCentredInside(100.f), kEqHpFreq, "HP", SR::Graphics::Layout::SR_DEFAULT_STYLE, true, false, -150.f, 150.f, -150.f, EDirection::Vertical, 4., 1.f), cEqHpFreq);

		pGraphics->AttachControl(new PlaceHolder(rectControls.GetGridCell(0, 2, 6, 12).FracRectHorizontal(4.f).FracRectVertical(2.f, true), "Frequency Response"));
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

		pGraphics->AttachControl(new PlaceHolder(rectControls.GetGridCell(0, 7, 6, 12).FracRectHorizontal(2.f).FracRectVertical(6.f, true), "Compressors"));
		//pGraphics->AttachControl(new PlaceHolder(rectControls.GetGridCell(0, 10, 6, 12).FracRectHorizontal(1.f).FracRectVertical(6.f, true), "Stereo"));
		
		pGraphics->AttachControl(new SR::Graphics::Controls::Knob(rectControls.GetGridCell(0, 10, 6, 12).GetCentredInside(100.f), kStereoPan, "Pan", SR::Graphics::Layout::SR_DEFAULT_STYLE, true, false, -150.f, 150.f, -150.f, EDirection::Vertical, 4., 1.f), cStereoPan);
		pGraphics->AttachControl(new SR::Graphics::Controls::Knob(rectControls.GetGridCell(1, 10, 6, 12).GetCentredInside(100.f), kStereoWidth, "Width", SR::Graphics::Layout::SR_DEFAULT_STYLE, true, false, -150.f, 150.f, -150.f, EDirection::Vertical, 4., 1.f), cStereoWidth);

		pGraphics->AttachControl(new IVMeterControl<2>(rectMeterVu.GetGridCell(0, 0, 1, 2), "In", SR::Graphics::Layout::SR_DEFAULT_STYLE, EDirection::Vertical, { "L", "R" }, 0, iplug::igraphics::IVMeterControl<2>::EResponse::Linear, -60.f, 0.f), cMeterIn);
		pGraphics->AttachControl(new IVMeterControl<2>(rectMeterVu.GetGridCell(0, 1, 1, 2), "Out", SR::Graphics::Layout::SR_DEFAULT_STYLE, EDirection::Vertical, { "L", "R" }, 0, iplug::igraphics::IVMeterControl<2>::EResponse::Linear, -60.f, 0.f), cMeterOut);





	};
#endif
}

#if IPLUG_DSP
void SRChannel::ProcessBlock(sample** inputs, sample** outputs, int nFrames)
{
	const int nChans = NOutChansConnected();
	for (int s = 0; s < nFrames; s++) {

		for (int c = 0; c < nChans; c++) {
			outputs[c][s] = inputs[c][s];
		}

		fGainIn.Process(outputs[0][s], outputs[1][s]);

		outputs[0][s] = fEqHp.Process(outputs[0][s], 0);
		outputs[1][s] = fEqHp.Process(outputs[1][s], 1);
		outputs[0][s] = fEqLp.Process(outputs[0][s], 0);
		outputs[1][s] = fEqLp.Process(outputs[1][s], 1);

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

		fGainOut.Process(outputs[0][s], outputs[1][s]);

		fMeterEnvelope[0].process(abs(inputs[0][s]), meterIn1);
		fMeterEnvelope[1].process(abs(inputs[1][s]), meterIn2);
		fMeterEnvelope[2].process(abs(outputs[0][s]), meterOut1);
		fMeterEnvelope[3].process(abs(outputs[1][s]), meterOut2);
		mBufferInput.ProcessBuffer(meterIn1, 0, s);
		mBufferInput.ProcessBuffer(meterIn2, 1, s);
		mBufferOutput.ProcessBuffer(meterOut1, 0, s);
		mBufferOutput.ProcessBuffer(meterOut2, 1, s);
	}
	//fEqHp.ProcessBlock(outputs, outputs, 2, nFrames);
	//fEqLp.ProcessBlock(outputs, outputs, 2, nFrames);
	mMeterSenderIn.ProcessBlock(mBufferInput.GetBuffer(), nFrames, cMeterIn);
	mMeterSenderOut.ProcessBlock(mBufferOutput.GetBuffer(), nFrames, cMeterOut);

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
	fEqHp.SetFilter(SR::DSP::SRFilterIIR<sample, 2>::BiquadHighpass, GetParam(kEqHpFreq)->Value() / samplerate, 0.707, 0., samplerate);
	fEqLp.SetFilter(SR::DSP::SRFilterIIR<sample, 2>::BiquadLowpass, GetParam(kEqLpFreq)->Value() / samplerate, 0.707, 0., samplerate);
	fEqHfBoost.SetFilter(SR::DSP::SRFilterIIR<sample, 2>::BiquadPeak, GetParam(kEqHfFreq)->Value() / samplerate, 0.707, GetParam(kEqHfBoost)->Value(), samplerate);
	fEqHfCut.SetFilter(SR::DSP::SRFilterIIR<sample, 2>::BiquadHighshelf, fmin(1.5 * GetParam(kEqHfFreq)->Value() / samplerate, .499), 0.707, -GetParam(kEqHfCut)->Value(), samplerate);
	fEqHmf.SetFilter(SR::DSP::SRFilterIIR<sample, 2>::BiquadPeak, GetParam(kEqHmfFreq)->Value() / samplerate, GetParam(kEqHmfQ)->Value(), GetParam(kEqHmfGain)->Value(), samplerate);
	fEqLmf.SetFilter(SR::DSP::SRFilterIIR<sample, 2>::BiquadPeak, GetParam(kEqLmfFreq)->Value() / samplerate, GetParam(kEqLmfQ)->Value(), GetParam(kEqLmfGain)->Value(), samplerate);
	fEqLfBoost.SetFilter(SR::DSP::SRFilterIIR<sample, 2>::BiquadLowshelf, GetParam(kEqLfFreq)->Value() / samplerate, 0.707, GetParam(kEqLfBoost)->Value(), samplerate);
	fEqLfCut.SetFilter(SR::DSP::SRFilterIIR<sample, 2>::BiquadLowshelf, 1.5 * GetParam(kEqLfFreq)->Value() / samplerate, 0.707, -GetParam(kEqLfCut)->Value(), samplerate);
}
void SRChannel::OnIdle()
{
	mMeterSenderIn.TransmitData(*this);
	mMeterSenderOut.TransmitData(*this);
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
	case kStereoWidth:
	case kStereoPan:
		fGainOut.SetGain(DBToAmp(GetParam(kGainOut)->Value()));
		fGainOut.SetPanPosition((GetParam(kStereoPan)->Value() + 100.) / 200.);
		fGainOut.SetWidth(GetParam(kStereoWidth)->Value() * 0.01);
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
	}
}
#endif
