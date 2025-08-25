#include "DistrhoPlugin.hpp"
#include "DistrhoPluginInfo.h"
#include <cstdint>
#include "../../../Classes/DSP/SRGain.h"
#include "../../../Classes/DSP/SRDynamics.h"

START_NAMESPACE_DISTRHO


class AutoLeveler : public Plugin {
public:
  AutoLeveler() : Plugin(kParametersCount, 0, 0)
  , mThreshPeakDb(0.f)
  , mThreshRMSDb(-12.f)
  , mCurrentGainReductionDb(0.f)
  , mEnvelopeRMS(0.f)
  , fGainProcessor(1000, SR::DSP::SRGain::kSinusodial, true)
  , fPreGainProcessor(100, SR::DSP::SRGain::kSinusodial, true)
  , fEnvelopeRMS(300., getSampleRate())
  {
    fGainProcessor.Reset(1.0, 0.5, 1.0, false, 1000, SR::DSP::SRGain::kSinusodial, true);
    fPreGainProcessor.Reset(1.0, 0.5, 1.0, false, 100, SR::DSP::SRGain::kSinusodial, true);
    fEnvelopeRMS.SetSampleRate(getSampleRate());
    fEnvelopeRMS.SetTc(300.);
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
      parameter.name = "Threshold Peak";
      parameter.symbol = "threshold_peak";
      parameter.ranges.def = 0.f;
      parameter.ranges.min = -60.f;
      parameter.ranges.max = 0.f;
      break;
    case kThreshRMS:
      parameter.name = "Threshold RMS";
      parameter.symbol = "threshold_rms";
      parameter.ranges.def = -12.f;
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
      return mThreshPeakDb;
    case kThreshRMS:
      return mThreshRMSDb;
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
      // Reset gain reduction when threshold changes
      mCurrentGainReductionDb=0.f;
      mThreshPeakDb = value;
      // calcGain();
      break;
    case kThreshRMS:
      // Reset gain reduction when threshold changes
      mCurrentGainReductionDb=0.f;
      mThreshRMSDb = value;
      // calcGain();
      break;
    case kPreGain:
      fPreGainProcessor.SetGainDb(value);
      break;
    case kPan:
      fPreGainProcessor.SetPanPosition(value);
      // calcGain();
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

    // Reset current peak for this buffer
    double currentMaxPeak = 0.0;
    double currentMaxRMS = 0.0;

    for (uint32_t i = 0; i < frames; i++) {
      // Create temp double values for processing
      double left = in1[i];
      double right = in2[i];

      // process pregain processor
      fPreGainProcessor.Process(left, right);

      // Get current linear max peak per buffer
      if (abs(left) > currentMaxPeak) {
        currentMaxPeak = abs(left);
      }
      if (abs(right) > currentMaxPeak) {
        currentMaxPeak = abs(right);
      }

      fEnvelopeRMS.Process(0.5 * (abs(left) + abs(right)), mEnvelopeRMS);

      // Get current RMS value
      if (mEnvelopeRMS > currentMaxRMS) {
        currentMaxRMS = mEnvelopeRMS;
      }

      // process gain processor
      fGainProcessor.Process(left, right);
      
      // get processed values back to floating output buffer
      out1[i] = left;
      out2[i] = right;
    }
    // Convert peak value to log scale
    double currentMaxPeakDb = SR::Utils::AmpToDB(currentMaxPeak);
    double currentMaxRMSDb = SR::Utils::AmpToDB(currentMaxRMS);

    // Get current peak overshoot in log scale and adjust gain reduction
    if (currentMaxPeakDb > mThreshPeakDb || currentMaxRMSDb > mThreshRMSDb) {
      // calculate gain reduction in dB, but don't raise it
      mCurrentGainReductionDb = std::min(std::min((0.f - (currentMaxPeakDb - mThreshPeakDb)), (0.f - (currentMaxRMSDb - mThreshRMSDb))), mCurrentGainReductionDb);
      calcGain();
    }
  }

private:
  float mThreshPeakDb, mThreshRMSDb;
  double mEnvelopeRMS,mCurrentGainReductionDb;
  SR::DSP::SRDynamicsEnvelope fEnvelopeRMS;
  SR::DSP::SRGain fGainProcessor;
  SR::DSP::SRGain fPreGainProcessor;

  void calcGain() {
    // It's a down leveler, we don't want to boost the gain
    if (mCurrentGainReductionDb > 0.f) {
      mCurrentGainReductionDb = 0.f;
    }
    fGainProcessor.SetGainDb(mCurrentGainReductionDb);
  }

  DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AutoLeveler);
};

Plugin *createPlugin() { return new AutoLeveler(); }

END_NAMESPACE_DISTRHO