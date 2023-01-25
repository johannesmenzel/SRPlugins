#include "SRDynamicsControl.h"
#include "IPlug_include_in_plug_src.h"
#include "IControls.h"
#include "../../Classes/Graphics/SRCustomKnob.h"
#include "../../Classes/Graphics/SRCustomLayout.h"

SRDynamicsControl::SRDynamicsControl(const InstanceInfo& info)
	: Plugin(info, MakeConfig(kNumParams, kNumPresets))
	, fInGain(100)
	, fOutGain(100)
	, fCompLevel()
	, fCompOpto()
	, fCompVca()
	//, fDeesser()
	//, fDeplosive()
	, fCompFet()
	, fCompLim()
	, fMeterEnvelope()
{
	GetParam(kInGain)->InitDouble("Input", 0., -12., 12., 0.01, "dB");
	GetParam(kOutGain)->InitDouble("Output", 0., -12., 12., 0.01, "dB");
	GetParam(kThresh)->InitDouble("Thresh", 0., 0., 100., 0.01, "%");
	GetParam(kCrest)->InitDouble("Crest", 50., 0., 100., 0.01, "%");
	GetParam(kRatio)->InitDouble("Ratio", 50., 0., 100., 0.01, "%");
	GetParam(kAttack)->InitDouble("Attack", 50., 0., 100., 0.01, "%");
	GetParam(kRelease)->InitDouble("Release", 50., 0., 100., 0.01, "%");
	GetParam(kSCFreq)->InitDouble("SC Freq", 0., 0., 2000., 1., "Hz", 0, "", IParam::ShapePowCurve(SR::Utils::SetShapeCentered(0., 2000., 200., .5)));
	GetParam(kMix)->InitDouble("Mix", 100., 0., 100., 0.01, "%");
	OnReset();


#if IPLUG_EDITOR // http://bit.ly/2S64BDd
	mMakeGraphicsFunc = [&]() {
		return MakeGraphics(*this, PLUG_WIDTH, PLUG_HEIGHT, PLUG_FPS, GetScaleForScreen(PLUG_WIDTH, PLUG_HEIGHT));
	};

	mLayoutFunc = [&](IGraphics* pGraphics) {
		pGraphics->AttachCornerResizer(EUIResizerMode::Scale, false);
		pGraphics->AttachPanelBackground(SR::Graphics::Layout::SR_DEFAULT_COLOR_CUSTOM_PANEL_BG);
		pGraphics->LoadFont("Roboto-Regular", ROBOTO_FN);
		// Make RECTs for Controls
		const IRECT b = pGraphics->GetBounds();
		const IRECT rectControls = b.GetPadded(-50.f, -20.f, -50.f, 0.f);
		const IRECT rectTitle = b.GetPadded(-50.f, 0.f, -50.f, -280.f);
		const IRECT rectMeterGr = b.GetFromLeft(50.f);
		const IRECT rectMeterVu = b.GetFromRight(50.f);
		// Atach Controls
		pGraphics->AttachControl(new ITextControl(rectTitle, "SRDynamicsControl " PLUG_VERSION_STR "-alpha", SR::Graphics::Layout::SR_DEFAULT_TEXT));
		pGraphics->AttachControl(new SR::Graphics::Controls::Knob(rectControls.GetGridCell(0, 0, 2, 5).GetCentredInside(100.f), kAttack, "Attack", SR::Graphics::Layout::SR_DEFAULT_STYLE, true, false, -150.f, 150.f, -150.f, EDirection::Vertical, 4., 1.f), cAttack, "Controls");
		pGraphics->AttachControl(new SR::Graphics::Controls::Knob(rectControls.GetGridCell(0, 1, 2, 5).GetCentredInside(100.f), kRelease, "Release", SR::Graphics::Layout::SR_DEFAULT_STYLE, true, false, -150.f, 150.f, -150.f, EDirection::Vertical, 4., 1.f), cRelease, "Controls");
		pGraphics->AttachControl(new SR::Graphics::Controls::Knob(rectControls.GetGridCell(0, 2, 2, 5).GetCentredInside(100.f), kCrest, "Crest", SR::Graphics::Layout::SR_DEFAULT_STYLE, true, false, -150.f, 150.f, -150.f, EDirection::Vertical, 4., 1.f), cCrest, "Controls");
		pGraphics->AttachControl(new SR::Graphics::Controls::Knob(rectControls.GetGridCell(0, 3, 2, 5).GetCentredInside(100.f), kSCFreq, "SC Freq", SR::Graphics::Layout::SR_DEFAULT_STYLE, true, false, -150.f, 150.f, -150.f, EDirection::Vertical, 4., 1.f), cSCFreq, "Controls");
		pGraphics->AttachControl(new SR::Graphics::Controls::Knob(rectControls.GetGridCell(0, 4, 2, 5).GetCentredInside(100.f), kInGain, "Input", SR::Graphics::Layout::SR_DEFAULT_STYLE, true, false, -150.f, 150.f, -150.f, EDirection::Vertical, 4., 1.f), cInGain, "Controls");
		pGraphics->AttachControl(new SR::Graphics::Controls::Knob(rectControls.GetGridCell(1, 0, 2, 5).GetCentredInside(100.f), kThresh, "Tresh", SR::Graphics::Layout::SR_DEFAULT_STYLE, true, false, -150.f, 150.f, -150.f, EDirection::Vertical, 4., 1.f), cThresh, "Controls");
		pGraphics->AttachControl(new SR::Graphics::Controls::Knob(rectControls.GetGridCell(1, 1, 2, 5).GetCentredInside(100.f), kRatio, "Ratio", SR::Graphics::Layout::SR_DEFAULT_STYLE, true, false, -150.f, 150.f, -150.f, EDirection::Vertical, 4., 1.f), cRatio, "Controls");
		pGraphics->AttachControl(new SR::Graphics::Controls::Knob(rectControls.GetGridCell(1, 2, 2, 5).GetCentredInside(100.f), kMix, "Mix", SR::Graphics::Layout::SR_DEFAULT_STYLE, true, false, -150.f, 150.f, -150.f, EDirection::Vertical, 4., 1.f), cMix, "Controls");
		pGraphics->AttachControl(new SR::Graphics::Controls::Knob(rectControls.GetGridCell(1, 4, 2, 5).GetCentredInside(100.f), kOutGain, "Output", SR::Graphics::Layout::SR_DEFAULT_STYLE, true, false, -150.f, 150.f, -150.f, EDirection::Vertical, 4., 1.f), cOutGain, "Controls");
		pGraphics->AttachControl(new IVMeterControl<2>(rectMeterVu.GetGridCell(0, 0, 1, 2), "In", SR::Graphics::Layout::SR_DEFAULT_STYLE, EDirection::Vertical, { "", "" }, 0, iplug::igraphics::IVMeterControl<2>::EResponse::Log, -60.f, 0.f), cMeterIn, "VCA Meter");
		pGraphics->AttachControl(new IVMeterControl<2>(rectMeterVu.GetGridCell(0, 1, 1, 2), "Out", SR::Graphics::Layout::SR_DEFAULT_STYLE, EDirection::Vertical, { "", "" }, 0, iplug::igraphics::IVMeterControl<2>::EResponse::Log, -60.f, 0.f), cMeterOut, "VCA Meter");
		pGraphics->AttachControl(new IVMeterControl<1>(rectMeterGr.GetGridCell(0, 0, 1, 5), "L", SR::Graphics::Layout::SR_DEFAULT_STYLE, EDirection::Vertical, { }, 0, iplug::igraphics::IVMeterControl<1>::EResponse::Log, -12.f, 0.f, {}), cMeterGrLevel, "GR Meter");
		pGraphics->AttachControl(new IVMeterControl<1>(rectMeterGr.GetGridCell(0, 1, 1, 5), "O", SR::Graphics::Layout::SR_DEFAULT_STYLE, EDirection::Vertical, { }, 0, iplug::igraphics::IVMeterControl<1>::EResponse::Log, -12.f, 0.f, {}), cMeterGrOpto, "GR Meter");
		pGraphics->AttachControl(new IVMeterControl<1>(rectMeterGr.GetGridCell(0, 2, 1, 5), "V", SR::Graphics::Layout::SR_DEFAULT_STYLE, EDirection::Vertical, { }, 0, iplug::igraphics::IVMeterControl<1>::EResponse::Log, -12.f, 0.f, {}), cMeterGrVca, "GR Meter");
		pGraphics->AttachControl(new IVMeterControl<1>(rectMeterGr.GetGridCell(0, 3, 1, 5), "F", SR::Graphics::Layout::SR_DEFAULT_STYLE, EDirection::Vertical, { }, 0, iplug::igraphics::IVMeterControl<1>::EResponse::Log, -12.f, 0.f, {}), cMeterGrFet, "GR Meter");
		pGraphics->AttachControl(new IVMeterControl<1>(rectMeterGr.GetGridCell(0, 4, 1, 5), "L", SR::Graphics::Layout::SR_DEFAULT_STYLE, EDirection::Vertical, { }, 0, iplug::igraphics::IVMeterControl<1>::EResponse::Log, -12.f, 0.f, {}), cMeterGrLim, "GR Meter");
		dynamic_cast<IVMeterControl<1>*>(pGraphics->GetControlWithTag(cMeterGrLevel))->SetBaseValue(1.);
		dynamic_cast<IVMeterControl<1>*>(pGraphics->GetControlWithTag(cMeterGrOpto))->SetBaseValue(1.);
		dynamic_cast<IVMeterControl<1>*>(pGraphics->GetControlWithTag(cMeterGrVca))->SetBaseValue(1.);
		dynamic_cast<IVMeterControl<1>*>(pGraphics->GetControlWithTag(cMeterGrFet))->SetBaseValue(1.);
		dynamic_cast<IVMeterControl<1>*>(pGraphics->GetControlWithTag(cMeterGrLim))->SetBaseValue(1.);
	};
#endif
}

