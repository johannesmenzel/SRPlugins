#include "SRDynamicsControl.h"
#include "IPlug_include_in_plug_src.h"
#include "IControls.h"
#include "../../Classes/Graphics/SRCustomKnob.h"
#include "../../Classes/Graphics/SRCustomLayout.h"

SRDynamicsControl::SRDynamicsControl(const InstanceInfo& info)
	: Plugin(info, MakeConfig(kNumParams, kNumPresets))
	, fOutGain(100)
	, fCompLevel()
	, fCompOpto()
	, fCompVca()
	//, fDeesser()
	//, fDeplosive()
	, fCompFet()
	, fCompLim()
{
	GetParam(kOutGain)->InitDouble("Output", 0., -12., 12., 0.01, "dB");
	GetParam(kThresh)->InitDouble("Thresh", 0., 0., 100., 0.01, "%");
	GetParam(kCrest)->InitDouble("Crest", 50., 0., 100., 0.01, "%");
	GetParam(kRatio)->InitDouble("Ratio", 50., 0., 100., 0.01, "%");
	GetParam(kAttack)->InitDouble("Attack", 50., 0., 100., 0.01, "%");
	GetParam(kRelease)->InitDouble("Release", 50., 0., 100., 0.01, "%");
	SetCompressorValues();


#if IPLUG_EDITOR // http://bit.ly/2S64BDd
	mMakeGraphicsFunc = [&]() {
		return MakeGraphics(*this, PLUG_WIDTH, PLUG_HEIGHT, PLUG_FPS, GetScaleForScreen(PLUG_WIDTH, PLUG_HEIGHT));
	};

	mLayoutFunc = [&](IGraphics* pGraphics) {
		pGraphics->AttachCornerResizer(EUIResizerMode::Scale, false);
		pGraphics->AttachPanelBackground(SR::Graphics::Layout::SR_DEFAULT_COLOR_CUSTOM_PANEL_BG);
		pGraphics->LoadFont("Roboto-Regular", ROBOTO_FN);
		const IRECT b = pGraphics->GetBounds();
		const IRECT c = b.GetFromBottom(200.f);
		const IRECT t = b.GetFromTop(50.f);
		pGraphics->AttachControl(new ITextControl(t, "SRDynamicsControl " PLUG_VERSION_STR "-alpha", SR::Graphics::Layout::SR_DEFAULT_TEXT));
		pGraphics->AttachControl(new SR::Graphics::Controls::Knob(c.GetGridCell(0, 0, 2, 4), kAttack, "Attack", SR::Graphics::Layout::SR_DEFAULT_STYLE, true, false, -150.f, 150.f, -150.f, EDirection::Vertical, 4., 1.f), cAttack);
		pGraphics->AttachControl(new SR::Graphics::Controls::Knob(c.GetGridCell(0, 1, 2, 4), kRelease, "Release", SR::Graphics::Layout::SR_DEFAULT_STYLE, true, false, -150.f, 150.f, -150.f, EDirection::Vertical, 4., 1.f), cRelease);
		pGraphics->AttachControl(new SR::Graphics::Controls::Knob(c.GetGridCell(0, 2, 2, 4), kCrest, "Crest", SR::Graphics::Layout::SR_DEFAULT_STYLE, true, false, -150.f, 150.f, -150.f, EDirection::Vertical, 4., 1.f), cCrest);
		pGraphics->AttachControl(new SR::Graphics::Controls::Knob(c.GetGridCell(1, 0, 2, 4), kThresh, "Tresh", SR::Graphics::Layout::SR_DEFAULT_STYLE, true, false, -150.f, 150.f, -150.f, EDirection::Vertical, 4., 1.f), cThresh);
		pGraphics->AttachControl(new SR::Graphics::Controls::Knob(c.GetGridCell(1, 1, 2, 4), kRatio, "Ratio", SR::Graphics::Layout::SR_DEFAULT_STYLE, true, false, -150.f, 150.f, -150.f, EDirection::Vertical, 4., 1.f), cRatio);
		pGraphics->AttachControl(new SR::Graphics::Controls::Knob(c.GetGridCell(1, 2, 2, 4), kOutGain, "Output", SR::Graphics::Layout::SR_DEFAULT_STYLE, true, false, -150.f, 150.f, -150.f, EDirection::Vertical, 4., 1.f), cOutGain);
		pGraphics->AttachControl(new IVMeterControl<2>(c.GetGridCell(0, 14, 1, 16), "In", SR::Graphics::Layout::SR_DEFAULT_STYLE, EDirection::Vertical, { "L", "R" }, 0, -60.f, 0.f), cMeterIn);
		pGraphics->AttachControl(new IVMeterControl<2>(c.GetGridCell(0, 15, 1, 16), "Out", SR::Graphics::Layout::SR_DEFAULT_STYLE, EDirection::Vertical, { "L", "R" }, 0, -60.f, 0.f), cMeterOut);
		pGraphics->AttachControl(new IVMeterControl<1>(c.GetGridCell(0, 30, 1, 40), "L", SR::Graphics::Layout::SR_DEFAULT_STYLE, EDirection::Vertical, { }, 0, -18.f, 0.f), cMeterGrLevel);
		pGraphics->AttachControl(new IVMeterControl<1>(c.GetGridCell(0, 31, 1, 40), "O", SR::Graphics::Layout::SR_DEFAULT_STYLE, EDirection::Vertical, { }, 0, -18.f, 0.f), cMeterGrOpto);
		pGraphics->AttachControl(new IVMeterControl<1>(c.GetGridCell(0, 32, 1, 40), "V", SR::Graphics::Layout::SR_DEFAULT_STYLE, EDirection::Vertical, { }, 0, -18.f, 0.f), cMeterGrVca);
		pGraphics->AttachControl(new IVMeterControl<1>(c.GetGridCell(0, 33, 1, 40), "F", SR::Graphics::Layout::SR_DEFAULT_STYLE, EDirection::Vertical, { }, 0, -18.f, 0.f), cMeterGrFet);
		pGraphics->AttachControl(new IVMeterControl<1>(c.GetGridCell(0, 34, 1, 40), "L", SR::Graphics::Layout::SR_DEFAULT_STYLE, EDirection::Vertical, { }, 0, -18.f, 0.f), cMeterGrLim);
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

	fCompLevel.ResetCompressor(
		thresh * 48. * (2. - crest), // thresh -48 .. -24 .. 0
		1. / (1. + ratio * 1.), // ratio 1 .. 1.5 .. 2
		20. + attack * 80., // attack 20 .. 60 .. 100
		500. + release * 4500., // release 500 .. 2500 .. 4500
		.0, // sidechain HP
		10., // knee dB
		false, // feedback
		true, // automake
		-18., // reference
		samplerate);
	fCompLevel.SetWindow(100.);

	fCompOpto.ResetCompressor(
		thresh * 27. * (1.5 - crest * .5), // thresh -27 .. -13.5 .. 0
		1. / (1. + ratio * 4.), // ratio 1 .. 3 .. 5
		3. + attack * 12., // attack 5 .. 15 .. 25
		200. + release * 1800., // release 200 .. 1100 .. 2000
		.0, // sidechain HP
		8., // knee dB
		true, // feedback
		true, // automake
		-18., // reference
		samplerate);
	fCompOpto.SetMaxGrDb(10.,true);
	fCompOpto.SetWindow(4.);

	fCompVca.ResetCompressor(
		thresh * 24., // thresh
		1. / (1. + ratio * 5.), // ratio
		3. + attack * 12., // attack
		50. + release * 450., // release
		.0, // sidechain HP
		6., // knee dB
		false, // feedback
		true, // automake
		-18., // reference
		samplerate);	
	
	fCompFet.ResetCompressor(
		thresh * 24. * (.5 + crest * .5), // thresh
		1. / (1. + ratio * 19.), // ratio
		1. + attack * 4., // attack
		40. + release * 360., // release
		.0, // sidechain HP
		4., // knee dB
		true, // feedback
		true, // automake
		-18., // reference
		samplerate);

	fCompLim.SetThresh(thresh * 6. * crest);
	// no ratio
	fCompLim.SetAttack(.02 + attack * 1.);
	fCompLim.SetRelease(5. + release * 10.);
	//fCompLim.ResetCompressor(
	//	thresh * 12. * crest, // thresh
	//	1. / (1. + ratio * 99.), // ratio
	//	.02 + attack * .08, // attack
	//	20. + release * 180., // release
	//	.0, // sidechain HP
	//	2., // knee dB
	//	false, // feedback
	//	true, // automake
	//	-18., // reference
	//	samplerate);
	
}

void SRDynamicsControl::ProcessBlock(sample** inputs, sample** outputs, int nFrames)
{
	const int nChans = NOutChansConnected();

	for (int s = 0; s < nFrames; s++) {
		for (int c = 0; c < nChans; c++) {
			outputs[c][s] = inputs[c][s];
		}

		fCompLevel.Process(outputs[0][s], outputs[1][s]);
		fCompOpto.Process(outputs[0][s], outputs[1][s]);
		fCompVca.Process(outputs[0][s], outputs[1][s]);
		fCompFet.Process(outputs[0][s], outputs[1][s]);
		fCompLim.Process(outputs[0][s], outputs[1][s]);

		for (int c = 0; c < nChans; c++) {
			fOutGain.Process(outputs[c][s], outputs[c][s]);
		}

		mBufferMeterGrLevel.ProcessBuffer(fCompLevel.GetGrLin(), 0, s);
		mBufferMeterGrOpto.ProcessBuffer(fCompOpto.GetGrLin(), 0, s);
		mBufferMeterGrVca.ProcessBuffer(fCompVca.GetGrLin(), 0, s);
		mBufferMeterGrFet.ProcessBuffer(fCompFet.GetGrLin(), 0, s);
		mBufferMeterGrLim.ProcessBuffer(fCompLim.GetGrLin(), 0, s);

	}

	mMeterSenderIn.ProcessBlock(inputs, nFrames, cMeterIn);
	mMeterSenderOut.ProcessBlock(outputs, nFrames, cMeterOut);
	mMeterSenderGrLevel.ProcessBlock(mBufferMeterGrLevel.GetBuffer(), nFrames, cMeterGrLevel);
	mMeterSenderGrOpto.ProcessBlock(mBufferMeterGrOpto.GetBuffer(), nFrames, cMeterGrOpto);
	mMeterSenderGrVca.ProcessBlock(mBufferMeterGrVca.GetBuffer(), nFrames, cMeterGrVca);
	mMeterSenderGrFet.ProcessBlock(mBufferMeterGrFet.GetBuffer(), nFrames, cMeterGrFet);
	mMeterSenderGrLim.ProcessBlock(mBufferMeterGrLim.GetBuffer(), nFrames, cMeterGrLim);
}

void SRDynamicsControl::OnReset() {

}

void SRDynamicsControl::OnParamChange(int paramIdx)
{
	switch (paramIdx)
	{
	case kOutGain: fOutGain.SetGain(DBToAmp(GetParam(paramIdx)->Value())); break;
	case kThresh:
	case kRatio:
	case kAttack:
	case kRelease:
	case kCrest:
		SetCompressorValues(); break;
	}
}
#endif
