//  Modifications on class:
//  Copyright Johannes Menzel 2019 MIT
//
//  This is an ever growing modified filter class, see the original licences below.
//
//  SVF: 
//
//  Based on IPlug2's SVF filter
//
//  Two Pole:
//
//  Created by Nigel Redmon on 11/24/12
//  EarLevel Engineering: earlevel.com
//  Copyright 2012 Nigel Redmon
//
//  For a complete explanation of the Biquad code:
//  http://www.earlevel.com/main/2012/11/25/biquad-c-source-code/
//
//  License:
//
//  This source code is provided as is, without warranty.
//  You may copy and distribute verbatim copies of this document.
//  You may modify and use this source code to create binary code
//  for your own purposes, free or commercial.
//



#pragma once
#define _USE_MATH_DEFINES
#include <math.h>
#include <cmath>

namespace SR {
  namespace DSP {

#define SVFMODES_VALIST "LowPass", "HighPass", "BandPass", "Notch", "Peak", "Bell", "LowPassShelf", "HighPassShelf"

    template<typename T = double, int MAXNUMCHANNELS = 1> class SVF {
    public:

      enum EFilterType {
        kLowPass = 0,
        kHighPass,
        kBandPass,
        kNotch,
        kPeak,
        kBell,
        kLowPassShelf,
        kHighPassShelf,
        kNumModes
      };

      SVF(EFilterType mode = kLowPass, double freqCPS = 1000.)
      {
        mNewState.mode = mState.mode = mode;
        mNewState.freq = mState.freq = freqCPS;
        Update();
      }

      void SetFreqCPS(double freqCPS) { mNewState.freq = Clip(freqCPS, 10, 20000.); }
      void SetQ(double q) { mNewState.q = Clip(Q, 0.1, 100.); }
      void SetGain(double gainDB) { mNewState.gain = Clip(gainDB, -36, 36.); }
      void SetMode(EFilterType mode) { mNewState.mode = mode; }
      void SetSampleRate(double sampleRate) { mNewState.sampleRate = sampleRate; }
      // SVF process block
      void ProcessBlock(T** inputs, T** outputs, int nChans, int nFrames)
      {
        assert(nChans <= MAXNUMCHANNELS);

        if (mState != mNewState)
          Update();

        for (auto c = 0; c < nChans; c++)
        {
          for (auto s = 0; s < nFrames; s++)
          {
            const double in = (double)inputs[c][s];

            mV3[c] = in - mIc2eq[c];
            mV1[c] = m_a1 * mIc1eq[c] + m_a2 * mV3[c];
            mV2[c] = mIc2eq[c] + m_a2 * mIc1eq[c] + m_a3 * mV3[c];
            mIc1eq[c] = 2. * mV1[c] - mIc1eq[c];
            mIc2eq[c] = 2. * mV2[c] - mIc2eq[c];

            outputs[c][s] = (T)m_m0 * in + m_m1 * mV1[c] + m_m2 * mV2[c];
          }
        }
      }
      void Reset()
      {
        for (auto c = 0; c < MAXNUMCHANNELS; c++)
        {
          mV1[c] = 0.;
          mV2[c] = 0.;
          mV3[c] = 0.;
          mIc1eq[c] = 0.;
          mIc2eq[c] = 0.;
        }
      }

    private:
      void Update()
      {
        mState = mNewState;

        const double w = std::tan(PI * mState.freq / mState.sampleRate);

        switch (mState.mode)
        {
        case kLowPass:
        {
          const double g = w;
          const double k = 1. / mState.q;
          m_a1 = 1. / (1. + g * (g + k));
          m_a2 = g * m_a1;
          m_a3 = g * m_a2;
          m_m0 = 0.;
          m_m1 = 0.;
          m_m2 = 1.;
          break;
        }
        case kHighPass:
        {
          const double g = w;
          const double k = 1. / mState.q;
          m_a1 = 1. / (1. + g * (g + k));
          m_a2 = g * m_a1;
          m_a3 = g * m_a2;
          m_m0 = 1.;
          m_m1 = -k;
          m_m2 = -1.;
          break;
        }
        case kBandPass:
        {
          const double g = w;
          const double k = 1. / mState.q;
          m_a1 = 1. / (1. + g * (g + k));
          m_a2 = g * m_a1;
          m_a3 = g * m_a2;
          m_m0 = 0.;
          m_m1 = 1.;
          m_m2 = 0.;
          break;
        }
        case kNotch:
        {
          const double g = w;
          const double k = 1. / mState.q;
          m_a1 = 1. / (1. + g * (g + k));
          m_a2 = g * m_a1;
          m_a3 = g * m_a2;
          m_m0 = 1.;
          m_m1 = -k;
          m_m2 = 0.;
          break;
        }
        case kPeak:
        {
          const double g = w;
          const double k = 1. / mState.q;
          m_a1 = 1. / (1. + g * (g + k));
          m_a2 = g * m_a1;
          m_a3 = g * m_a2;
          m_m0 = 1.;
          m_m1 = -k;
          m_m2 = -2.;
          break;
        }
        case kBell:
        {
          const double A = std::pow(10., mState.gain / 40.);
          const double g = w;
          const double k = 1 / mState.q;
          m_a1 = 1. / (1. + g * (g + k));
          m_a2 = g * m_a1;
          m_a3 = g * m_a2;
          m_m0 = 1.;
          m_m1 = k * (A * A - 1.);
          m_m2 = 0.;
          break;
        }
        case kLowPassShelf:
        {
          const double A = std::pow(10., mState.gain / 40.);
          const double g = w / std::sqrt(A);
          const double k = 1. / mState.q;
          m_a1 = 1. / (1. + g * (g + k));
          m_a2 = g * m_a1;
          m_a3 = g * m_a2;
          m_m0 = 1.;
          m_m1 = k * (A - 1.);
          m_m2 = (A * A - 1.);
          break;
        }
        case kHighPassShelf:
        {
          const double A = std::pow(10., mState.gain / 40.);
          const double g = w / std::sqrt(A);
          const double k = 1. / mState.q;
          m_a1 = 1. / (1. + g * (g + k));
          m_a2 = g * m_a1;
          m_a3 = g * m_a2;
          m_m0 = A * A;
          m_m1 = k * (1. - A)*A;
          m_m2 = (1. - A * A);
          break;
        }
        default:
          break;
        }
      }

    private:
      double mV1[MAXNUMCHANNELS] = {};
      double mV2[MAXNUMCHANNELS] = {};
      double mV3[MAXNUMCHANNELS] = {};
      double mIc1eq[MAXNUMCHANNELS] = {};
      double mIc2eq[MAXNUMCHANNELS] = {};
      double m_a1 = 0.;
      double m_a2 = 0.;
      double m_a3 = 0.;
      double m_m0 = 0.;
      double m_m1 = 0.;
      double m_m2 = 0.;

      struct Settings
      {
        EFilterType mode;
        double freq = 1000.;
        double q = 0.1;
        double gain = 1.;
        double sampleRate = 44100.;

        bool operator != (const Settings &other) const
        {
          return !(mode == other.mode && freq == other.freq && q == other.q && gain == other.gain && sampleRate == other.sampleRate);
        }
      };

      Settings mState, mNewState;
    };




