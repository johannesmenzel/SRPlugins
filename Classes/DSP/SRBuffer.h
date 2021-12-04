/*
How is the buffer implemented?
1 - std::vector
2 - Ptrlist
3 - T**
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

    // Buffer in progress with PtrList of WDL_Typedbuf, vectors or simple T**
    template<typename T = double, int MAXNUMCHANNELS = 1, int MAXNUMFRAMES = DEFAULT_BLOCK_SIZE>
    class SRBuffer {
    public:

      // Construct class
      SRBuffer(int nChannels = MAXNUMCHANNELS, int nFrames = MAXNUMFRAMES)
        : mNumChannels(nChannels)
        , mNumFrames(nFrames)
#if BUFFERIMPL == 3 // T**
        , mBuffer(new T*[nChannels])
#endif
      {
#if BUFFERIMPL == 1 // vec
        mBuffer.reserve(MAXNUMCHANNELS);
        mBuffer.resize(mNumChannels);
        for (int c = 0; c < mNumChannels; c++) {
          mBuffer[c].Resize(mNumFrames);
        }
#elif BUFFERIMPL == 2 // ptr
        for (int c = 0; c < MAXNUMCHANNELS; c++) {
          mBuffer.Add(new WDL_TypedBuf<T>);
          mBuffer.Get(c)->Resize(mNumFrames);
        }
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
        //for (int c = 0; c < MAXNUMCHANNELS; c++) {
        //  mBuffer.Delete(c);
        //}
        mBuffer.Empty();
#elif BUFFERIMPL == 3 // T**
        for (int c = 0; c < MAXNUMCHANNELS; c++) {
          delete[] mBuffer[c];
        }
        delete[] mBuffer;
#endif
      }

      // Set blocksize
      void SetNumFrames(int nFrames = MAXNUMFRAMES) {
        mNumFrames = nFrames;
        for (int c = 0; c < mNumChannels; c++) {
#if BUFFERIMPL == 1 // vec
          mBuffer[c].Resize(mNumFrames, true);
#elif BUFFERIMPL == 2 // ptr
          mBuffer.Get(c)->Resize(mNumFrames, true);
#elif BUFFERIMPL == 3 // T**
          if (nFrames != mNumFrames)
            ResetBuffer(mNumChannels, nFrames);
#endif
        }
      }

      // Set number of channels
      void SetNumChannels(int nChannels) {
#if BUFFERIMPL == 1 // vec
        // TODO
#elif BUFFERIMPL == 2 // ptr
        // TODO
#elif BUFFERIMPL == 3 // T**
        if (nChannels != mNumChannels)
          ResetBuffer(nChannels, mNumFrames);
#endif
      }

      // Set or Reset buffer with specified channel count and block size
      void ResetBuffer(int nChannels = MAXNUMCHANNELS, int nFrames = MAXNUMFRAMES) {
        assert(nChannels <= MAXNUMCHANNELS);
        mNumChannels = nChannels;
#if BUFFERIMPL == 1 // vec
        for (int c = 0; c < mNumChannels; c++) {
          mBuffer[c].Resize(mNumFrames, true);
        }
        SetNumFrames(nFrames);
        //ClearBuffer();
#elif BUFFERIMPL == 2 // ptr
        //// delete dynamic 2D array
        //for (int c = 0; c < mNumChannels; c++) {
        //  if (mBuffer.Get(c)) {
        //    //delete[] mBuffer.Get(c);
        //    mBuffer.Delete(c);
        //  }
        //}
        ////if (mBuffer) mBuffer.Empty();

        //// change channel and frames count
        //mNumChannels = nChannels;
        //mNumFrames = nFrames;
        //ClearBuffer();

        //// create new 2D array;
        //for (int c = 0; c < mNumChannels; c++) {
        //  mBuffer.Add(new WDL_TypedBuf<T>);
        //  mBuffer.Get(c)->Resize(mNumFrames);
        //}

        for (int c = 0; c < MAXNUMCHANNELS; c++) {
          mBuffer.Get(c)->Resize(mNumFrames, true);
        }
        ClearBuffer();
#elif BUFFERIMPL == 3 // T**
        // delete dynamic 2D array
        for (int c = 0; c < mNumChannels; c++) {
          if (mBuffer[c]) delete[] mBuffer[c];
        }
        if (mBuffer) delete[] mBuffer;

        // change channel and frames count
        mNumChannels = nChannels;
        mNumFrames = nFrames;

        // create new 2D array;
        mBuffer = new T*[mNumChannels];
        for (int c = 0; c < mNumChannels; c++) {
          mBuffer[c] = new T[mNumFrames];
        }
#endif
      }


      // Clear buffer (set data to all zeros)
      void ClearBuffer() {
#if BUFFERIMPL == 1 // vec
        mBuffer.clear();
#elif BUFFERIMPL == 2 // ptr
        //memset(mBuffer.GetList(), 0, MAXNUMCHANNELS * mNumFrames * sizeof(T));
        for (int c = 0; c < MAXNUMCHANNELS; c++) {
          memset(mBuffer.Get(c), 0, mNumFrames * sizeof(T));
        }
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
        mBuffer.Get(channel)->Get()[sample] = in;
#elif BUFFERIMPL == 3 // T**
        mBuffer[channel][sample] = in;
#endif
      }


      // Process one channel of data
      void ProcessBuffer(T* in, int channel) {
#if BUFFERIMPL == 1 // vec
        mBuffer[channel].Get() = in;
#elif BUFFERIMPL == 2
        mBuffer.Get(channel) = in;
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
        return mBuffer.Get(channel)[sample];
#elif BUFFERIMPL == 3 // T**
        return mBuffer[channel][sample];
#endif
      }


      // Get channel from buffer
      T* GetBuffer(int channel) {
#if BUFFERIMPL == 1 // vector
        return mBuffer[channel].Get();
#elif BUFFERIMPL == 2 // ptrlist
        return mBuffer.Get(channel);
#elif BUFFERIMPL == 3 // T**
        return mBuffer[channel];
#endif

      }


      // Get entire buffer
      T** GetBuffer() {
#if BUFFERIMPL == 1 // vector
        T** buffer = new T*[mNumChannels];
        for (int c = 0; c < mNumChannels; c++) {
          buffer[c] = mBuffer[c].Get();
        }
        return buffer;
#elif BUFFERIMPL == 2 // ptrlist
        //T** buffer = new T*[mNumChannels];
        //for (int c = 0; c < mNumChannels; c++) {
        //  buffer[c] = mBuffer.Get(c);
        //}
        //return buffer;
        // or?
        return (T**)mBuffer.GetList();
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
          sum += (T)mBuffer.Get(channel)->Get()[s];
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
          sum += std::fabs((T)mBuffer.Get(channel)->Get()[s]);
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
#if BUFFERIMPL == 3 // T**
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
      WDL_PtrList<WDL_TypedBuf<T>> mBuffer;
#elif BUFFERIMPL == 3 // T**
      T **mBuffer;
#endif
    };

    // End of namespaces:
  }
}
#endif //SRBUFFER_H