/* -*- mode:c++ -*- ********************************************************
 * file:        UWBIRIEEE802154APathlossModel.h
 *
 * author:      Jerome Rousselot <jerome.rousselot@csem.ch>
 *
 * copyright:   (C) 2008-2009 Centre Suisse d'Electronique et Microtechnique (CSEM) SA
 * 				Systems Engineering
 *              Real-Time Software and Networking
 *              Jaquet-Droz 1, CH-2002 Neuchatel, Switzerland.
 *
 *              This program is free software; you can redistribute it
 *              and/or modify it under the terms of the GNU General Public
 *              License as published by the Free Software Foundation; either
 *              version 2 of the License, or (at your option) any later
 *              version.
 *              For further information see file COPYING
 *              in the top level directory
 * description: this AnalogueModel implements the IEEE 802.15.4A Channel Model
 ***************************************************************************/

#include "UWBIRIEEE802154APathlossModel.h"

#include <map>
#include <limits>

#include "IEEE802154A.h"
#include "MiXiMAirFrame.h"

const double UWBIRIEEE802154APathlossModel::PL0 = 0.000040738; // -43.9 dB
const double UWBIRIEEE802154APathlossModel::pathloss_exponent = 1.79;
const double UWBIRIEEE802154APathlossModel::meanL = 3;
const double UWBIRIEEE802154APathlossModel::Lambda = 0.047E9;
const double UWBIRIEEE802154APathlossModel::lambda1 = 1.54E9;
const double UWBIRIEEE802154APathlossModel::lambda2 = 0.15E9;
const double UWBIRIEEE802154APathlossModel::Beta = 0.095;
const double UWBIRIEEE802154APathlossModel::Gamma = 22.61E-9;
const double UWBIRIEEE802154APathlossModel::k_gamma = 0;
const double UWBIRIEEE802154APathlossModel::gamma_0 = 12.53 * 0.001 * 0.001 * 0.01;
const double UWBIRIEEE802154APathlossModel::sigma_cluster = 1.883649089; // 2.75 dB

const double UWBIRIEEE802154APathlossModel::fc = 4.492E9; // mandatory band 3, center frequency, Hz
const double UWBIRIEEE802154APathlossModel::BW = 500E6;  //  mandatory band 3, bandwidth, Hz
const double UWBIRIEEE802154APathlossModel::fcMHz = 4492; // mandatory band 3, center frequency, MHz

const double UWBIRIEEE802154APathlossModel::d0 = 1;

    // antenna parameters
const double UWBIRIEEE802154APathlossModel::ntx = 1;
const double UWBIRIEEE802154APathlossModel::nrx = 1;


const bool UWBIRIEEE802154APathlossModel::implemented_CMs[] = {
        		false, //  There is no Channel Model 0: start at 1
        		true,  //  CM1
        		true,  //  CM2
        		true,  //  CM3
        		false, //  CM4 (requires alternate PDP)
        		true,  //  CM5
        		true,  //  CM6
        		true,  //  CM7
        		false,  //  CM8 (alternate PDP)
        		false  //  CM9 (requires alternate PDP)
        };