    // TWO POLE FILTERS
    template<typename T = double, int MAXNUMCHANNELS = 1>
    class SRFilterIIR {
    public:

      enum EFilterType {
        BiquadLowpass = 0,
        BiquadHighpass,
        BiquadBandpass,
        BiquadNotch,
        BiquadPeak,
        BiquadPeakNcq,
        BiquadLowshelf,
        BiquadHighshelf,
        BiquadLinkwitzHighpass,
        BiquadLinkwitzLowpass,
        OnepoleHighpass,
        OnepoleLowpass
      };

      SRFilterIIR() {
        mType = EFilterType::BiquadLowpass;
        a0 = 1.0;
        a1 = a2 = b1 = b2 = 0.0;
        mFreq = 0.50;
        mQ = 0.70710678;
        mPeakGain = 0.0;
        for (int i = 0; i < MAXNUMCHANNELS; i++) {
          z1[i] = z2[i] = 0.0;
        }
        mSamplerate = 44100;
        mPoles = 2;
      }

      SRFilterIIR(EFilterType type, double frequency, double q, double peakGainDB, double samplerate) {
        SetFilter(type, frequency, q, peakGainDB, samplerate);
        for (int i = 0; i < MAXNUMCHANNELS; i++) {
          z1[i] = z2[i] = 0.0;
        }
      } // Two pole filters

