#include "DistrhoPlugin.hpp"
#include "DistrhoPluginInfo.h"
#include <cstdint>
#include "../../../Classes/DSP/SRGain.h"

START_NAMESPACE_DISTRHO


class AutoLeveler : public Plugin {
public:
  AutoLeveler() : Plugin(kParametersCount, 0, 0)
  , mThreshPeak(1.f)
  , mCurrentGainReduction(1.f)
  , fGainProcessor(1000, SR::DSP::SRGain::kSinusodial, true)
  , fPreGainProcessor(100, SR::DSP::SRGain::kSinusodial, true)
  {
    fGainProcessor.Reset(1.0, 0.5, 1.0, false, 1000, SR::DSP::SRGain::kSinusodial, true);
    fPreGainProcessor.Reset(1.0, 0.5, 1.0, false, 100, SR::DSP::SRGain::kSinusodial, true);
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
    case kPreGain:
      parameter.name = "PreGain";
      parameter.symbol = "pregain";
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
      return SR::Utils::AmpToDB(mThreshPeak);
    case kPreGain:
      return fPreGainProcessor.GetGainDb();
    case kPan:
      return fPreGainProcessor.GetPanPosition();
    default:
      return 0.0;
    }
  }

  void setParameterValue(uint32_t index, float value) override {
    switch (index) {
    case kThreshPeak:
      mCurrentGainReduction=1.f;
      mThreshPeak = SR::Utils::DBToAmp(value);
      calcGain();
      break;
    case kPreGain:
      fPreGainProcessor.SetGainDb(value);
      break;
    case kPan:
      fPreGainProcessor.SetPanPosition(value);
      calcGain();
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

    float currentpeak = mThreshPeak;
    
    for (uint32_t i = 0; i < frames; i++) {
      // Create temp double values for processing
      double left = in1[i];
      double right = in2[i];

      // process gain processor
      fPreGainProcessor.Process(left, right);
      fGainProcessor.Process(left, right);

      // get processed values back to floating output pointer
      if (abs(left) > currentpeak) {
        currentpeak = abs(left);
      }
      if (abs(right) > currentpeak) {
        currentpeak = abs(right);
      }
      out1[i] = left;
      out2[i] = right;
    }
    if (currentpeak > mThreshPeak) {
      mCurrentGainReduction -= (currentpeak - mThreshPeak);
      calcGain();
    }
  }

private:
  float mThreshPeak, mCurrentGainReduction;
  SR::DSP::SRGain fGainProcessor;
  SR::DSP::SRGain fPreGainProcessor;

  void calcGain() {
    if (mCurrentGainReduction > 1.f) {
      mCurrentGainReduction = 1.f;
    }
    fGainProcessor.SetGainLin(mCurrentGainReduction);
  }

  DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AutoLeveler);
};

Plugin *createPlugin() { return new AutoLeveler(); }

END_NAMESPACE_DISTRHO