#if IPLUG_DSP
void SRDynamicsControl::OnIdle()
{
	mMeterSenderIn.TransmitData(*this);
	mMeterSenderOut.TransmitData(*this);
	mMeterSenderGrLevel.TransmitData(*this);
	mMeterSenderGrOpto.TransmitData(*this);
	mMeterSenderGrVca.TransmitData(*this);
	mMeterSenderGrFet.TransmitData(*this);
	mMeterSenderGrLim.TransmitData(*this);
}
void SRDynamicsControl::SetCompressorValues()
{
	const double samplerate = GetSampleRate();
	const double thresh = GetParam(kThresh)->Value() * -.01;
	const double crest = GetParam(kCrest)->Value() * .02;
	const double ratio = GetParam(kRatio)->Value() * .01;
	const double attack = GetParam(kAttack)->Value() * .01;
	const double release = GetParam(kRelease)->Value() * .01;
	const double scnorm = GetParam(kSCFreq)->Value() / samplerate;

	fCompLevel.SetThresh(thresh * 48. * (2. - crest)); // thresh -48 .. -24 .. 0
	fCompLevel.SetRatio(1. / (1. + ratio * 1.)); // ratio 1 .. 1.5 .. 2
	fCompLevel.SetAttack(20. + attack * 80.); // attack 20 .. 60 .. 100
	fCompLevel.SetRelease(500. + release * 4500.); // release 500 .. 2500 .. 4500
	fCompLevel.SetSidechainFilterFreq(scnorm);

	fCompOpto.SetThresh(thresh * 27. * (1.5 - crest * .5)); // thresh -27 .. -13.5 .. 0
	fCompOpto.SetRatio(1. / (1. + ratio * 4.)); // ratio 1 .. 3 .. 5
	fCompOpto.SetAttack(3. + attack * 12.); // attack 5 .. 15 .. 25
	fCompOpto.SetRelease(200. + release * 1800.); // release 200 .. 1100 .. 2000
	fCompOpto.SetSidechainFilterFreq(scnorm);

	fCompVca.SetThresh(thresh * 24.); // thresh
	fCompVca.SetRatio(1. / (1. + ratio * 5.)); // ratio
	fCompVca.SetAttack(3. + attack * 12.); // attack
	fCompVca.SetRelease(50. + release * 450.); // release
	fCompVca.SetSidechainFilterFreq(scnorm);

	fCompFet.SetThresh(thresh * 24. * (.5 + crest * .5));
	fCompFet.SetRatio(1. / (1. + ratio * 19.));
	fCompFet.SetAttack(1. + attack * 4.);
	fCompFet.SetRelease(40. + release * 360.);
	fCompFet.SetSidechainFilterFreq(scnorm);

	fCompLim.SetThresh(thresh * 6. * crest);
	fCompLim.SetAttack(.02 + attack * 1.);
	fCompLim.SetRelease(5. + release * 10.);
}

