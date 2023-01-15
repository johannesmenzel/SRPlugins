#include "SRChannel.h"
#include "IPlug_include_in_plug_src.h"
#include "IControls.h"
#include "../../Classes/Graphics/SRCustomKnob.h"
#include "../../Classes/Graphics/SRCustomLayout.h"

SRChannel::SRChannel(const InstanceInfo& info)
: Plugin(info, MakeConfig(kNumParams, kNumPresets))
{
    GetParam(kInputGain)->InitDouble("Input", 0., -72., 12., 0.01, "dB", 0, "", IParam::ShapePowCurve(SR::Utils::SetShapeCentered(-72., 12., 0., .5)));
    GetParam(kOutputGain)->InitDouble("Output", 0., -72., 12., 0.01, "dB", 0, "", IParam::ShapePowCurve(SR::Utils::SetShapeCentered(-72., 12., 0., .5)));

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
    pGraphics->AttachControl(new SR::Graphics::Controls::Knob(rectControls.GetGridCell(3, 4, 5, 5).GetCentredInside(100.f), kInputGain, "Input", SR::Graphics::Layout::SR_DEFAULT_STYLE, true, false, -150.f, 150.f, -150.f, EDirection::Vertical, 4., 1.f), cInputGain);
    pGraphics->AttachControl(new SR::Graphics::Controls::Knob(rectControls.GetGridCell(4, 4, 5, 5).GetCentredInside(100.f), kOutputGain, "Output", SR::Graphics::Layout::SR_DEFAULT_STYLE, true, false, -150.f, 150.f, -150.f, EDirection::Vertical, 4., 1.f), cOutputGain);


  };
#endif
}

#if IPLUG_DSP
void SRChannel::ProcessBlock(sample** inputs, sample** outputs, int nFrames)
{
  const double gain = GetParam(kInputGain)->Value() / 100.;
  const int nChans = NOutChansConnected();
  
  for (int s = 0; s < nFrames; s++) {
    for (int c = 0; c < nChans; c++) {
      outputs[c][s] = inputs[c][s] * gain;
    }
  }
}
void SRChannel::OnReset()
{
}
void SRChannel::OnIdle()
{
}
void SRChannel::OnParamChange(int paramIdx)
{
}
#endif
