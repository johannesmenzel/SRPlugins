#include "SRDynamicsControl.h"
#include "IPlug_include_in_plug_src.h"
#include "IControls.h"
#include "../../Classes/Graphics/SRCustomKnob.h"

SRDynamicsControl::SRDynamicsControl(const InstanceInfo& info)
: Plugin(info, MakeConfig(kNumParams, kNumPresets))
, fOutGain(100)
{
  GetParam(kGain)->InitDouble("Gain", 0., -12., 12.0, 0.01, "dB");
  GetParam(kThreshLevel)->InitDouble("Thresh Level", 0., -30., 0., 0.01, "dB");
  GetParam(kThreshTrans)->InitDouble("Thresh Trans", 0., -30., 0.0, 0.01, "dB");

#if IPLUG_EDITOR // http://bit.ly/2S64BDd
  mMakeGraphicsFunc = [&]() {
    return MakeGraphics(*this, PLUG_WIDTH, PLUG_HEIGHT, PLUG_FPS, GetScaleForScreen(PLUG_WIDTH, PLUG_HEIGHT));
  };
  
  mLayoutFunc = [&](IGraphics* pGraphics) {
    pGraphics->AttachCornerResizer(EUIResizerMode::Scale, false);
    pGraphics->AttachPanelBackground(COLOR_GRAY);
    pGraphics->LoadFont("Roboto-Regular", ROBOTO_FN);
    const IRECT b = pGraphics->GetBounds();
    //pGraphics->AttachControl(new ITextControl(b.GetMidVPadded(50), "SRDynamicsControl", IText(50)));
    //pGraphics->AttachControl(new IVKnobControl(b.GetCentredInside(100).GetVShifted(-100), kGain));
    pGraphics->AttachControl(new Knob(b.GetGridCell(1, 0, 2, 3), kGain));
    pGraphics->AttachControl(new Knob(b.GetGridCell(1, 1, 2, 3), kThreshLevel));
    pGraphics->AttachControl(new Knob(b.GetGridCell(1, 2, 2, 3), kThreshTrans));
  };
#endif
}

#if IPLUG_DSP
void SRDynamicsControl::ProcessBlock(sample** inputs, sample** outputs, int nFrames)
{
  const double gain = DBToAmp(GetParam(kGain)->Value());
  const int nChans = NOutChansConnected();
  
  for (int s = 0; s < nFrames; s++) {
    for (int c = 0; c < nChans; c++) {
      outputs[c][s] = inputs[c][s];
      fOutGain.Process(outputs[c][s], outputs[c][s]);
    }
  }
}

//void SRDynamicsControl::OnReset() {
//    fOutGain.InitGain(100);
//}
void SRDynamicsControl::OnParamChange(int paramIdx)
{
    switch (paramIdx)
    {
    case kGain: fOutGain.SetGain(DBToAmp(GetParam(paramIdx)->Value())); break;
    }
}
#endif
