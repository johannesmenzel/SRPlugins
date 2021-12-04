#pragma once

#define _USE_MATH_DEFINES
#include <math.h>
#include <cmath>
#include <string>

namespace SR {
  namespace Utils {

    // CONVERSION
    // ----------

    // Decibel - Linear conversion
    static inline double DBToAmp(double dB) { return exp(0.11512925464970 * dB); }
    static inline double AmpToDB(double amp) { return 8.685889638065036553 * log(fabs(amp)); }

    // double to char conversion
    // DOESNT WORK
    inline char* setCharFromDouble(double doubleValue) {
      std::string stringValue = std::to_string(doubleValue);
      char *charValue = new char[stringValue.length() + 1];
      return strcpy(charValue, stringValue.c_str());
    }

    inline double SetShapeCentered(double cMinValue, double cMaxValue, double cCenteredValue, double cControlPosition) {
      return log((cCenteredValue - cMinValue) / (cMaxValue - cMinValue)) / log(cControlPosition);
    }

    // DSP
    // ---



    // tanh
    inline double fast_tanh(double x) {
      x = exp(x + x);
      return (x - 1) / (x + 1);
    }

    inline double vox_fasttanh2(const double x)
    {
      const double ax = fabs(x);
      const double x2 = x * x;

      return(x * (2.45550750702956 + 2.45550750702956 * ax +
        (0.893229853513558 + 0.821226666969744 * ax) * x2) /
        (2.44506634652299 + (2.44506634652299 + x2) *
          fabs(x + 0.814642734961073 * x * ax)));
    }

  }
}
// end namespaces