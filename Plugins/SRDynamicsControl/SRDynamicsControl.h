#pragma once

#include "IPlug_include_in_plug_hdr.h"
#include "../../Classes/DSP/SRGain.h"

const int kNumPresets = 1;

enum EParams
{
  kGain = 0,
  kThreshLevel,
  kThreshTrans,
  kNumParams
};

using namespace iplug;
using namespace igraphics;

class SRDynamicsControl final : public Plugin
{
public:
  SRDynamicsControl(const InstanceInfo& info);

#if IPLUG_DSP // http://bit.ly/2S64BDd
  void ProcessBlock(sample** inputs, sample** outputs, int nFrames) override;
  //void OnReset() override;
  void OnParamChange(int paramIdx) override;
#endif
private:
	SR::DSP::SRGain fOutGain;
};
