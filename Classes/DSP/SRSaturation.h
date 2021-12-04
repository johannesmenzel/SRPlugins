//  SRSaturation.h

// Implementation:
//   Note that you might need a class for each channel
//   (so 2 for stereo processing.)
//
// Header:
//   private:
//     Impementation as object:
//       SRSaturation name;
//     Implementation as pointer:
//       SRSaturation *name = new SRSaturation();
//
// Class: Constructor, Reset()
//     Implementation as object:
//       name.setClass(pType, pVar1, pVar2, pVar3); 
//     Implementation as pointer:
//       name->setClass(pType, pVar1, pVar2, pVar3);
//
// Class: ProcessDoubleReplacing()
//   Per sample and channel:
//     Implementation as object:
//       *out1 = name.process(*in1);
//     Implementation as pointer:
//       *out1 = name->process(*in1);

#pragma once

#include "../Utils/SRHelpers.h"
#include <functional>
//#include "Oversampler.h"
// #include this and that


namespace SR {
  namespace DSP {



    class SRSaturation {
    public:
      // If type definitions of type int needed:
      enum ESaturationType {
        kMusicDSP = 0,
        kZoelzer,
        kPirkle,
        kPirkleMod,
        kSoftSat,
        kRectHalf,
        kRectFull,
        kNumTypes
        // ...
      };

      SRSaturation()
        : mType(kMusicDSP)
        , mDriveNormalized(1.0)
        , mAmountNormalized(0.0)
        , mHarmonicsNormalized(1.0)
        , mPositive(true)
        , mSkewNormalized(0.0)
        , mWetNormalized(1.0)
        , mSamplerate(44100.0)
        , mAmount(0.)
      {
      }

      SRSaturation(
        SRSaturation::ESaturationType pType,
        double pDriveDb,
        double pAmountNormalized,
        double pHarmonicsNormalized,
        bool pPositiveSide,
        double pSkewNormalized,
        double pWet,
        double pSamplerate
      )
        : prev(0.0)
        , dry(0.0)
      {
        SetSaturation(pType, pDriveDb, pAmountNormalized, pHarmonicsNormalized, pPositiveSide, pSkewNormalized, pWet, pSamplerate);
      }

      ~SRSaturation() {}

      void SetType(SRSaturation::ESaturationType pType) { mType = pType; calcSaturation(); }
      void SetDrive(double pDriveDb) { mDriveNormalized = SR::Utils::DBToAmp(pDriveDb); }
      void SetAmount(double pAmountNormalized) { mAmountNormalized = pAmountNormalized; calcSaturation(); }
      void SetHarmonics(double pHarmonicsNormalized) { mHarmonicsNormalized = pHarmonicsNormalized; }
      void SetPositive(bool pPositive) { mPositive = pPositive; }
      void SetSkew(double pSkewNormalized) { mSkewNormalized = pSkewNormalized; calcSaturation(); }
      void SetWet(double pWetNormalized) { mWetNormalized = pWetNormalized; }
      void SetSamplerate(double pSamplerate) { mSamplerate = pSamplerate; }
      void SetSaturation(
        SRSaturation::ESaturationType pType,
        double pDriveDb,
        double pAmountNormalized,
        double pHarmonicsNormalized,
        bool pPositive,
        double pSkewNormalized,
        double pWetNormalized,
        double pSamplerate
      )
      {
        mType = pType;
        mDriveNormalized = SR::Utils::DBToAmp(pDriveDb);
        mAmountNormalized = pAmountNormalized;
        mHarmonicsNormalized = pHarmonicsNormalized;
        mPositive = pPositive;
        mSkewNormalized = pSkewNormalized;
        mWetNormalized = pWetNormalized;
        mSamplerate = pSamplerate;
        calcSaturation();
      }

      double Process(double in);



