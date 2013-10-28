/*
 * Decider80211.cc
 *
 *  Created on: 11.02.2009
 *      Author: karl wessel
 */

#include "./Decider80211.h"

#include <cassert>

#include <INETDefs.h>
#define ERFC(x) erfc(x)

#include "IRadio.h"
#include "DeciderResult80211.h"
#include "Ieee80211Consts.h"
#include "Mapping.h"
#include "DetailedRadioFrame.h"

Decider80211::Decider80211( DeciderToPhyInterface* phy
                          , double                 sensitivity
                          , int                    myIndex)
    : BaseDecider(phy, sensitivity, myIndex)
    , snrThreshold(0)
    , centerFrequency(0)
{
    int radioChannel = dynamic_cast<IRadio*>(phy)->getRadioChannel();
	assert(1 <= radioChannel);
	assert(radioChannel <= 14);
	centerFrequency = CENTER_FREQUENCIES[radioChannel];
}

bool Decider80211::initFromMap(const ParameterMap& params) {
    ParameterMap::const_iterator it           = params.find("threshold");
    bool                         bInitSuccess = true;
    if(it != params.end()) {
        snrThreshold = ParameterMap::mapped_type(it->second).doubleValue();
    }
    else {
        bInitSuccess = false;
        opp_warning("No threshold defined in config.xml for Decider80211!");
    }
    return BaseDecider::initFromMap(params) && bInitSuccess;
}

double Decider80211::getFrameReceivingPower(DetailedRadioFrame* frame) const
{
	// get the receiving power of the Signal at start-time and center frequency
    DetailedRadioSignal& signal = frame->getSignal();
	Argument argStart(DimensionSet::timeFreqDomain);

	argStart.setTime(MappingUtils::post(signal.getReceptionStart()));
	argStart.setArgValue(Dimension::frequency, centerFrequency);

	return signal.getReceivingPower()->getValue(argStart);
}

BaseDecider::channel_sense_rssi_t Decider80211::calcChannelSenseRSSI(simtime_t_cref start, simtime_t_cref end) const {
    rssi_mapping_t pairMapMaxEnd = calculateRSSIMapping(start, end);
	Argument       argMin(getLowerBandFrequency(start));
	Argument       argMax(getUpperBandFrequency(end));

	Mapping::argument_value_t rssi = MappingUtils::findMax(*pairMapMaxEnd.first, argMin, argMax, Argument::MappedZero /* the value if no maximum will be found */);

	delete pairMapMaxEnd.first;
	return std::make_pair(rssi, pairMapMaxEnd.second);
}

DeciderResult* Decider80211::createResult(const DetailedRadioFrame* frame) const
{
	// check if the snrMapping is above the Decider's specific threshold,
	// i.e. the Decider has received it correctly

	// first collect all necessary information
	Mapping* snrMap = calculateSnrMapping(frame);
	assert(snrMap);

	const DetailedRadioSignal& s     = frame->getSignal();
	simtime_t     start = s.getReceptionStart();
	simtime_t     end   = s.getReceptionEnd();

	start = start + RED_PHY_HEADER_DURATION; //its ok if the phy header is received only
											 //partly - TODO: maybe solve this nicer

	Argument argMin(getLowerBandFrequency(start));
	Argument argMax(getUpperBandFrequency(end));

	Mapping::argument_value_t snirMin = MappingUtils::findMin(*snrMap, argMin, argMax, Argument::MappedZero /* the value if no minimum will be found */);

	EV_DEBUG << " snrMin: " << snirMin << endl;

	ConstMappingIterator* bitrateIt = s.getBitrate()->createConstIterator();
	bitrateIt->next(); //iterate to payload bitrate indicator
	double payloadBitrate = bitrateIt->getValue();
	delete bitrateIt;

	DeciderResult80211* result = NULL;

	if (snirMin > snrThreshold) {
		if(packetOk(snirMin, frame->getBitLength() - (int)PHY_HEADER_LENGTH, payloadBitrate)) {
			result = new DeciderResult80211(!frame->hasBitError(), payloadBitrate, snirMin);
		} else {
			EV_DEBUG << "Packet has BIT ERRORS! It is lost!\n";
			result = new DeciderResult80211(false, payloadBitrate, snirMin);
		}
	} else {
		EV_DEBUG << "Packet has ERRORS! It is lost!\n";
		result = new DeciderResult80211(false, payloadBitrate, snirMin);
	}

	delete snrMap;
	snrMap = 0;

	return result;
}

bool Decider80211::packetOk(double snirMin, int lengthMPDU, double bitrate) const
{
    double berHeader, berMPDU;

    berHeader = 0.5 * exp(-snirMin * BANDWIDTH / BITRATE_HEADER);
    //if PSK modulation
    if (bitrate == 1E+6 || bitrate == 2E+6) {
        berMPDU = 0.5 * exp(-snirMin * BANDWIDTH / bitrate);
    }
    //if CCK modulation (modeled with 16-QAM)
    else if (bitrate == 5.5E+6) {
        berMPDU = 2.0 * (1.0 - 1.0 / sqrt(pow(2.0, 4))) * ERFC(sqrt(2.0*snirMin * BANDWIDTH / bitrate));
    }
    else {                       // CCK, modelled with 256-QAM
        berMPDU = 2.0 * (1.0 - 1.0 / sqrt(pow(2.0, 8))) * ERFC(sqrt(2.0*snirMin * BANDWIDTH / bitrate));
    }

    //probability of no bit error in the PLCP header
    double headerNoError = pow(1.0 - berHeader, HEADER_WITHOUT_PREAMBLE);

    //probability of no bit error in the MPDU
    double MpduNoError = pow(1.0 - berMPDU, lengthMPDU);
    EV_DEBUG << "berHeader: " << berHeader << " berMPDU: " << berMPDU << endl;
    double rand = dblrand();

    //if error in header
    if (rand > headerNoError)
        return (false);
    else
    {
        rand = dblrand();

        //if error in MPDU
        if (rand > MpduNoError)
            return (false);
        //if no error
        else
            return (true);
    }
}
