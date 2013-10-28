#include "SNRThresholdDecider.h"

#include <cassert>

#include "DetailedRadioFrame.h"
#include "Mapping.h"

bool SNRThresholdDecider::initFromMap(const ParameterMap& params) {
    ParameterMap::const_iterator it           = params.find("snrThreshold");
    bool                         bInitSuccess = true;
    if (it != params.end()) {
        snrThreshold = ParameterMap::mapped_type(it->second).doubleValue();
    }
    else if ((it = params.find("threshold")) != params.end()) {
        snrThreshold = ParameterMap::mapped_type(it->second).doubleValue();
    }
    else {
        bInitSuccess = false;
        opp_warning("No threshold defined in config.xml for Decider80211!");
    }
    if ((it = params.find("busyThreshold")) != params.end()) {
        busyThreshold = ParameterMap::mapped_type(it->second).doubleValue();
    }
    else {
        EV_DEBUG << "No busy threshold defined for SNRThresholdDecider. Using"
                  << " phy layers sensitivity as busy threshold."      << endl;
    }
    return BaseDecider::initFromMap(params) && bInitSuccess;
}

// TODO: for now we check a larger mapping within an interval
bool SNRThresholdDecider::checkIfAboveThreshold(Mapping* map, simtime_t_cref start, simtime_t_cref end) const
{
	assert(map);

    EV_DEBUG << "Checking if SNR is above Threshold of " << snrThreshold << endl;

	// check every entry in the mapping against threshold value
	ConstMappingIterator* it = map->createConstIterator(Argument(start));
	// check if values at start-time fulfill snrThreshold-criterion
    EV_DEBUG << "SNR at time " << start << " is " << it->getValue() << endl;
	if ( it->getValue() <= snrThreshold ){
		delete it;
		return false;
	}

	while ( it->hasNext() && it->getNextPosition().getTime() < end)
	{
		it->next();

        EV_DEBUG << "SNR at time " << it->getPosition().getTime() << " is " << it->getValue() << endl;

		// perform the check for smaller entry
		if ( it->getValue() <= snrThreshold) {
			delete it;
			return false;
		}
	}

	it->iterateTo(Argument(end));
    EV_DEBUG << "SNR at time " << end << " is " << it->getValue() << endl;

	if ( it->getValue() <= snrThreshold ){
		delete it;
		return false;
	}

	delete it;
	return true;
}

ChannelState SNRThresholdDecider::getChannelState() const
{
	ChannelState csBase = BaseDecider::getChannelState();

	return ChannelState(/*csBase.isIdle() &&*/ isIdleRSSI(csBase.getRSSI()), csBase.getRSSI());
}

void SNRThresholdDecider::answerCSR(CSRInfo& requestInfo)
{
	// put the sensing-result to the request and
	// send it to the Mac-Layer as Control-message (via Interface)
	requestInfo.getRequest()->setResult( getChannelState() );
	phy->sendControlMsgToMac(requestInfo.getRequest());

	requestInfo.clear();
}

simtime_t SNRThresholdDecider::canAnswerCSR(const CSRInfo& requestInfo) const {
	const ChannelSenseRequest* request = requestInfo.getRequest();
	assert(request);

	if(request->getSenseMode() == UNTIL_TIMEOUT) {
		opp_error("SNRThresholdDecider received an UNTIL_TIMEOUT ChannelSenseRequest.\n"
				  "SNRThresholdDecider can only handle UNTIL_IDLE or UNTIL_BUSY requests because it "
				  "implements only instantaneous sensing where UNTIL_TIMEOUT requests "
				  "don't make sense. Please refer to ChannelSenseRequests documentation "
				  "for details.");
	}

	simtime_t requestTimeout = requestInfo.getSenseStart() + request->getSenseTimeout();

	simtime_t now = phy->getSimTime();

	ConstMapping* rssiMapping = calculateRSSIMapping(now, requestTimeout).first;

	//this Decider only works for time-only signals
	assert(rssiMapping->getDimensionSet() == DimensionSet::timeDomain);

	ConstMappingIterator* it = rssiMapping->createConstIterator(Argument(now));

	assert(request->getSenseMode() == UNTIL_IDLE
		   || request->getSenseMode() == UNTIL_BUSY);
	bool untilIdle = request->getSenseMode() == UNTIL_IDLE;

	simtime_t answerTime = requestTimeout;
	//check if the current rssi value enables us to answer the request
	if(isIdleRSSI(it->getValue()) == untilIdle) {
		answerTime = now;
	}
	else {
		//iterate through request interval to check when the rssi level
		//changes such that the request can be answered
		while(it->hasNext() && it->getNextPosition().getTime() < requestTimeout) {
			it->next();

			if(isIdleRSSI(it->getValue()) == untilIdle) {
				answerTime = it->getPosition().getTime();
				break;
			}
		}
	}

	delete it;
	delete rssiMapping;

	return answerTime;
}

DeciderResult* SNRThresholdDecider::createResult(const DetailedRadioFrame* frame) const
{
	// first collect all necessary information
	Mapping* snrMap = calculateSnrMapping(frame);
	assert(snrMap);

	const DetailedRadioSignal& signal = frame->getSignal();

	// NOTE: Since this decider does not consider the amount of time when the signal's SNR is
	// below the threshold even the smallest (normally insignificant) drop causes this decider
	// to reject reception of the signal.
	// Since the default MiXiM-signal is still zero at its exact start and end, these points
	// are ignored in the interval passed to the following method.
	bool aboveThreshold = checkIfAboveThreshold(snrMap,
	                                            MappingUtils::post(signal.getReceptionStart()),
	                                            MappingUtils::pre(signal.getReceptionEnd()));

	delete snrMap; snrMap = NULL;

	// check if the snrMapping is above the Decider's specific threshold,
	// i.e. the Decider has received it correctly
	if (aboveThreshold) {
		EV_DEBUG << "SNR is above threshold("<<snrThreshold<<") -> sending up." << endl;
	}
	else {
		EV_DEBUG << "SNR is below threshold("<<snrThreshold<<") -> dropped." << endl;
	}

	return new DeciderResult(aboveThreshold && !frame->hasBitError());
}

