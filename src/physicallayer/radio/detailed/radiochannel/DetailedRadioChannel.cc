#include "DetailedRadioChannel.h"

#include <cmath>

Define_Module(DetailedRadioChannel);

double DetailedRadioChannel::calcInterfDist()
{
    double interfDistance;

    //the minimum carrier frequency for this cell
    double carrierFrequency = par("carrierFrequency").doubleValue();
    //maximum transmission power possible
    double pMax = par("pMax").doubleValue();
	if (pMax <=0) {
        error("Max transmission power is <=0!");
    }
    //minimum signal attenuation threshold
    double sat = par("sat").doubleValue();
    //minimum path loss coefficient
    double alpha = par("alpha").doubleValue();

    double waveLength = SPEED_OF_LIGHT / carrierFrequency;
    //minimum power level to be able to physically receive a signal
    double minReceivePower = pow(10.0, sat / 10.0);

	interfDistance = pow(waveLength * waveLength * pMax
					       / (16.0*M_PI*M_PI*minReceivePower),
					     1.0 / alpha);

    EV_INFO << "max interference distance:" << interfDistance << endl;

    return interfDistance;
}

