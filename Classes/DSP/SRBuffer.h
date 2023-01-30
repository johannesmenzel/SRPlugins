/*
How is the buffer implemented?
1 - std::vector of WDL_Typedbuf (high cpu and dropouts)
2 - WDL_PtrList/WDL_Typedbuf (Works now, but some dropouts, further investigation in IPlugInstrument_DSP.h example)
3 - T** (Works best, but not resizeable (May be problematic when blocksize exceeds MAXNUMFRAMES, same for channels
*/
#define BUFFERIMPL 3

#ifndef SRBUFFER_H
#define SRBUFFER_H

//#include "IGraphics_include_in_plug_hdr.h"
#include <cassert>

#if BUFFERIMPL == 1 // vec
#include <vector>
#elif BUFFERIMPL == 2 // ptr
#include "ptrlist.h"
#include "heapbuf.h"
#endif

namespace SR {
	namespace DSP {

		template<typename T = double, int MAXNUMCHANNELS = 1, int MAXNUMFRAMES = DEFAULT_BLOCK_SIZE>
		// Buffer in progress with PtrList of WDL_Typedbuf, vectors or simple T**
		class SRBuffer {
		public:

			// Construct class
			SRBuffer(int nChannels = MAXNUMCHANNELS, int nFrames = MAXNUMFRAMES)
				: mNumChannels(nChannels)
				, mNumFrames(nFrames)
#if BUFFERIMPL == 3 // T**
				, mBuffer(new T* [nChannels])
#endif
			{
#if BUFFERIMPL == 1 // vec
				mBuffer.reserve(MAXNUMCHANNELS);
				mBuffer.resize(mNumChannels);
				for (int c = 0; c < mNumChannels; c++) {
					mBuffer[c].Resize(mNumFrames);
				}
#elif BUFFERIMPL == 2 // ptr
				ResetBuffer(nChannels, nFrames);
#elif BUFFERIMPL == 3 // T**
				for (int c = 0; c < mNumChannels; c++) {
					mBuffer[c] = new T[mNumFrames];
				}
#endif
			}

			// Destruct class
			~SRBuffer() {
#if BUFFERIMPL == 1 // vec
				mBuffer.clear();
#elif BUFFERIMPL == 2 //ptr
				mBuffer.Empty();
#elif BUFFERIMPL == 3 // T**
				for (int c = 0; c < mNumChannels; c++) {
					delete[] mBuffer[c];
				}
				delete[] mBuffer;
#endif
			}

			// Set or Reset buffer with specified channel count and block size
			void ResetBuffer(int nChannels = MAXNUMCHANNELS, int nFrames = MAXNUMFRAMES) {
				assert(nChannels <= MAXNUMCHANNELS);
#if BUFFERIMPL == 1 // vec
				for (int c = 0; c < nChannels; c++) {
					mBuffer[c].Resize(nFrames, true);
				}
				//ClearBuffer();
#elif BUFFERIMPL == 2 // ptr
				mBufferData.Resize(nFrames * nChannels);
				mBuffer.Empty();
				for (int c = 0; c < nChannels; c++) {
					mBuffer.Add(mBufferData.Get() + (nFrames * c));
				}
#elif BUFFERIMPL == 3 // T**
				// TODO: Maybe very slow to do this on every OnReset()
				// maybe we just keep it at MAX-Sizes, but then we must make sure that with BUFFER.Get() you just get what you need (blocksize/ch-count)

				// delete dynamic 2D array
				for (int c = 0; c < mNumChannels; c++) {
					if (mBuffer[c]) delete[] mBuffer[c];
				}
				if (mBuffer) delete[] mBuffer;

				// create new 2D array;
				mBuffer = new T * [nChannels];
				for (int c = 0; c < nChannels; c++) {
					mBuffer[c] = new T[nFrames];
				}
#endif //
				mNumChannels = nChannels;
				mNumFrames = nFrames;
			}


			// Clear buffer (set data to all zeros)
			void ClearBuffer() {
#if BUFFERIMPL == 1 // vec
				mBuffer.clear();
#elif BUFFERIMPL == 2 // ptr
				memset(mBufferData.Get(), 0, mBufferData.GetSize);
				mBuffer.Empty();
#elif BUFFERIMPL == 3 // T**
				memset(mBuffer, 0, mNumChannels * mNumFrames * sizeof(T));
				// TODO
#endif
			}


			// Process single sample of data
			void ProcessBuffer(T in, int channel, int sample) {
#if BUFFERIMPL == 1 // vec
				mBuffer[channel].Get()[sample] = in;
#elif BUFFERIMPL == 2 // ptr
				mBuffer.GetList()[channel][sample] = in;
#elif BUFFERIMPL == 3 // T**
				mBuffer[channel][sample] = in;
#endif
			}