const UWBIRIEEE802154APathlossModel::CMconfig UWBIRIEEE802154APathlossModel::CMconfigs[] = {
		// CM0
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		// *CM1*
			//PL0, 			n, sigma_s (undefined)
		{	0.000040738, 1.79, 	0,
			//Aant, kappa, Lmean, 	Lambda
			3, 		1.12, 	3, 		0.047E9,
			//lambda_1, lambda_2, 	Beta
			1.54E9, 	0.15E9, 	0.095,
			//Gamma, k_gamma, gamma_0, sigma_cluster,
			22.61E-9, 0, 	12.53E-9, 1.883649089,
			//m_0, k_m, var_m_0, var_k_m, strong_m_0 (undefined)
			0.67, 	0, 	0.28, 		0, 		0,
			//gamma_rise, gamma_1, xi (all undefined)
			0, 0, 0},

		// *CM2*
			//PL0, 			n, sigma_s (undefined)
		{	0.00001349, 4.58, 0,
			//Aant, kappa, Lmean, 	Lambda
			3, 		1.12, 	3.5,	0.12E9,
			//lambda_1, lambda_2, 	Beta
			1.77E9, 	0.15E9, 	0.045,
			//Gamma, k_gamma, gamma_0, sigma_cluster,
			26.27E-9, 0, 	17.5E-9,  1.963360277,
			//m_0, k_m, var_m_0, var_k_m, strong_m_0 (undefined)
			0.69, 	0, 	0.32, 		0, 		0,
			//gamma_rise, gamma_1, xi (all undefined)
			0, 0, 0},

		// *CM3*
			//PL0, 			n, sigma_s (undefined)
		{	0.000218776, 1.63, 1.9,
			//Aant, kappa, Lmean, 	Lambda
			3, 		-3.5, 	5.4,	0.016E9,
			//lambda_1, lambda_2, 	Beta
			0.19E9, 	2.97E9, 	0.0184,
			//Gamma, k_gamma, gamma_0, sigma_cluster (undefined),
			14.6E-9, 0, 	6.4E-9,  0,
			//m_0, k_m, var_m_0, var_k_m, strong_m_0 (undefined)
			0.42, 	0, 	0.31, 		0, 		0,
			//gamma_rise, gamma_1, xi (all undefined)
			0, 0, 0},

		// *CM4*
			//PL0, 			n, sigma_s (undefined)
		{	0.000007244, 3.07, 3.9,
			//Aant, kappa, Lmean, 	Lambda (undefined)
			3, 		-5.3, 	1,	0,
			//lambda_1, lambda_2, 	Beta (all undefined)
			0, 	0, 	0,
			//Gamma, k_gamma, gamma_0, sigma_cluster (all undefined),
			0, 0, 	0,  0,
			//m_0, k_m, var_m_0, var_k_m, strong_m_0 (undefined)
			0.5, 	0, 	0.25, 		0, 		0,
			//gamma_rise, gamma_1, xi
			15.21, 11.84, 0.86},

		// *CM5* Outdoor LOS
			//PL0, 			n, sigma_s
		{	0.000046881, 1.76, 0.83,
			//Aant, kappa, Lmean, 	Lambda
			3, 		-1.6, 	13.6,	0.0048E9,
			//lambda_1, lambda_2, 	Beta
			0.27E9, 	2.41E9, 	0.062,
			//Gamma, k_gamma, gamma_0, sigma_cluster (undefined),
			31.7E-9, 0, 	3.7E-9,  0,
			//m_0, k_m, var_m_0, var_k_m, strong_m_0 (undefined)
			0.77, 	0, 	0.78, 		0, 		0,
			//gamma_rise, gamma_1, xi (all undefined)
			0, 0, 0},

			// *CM6* Outdoor NLOS
				//PL0, 			n, sigma_s
		{	0.000046881, 2.5, 2,
			//Aant, kappa, Lmean, 	Lambda
			3, 		0.4, 	10.5,	0.0243E9,
			//lambda_1, lambda_2, 	Beta
			0.15E9, 	1.13E9, 	0.062,
			//Gamma, k_gamma, gamma_0, sigma_cluster (undefined),
			104.7E-9, 0, 	9.3E-9,  0,
			//m_0, k_m, var_m_0, var_k_m, strong_m_0 (undefined)
			0.56, 	0, 	0.25, 		0, 		0,
			//gamma_rise, gamma_1, xi (all undefined)
			0, 0, 0},

			// *CM7* Open Outdoor NLOS
				//PL0, 			n, sigma_s
		{	0.000012706, 1.58, 3.96,
			//Aant, kappa (?), Lmean, 	Lambda
			3, 		0, 	3.31,	0.0305E9,
			//lambda_1, lambda_2, 	Beta
			0.0225E9, 	0, 	0,
			//Gamma, k_gamma, gamma_0, sigma_cluster (undefined),
			56E-9, 0, 	0.92E-9,  0,
			//m_0, k_m, var_m_0, var_k_m, strong_m_0 (undefined)
			4.1, 	0, 	2.5, 		0, 		0,
			//gamma_rise, gamma_1, xi (all undefined)
			0, 0, 0},

			// *CM8* Industrial LOS
				//PL0, 			n, sigma_s
		{	0.000002138, 1.2, 6,
			//Aant, kappa (?), Lmean, 	Lambda
			3, 		-5.6, 	4.75,	0.0709E9,
			//lambda_1, lambda_2, 	Beta (all undefined)
			0, 	0, 	0,
			//Gamma, k_gamma, gamma_0, sigma_cluster (undefined),
			13.47E-9, 0.926, 	0.651E-9,  4.32,
			//m_0, k_m, var_m_0, var_k_m, strong_m_0
			0.36, 	0, 	1.13, 		0, 		12.99,
			//gamma_rise, gamma_1, xi (all undefined)
			0, 0, 0},

		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}  // CM9
};