    protected:
      double processMusicDSP(double in);
      double processZoelzer(double in);
      double processPirkle(double in);
      double processPirkleModified(double in);
      double processSoftSat(double in);
      double processRectHalf(double in);
      double processRectFull(double in);
      void calcSaturation() {
        switch (mType) {
        case kMusicDSP:
          mAmount = (1. - mAmountNormalized);
          break;
          //case kZoelzer:
          //  break;
        case kPirkle:
        case kPirkleMod:
          mAmount = pow(mAmountNormalized, 3.);
          break;
        case kSoftSat:
          mAmount = 1. / mAmountNormalized;
          break;
          //case kRectHalf:
          //  break;
          //case kRectFull:
          //  break;
        default:
          mAmount = mAmountNormalized;
          break;
        }
        return;
      }

      SRSaturation::ESaturationType mType;
      double mDriveNormalized;
      double mAmountNormalized;
      double mAmount;
      double mHarmonicsNormalized;
      bool mPositive; // if aiming for even harmonics, the positive side of the envelope will be affected if true, otherwise the negative side
      double mSkewNormalized;
      double mWetNormalized;
      double mSamplerate;

      double prev;
      double dry;
    };
    // end of class



    // INLINE PROCESSING FUNCTIONS
    // ----------------------------------------------------------------------------

    inline double SRSaturation::Process(double in) {
      // Don't process if amount is zero, also preventing dividing zeros
      if (mAmountNormalized == 0.0)
        return in;

      // apply drive
      in *= mDriveNormalized;

      // create driven dry samples
      dry = in;

      // call specific inline functions
      switch (mType) {
      case kMusicDSP: in = processMusicDSP(in); break;
      case kZoelzer: in = processZoelzer(in); break;
      case kPirkle: in = processPirkle(in); break;
      case kPirkleMod: in = processPirkleModified(in); break;
      case kSoftSat: in = processSoftSat(in); break;
      case kRectHalf: in = processRectHalf(in); break;
      case kRectFull: in = processRectFull(in); break;
      default: break;
      }

      prev = dry;

      if (!mPositive && in < 0.) in = in * mHarmonicsNormalized + dry * (1. - mHarmonicsNormalized);
      if (mPositive && in > 0.) in = in * mHarmonicsNormalized + dry * (1. - mHarmonicsNormalized);

      // Apply Dry/Wet
      if (mWetNormalized < 1.0)
        in = mWetNormalized * in + (1. - mWetNormalized) * dry;

      // return to old drive level
      in *= (1. / mDriveNormalized);

      return in;
    }

    inline double SRSaturation::processMusicDSP(double in) {
      if (fabs(in) > mAmount) {
        in = (in > 0.)
          ? (mAmount + (fabs(in) - mAmount) / (1. + pow((fabs(in) - mAmount) / (1. - mAmount), 2.))) * (in / fabs(in))
          : (mAmount + (fabs(in) - mAmount) / (1. + pow((fabs(in) - mAmount) / (1. - mAmount), 2.))) * (in / fabs(in));
      }

      // Soften by (1 - Amount)
      in = (1. - mAmount) * in + mAmount * dry;

      // Saturation Normalization
      in *= (1. / ((mAmount + 1.) / 2.));

      return in;
    }

    inline double SRSaturation::processZoelzer(double in) {
      in = (in > 0.)
        ? 1. - exp(-in)
        : -1. + exp(in);
      return in;
    }

    inline double SRSaturation::processPirkle(double in) {
      in = tanh(mAmount * in) / tanh(mAmount);
      return in;
    }

    inline double SRSaturation::processPirkleModified(double in) {
      const double a = mAmount * (1. + (in - prev) * (1. / mDriveNormalized) * mSkewNormalized);
      in = tanh(a * in) / tanh(a);
      return in;
    }

    inline double SRSaturation::processSoftSat(double in) {
      in = (in > 1.)
        ? mAmount * (1. - mAmount / (in + (mAmount - 1.))) + 1.
        : (in < -1.)
        ? -mAmount * (1. + mAmount / (in - (mAmount - 1.))) - 1.
        : in;
      return in;
    }

    inline double SRSaturation::processRectHalf(double in) {
      in = (in < 0.) ? 0. : in;
      return in;
    }

    inline double SRSaturation::processRectFull(double in) {
      in = fabs(in);
      return in;
    }
  }
}