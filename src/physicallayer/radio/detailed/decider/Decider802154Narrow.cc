/*
 * Decider802154Narrow.cc
 *
 *  Created on: 11.02.2009
 *      Author: karl wessel
 */

#include "Decider802154Narrow.h"

#include <cmath>

#include <INETDefs.h>
#define ERFC(x)   erfc(x)
#define MW2DBM(x) (10.0*log10(x))

#include "DeciderResult802154Narrow.h"
#include "PhyToMacControlInfo.h"
#include "DetailedRadioFrame.h"
#include "Mapping.h"

bool Decider802154Narrow::initFromMap(const ParameterMap& params) {
    bool                         bInitSuccess = true;
    ParameterMap::const_iterator it           = params.find("sfdLength");
    if(it != params.end()) {
        sfdLength = ParameterMap::mapped_type(it->second).longValue();
    }
    else {
        bInitSuccess = false;
        opp_warning("No sfdLength defined in config.xml for Decider802154Narrow!");
    }
    it = params.find("berLowerBound");
    if(it != params.end()) {
        BER_LOWER_BOUND = ParameterMap::mapped_type(it->second).doubleValue();
    }
    else {
        bInitSuccess = false;
        opp_warning("No berLowerBound defined in config.xml for Decider802154Narrow!");
    }
    it = params.find("modulation");
    if(it != params.end()) {
        modulation = ParameterMap::mapped_type(it->second).stringValue();
    }
    else {
        bInitSuccess = false;
        opp_warning("No modulation defined in config.xml for Decider802154Narrow!");
    }
    it = params.find("recordStats");
    if(it != params.end()) {
        recordStats = ParameterMap::mapped_type(it->second).boolValue();
    }
    return BaseDecider::initFromMap(params) && bInitSuccess;
}

bool Decider802154Narrow::syncOnSFD(DetailedRadioFrame* frame) const {
	double BER;
	double sfdErrorProbability;

	BER = evalBER(frame);
	sfdErrorProbability = 1.0 - pow((1.0 - BER), sfdLength);

	return sfdErrorProbability < uniform(0, 1, 0);
}

double Decider802154Narrow::evalBER(DetailedRadioFrame* frame) const {
    DetailedRadioSignal&       signal     = frame->getSignal();
	simtime_t     time       = MappingUtils::post(phy->getSimTime());
    Argument      argStart(time);
	double        rcvPower   = signal.getReceivingPower()->getValue(argStart);

	if (rcvPower == Argument::MappedZero) {
	    return 1.0;
	}

	ConstMapping* noise      = calculateRSSIMapping(time, time, frame).first;
	double        noiseLevel = noise->getValue(argStart);
    double        ber        = getBERFromSNR(rcvPower/noiseLevel); //std::max(0.5 * exp(-rcvPower / (2 * noiseLevel)), DEFAULT_BER_LOWER_BOUND);

	delete noise;

    if(recordStats) {
      berlog.record(ber);
      snrlog.record(MW2DBM(rcvPower/noiseLevel));
    }
    return ber;
}

simtime_t Decider802154Narrow::processSignalHeader(DetailedRadioFrame* frame)
{
	if (!syncOnSFD(frame)) {
		currentSignal.finishProcessing();
		return notAgain;
	}

	// store this frame as signal to receive and set state
	setSignalState(frame, getNextSignalState(EXPECT_HEADER));

	//TODO: publish rssi and channel state
	// Inform the MAC that we started receiving a frame
	phy->sendControlMsgToMac(new cMessage("start_rx",RECEPTION_STARTED));

	return frame->getSignal().getReceptionEnd();
}

