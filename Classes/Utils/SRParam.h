#pragma once
//#include "IPlug/IPlug_include_in_plug_hdr.h"
#include "IPlugLogger.h"
#include <variant>
#include "../DSP/SRFilters.h"
#include "ptrlist.h"


namespace SR {
  namespace DSP {

    /*
    Class acting as a parameter value smoother.
    Works with linear paramter changes.
    */
    class SRParamSmooth {
    public:
      // Constructors & Destructors
      SRParamSmooth()
        : mCurrentValue(1.0)
        , mTargetValue(1.0)
        , mNumSmoothSamples(0)
        , mNumSmoothSamplesLeft(0)
        , mValueChangePerSample(0.0)
      {
      }

      SRParamSmooth(double value, int numSmoothSamples)
        : mTargetValue(value)
        , mCurrentValue(value)
        , mNumSmoothSamples(numSmoothSamples)
      {
      }

      ~SRParamSmooth()
      {
      }

      // Setters
      void Set(double value) {
        if (mTargetValue != value) {
          mTargetValue = value;
          mNumSmoothSamplesLeft = mNumSmoothSamples;
          mValueChangePerSample = (mTargetValue - mCurrentValue) / mNumSmoothSamples;
        }
      }

      void SetStrict(double value) {
        mCurrentValue = mTargetValue = value;
        mNumSmoothSamplesLeft = 0;
        mValueChangePerSample = 0.0;
      }

      void SetNumSmoothSamples(int numSmoothSamples) {
        mNumSmoothSamples = numSmoothSamples;
      }

      // Getters
      double Get() { return mCurrentValue; }
      int GetNumSmoothSamples() { return mNumSmoothSamples; }
      int GetNumSmoothSamplesLeft() { return mNumSmoothSamplesLeft; }

      inline void Process() {
        if (mNumSmoothSamplesLeft > 0) {
          mCurrentValue += mValueChangePerSample;
          mNumSmoothSamplesLeft--;
          if (mNumSmoothSamplesLeft == 0) {
            mValueChangePerSample = 0.0;
            mCurrentValue = mTargetValue;
          }
        }
      }

    protected:

    private:
      double mCurrentValue, mTargetValue;
      int mNumSmoothSamples;
      int mNumSmoothSamplesLeft;
      double mValueChangePerSample;
    };

    using Values = std::variant<double, float, int, bool>;
    enum EType {
      kDouble = 0,
      kFloat,
      kInt,
      kBool,
      kNoninit,
      kNumTypes
    };
    class SRParam
    {
    public:
      SRParam(int numSmoothSamples = 0)
        : mNumSmoothSamples(numSmoothSamples)
        , mNumParams(0)
        , mParams()
      {
      }
      ~SRParam()
      {
        for (int i = 0; i < mNumParams; i++) {
          delete(mParams.Get(i));
          mParams.Delete(i);
        }
        mParams.Empty();
      }

      void Add(int paramIdx, int type, bool smoothing = false)
      {
        mParams.Add(new Param);
        mNumParams++;
        Param* thisParam = mParams.Get(paramIdx);
        thisParam->mType = type;
        SetSmoothing(paramIdx, smoothing);
        switch (thisParam->mType) {
        case EType::kDouble:
          thisParam->mValue = 0.0;
          thisParam->mTargetValue = 0.0;
          break;
        case EType::kFloat:
          thisParam->mValue = 0.f;
          thisParam->mTargetValue = 0.f;
          break;
        case EType::kInt:
          thisParam->mValue = 0;
          thisParam->mTargetValue = 0;
          break;
        case EType::kBool:
          thisParam->mValue = false;
          thisParam->mTargetValue = false;
          break;
        default:
          break;
        }


      } // Add Parameter, use EParams loop. You have to set value and if it's smoothing after that

