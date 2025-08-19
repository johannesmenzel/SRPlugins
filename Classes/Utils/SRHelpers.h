#pragma once

#define _USE_MATH_DEFINES
#include <math.h>
#include <cmath>
#include <string>

namespace SR {
  namespace Utils {

    // CONVERSION
    // ----------

    // Decibel - Linear conversion for classes that doesn't include IPlug
    static inline double DBToAmp(double dB) { return exp(0.11512925464970 * dB); }
    static inline double AmpToDB(double amp) { return 8.685889638065036553 * log(fabs(amp)); }

    // double to char conversion
    // DOESNT WORK
    //inline char* setCharFromDouble(double doubleValue) {
    //  std::string stringValue = std::to_string(doubleValue);
    //  char *charValue = new char[stringValue.length() + 1];
    //  return strcpy(charValue, stringValue.c_str());
    //}

    /** Get value for IPlug2 IParam::ShapePowCurve(shape)
    * @param minValue Pass parameters min value
    * @param maxValue Pass parameters max value
    * @param centeredValue Pass value you want to have in the middle of your control
    * @param controlPosition Pass position of centered value if it shouldn't be in the middle (0. .. 1.)
    */
    double SetShapeCentered(double minValue, double maxValue, double centeredValue, double controlPosition = .5) {
      return log((centeredValue - minValue) / (maxValue - minValue)) / log(controlPosition);
    }

    // DSP
    // ---

    static const double PI = 3.1415926535897932384626433832795;

    // Fast tanh algorithm for real time audio (source unknown)
    inline double fast_tanh(double x) {
      x = exp(x + x);
      return (x - 1) / (x + 1);
    }

    // Fast tanh algorithm for real time audio
    // See https://www.kvraudio.com/forum/viewtopic.php?t=388650&sid=c02be30a126707583d5c3da4febe2de4
    inline double vox_fasttanh2(const double x)
    {
      const double ax = fabs(x);
      const double x2 = x * x;

      return(x * (2.45550750702956 + 2.45550750702956 * ax +
        (0.893229853513558 + 0.821226666969744 * ax) * x2) /
        (2.44506634652299 + (2.44506634652299 + x2) *
          fabs(x + 0.814642734961073 * x * ax)));
    }
    template <typename T>
    T Clip(T x, T lo, T hi) { return std::min(std::max(x, lo), hi); }
  }
}
// end namespaces