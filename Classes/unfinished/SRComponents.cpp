#include "SRComponents.h"
//#include this and that
//#include <math.h>

namespace SRPlugins
{
namespace SRComponents
{

SRTube::SRTube()
{
	// declare variable values so that non empty
	mType = typemusi;
	// member variables
	mDriveNormalized = 1.;
	mAmountNormalized = 0.;
	mHarmonicsNormalized = .5;
	mPositive = true;
	mSkewNormalized = 0.;
	mWetNormalized = 1.;
}

// Constructor
SRTube::SRTube(int pType, double pDriveDb, double pAmountNormalized, double pHarmonicsNormalized, bool pPositiveSide, double pSkewNormalized, double pWet)
{
	// internal variables
	setSaturation(pType, pDriveDb, pAmountNormalized, pHarmonicsNormalized, pPositiveSide, pSkewNormalized, pWet);
	mAmount = 0.;
	in1PrevSample = 0.;
	in2PrevSample = 0.;
	in1Dry = 0.;
	in2Dry = 0.;
}

// Destructor
SRTube::~SRTube()
{
}

void SRTube::setType(int pType)
{
	this->mType = pType;
	calcSaturation();
}

void SRTube::setDrive(double pDriveDb)
{
	this->mDriveNormalized = SRPlugins::SRHelpers::DBToAmp(pDriveDb);
}

void SRTube::setAmount(double pAmountNormalized)
{
	this->mAmountNormalized = pAmountNormalized;
	calcSaturation();
}

void SRTube::setHarmonics(double pHarmonicsNormalized)
{
	this->mHarmonicsNormalized = pHarmonicsNormalized;
}

void SRTube::setPositive(bool pPositive)
{
	this->mPositive = pPositive;
}

void SRTube::setSkew(double pSkewNormalized)
{
	this->mSkewNormalized = pSkewNormalized;
}

void SRTube::setWet(double pWetNormalized)
{
	this->mWetNormalized = pWetNormalized;
}

void SRTube::setTube(int pType, double pDriveDb, double pAmountNormalized, double pHarmonicsNormalized, double pPositive, double pSkewNormalized, double pWetNormalized)
{
	this->mType = pType;
	this->mDriveNormalized = SRPlugins::SRHelpers::DBToAmp(pDriveDb);
	this->mAmountNormalized = pAmountNormalized;
	this->mHarmonicsNormalized = pHarmonicsNormalized;
	this->mPositive = pPositive;
	this->mSkewNormalized = pSkewNormalized;
	this->mWetNormalized = pWetNormalized;
	calcTube();
}

// Internal calculations. Example on calculation depending on mType:
void SRTube::calcTube(void)
{
	switch (this->mType)
	{
	case type12AX7;
		break;
		default:
		break;
	}
	return;
}
} // namespace SRComponents

} // namespace SRPlugins