bool UWBIRIEEE802154APathlossModel::initFromMap(const ParameterMap& params) {
    ParameterMap::const_iterator it;
    bool                         bInitSuccess = true;

    if ((it = params.find("seed")) != params.end()) {
        srand( ParameterMap::mapped_type(it->second).longValue() );
    }
    if ((it = params.find("CM")) != params.end()) {
        channelModel = ParameterMap::mapped_type(it->second).longValue();
        // Check that this model is supported
        assert(implemented_CMs[channelModel]);
        // load the model parameters
        cfg          = CMconfigs[channelModel];
    }
    else {
        bInitSuccess = false;
        opp_warning("No channel model defined in config.xml for UWBIRIEEE802154APathlossModel!");
    }
    if ((it = params.find("Threshold")) != params.end()) {
        tapThreshold = ParameterMap::mapped_type(it->second).doubleValue();
    }
    else {
        bInitSuccess = false;
        opp_warning("No Threshold defined in config.xml for UWBIRIEEE802154APathlossModel!");
    }
    if ((it = params.find("shadowing")) != params.end()) {
        doShadowing = ParameterMap::mapped_type(it->second).boolValue();
    }
    else {
        doShadowing = false;
    }

    return AnalogueModel::initFromMap(params) && bInitSuccess;
}

void UWBIRIEEE802154APathlossModel::filterSignal(airframe_ptr_t frame, const Coord& sendersPos, const Coord& receiverPos)
{
    Signal& signal = frame->getSignal();
    // We create a new "fake" txPower to add multipath taps
    // and then attenuation is applied to all pulses.

    // (1) Power Delay Profile realization
    using std::max;

    txPower    = signal.getTransmissionPower();
    newTxPower = new TimeMapping<Linear>(); //dynamic_cast<TimeMapping<Linear>*> (txPower->clone()); // create working copy
    pulsesIter = newTxPower->createIterator(); // create an iterator that we will use many times in addEchoes
    // generate number of clusters for this channel (channel coherence time > packet air time)
    L = max(1, poisson(cfg.Lmean));
    // Choose block shadowing
    S = powf(10.,(normal(0, cfg.sigma_s)/10.));

    // Loop on each value of the original mapping and generate multipath echoes
    ConstMappingIterator* iter = txPower->createConstIterator();

    while (iter->inRange()) {
        // generate echoes for each non zero value
        if (iter->getValue() != 0) {
        	// give the pulse start position
            addEchoes(iter->getPosition().getTime() - IEEE802154A::mandatory_pulse/2);
        }
        if (!iter->hasNext()) {
            break;
        }
        iter->next();
    }
    delete iter;
    delete pulsesIter;
    signal.setTransmissionPower(newTxPower);


    // Total radiated power Prx at that distance  [W]
    //double attenuation = 0.5 * ntx * nrx * cfg.PL0 / pow(distance / d0, cfg.n);
    double attenuation = getPathloss(fc, BW, receiverPos.distance(sendersPos));
    pathlosses.record(attenuation);
    // Power intensity I at that distance [W/m²]
    //attenuation = attenuation /(4*PI*pow(distance, cfg.n));
    // create mapping
    SimpleTimeConstMapping* attMapping = new SimpleTimeConstMapping(
    		attenuation, signal.getReceptionStart(), signal.getReceptionEnd());

    signal.addAttenuation(attMapping);

}