      ~SRFilterIIR() {
      }

      // Setter methods
      void SetType(EFilterType type) {
        mType = type;
        mPoles = (type >= EFilterType::OnepoleHighpass) ? 1 : 2;
        Update();
      } // Set filter type

      void SetQ(double q) {
        mQ = q;
        Update();
      } // Set filter Q

      void SetFreq(double frequency) {
        mFreq = frequency;
        Update();
      } // Set filter center frequency

      void SetPeakGain(double peakGainDb) {
        mPeakGain = peakGainDb;
        Update();
      } // Set filter peak gain

      void SetFilter(EFilterType type, double frequency, double q, double peakGainDb, double samplerate) {
        mType = type;
        mQ = q;
        mFreq = frequency;
        mSamplerate = samplerate;
        mPeakGain = peakGainDb;
        mPoles = (type >= EFilterType::OnepoleHighpass) ? 1 : 2;
        Update();
      } // Set all filter members at once

      // Process methods
      // IIR processing
      T Process(T in, int channel) {
        assert(channel < MAXNUMCHANNELS);
        switch (mType)
        {
        case OnepoleHighpass:
          z1[channel] = in * a0 + z1[channel] * b1;
          in -= z1[channel];
          return in;
          break;
        case OnepoleLowpass:
          z1[channel] = in * a0 + z1[channel] * b1;
          return z1[channel];
          break;
        default: // for all two poles
          double out = in * a0 + z1[channel];
          z1[channel] = in * a1 + z2[channel] - b1 * out;
          z2[channel] = in * a2 - b2 * out;
          return out;
          break;
        }
      } // Process one sample

      void ProcessBlock(T** inputs, T** outputs, int nChans, int nFrames) {
        for (auto c = 0; c < nChans; c++)
        {
          for (auto s = 0; s < nFrames; s++)
          {
            Process(inputs[c][s], c);
          }
        }
      } // Process entire buffer

      void GetFrequencyResponse(double* values, int points, double rangeDb, bool returnPhase) {
        for (int i = 0; i < points; ++i) {
          values[i] = GetFrequencyResponse(0.5 * double(i) / double(points), rangeDb, returnPhase);
        }
      } // Get frequency response or phase at specified points

      double GetFrequencyResponse(double atNormalizedFrequency, double rangeDb, bool returnPhase) {
        assert(atNormalizedFrequency <= 0.5);
        double w = 2. * M_PI * atNormalizedFrequency;

        double cos1 = cos(-1. * w);
        double cos2 = cos(-2. * w);

        double sin1 = sin(-1. * w);
        double sin2 = sin(-2. * w);

        double realZeros = a0 + a1 * cos1 + a2 * cos2;
        double imagZeros = a1 * sin1 + a2 * sin2;

        double realPoles = 1. + b1 * cos1 + b2 * cos2;
        double imagPoles = b1 * sin1 + b2 * sin2;

        double divider = realPoles * realPoles + imagPoles * imagPoles;

        double realHw = (realZeros * realPoles + imagZeros * imagPoles) / divider;
        double imagHw = (imagZeros * realPoles - realZeros * imagPoles) / divider;

        if (returnPhase) {
          double phase = atan2(imagHw, realHw); // Not needed yet
          return phase;                   //phase
        }

        double magnitude = sqrt(realHw * realHw + imagHw * imagHw);   // magnitude linear
        magnitude = 20. * log10(magnitude);    // gain in dB
        magnitude = magnitude / rangeDb;    // GetNormal
                                            //magnitude = std::fmin(std::fmax(magnitude, -1.), 1.);

        return magnitude;
      } // Get frequency response or phase at specified frequency