void SRDynamicsControl::ProcessBlock(sample** inputs, sample** outputs, int nFrames)
{
	const int nChans = NOutChansConnected();
	const double mix = GetParam(kMix)->Value() * .01;

	for (int s = 0; s < nFrames; s++) {
		for (int c = 0; c < nChans; c++) {
			outputs[c][s] = inputs[c][s];
		}
		fInGain.Process(outputs[0][s], outputs[1][s]);

		fCompLevel.Process(outputs[0][s], outputs[1][s]);
		fCompOpto.Process(outputs[0][s], outputs[1][s]);
		fCompVca.Process(outputs[0][s], outputs[1][s]);
		fCompFet.Process(outputs[0][s], outputs[1][s]);

		for (int c = 0; c < nChans; c++) {
			outputs[c][s] = ((1. - mix) * inputs[c][s]) + mix * outputs[c][s];
		}
		fOutGain.Process(outputs[0][s], outputs[1][s]);
		fCompLim.Process(outputs[0][s], outputs[1][s]);

		mBufferMeterGrLevel.ProcessBuffer(fCompLevel.GetGrLin(), 0, s);
		mBufferMeterGrOpto.ProcessBuffer(fCompOpto.GetGrLin(), 0, s);
		mBufferMeterGrVca.ProcessBuffer(fCompVca.GetGrLin(), 0, s);
		mBufferMeterGrFet.ProcessBuffer(fCompFet.GetGrLin(), 0, s);
		mBufferMeterGrLim.ProcessBuffer(fCompLim.GetGrLin(), 0, s);

		
		fMeterEnvelope[0].process(abs(inputs[0][s]), meterIn1);
		fMeterEnvelope[1].process(abs(inputs[1][s]), meterIn2);
		fMeterEnvelope[2].process(abs(outputs[0][s]), meterOut1);
		fMeterEnvelope[3].process(abs(outputs[1][s]), meterOut2);
		mBufferInput.ProcessBuffer(meterIn1, 0, s);
		mBufferInput.ProcessBuffer(meterIn2, 1, s);
		mBufferOutput.ProcessBuffer(meterOut1, 0, s);
		mBufferOutput.ProcessBuffer(meterOut2, 1, s);

	}

	mMeterSenderIn.ProcessBlock(mBufferInput.GetBuffer(), nFrames, cMeterIn);
	mMeterSenderOut.ProcessBlock(mBufferOutput.GetBuffer(), nFrames, cMeterOut);
	mMeterSenderGrLevel.ProcessBlock(mBufferMeterGrLevel.GetBuffer(), nFrames, cMeterGrLevel);
	mMeterSenderGrOpto.ProcessBlock(mBufferMeterGrOpto.GetBuffer(), nFrames, cMeterGrOpto);
	mMeterSenderGrVca.ProcessBlock(mBufferMeterGrVca.GetBuffer(), nFrames, cMeterGrVca);
	mMeterSenderGrFet.ProcessBlock(mBufferMeterGrFet.GetBuffer(), nFrames, cMeterGrFet);
	mMeterSenderGrLim.ProcessBlock(mBufferMeterGrLim.GetBuffer(), nFrames, cMeterGrLim);
}

