#include "DistrhoPlugin.hpp"
#include "DistrhoPluginInfo.h"
#include <cstdint>
#include "../../../Classes/DSP/SRGain.h"

START_NAMESPACE_DISTRHO


class AutoLeveler : public Plugin {
public:
  AutoLeveler() : Plugin(kParametersCount, 0, 0)
  , mThreshPeak(0.0)
  , fGainProcessor(100, SR::DSP::SRGain::kSinusodial, true) {
    fGainProcessor.Reset(1.0, 0.5, 1.0, false, 100, SR::DSP::SRGain::kSinusodial, true);
  }

protected:
  const char *getLabel() const override { return "AutoLeveler"; }
  const char *getDescription() const override { return "Auto Leveler for Zynthian Chains"; }
  const char *getMaker() const override { return "SRPlugins"; }
  const char *getLicense() const override { return "GPL"; }
  uint32_t getVersion() const override { return d_version(0, 0, 1); }
  int64_t getUniqueId() const override { return d_cconst('S', 'R', 'A', 'L'); }

  void initParameter(uint32_t index, Parameter &parameter) override {
    switch (index) {
    case kThreshPeak:
      parameter.name = "Threshold";
      parameter.symbol = "threshold";
      parameter.ranges.def = 0.f;
      parameter.ranges.min = -60.f;
      parameter.ranges.max = 0.f;
      break;
    case kGain:
      parameter.name = "Gain";
      parameter.symbol = "gain";
      parameter.ranges.def = 0.f;
      parameter.ranges.min = -60.f;
      parameter.ranges.max = 12.f;
      break;
    case kPan:
      parameter.name = "Pan";
      parameter.symbol = "pan";
      parameter.ranges.def = 0.5f;
      parameter.ranges.min = 0.f;
      parameter.ranges.max = 1.f;
      break;
    default:
      break;
    }
  }

  float getParameterValue(uint32_t index) const override {
    switch (index) {
    case kThreshPeak:
      return mThreshPeak;
    case kGain:
      return fGainProcessor.GetGainDb();
    case kPan:
      return fGainProcessor.GetPanPosition();
    default:
      return 0.0;
    }
  }

  void setParameterValue(uint32_t index, float value) override {
    switch (index) {
    case kThreshPeak:
      mThreshPeak = value;
      break;
    case kGain:
      fGainProcessor.SetGainDb(value);
      break;
    case kPan:
      fGainProcessor.SetPanPosition(value);
      break;
    default:
      break;
    }
  }
  
  void run(const float **inputs, float **outputs, uint32_t frames) override {
    const float *const in1 = inputs[0];
    const float *const in2 = inputs[1];
    float *const out1 = outputs[0];
    float *const out2 = outputs[1];
    
    for (uint32_t i = 0; i < frames; i++) {
      double left = in1[i];
      double right = in2[i];
      fGainProcessor.Process(left, right);
      out1[i] = left;
      out2[i] = right;
    }
  }

private:
  float mThreshPeak;
  SR::DSP::SRGain fGainProcessor;

  DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AutoLeveler);
};

Plugin *createPlugin() { return new AutoLeveler(); }

END_NAMESPACE_DISTRHO