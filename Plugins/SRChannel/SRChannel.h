#pragma once

#include "IPlug_include_in_plug_hdr.h"
#include "../../Dependencies/iir1/Iir.h"

const int kNumPresets = 1;

enum EParams
{
  kHighshelfFreq = 0,
  kHighshelfGain,
  kNumParams
};

using namespace iplug;
using namespace igraphics;

class SRChannel final : public Plugin
{
public:
  SRChannel(const InstanceInfo& info);

#if IPLUG_DSP // http://bit.ly/2S64BDd
  void ProcessBlock(sample** inputs, sample** outputs, int nFrames) override;
  void OnParamChange(int paramIdx) override;
  void OnReset() override;
private:
	Iir::Butterworth::HighShelf<4> fHighshelfLeft;
	Iir::Butterworth::HighShelf<4> fHighshelfRight;
	
#endif
};