      void SetDouble(int paramIdx, double value, bool smooth = true) {
        Param* paramToSet = mParams.Get(paramIdx);
        paramToSet->mTargetValue = (double)value;
        if (paramToSet->mType != EType::kDouble) paramToSet->mType = EType::kDouble;
        if (smooth) paramToSet->mSmoothSamplesLeft = mNumSmoothSamples;
        else paramToSet->mValue = (double)value;
        //paramToSet->mValue = value;

        //if (paramIdx == 0) DBGMSG("Set: i: %i; Tp: %i; v: %f; V: %f; TV: %f\n", paramIdx, paramToSet->mType, value, paramToSet->mValue, paramToSet->mTargetValue);
      } // Set double value at index
      void SetFloat(int paramIdx, float value, bool smooth = true) {
        Param* paramToSet = mParams.Get(paramIdx);
        paramToSet->mTargetValue = (float)value;
        if (paramToSet->mType != EType::kFloat)paramToSet->mType = EType::kFloat;
        if (smooth) paramToSet->mSmoothSamplesLeft = mNumSmoothSamples;
        else paramToSet->mValue = (float)value;
        //paramToSet->mValue = value;

      } // Set floating value at index
      void SetInt(int paramIdx, int value) {
        Param* paramToSet = mParams.Get(paramIdx);
        paramToSet->mValue = value;
        if (paramToSet->mType != EType::kInt) paramToSet->mType = EType::kInt;
      } // Set integer value at index
      void SetBool(int paramIdx, bool value) {
        Param* paramToSet = mParams.Get(paramIdx);
        paramToSet->mValue = value;
        if (paramToSet->mType != EType::kBool) paramToSet->mType = EType::kBool;
      } // Set boolean value at index

      double GetDouble(int paramIdx) { return std::get<double>(mParams.Get(paramIdx)->mValue); } // Get double value at index
      float GetFloat(int paramIdx) { return std::get<float>(mParams.Get(paramIdx)->mValue); } // Get floating value at index
      int GetInt(int paramIdx) { return std::get<int>(mParams.Get(paramIdx)->mValue); } // Get integer value at index
      bool GetBool(int paramIdx) { return std::get<bool>(mParams.Get(paramIdx)->mValue); } // Get boolean value at index

      void SetSmoothing(int paramIdx, bool smoothing)
      {
        mParams.Get(paramIdx)->mSmooth = (mNumSmoothSamples > 0) ? smoothing : false;
      } // Set if parameter changes should be smoothed
      void SetGlobalNumSmoothSamples(int samples) {
        mNumSmoothSamples = samples;
      }
      bool IsCurrentlySmoothing(int paramIdx) {
        return (mParams.Get(paramIdx)->mSmoothSamplesLeft > 0) ? true : false;
      } // Is this parameter currently in its smoothing loop?

      void Process(int paramIdx) {
        Param* paramToProcess = mParams.Get(paramIdx);
        if (paramToProcess->mSmoothSamplesLeft > 0) {
          switch (mParams.Get(paramIdx)->mType) {
          case EType::kDouble:
            //if (paramIdx == 7) DBGMSG("%f %f %i", std::get<double>(paramToProcess->mTargetValue), std::get<double>(paramToProcess->mValue), paramToProcess->mIsCurrentlySmoothing);
            //paramToProcess->mValue = paramToProcess->mSmoothFilter.Process(std::get<double>(paramToProcess->mTargetValue));
            paramToProcess->mValue = double(std::get<double>(paramToProcess->mValue) + (std::get<double>(paramToProcess->mTargetValue) - std::get<double>(paramToProcess->mValue)) * (1. / paramToProcess->mSmoothSamplesLeft));
            paramToProcess->mSmoothSamplesLeft--;
            if (paramToProcess->mSmoothSamplesLeft == 0) paramToProcess->mValue = paramToProcess->mTargetValue;
            break;
          case EType::kFloat:
            paramToProcess->mValue = float(std::get<float>(paramToProcess->mValue) + (std::get<float>(paramToProcess->mTargetValue) - std::get<float>(paramToProcess->mValue)) * (1.f / paramToProcess->mSmoothSamplesLeft));
            paramToProcess->mSmoothSamplesLeft--;
            if (paramToProcess->mSmoothSamplesLeft == 0) paramToProcess->mValue = paramToProcess->mTargetValue;
            break;
          case EType::kInt:
          case EType::kBool:
            if (paramToProcess->mValue != paramToProcess->mTargetValue)
              paramToProcess->mValue = paramToProcess->mTargetValue;
            break;
          default:
            if (paramToProcess->mValue != paramToProcess->mTargetValue)
              paramToProcess->mValue = paramToProcess->mTargetValue;
            break;
          }
        }
      }
      void ProcessAll() {
        for (int paramIdx = 0; paramIdx < mNumParams; paramIdx++) {
          Process(paramIdx);
        }
      }

    private:
      // Holds values of each parameter
      struct Param
      {
        Values mValue;
        Values mTargetValue;
        bool mSmooth;
        int mSmoothSamplesLeft;
        int mType;
        Param()
          : mType(EType::kNoninit)
          , mValue(0.0)
          , mTargetValue(0.0)
          , mSmooth(false)
          , mSmoothSamplesLeft(0)
        {
        }
      };
      int mNumParams;
      int mNumSmoothSamples;
      WDL_PtrList<Param> mParams;
    };

  }
}