			// Process one channel of data
			void ProcessBuffer(T* in, int channel) {
#if BUFFERIMPL == 1 // vec
				mBuffer[channel].Get() = in;
#elif BUFFERIMPL == 2
				mBuffer.GetList()[channel] = in;
#elif BUFFERIMPL == 3 // T**
				mBuffer[channel] = in;
#endif
			}


			// Process all channels of data at once
			void ProcessBuffer(T** in) {
#if BUFFERIMPL == 1 // vec
				for (int c = 0; c < mNumChannels; c++) {
					mBuffer[c].Get() = in[c];
				}
#elif BUFFERIMPL == 2 // ptr
				mBuffer.GetList() = in;
#elif BUFFERIMPL == 3 // T**
				mBuffer = in;
#endif
			}


			// Get sample from channel from buffer
			T GetBuffer(int channel, int sample) {
#if BUFFERIMPL == 1 // vector
				return mBuffer[channel].Get()[sample];
#elif BUFFERIMPL == 2 // ptrlist
				return mBuffer.GetList()[channel][sample];
#elif BUFFERIMPL == 3 // T**
				return mBuffer[channel][sample];
#endif
			}


			// Get channel from buffer
			T* GetBuffer(int channel) {
#if BUFFERIMPL == 1 // vector
				return mBuffer[channel].Get();
#elif BUFFERIMPL == 2 // ptrlist
				return mBuffer.GetList()[channel];
#elif BUFFERIMPL == 3 // T**
				return mBuffer[channel];
#endif

			}


			// Get entire buffer
			T** GetBuffer() {
#if BUFFERIMPL == 1 // vector
				T** buffer = new T * [mNumChannels];
				for (int c = 0; c < mNumChannels; c++) {
					buffer[c] = mBuffer[c].Get();
				}
				return buffer;
#elif BUFFERIMPL == 2 // ptrlist
				return mBuffer.GetList();
#elif BUFFERIMPL == 3 // T**
				return mBuffer;
#endif
			}

			// Sum all data of all channels
			T SumBuffer() {
				T sum = (T)0.0;
				for (int c = 0; c < mNumChannels; c++) {
					sum += SumBuffer(c);
				}
				return sum;
			}

			// Sum all data of all channels
			T SumBufferAbs() {
				T sum = (T)0.0;
				for (int c = 0; c < mNumChannels; c++) {
					sum += SumBufferAbs(c);
				}
				return sum;
			}

			// Sum data of specified channel
			T SumBuffer(int channel) {
				T sum = (T)0.0;
				for (int s = 0; s < mNumFrames; s++) {
#if BUFFERIMPL == 1 // vector
					sum += (T)mBuffer[channel].Get()[s];
#elif BUFFERIMPL == 2 // ptr
					sum += (T)mBuffer.GetList()[channel][s];
#elif BUFFERIMPL == 3 // T**
					sum += (T)mBuffer[channel][s];
#endif
				}
				return sum;
			}

			// Sum abs data of specified channel
			T SumBufferAbs(int channel) {
				T sum = (T)0.0;
				for (int s = 0; s < mNumFrames; s++) {
#if BUFFERIMPL == 1 // vector
					sum += std::fabs((T)mBuffer[channel].Get()[s]);
#elif BUFFERIMPL == 2 // ptr
					sum += std::fabs((T)mBuffer.GetList()[channel][s]);
#elif BUFFERIMPL == 3 // T**
					sum += std::fabs((T)mBuffer[channel][s]);
#endif
				}
				return sum;
			}

			// Calculate average of all data over all channels
			T AverageBuffer() { return SumBuffer() / (mNumFrames * mNumChannels); }

			// Calculate average of data of specified channel
			T AverageBuffer(int channel) { return SumBuffer(channel) / (T)mNumFrames; }


			// Calculate average of all abs data over all channels
			T AverageBufferAbs() { return SumBufferAbs() / (mNumFrames * mNumChannels); }

			// Calculate average of abs data of specified channel
			T AverageBufferAbs(int channel) { return SumBufferAbs(channel) / (T)mNumFrames; }

			T GetMax(int channel) {
				T max = (T)0.0;
				for (int s = 0; s < mNumFrames; s++) {
#if BUFFERIMPL == 2 // WDL
					max = std::max(max, (T)mBuffer.GetList()[channel][s]);
#elif BUFFERIMPL == 3 // T**
					max = std::max(max, (T)mBuffer[channel][s]);
#endif
				}
				return max;
			}


			T GetMin() {}

		private:
			unsigned int mNumFrames;
			unsigned int mNumChannels;
#if BUFFERIMPL == 1 // vector
			std::vector<WDL_TypedBuf<T>> mBuffer;
#elif BUFFERIMPL == 2 // ptrlist
			WDL_TypedBuf<T> mBufferData;
			WDL_PtrList<T> mBuffer;
#elif BUFFERIMPL == 3 // T**
			T** mBuffer;
#endif
		};

		// End of namespaces:
	}
}
#endif //SRBUFFER_H