void UWBIRIEEE802154APathlossModel::addEchoes(simtime_t_cref pulseStart) {
    using std::map;
    using std::numeric_limits;

    // statistics
    nbCalls = nbCalls + 1;
    double power = 0;
    // loop control variables
    bool moreTaps = true;
    arg.setTime(pulseStart + IEEE802154A::mandatory_pulse/2);
    double pulseEnergy = txPower->getValue(arg);
    if(doShadowing) {
    	pulseEnergy = pulseEnergy - S;
    }
    simtime_t tau_kl = SIMTIME_ZERO;
    //simtime_t fromClusterStart = SIMTIME_ZERO;
    // start time of cluster number "cluster"
    clusterStart = 0;
    gamma_l = cfg.gamma_0;
    Mcluster = normal(0, cfg.sigma_cluster);
    Omega_l = pow(10, Mcluster / 10);
    // tapEnergy values are normalized
    double tapEnergy = sqrt( Omega_l / ( cfg.gamma_0 * ( (1-cfg.Beta)*cfg.lambda_1 + cfg.Beta*cfg.lambda_2 + 1 ) ) );
    // nakagami fading parameters
    //double mfactor = 0;
    //double mmean = 0, msigma = 0;
    //bool firstTap = true;
    simtime_t echoEnd = SIMTIME_ZERO;
    for (int cluster = 0; cluster < L; cluster++) {
        while (moreTaps) {
            // modify newTxPower
            // we add three points for linear interpolation
        	// we set the start and end point before adding the new values for correct interpolation
            double pValueStart = newTxPower->getValue(arg);
            double pValueEnd = newTxPower->getValue(arg);
            simtime_t echoStart = pulseStart + clusterStart + tau_kl;
            echoEnd = pulseStart + clusterStart + tau_kl + IEEE802154A::mandatory_pulse;
            simtime_t echoPeak = pulseStart + clusterStart + tau_kl + IEEE802154A::mandatory_pulse/2;
            arg.setTime(echoEnd);
            newTxPower->setValue(arg, pValueEnd);
            arg.setTime(echoStart);
            newTxPower->setValue(arg, pValueStart);
            bool raising = true;

            /*if(!firstTap) {
            	mfactor = cfg.m_0;
            } else {
            	mmean = cfg.m_0 - cfg.k_m*SIMTIME_DBL(tau_kl);
            	msigma = cfg.var_m_0 - cfg.var_k_m*SIMTIME_DBL(tau_kl);
            	mfactor = normal(mmean, msigma);
            }*/
            /*
            if(doSmallScaleShadowing) {
            	double tmp = sqrt(mfactor*mfactor-mfactor);
            	double riceK = tmp/(mfactor-tmp);
            	//dw;
            	finalTapEnergy = finalTapEnergy * 10^(mfactor/10);
            }
            */
            double finalTapEnergy = tapEnergy * pulseEnergy;

            // We cannot add intermediate points "online" as we need the interpolated
            // values of the signal before adding this echo pulse.
            map<simtime_t, double> intermediatePoints;
            // We need to evaluate all points of the mapping already stored that intersect
            // with this pulse.
            pulsesIter->jumpTo(arg);
            simtime_t currentPoint = echoStart;
            while(currentPoint < echoEnd) {
            	if(raising && pulsesIter->getNextPosition().getTime() < echoPeak) {
            		// there is a point in the first half of the echo
            		// retrieve its value
            		pulsesIter->next();
            		double oldValue = pulsesIter->getValue();
            		currentPoint = pulsesIter->getPosition().getTime();
            		// interpolate current echo point
            		double echoValue = SIMTIME_DBL((currentPoint - echoStart)/(0.5*IEEE802154A::mandatory_pulse));
            		echoValue = echoValue * finalTapEnergy;
            		intermediatePoints[currentPoint] = echoValue + oldValue;
            	} else if(raising && pulsesIter->getNextPosition().getTime() >= echoPeak) {
					// We reached the peak of the pulse.
            		currentPoint = echoPeak;
            		arg.setTime(currentPoint);
            		pulsesIter->jumpTo(arg);
            		double oldValue = pulsesIter->getValue();
            		intermediatePoints[currentPoint] = oldValue + finalTapEnergy;
					raising = false;
            	} else if(!raising && pulsesIter->getNextPosition().getTime() < echoEnd) {
            		// there is a point in the second half of the echo
            		// retrieve its value
            		pulsesIter->next();
            		double oldValue = pulsesIter->getValue();
            		currentPoint = pulsesIter->getPosition().getTime();
            		// interpolate current echo point
            		double echoValue = 1 - SIMTIME_DBL((currentPoint - echoPeak)/(0.5*IEEE802154A::mandatory_pulse));
            		echoValue = echoValue * finalTapEnergy;
            		intermediatePoints[currentPoint] = echoValue + oldValue;
            	} else if (!raising && pulsesIter->getNextPosition().getTime() >= echoEnd) {
            		currentPoint = echoEnd; // nothing to do, we already set this point
            	}
            }

            // Add all points stored in intermediatePoints.
            map<simtime_t, double>::iterator newPointsIter;
            newPointsIter = intermediatePoints.begin();
            while(newPointsIter != intermediatePoints.end()) {
            	arg.setTime(newPointsIter->first);
            	newTxPower->setValue(arg, newPointsIter->second);
            	newPointsIter++;
            }

            power = power + finalTapEnergy; // statistics

            // Update values for next iteration
            double mix1 = exponential(1 / cfg.lambda_1);
            double mix2 = exponential(1 / cfg.lambda_2);
            if(mix1 == numeric_limits<double>::infinity()) {
                mix1 = 0;
            }
            if(mix2 == numeric_limits<double>::infinity()) {
                mix2 = 0;
            }
            tau_kl += mix1 * cfg.Beta + (1 - cfg.Beta) * mix2;
            //fromClusterStart += tau_kl;
            tapEnergy = SIMTIME_DBL(gamma_l) * ((1 - cfg.Beta) * cfg.lambda_1 + cfg.Beta * cfg.lambda_2 + 1);
            tapEnergy = Omega_l / tapEnergy;
            tapEnergy = tapEnergy * exp(-SIMTIME_DBL(tau_kl) / SIMTIME_DBL(gamma_l));
            tapEnergy = sqrt(tapEnergy);
            if (tapEnergy < tapThreshold) {
                moreTaps = false;
            }
        }
        double nextClusterStart = exponential(1 / cfg.Lambda);
        if(nextClusterStart > 0.001 || nextClusterStart == numeric_limits<double>::infinity()) {
          moreTaps = false;
        } else {
          clusterStart = clusterStart + nextClusterStart; // sum(x_n) over n=1..cluster
          gamma_l = cfg.k_gamma * clusterStart + cfg.gamma_0;
          Mcluster = normal(0, cfg.sigma_cluster);
          Omega_l = pow(10, (10 * log( exp( -SIMTIME_DBL(clusterStart) / cfg.Gamma ) ) + Mcluster) / 10);
          // new constraint to increase speed
          if(Omega_l > 0.1) {
            moreTaps = true;
          } else {
        	moreTaps = false;
          }
        }
    }
    arg.setTime(echoEnd);
    newTxPower->setValue(arg, 0);
    averagePower = averagePower + power;
    averagePowers.record( averagePower / ((double) nbCalls));
}

/*
 * Returns the integral of formula (12) assuming constant nrx, ntx over BW
 * and kappa != 0.5
 */
double UWBIRIEEE802154APathlossModel::getPathloss(double /*fc*/, double /*BW*/, double distance) const {
	double pathloss = 0.5; // "antenna attenuation factor"
	pathloss = pathloss * cfg.PL0;  // pathloss at reference distance
	pathloss = pathloss * ntx * nrx; // antenna effects
	pathloss = pathloss / pow(distance/d0, cfg.n); // distance
	/*
	// and frequency dependent effects
	pathloss = pathloss / pow(fc, 2*(cfg.kappa+1));
	pathloss = pathloss / (-2*cfg.kappa-1);
	pathloss = pathloss * ( pow(fc+BW/2,-2*cfg.kappa-1) - pow(fc-BW/2,-2*cfg.kappa-1) );
	*/
	// old version was ignoring frequency dependent effects
	//double attenuation = 0.5 * ntx * nrx * cfg.PL0 / pow(distance / d0, cfg.n);
	return pathloss;
}

double UWBIRIEEE802154APathlossModel::Rayleigh(double param) {
    return weibull(2, sqrt(2.0) * param);
}

