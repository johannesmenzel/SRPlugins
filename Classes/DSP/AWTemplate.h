#pragma once
/*
Steps:
1. Copy this .h file and rename accordingly
2. Replace all occurances of PLUGIN with actual plugin name
*/

/* ========================================
 *  airwindows PLUGIN
 *  Copyright (c) airwindows, Airwindows uses the MIT license
 * ======================================== */
#include <set>
#include <string>
#include <math.h>

namespace SR {
	namespace DSP {
		namespace Airwindows {
			// DSP class from airwindows PLUGIN
			class PLUGIN {
			public:
				enum EParam {
					// [INSERT] from PLUGIN.h param enum insert block of params, optionally rename them
					kNumParameters
				};
				PLUGIN(double samplerate = 44100.) {
					Reset(samplerate);
					// [REPLACE] from PLUGIN.cpp insert constructor the block containing default values, excluding _canDo... and following
				}
				void Reset(double samplerate) {
					mSampleRate = samplerate;
				}
				double getSampleRate() { return mSampleRate; }
				// [REPLACE] Help the user to understand when setting, get information from PLUGIN.cpp getParameterDisplay and getParameter 
				/** 
				* @param index [EXPLAIN EACH PARAMETER, IF INT-ISH TELL ALSO RANGES]
				* @param value Set double value between 0. and 1.
				*/
				void PLUGIN::SetParameterNormalized(EParam index, double value) {
					// [REPLACE] from PLUGIN.cpp insert block of PLUGIN::setParameter(VstInt32 index, float value)
				}
				double PLUGIN::GetParameterNormalized(EParam index) {
					// [REPLACE] from PLUGIN.cpp insert block of PLUGIN::getParameter(VstInt32 index)
				}
				void ProcessBlock(double** inputs, double** outputs, int sampleFrames);
			private:
				double mSampleRate;
				// [REPLACE] from PLUGIN.h insert block of private members, replace type of params from float to double
			};

			inline void PLUGIN::ProcessBlock(double** inputs, double** outputs, int sampleFrames) {
				// [REPLACE] from PLUGINProc.cpp insert block of processDoubleReplacing
			}

		}
	}
}