    protected:
      void Update(void) {
        double norm, O, K, V;
        double L, E, B, G;
        switch (mType) {
        case BiquadLowpass:
          K = tan(M_PI * mFreq);
          norm = 1 / (1 + K / mQ + K * K);
          a0 = K * K * norm;
          a1 = 2 * a0;
          a2 = a0;
          b1 = 2 * (K * K - 1) * norm;
          b2 = (1 - K / mQ + K * K) * norm;
          break;

        case BiquadHighpass:
          K = tan(M_PI * mFreq);
          norm = 1 / (1 + K / mQ + K * K);
          a0 = 1 * norm;
          a1 = -2 * a0;
          a2 = a0;
          b1 = 2 * (K * K - 1) * norm;
          b2 = (1 - K / mQ + K * K) * norm;
          break;

        case BiquadBandpass:
          K = tan(M_PI * mFreq);
          norm = 1 / (1 + K / mQ + K * K);
          a0 = K / mQ * norm;
          a1 = 0;
          a2 = -a0;
          b1 = 2 * (K * K - 1) * norm;
          b2 = (1 - K / mQ + K * K) * norm;
          break;

        case BiquadNotch:
          K = tan(M_PI * mFreq);
          norm = 1 / (1 + K / mQ + K * K);
          a0 = (1 + K * K) * norm;
          a1 = 2 * (K * K - 1) * norm;
          a2 = a0;
          b1 = a1;
          b2 = (1 - K / mQ + K * K) * norm;
          break;

        case BiquadPeak:
          V = pow(10, fabs(mPeakGain) / 20.0);
          K = tan(M_PI * mFreq);
          if (mPeakGain >= 0) {    // boost
            norm = 1 / (1 + 1 / mQ * K + K * K);
            a0 = (1 + V / mQ * K + K * K) * norm;
            a1 = 2 * (K * K - 1) * norm;
            a2 = (1 - V / mQ * K + K * K) * norm;
            b1 = a1;
            b2 = (1 - 1 / mQ * K + K * K) * norm;
          }
          else {    // cut
            norm = 1 / (1 + V / mQ * K + K * K);
            a0 = (1 + 1 / mQ * K + K * K) * norm;
            a1 = 2 * (K * K - 1) * norm;
            a2 = (1 - 1 / mQ * K + K * K) * norm;
            b1 = a1;
            b2 = (1 - V / mQ * K + K * K) * norm;
          }
          break;

        case BiquadPeakNcq:
          V = pow(10, fabs(mPeakGain) / 20.0);
          L = 2 * M_PI * mFreq;
          E = 4 / (1 + V);
          B = .5 * ((1 - E * tan(L / 2 * mQ)) / (1 + E * tan(L / 2 * mQ)));
          G = (.5 + B) * cos(L);
          if (mPeakGain >= 0) {    // boost
            norm = 1 / (1 + 1 / mQ * L + L * L);
            a0 = .5 - B * norm;
            a1 = 0. * norm;
            a2 = -(.5 - B) * norm;
            b1 = -2 * G * norm;
            b2 = 2 * B * norm;
          }
          else {    // cut
            norm = 1 / (1 + V / mQ * L + L * L);
            a0 = .5 - B * norm;
            a1 = 0. * norm;
            a2 = -(.5 - B) * norm;
            b1 = -2 * G * norm;
            b2 = 2 * B * norm;
          }
          break;


        case BiquadLowshelf:
          V = pow(10, fabs(mPeakGain) / 20.0);
          K = tan(M_PI * mFreq);
          if (mPeakGain >= 0) {    // boost
            norm = 1 / (1 + sqrt(2) * K + K * K);
            a0 = (1 + sqrt(2 * V) * K + V * K * K) * norm;
            a1 = 2 * (V * K * K - 1) * norm;
            a2 = (1 - sqrt(2 * V) * K + V * K * K) * norm;
            b1 = 2 * (K * K - 1) * norm;
            b2 = (1 - sqrt(2) * K + K * K) * norm;
          }
          else {    // cut
            norm = 1 / (1 + sqrt(2 * V) * K + V * K * K);
            a0 = (1 + sqrt(2) * K + K * K) * norm;
            a1 = 2 * (K * K - 1) * norm;
            a2 = (1 - sqrt(2) * K + K * K) * norm;
            b1 = 2 * (V * K * K - 1) * norm;
            b2 = (1 - sqrt(2 * V) * K + V * K * K) * norm;
          }
          break;

        case BiquadHighshelf:
          V = pow(10, fabs(mPeakGain) / 20.0);
          K = tan(M_PI * mFreq);
          if (mPeakGain >= 0) {    // boost
            norm = 1 / (1 + sqrt(2) * K + K * K);
            a0 = (V + sqrt(2 * V) * K + K * K) * norm;
            a1 = 2 * (K * K - V) * norm;
            a2 = (V - sqrt(2 * V) * K + K * K) * norm;
            b1 = 2 * (K * K - 1) * norm;
            b2 = (1 - sqrt(2) * K + K * K) * norm;
          }
          else {    // cut
            norm = 1 / (V + sqrt(2 * V) * K + K * K);
            a0 = (1 + sqrt(2) * K + K * K) * norm;
            a1 = 2 * (K * K - 1) * norm;
            a2 = (1 - sqrt(2) * K + K * K) * norm;
            b1 = 2 * (K * K - V) * norm;
            b2 = (V - sqrt(2 * V) * K + K * K) * norm;
          }
          break;

        case BiquadLinkwitzHighpass:
          O = M_PI * mFreq * mSamplerate; // OMEGAc from Pirkle, p. 186
          K = O / tan(M_PI * mFreq);
          norm = 1 / (K * K + pow(O, 2) + 2 * K * O);
          a0 = (K * K) * norm;
          a1 = (-2 * (K * K)) * norm;
          a2 = a0;
          b1 = (-2 * K * K + 2 * O * O) * norm;
          b2 = (-2 * K * O + K * K + O * O) * norm;
          break;

        case BiquadLinkwitzLowpass:
          O = M_PI * mFreq * mSamplerate; // OMEGAc from Pirkle, p. 186
          K = O / tan(M_PI * mFreq);
          norm = 1 / (K * K + pow(O, 2) + 2 * K * O);
          a0 = (O * O) * norm;
          a1 = 2 * ((O * O) * norm);
          a2 = a0;
          b1 = (-2 * K * K + 2 * O * O) * norm;
          b2 = (-2 * K * O + K * K + O * O) * norm;
          break;

        case OnepoleHighpass:
          b1 = exp(-2.0 * M_PI * mFreq);
          a0 = 1.0 - b1;
          break;

        case OnepoleLowpass:
          b1 = -exp(-2.0 * M_PI * (0.5 - mFreq));
          a0 = 1.0 + b1;
          break;
        }

        return;
      }

