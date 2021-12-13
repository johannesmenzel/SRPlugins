#include "SRChannel.h"
#include "IPlug_include_in_plug_src.h"
#include "IControls.h"
#include "../../Classes/Graphics/SRCustomKnob.h"

SRChannel::SRChannel(const InstanceInfo& info)
	: Plugin(info, MakeConfig(kNumParams, kNumPresets))
	//, fHighshelfLeft(Iir::ChebyshevII::HighShelf<4>())
	//, fHighshelfRight(Iir::ChebyshevII::HighShelf<4>())
{
	GetParam(kHighshelfFreq)->InitDouble("Freq", 500., 20., 20000., 0.01, "Hz");
	GetParam(kHighshelfGain)->InitDouble("Gain", 0., -12., 12., 0.01, "dB");
	OnReset();


#if IPLUG_EDITOR // http://bit.ly/2S64BDd
	mMakeGraphicsFunc = [&]() {
		return MakeGraphics(*this, PLUG_WIDTH, PLUG_HEIGHT, PLUG_FPS, GetScaleForScreen(PLUG_WIDTH, PLUG_HEIGHT));
	};

	mLayoutFunc = [&](IGraphics* pGraphics) {
		pGraphics->AttachCornerResizer(EUIResizerMode::Scale, false);
		pGraphics->AttachPanelBackground(COLOR_GRAY);
		pGraphics->LoadFont("Roboto-Regular", ROBOTO_FN);
		const IRECT b = pGraphics->GetBounds();
		pGraphics->AttachControl(new ITextControl(b.GetMidVPadded(50), "Hello iPlug 2!", IText(50)));
		pGraphics->AttachControl(new SR::Graphics::Controls::Knob(b.GetGridCell(0, 0, 1, 2), kHighshelfFreq, "Freq"));
		pGraphics->AttachControl(new SR::Graphics::Controls::Knob(b.GetGridCell(0, 1, 1, 2), kHighshelfGain, "Gain"));
	};
#endif
}

#if IPLUG_DSP
void SRChannel::ProcessBlock(sample** inputs, sample** outputs, int nFrames)
{
	const int nChans = NOutChansConnected();

	for (int s = 0; s < nFrames; s++) {
		for (int c = 0; c < nChans; c++)
		{
			outputs[c][s] = inputs[c][s];
		}
		outputs[0][s] = fHighshelfLeft.filter(outputs[0][s]);
		outputs[1][s] = fHighshelfRight.filter(outputs[1][s]);
	}
}

void SRChannel::OnParamChange(int paramIdx)
{
	switch (paramIdx)
	{
	default:
		OnReset();
		break;
	}
}

void SRChannel::OnReset()
{
	fHighshelfLeft.reset();
	fHighshelfRight.reset();
	fHighshelfLeft.setup(GetSampleRate(), GetParam(kHighshelfFreq)->Value(), GetParam(kHighshelfGain)->Value());
	fHighshelfRight.setup(GetSampleRate(), GetParam(kHighshelfFreq)->Value(), GetParam(kHighshelfGain)->Value());
}


#endif