DeciderResult* Decider802154Narrow::createResult(const DetailedRadioFrame* frame) const
{
	ConstMapping* snrMapping = calculateSnrMapping(frame);
	const DetailedRadioSignal& s          = frame->getSignal();
	simtime_t     start      = s.getReceptionStart();
	simtime_t     end        = s.getReceptionEnd();

	AirFrameVector channel;
	getChannelInfo(start, end, channel);

	double bitrate  = s.getBitrate()->getValue(Argument(start));
	double avgBER   = 0;
	double bestBER  = 0.5;
	double snirAvg  = 0;
	bool   noErrors = true;
	double ber;
	double errorProbability;
	double maxErrProb = 0.0;

	simtime_t             receivingStart = MappingUtils::post(start);
	Argument              argStart(receivingStart);
	ConstMappingIterator* iter    = snrMapping->createConstIterator(argStart);
	double                snirMin = iter->getValue();
	// Evaluate bit errors for each snr value
	// and stops as soon as we have an error.

	simtime_t curTime = iter->getPosition().getTime();
	simtime_t snrDuration;
	while(curTime < end) {
		//get SNR for this interval
		double snr = iter->getValue();

		//determine end of this interval
		simtime_t nextTime = end;	//either the end of the signal...
		if(iter->hasNext()) {		//or the position of the next entry
			const Argument& argNext = iter->getNextPosition();
			nextTime = std::min(argNext.getTime(), nextTime);
			iter->next();	//the iterator will already point to the next entry
		}

		if (noErrors) {
			snrDuration = nextTime - curTime;

			int nbBits = int (SIMTIME_DBL(snrDuration) * bitrate);

			// non-coherent detection of m-ary orthogonal signals in an AWGN
			// Channel
			// Digital Communications, John G. Proakis, section 4.3.2
			// p. 212, (4.3.32)
			//  Pm = sum(n=1,n=M-1){(-1)^(n+1)choose(M-1,n) 1/(n+1) exp(-nkgamma/(n+1))}
			// Pb = 2^(k-1)/(2^k - 1) Pm

			ber     = getBERFromSNR(snr);
			avgBER  = ber*nbBits;
			snirAvg = snirAvg + snr*SIMTIME_DBL(snrDuration);

            if(recordStats) {
              berlog.record(ber);
              snrlog.record(MW2DBM(snr));
            }

			if(ber < bestBER) {
				bestBER = ber;
			}
			errorProbability = 1.0 - pow((1.0 - ber), nbBits);
			noErrors         = errorProbability < uniform(0, 1);
            if(errorProbability > maxErrProb)
                maxErrProb = errorProbability;
		}
		if (snr < snirMin)
			snirMin = snr;

		curTime = nextTime;
	}
	delete iter;
	delete snrMapping;

	avgBER  = avgBER / frame->getBitLength();
	snirAvg = snirAvg / (end - start);

    if(recordStats) {
      snirReceived.record(MW2DBM(snirMin));  // in dB
    }

	return new DeciderResult802154Narrow(noErrors && !frame->hasBitError(), bitrate, snirMin, avgBER, calcChannelSenseRSSI(start, end).first, maxErrProb);
}

double Decider802154Narrow::n_choose_k(int n, int k) {
	if (n < k)
		return 0.0;

	const int       iK     = (k<<1) > n ? n-k : k;
	const double    dNSubK = (n-iK);
	register int    i      = 1;
	register double dRes   = i > iK ? 1.0 : (dNSubK+i);

	for (++i; i <= iK; ++i) {
		dRes *= dNSubK+i;
		dRes /= i;
	}
	return dRes;
}

double Decider802154Narrow::getBERFromSNR(double snr) const {
	double ber = BER_LOWER_BOUND;
	if(modulation == "msk") {
		// valid for IEEE 802.15.4 868 MHz BPSK modulation
		ber = 0.5 *  ERFC(sqrt(snr));
	} else if (modulation == "oqpsk16") {
		// valid for IEEE 802.15.4 2.45 GHz OQPSK modulation
		// Following formula is defined in IEEE 802.15.4 standard, please check the 
		// 2006 standard, page 268, section E.4.1.8 Bit error rate (BER) 
		// calculations, formula 7). Here you cab see that the factor of 20.0 is correct ;).
		const double dSNRFct = 20.0 * snr;
		double       dSumK   = 0;
		register int k       = 2;
		/* following loop was optimized by using n_choose_k symmetries
		for (k=2; k <= 16; ++k) {
			dSumK += pow(-1.0, k) * n_choose_k(16, k) * exp(dSNRFct * (1.0 / k - 1.0));
		}
		*/
		// n_choose_k(16, k) == n_choose_k(16, 16-k)
		for (; k < 8; k += 2) {
			// k will be 2, 4, 6 (symmetric values: 14, 12, 10)
			dSumK += n_choose_k(16, k) * (exp(dSNRFct * (1.0 / k - 1.0)) + exp(dSNRFct * (1.0 / (16 - k) - 1.0)));
		}
		// for k =  8 (which does not have a symmetric value)
		k = 8; dSumK += n_choose_k(16, k) * exp(dSNRFct * (1.0 / k - 1.0));
		for (k = 3; k < 8; k += 2) {
			// k will be 3, 5, 7 (symmetric values: 13, 11, 9)
			dSumK -= n_choose_k(16, k) * (exp(dSNRFct * (1.0 / k - 1.0)) + exp(dSNRFct * (1.0 / (16 - k) - 1.0)));
		}
		// for k = 15 (because of missing k=1 value)
		k   = 15; dSumK -= n_choose_k(16, k) * exp(dSNRFct * (1.0 / k - 1.0));
		// for k = 16 (because of missing k=0 value)
		k   = 16; dSumK += n_choose_k(16, k) * exp(dSNRFct * (1.0 / k - 1.0));
		ber = (8.0 / 15) * (1.0 / 16) * dSumK;
	} else if(modulation == "gfsk") {
		// valid for Bluetooth 4.0 PHY mandatory base rate 1 Mbps
		// Please note that this is not the correct expression for
		// the enhanced data rates (EDR), which uses another modulation.
		ber = 0.5 * ERFC(sqrt(0.5 * snr));
	} else {
		opp_error("The selected modulation is not supported.");
	}
	return std::max(ber, BER_LOWER_BOUND);
}