      EFilterType mType;
      double a0, a1, a2; // poles
      double b1, b2; // zeros
      T z1[MAXNUMCHANNELS] = {}, z2[MAXNUMCHANNELS] = {}; // past samples
      double mFreq, mQ, mPeakGain, mSamplerate; // members
      int mPoles;
    };


    // ONE-POLE FILTERS
    template<typename T = double>
    class SRFilterOnePole {
    public:
      SRFilterOnePole() {
        a0 = 1.0;
        b1 = 0.0;
        z1 = 0.0;
      }

      SRFilterOnePole(double frequency, double samplerate) {
        SetFilter(type, frequency, samplerate);
        z1 = 0.0;
        SetType(type);
        SetFreq(frequency);
      }

      ~SRFilterOnePole() {}

      void SetFilter(int type, double frequency, double samplerate) {
        this->mType = type;
        this->mFreq = frequency;
        this->mSamplerate = samplerate;
        Update();
      }
      void SetType(int type) {
        this->mType = type;
        Update();
      }
      void SetFreq(double frequency) {
        this->mFreq = frequency;
        Update();
      }

      T Process(T in) {
        //	return z1 = in * a0 + z1 * b1;
        z1 = in * a0 + z1 * b1;
        in -= z1;
        return in;
      }

    protected:
      void Update() {
        b1 = exp(-2.0 * M_PI * mFreq);
        a0 = 1.0 - b1;
        //	b1 = -exp(-2.0 * M_PI * (0.5 - mFreq));
        //	a0 = 1.0 + b1;
      }
      T a0;                  // poles
      T b1;                  // zeros
      T z1;                  // last samples
      double mFreq, mSamplerate;  // members
    };


    // PARAMETER SMOOTHING FILTER
    class SRFilterParamSmooth
    {
    public:
      SRFilterParamSmooth(double smoothingTimeInMs = 10.0, double samplerate = 44100.0)
      {
        Reset(smoothingTimeInMs, samplerate);
      }

      ~SRFilterParamSmooth() {}

      void Reset(double smoothingTimeInMs, double samplerate) {
        const double twoPi = 6.283185307179586476925286766559;
        a = exp(-twoPi / (smoothingTimeInMs * 0.001 * samplerate));
        b = 1.0 - a;
        z = 0.0;
      };

      inline double Process(double in) { z = (in * b) + (z * a); return z; }

    private:
      double a, b, z;
    };


  }
}
// end namespace SRFilters