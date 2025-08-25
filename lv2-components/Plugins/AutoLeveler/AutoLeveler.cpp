#include "DistrhoPlugin.hpp"
#include "DistrhoPluginInfo.h"
#include <cstdint>
#include "../../../Classes/DSP/SRGain.h"
#include "../../../Classes/DSP/SRDynamics.h"

START_NAMESPACE_DISTRHO


class AutoLeveler : public Plugin {
public:
  AutoLeveler() : Plugin(kParametersCount, 0, 0)
  , mListen(true)
  , mThreshPeakDb(0.f)
  , mThreshRMSDb(-12.f)
  , mCurrentGainReductionDb(0.f)
  , mEnvelopeRMS(0.f)
  , fGainProcessor(10000, SR::DSP::SRGain::kSinusodial, true)
  , fPreGainProcessor(1000, SR::DSP::SRGain::kSinusodial, true)
  , fEnvelopeRMS(300., 44100)
  {
    fGainProcessor.Reset(1.0, 0.5, 1.0, false, 10000, SR::DSP::SRGain::kSinusodial, true);
    fPreGainProcessor.Reset(1.0, 0.5, 1.0, false, 1000, SR::DSP::SRGain::kSinusodial, true);
    fEnvelopeRMS.SetSampleRate(getSampleRate());
    fEnvelopeRMS.SetTc(300.);
  }

protected:
  const char *getLabel() const override { return "AutoLeveler"; }
  const char *getDescription() const override { return "Automatically level down audio signals according to peak and rms threshold. Use on chains or master chain."; }
  const char *getMaker() const override { return "SRPlugins"; }
  const char *getLicense() const override { return "GPL"; }
  uint32_t getVersion() const override { return d_version(0, 0, 2); }
  int64_t getUniqueId() const override { return d_cconst('S', 'R', 'A', 'L'); }

  // Control groups
  enum {
    kPortGroupLeveler = 0,
    kPortGroupPre
  };

  void initParameter(uint32_t index, Parameter &parameter) override {
    switch (index) {
    case kThreshPeak:
      parameter.name = "Threshold Peak";
      parameter.symbol = "threshold_peak";
      parameter.unit = "dB";
      parameter.groupId = kPortGroupLeveler;
      parameter.ranges.def = 0.f;
      parameter.ranges.min = -60.f;
      parameter.ranges.max = 0.f;
      break;
    case kThreshRMS:
      parameter.name = "Threshold RMS";
      parameter.symbol = "threshold_rms";
      parameter.unit = "dB";
      parameter.groupId = kPortGroupLeveler;
      parameter.ranges.def = -12.f;
      parameter.ranges.min = -60.f;
      parameter.ranges.max = 0.f;
      break;
    case kListen:
      parameter.name = "Listen";
      parameter.symbol = "listen";
      parameter.unit = "";
      parameter.groupId = kPortGroupLeveler;
      parameter.ranges.def = 1.f;
      parameter.ranges.min = 0.f;
      parameter.ranges.max = 1.f;
      parameter.hints = kParameterIsBoolean;
      break;
    case kPreGain:
      parameter.name = "PreGain";
      parameter.symbol = "pregain";
      parameter.unit = "dB";
      parameter.groupId = kPortGroupPre;
      parameter.ranges.def = 0.f;
      parameter.ranges.min = -60.f;
      parameter.ranges.max = 12.f;
      break;
    case kPan:
      parameter.name = "Pan";
      parameter.symbol = "pan";
      parameter.unit = "%";
      parameter.groupId = kPortGroupPre;
      parameter.ranges.def = 0.f;
      parameter.ranges.min = -100.f;
      parameter.ranges.max = 100.f;
      break;
    default:
      break;
    }
  }

    void initPortGroup(uint32_t groupId, PortGroup& portGroup) override
    {
        switch (groupId) {
        case kPortGroupLeveler:
            portGroup.name = "Leveller";
            portGroup.symbol = "leveller";
            break;
        case kPortGroupPre:
            portGroup.name = "Pre";
            portGroup.symbol = "pre";
            break;
        }
    }


  float getParameterValue(uint32_t index) const override {
    switch (index) {
    case kThreshPeak:
      return mThreshPeakDb;
    case kThreshRMS:
      return mThreshRMSDb;
    case kListen:
      return mListen ? 1.f : 0.f;
    case kPreGain:
      return fPreGainProcessor.GetGainDb();
    case kPan:
      return fPreGainProcessor.GetPanPosition() * 200.f - 100.f;
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
    case kListen:
      if (value > 0.5f) {
        mCurrentGainReductionDb=0.f;
        mListen = true;
      } else {
        mListen = false;
      }
      break;
    case kPreGain:
      fPreGainProcessor.SetGainDb(value);
      break;
    case kPan:
      fPreGainProcessor.SetPanPosition((value + 100.f) / 200.f);
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
      if (mListen) {
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
      }

      // process gain processor
      fGainProcessor.Process(left, right);
      
      // get processed values back to floating output buffer
      out1[i] = left;
      out2[i] = right;
    }
    // Convert max values to log scale
    double currentMaxPeakDb = SR::Utils::AmpToDB(currentMaxPeak);
    double currentMaxRMSDb = SR::Utils::AmpToDB(currentMaxRMS);

    if (mListen) {
      // Get current peak and rms overshoot in log scale and adjust gain reduction
      if (currentMaxPeakDb > mThreshPeakDb || currentMaxRMSDb > mThreshRMSDb) {
        // calculate gain reduction in dB, chose if peak or rms detector is lower, and never raise current gain reduction
        mCurrentGainReductionDb = std::min(std::min((0.f - (currentMaxPeakDb - mThreshPeakDb)), (0.f - (currentMaxRMSDb - mThreshRMSDb))), mCurrentGainReductionDb);
        // It's a down leveler, we don't want to boost the gain
        if (mCurrentGainReductionDb > 0.f) {
          mCurrentGainReductionDb = 0.f;
        }
        fGainProcessor.SetGainDb(mCurrentGainReductionDb);      }
    }
  }

private:
  bool mListen;
  float mThreshPeakDb, mThreshRMSDb;
  double mEnvelopeRMS,mCurrentGainReductionDb;
  SR::DSP::SRDynamicsEnvelope fEnvelopeRMS;
  SR::DSP::SRGain fGainProcessor;
  SR::DSP::SRGain fPreGainProcessor;

  DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AutoLeveler);
};

Plugin *createPlugin() { return new AutoLeveler(); }

END_NAMESPACE_DISTRHO