void SRDynamicsControl::OnReset() {
	const double samplerate = GetSampleRate();
	const double thresh = GetParam(kThresh)->Value() * -.01;
	const double crest = GetParam(kCrest)->Value() * .02;
	const double ratio = GetParam(kRatio)->Value() * .01;
	const double attack = GetParam(kAttack)->Value() * .01;
	const double release = GetParam(kRelease)->Value() * .01;
	const double scnorm = GetParam(kSCFreq)->Value() / samplerate;

	fCompLevel.Reset();
	fCompLevel.ResetCompressor(
		thresh * 48. * (2. - crest), // thresh -48 .. -24 .. 0
		1. / (1. + ratio * 1.), // ratio 1 .. 1.5 .. 2
		20. + attack * 80., // attack 20 .. 60 .. 100
		500. + release * 4500., // release 500 .. 2500 .. 4500
		scnorm, // sidechain HP
		10., // knee dB
		false, // feedback
		true, // automake
		-18., // reference
		1., // mix
		samplerate);
	fCompLevel.SetWindow(100.);
	
	fCompOpto.Reset();
	fCompOpto.ResetCompressor(
		thresh * 27. * (1.5 - crest * .5), // thresh -27 .. -13.5 .. 0
		1. / (1. + ratio * 4.), // ratio 1 .. 3 .. 5
		3. + attack * 12., // attack 5 .. 15 .. 25
		200. + release * 1800., // release 200 .. 1100 .. 2000
		scnorm, // sidechain HP
		8., // knee dB
		true, // feedback
		true, // automake
		-18., // reference
		1., // mix
		samplerate);
	fCompOpto.SetMaxGrDb(10., true);
	fCompOpto.SetWindow(4.);

	fCompVca.Reset();
	fCompVca.ResetCompressor(
		thresh * 24., // thresh
		1. / (1. + ratio * 5.), // ratio
		3. + attack * 12., // attack
		50. + release * 450., // release
		scnorm, // sidechain HP
		6., // knee dB
		false, // feedback
		true, // automake
		-18., // reference
		1., // mix
		samplerate);

	fCompFet.Reset();
	fCompFet.ResetCompressor(
		thresh * 24. * (.5 + crest * .5), // thresh
		1. / (1. + ratio * 19.), // ratio
		1. + attack * 4., // attack
		40. + release * 360., // release
		scnorm, // sidechain HP
		4., // knee dB
		true, // feedback
		true, // automake
		-18., // reference
		1., // mix
		samplerate);

	fCompLim.Reset();
	fCompLim.SetSampleRate(samplerate);
	fCompLim.SetThresh(thresh * 6. * crest);
	fCompLim.SetAttack(.02 + attack * 1.);
	fCompLim.SetRelease(5. + release * 10.);

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
}

void SRDynamicsControl::OnParamChange(int paramIdx)
{
	switch (paramIdx)
	{
	case kInGain: 
		fInGain.SetGain(DBToAmp(GetParam(paramIdx)->Value())); 
		break;
	case kOutGain: 
		fOutGain.SetGain(DBToAmp(GetParam(paramIdx)->Value())); 
		break;
	case kThresh:
	case kRatio:
	case kAttack:
	case kRelease:
	case kSCFreq:
	case kCrest:
		SetCompressorValues();
		break;
	}
}
#endif
