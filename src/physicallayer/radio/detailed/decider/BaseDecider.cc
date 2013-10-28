/*
 * BaseDecider.cc
 *
 *  Created on: 24.02.2009
 *      Author: karl
 */

#include "BaseDecider.h"

#include <cassert>

#include "IRadio.h"
#include "DetailedRadioFrame.h"
#include "PhyToMacControlInfo.h"
#include "FWMath.h"

/** @brief Flag for channel sense (channel idle) handling.
 *
 * In the past we checked for IDLE channel state if we do not process a signal (frame).
 * If bUseNewSense is <tt>true</tt> then the channel IDLE state will be done by looking
 * for existing frames on the channel and checking the signal reception time (if it is on air).
 * The new tests uses the RSSI calculation for the check, so that it contains a little bit more
 * calculation time.
 * Maybe this should be done only in RX mode!?
 */
static const bool bUseNewSense = true;

std::size_t BaseDecider::tProcessingSignal::interferenceWith(const first_type& frame) {
    if (frame->getSignal().getReceptionEnd() > busyUntilTime) {
        busyUntilTime = frame->getSignal().getReceptionEnd();
    }
    if (first != frame)
        ++iInterferenceCnt;
    return iInterferenceCnt;
}
void BaseDecider::tProcessingSignal::startProcessing(first_type frame, second_type state) {
    first  = frame;
    second = state;
    if (first != NULL) {
        busyUntilTime    = first->getSignal().getReceptionEnd();
        iInterferenceCnt = 0u;
    }
}

simtime_t BaseDecider::processSignal(DetailedRadioFrame* frame) {
	EV_DEBUG << "Processing AirFrame with ID " << frame->getId() << "..." << endl;

	simtime_t HandleAgain = notAgain;

    if(phy->getNbRadioChannels() > 1 && frame->getChannel() != dynamic_cast<IRadio*>(phy)->getRadioChannel()) {
        // we cannot synchronize on a frame on another channel.
        return notAgain;
    }

    bool bInteruptedProcessing = currentSignal.isProcessing();
    bool bFinishProcessing     = false;

	switch(getSignalState(frame)) {
        case NEW: {
            EV_DEBUG << "... AirFrame processing as NewSignal..." << endl;
            HandleAgain           = processNewSignal(frame);
            bInteruptedProcessing = bInteruptedProcessing && !currentSignal.isProcessing();
        } break;
        case EXPECT_HEADER: {
            EV_DEBUG << "... AirFrame processing as SignalHeader..." << endl;
            HandleAgain           = processSignalHeader(frame);
            bInteruptedProcessing = bInteruptedProcessing && !currentSignal.isProcessing();
        } break;
        case EXPECT_END: {
            EV_DEBUG << "... AirFrame processing as SignalEnd..." << endl;
            bFinishProcessing     = frame == currentSignal.first;
            HandleAgain           = processSignalEnd(frame);
            bFinishProcessing     = bFinishProcessing && bInteruptedProcessing && (HandleAgain < SIMTIME_ZERO);
            bInteruptedProcessing = false;
        } break;
        default: {
            EV_DEBUG << "... AirFrame processing as UnknownSignal..." << endl;
            bFinishProcessing     = frame == currentSignal.first;
            HandleAgain           = processUnknownSignal(frame);
            bFinishProcessing     = bFinishProcessing && bInteruptedProcessing && (HandleAgain < SIMTIME_ZERO);
            bInteruptedProcessing = false;
        } break;
	}

	if (bFinishProcessing || bInteruptedProcessing) {
	    if (!bFinishProcessing && bInteruptedProcessing) {
	        if (currentSignal.getInterferenceCnt() > 0) {
	            ++nbFramesWithInterferencePartial;
	        }
	        else {
	            ++nbFramesWithoutInterferencePartial;
	        }
	    }
	    else if (bFinishProcessing) {
	        if (currentSignal.isProcessing())
	            currentSignal.finishProcessing();
	    }
	}

	// following call is important for channel sense request
	// handling, in the past this call was done in the process...
	// methods, but it shall be called always on processSignal
	// messages
	channelStateChanged();

	return HandleAgain;
}

double BaseDecider::getFrameReceivingPower(DetailedRadioFrame* frame) const {
	// get the receiving power of the Signal at start-time
	//Note: We assume the transmission power is represented by a rectangular function
	//which discontinuities (at start and end of the signal) are represented
	//by two key entries with different values very close to each other (see
	//MappingUtils "addDiscontinuity" method for details). This means
	//the transmission- and therefore also the receiving-power-mapping is still zero
	//at the exact start of the signal and not till one time step after the start its
	//at its actual transmission(/receiving) power.
	//Therefore we use MappingUtils "post"-method to ask for the receiving power
	//at the correct position.
    DetailedRadioSignal&   signal         = frame->getSignal();
	simtime_t receivingStart = MappingUtils::post(signal.getReceptionStart());

	return signal.getReceivingPower()->getValue(Argument(receivingStart));
}

simtime_t BaseDecider::processNewSignal(DetailedRadioFrame* frame) {

	if(currentSignal.isProcessing()) {
		EV_DEBUG << "Already receiving another AirFrame!" << endl;
		currentSignal.interferenceWith(frame);
		return notAgain;
	}

	const bool   bCheckSensitivity = sensitivity > 0.;
	const double recvPower         = bCheckSensitivity ? getFrameReceivingPower(frame) : 0.;

	// check whether signal is strong enough to receive
	if ( bCheckSensitivity && recvPower < sensitivity ) {
		EV_DEBUG << "Signal is to weak (" << recvPower << " < " << sensitivity
				<< ") -> do not receive." << endl;
		// Signal too weak, we can't receive it, tell PhyLayer that we don't want it again
		return notAgain;
	}
	else if (bCheckSensitivity && recvPower != 0) {
		// Signal is strong enough, receive this Signal and schedule it
		EV_DEBUG << "Signal is strong enough (" << recvPower << " > " << sensitivity
				<< ") -> Trying to receive AirFrame." << endl;
	}

	if (!phy->isRadioInRX()) {
        frame->setBitError(true);
        EV_DEBUG << "AirFrame with ID " << frame->getId() << " (" << recvPower << ") received, while not receiving. Setting BitErrors to true." << endl;
	}

	currentSignal.startProcessing(frame, getNextSignalState(NEW));

	return getNextSignalHandleTime(frame);
}

simtime_t BaseDecider::processSignalEnd(DetailedRadioFrame* frame) {
    if (frame != currentSignal.first)
        return notAgain; // it is not the frame which we are processing

	DeciderResult* pResult = createResult(frame);

	if (pResult != NULL && pResult->isSignalCorrect() && !frame->hasBitError()) {
        EV_DEBUG << "AirFrame was received correctly, it is now handed to upper layer..." << endl;
        // go on with processing this AirFrame, send it to the Mac-Layer
        if (currentSignal.getInterferenceCnt() > 0) {
            ++nbFramesWithInterference;
        }
        else {
            ++nbFramesWithoutInterference;
        }
        phy->sendUp(frame, pResult);
    }
	else {
        EV_DEBUG << "AirFrame was not received correctly, sending it as control message to upper layer" << endl;
        cPacket* pMacPacket = frame->decapsulate();
        if (currentSignal.getInterferenceCnt() > 0) {
            ++nbFramesWithInterferenceDropped;
        }
        else {
            ++nbFramesWithoutInterferenceDropped;
        }
        if (pMacPacket) {
            pMacPacket->setName("ERROR");
            pMacPacket->setKind(PACKET_DROPPED);
            if (pResult) {
                PhyToMacControlInfo::setControlInfo(pMacPacket, pResult);
            }
            phy->sendControlMsgToMac(pMacPacket);
        }
    }
    currentSignal.finishProcessing();

	return getNextSignalHandleTime(frame);
}

DeciderResult* BaseDecider::createResult(const DetailedRadioFrame* frame) const
{
    return new DeciderResult(!frame->hasBitError());
}

simtime_t BaseDecider::processUnknownSignal(DetailedRadioFrame* frame)
{
	opp_error("Unknown state for the AirFrame with ID %d", frame->getId());
	return notAgain;
}

ChannelState BaseDecider::getChannelState() const {

	simtime_t            now            = phy->getSimTime();
	channel_sense_rssi_t pairRssiMaxEnd = calcChannelSenseRSSI(now, now);

	return ChannelState(!currentSignal.isProcessing() && (!bUseNewSense || pairRssiMaxEnd.second <= now), pairRssiMaxEnd.first);
}

simtime_t BaseDecider::handleChannelSenseRequest(ChannelSenseRequest* request)
{

	assert(request);

	if (currentChannelSenseRequest.getRequest() == NULL) {
		return handleNewSenseRequest(request);
	}

	if (currentChannelSenseRequest.getRequest() != request) {
		opp_error("Got a new ChannelSenseRequest while already handling another one!");
		return notAgain;
	}
	simtime_t now = phy->getSimTime();

	if(now >= currentChannelSenseRequest.getAnswerTime()) {
	    simtime_t canAnswerAt = canAnswerCSR(currentChannelSenseRequest);

	    if (canAnswerAt > now) {
	        currentChannelSenseRequest.setAnswerTime( canAnswerAt );
	        return currentChannelSenseRequest.getAnswerTime();
	    }
	}
	handleSenseRequestEnd(currentChannelSenseRequest);
	// say that we don't want to have it again
	return notAgain;
}

simtime_t BaseDecider::handleNewSenseRequest(ChannelSenseRequest* request)
{
	// no request handled at the moment, handling the new one
	simtime_t now = phy->getSimTime();

	// saving the pointer to the request and its start-time (now)
	currentChannelSenseRequest.setRequest(request);
	currentChannelSenseRequest.setSenseStart(now);

	//get point in time when we can answer the request (as far as we
	//know at this point in time)
	currentChannelSenseRequest.setAnswerTime( canAnswerCSR(currentChannelSenseRequest) );

	//check if we can already answer the request
	if(now >= currentChannelSenseRequest.getAnswerTime()) {
		answerCSR(currentChannelSenseRequest);
		return notAgain;
	}

	return currentChannelSenseRequest.getAnswerTime();
}

void BaseDecider::channelChanged(int newChannel) {
    currentSignal.clear();

    simtime_t            now            = phy->getSimTime();
    channel_sense_rssi_t pairRssiMaxEnd = calcChannelSenseRSSI(now, now);

    currentSignal.busyUntilTime = pairRssiMaxEnd.second;

    channelStateChanged();
}

void BaseDecider::handleSenseRequestEnd(CSRInfo& requestInfo) {
	//assert(canAnswerCSR(requestInfo) <= phy->getSimTime());
	answerCSR(requestInfo);
}

BaseDecider::eSignalState BaseDecider::getSignalState(const DetailedRadioFrame* frame) const {
	if (frame == currentSignal.first)
		return currentSignal.second;

	return NEW;
}

BaseDecider::eSignalState BaseDecider::setSignalState(const DetailedRadioFrame* frame, BaseDecider::eSignalState newState) {
    if (frame == currentSignal.first) {
        currentSignal.second = newState;
        return currentSignal.second;
    }

    return NEW;
}

simtime_t BaseDecider::getNextSignalHandleTime(const DetailedRadioFrame* frame) const {
    if (frame != currentSignal.first)
        return notAgain;
    switch(getSignalState(frame)) {
        case NEW:           return notAgain; break;
        case EXPECT_HEADER: {
            // we expect the header first, so we must return the time after header is arrived
            const DetailedRadioSignal& FrameSignal = frame->getSignal();
            double        dBitrate    = FrameSignal.getBitrate()->getValue(Argument(FrameSignal.getReceptionStart()));

            assert(dBitrate != 0.0);

            // frame->getBitLength() should store the phy-header-length (@see BasePhyLayer::encapsMsg).
            //simtime_t    tHandleTime  = FrameSignal.getReceptionStart() + (static_cast<double>(frame->getBitLength()) / dBitrate);
            simtime_t    tHandleTime  = FrameSignal.getReceptionStart() + (static_cast<double>(phy->getPhyHeaderLength()) / dBitrate);
            if (tHandleTime < frame->getSignal().getReceptionEnd())
                return tHandleTime;
            return MappingUtils::pre(frame->getSignal().getReceptionEnd());
        } break;
        default: break;
    }
    return frame->getSignal().getReceptionEnd();
}

void BaseDecider::channelStateChanged()
{
	if(!currentChannelSenseRequest.getRequest())
		return;

	//check if the point in time when we can answer the request has changed
	simtime_t canAnswerAt = canAnswerCSR(currentChannelSenseRequest);

	//check if answer time has changed
	if(canAnswerAt != currentChannelSenseRequest.getAnswerTime()) {
		//can we answer it now?
		if(canAnswerAt <= phy->getSimTime()) {
			phy->cancelScheduledMessage(currentChannelSenseRequest.getRequest());
			answerCSR(currentChannelSenseRequest);
		} else {
            currentChannelSenseRequest.setAnswerTime( canAnswerAt );
			phy->rescheduleMessage( currentChannelSenseRequest.getRequest()
			                      , currentChannelSenseRequest.getAnswerTime());
		}
	}
}

simtime_t BaseDecider::canAnswerCSR(const CSRInfo& requestInfo) const
{
	assert(requestInfo.getRequest());

	bool      modeFulfilled = false;
	simtime_t now           = phy->getSimTime();
	simtime_t canAnswerAt   = requestInfo.getSenseStart() + requestInfo.getRequest()->getSenseTimeout();

	switch(requestInfo.getRequest()->getSenseMode())
	{
		case UNTIL_IDLE: {
			modeFulfilled = !currentSignal.isProcessing();
			if (bUseNewSense && modeFulfilled) {
			    modeFulfilled = currentSignal.getBusyEndTime() <= now;
			    if (!modeFulfilled && currentSignal.getBusyEndTime() < canAnswerAt) {
			        canAnswerAt = currentSignal.getBusyEndTime();
			    }
			    EV_DEBUG << "canAnswerCSR(UNTIL_IDLE): busy end time = " << SIMTIME_STR(currentSignal.getBusyEndTime()) << ", now = " << SIMTIME_STR(now) << " isIdle = " << (modeFulfilled ? "true" : "false") << ", canAnswerAt: " << SIMTIME_STR(canAnswerAt) << std::endl;
			}
		} break;
		case UNTIL_BUSY: {
			modeFulfilled = currentSignal.isProcessing();
            if (bUseNewSense && !modeFulfilled) {
                modeFulfilled = currentSignal.getBusyEndTime() > now;
                EV_DEBUG << "canAnswerCSR(UNTIL_BUSY): busy end time = " << SIMTIME_STR(currentSignal.getBusyEndTime()) << ", " << SIMTIME_STR(now) << " isBusy = " << (modeFulfilled ? "true" : "false") << ", canAnswerAt: " << SIMTIME_STR(canAnswerAt) << std::endl;
            }
		} break;
		default: break;
	}

	if(modeFulfilled) {
		return now;
	}

	return canAnswerAt;
}

BaseDecider::channel_sense_rssi_t BaseDecider::calcChannelSenseRSSI(simtime_t_cref start, simtime_t_cref end) const {
    rssi_mapping_t pairMapMaxEnd = calculateRSSIMapping(start, end);

	// the sensed RSSI-value is the maximum value between (and including) the interval-borders
	Mapping::argument_value_t rssi = MappingUtils::findMax(*pairMapMaxEnd.first, Argument(start), Argument(end), Argument::MappedZero /* the value if no maximum will be found */);

	delete pairMapMaxEnd.first;
	return std::make_pair(rssi, pairMapMaxEnd.second);
}

void BaseDecider::answerCSR(CSRInfo& requestInfo)
{
    simtime_t            now            = phy->getSimTime(); // maybe better requestInfo.getAnswerTime()
    channel_sense_rssi_t pairRssiMaxEnd = calcChannelSenseRSSI(requestInfo.getSenseStart(), now);

	// put the sensing-result to the request and
	// send it to the Mac-Layer as Control-message (via Interface)
	requestInfo.getRequest()->setResult( ChannelState(!currentSignal.isProcessing() && (!bUseNewSense || pairRssiMaxEnd.second <= now), pairRssiMaxEnd.first) );

    EV_DEBUG << "answerCSR: channel_sense_rssi_t(" << pairRssiMaxEnd.first << ", " << pairRssiMaxEnd.second << ")@[" << SIMTIME_STR(requestInfo.getSenseStart()) << ", " << SIMTIME_STR(now) << "] ChannelState(" << requestInfo.getRequest()->getResult().isIdle() << ", " << requestInfo.getRequest()->getResult().getRSSI() << ")" << std::endl;

	phy->sendControlMsgToMac(requestInfo.getRequest());
	requestInfo.clear();
}

Mapping* BaseDecider::calculateSnrMapping(const DetailedRadioFrame* frame) const
{
	/* calculate Noise-Strength-Mapping */
	const DetailedRadioSignal& signal = frame->getSignal();

	simtime_t     start = signal.getReceptionStart();
	simtime_t     end   = signal.getReceptionEnd();

	Mapping*                  noiseMap     = calculateRSSIMapping(start, end, frame).first;
	const ConstMapping *const recvPowerMap = signal.getReceivingPower();
    assert(noiseMap);
	assert(recvPowerMap);

	//TODO: handle noise of zero (must not devide with zero!)
	Mapping* snrMap = MappingUtils::divide( *recvPowerMap, *noiseMap, Argument::MappedZero );

	delete noiseMap;
	noiseMap = NULL;

	return snrMap;
}

void BaseDecider::getChannelInfo( simtime_t_cref  start
                                , simtime_t_cref  end
                                , AirFrameVector& out) const
{
	phy->getChannelInfo(start, end, out);
}

BaseDecider::rssi_mapping_t
BaseDecider::calculateRSSIMapping( simtime_t_cref       start,
                                   simtime_t_cref       end,
                                   const DetailedRadioFrame* exclude ) const
{
	if(exclude) {
		EV_DEBUG << "Creating RSSI map for range [" << SIMTIME_STR(start) << "," << SIMTIME_STR(end) << "] excluding AirFrame with id " << exclude->getId() << endl;
	}
	else {
		EV_DEBUG << "Creating RSSI map for range [" << SIMTIME_STR(start) << "," << SIMTIME_STR(end) << "]" << endl;
	}

	AirFrameVector airFrames;
	simtime_t      MaxReceptionEnd = notAgain;

	// collect all AirFrames that intersect with [start, end]
	getChannelInfo(start, end, airFrames);

	// create an empty mapping
	Mapping* resultMap = MappingUtils::createMapping(Argument::MappedZero, DimensionSet::timeDomain);

	//add thermal noise
	ConstMapping* thermalNoise = phy->getThermalNoise(start, end);
	if(thermalNoise) {
		Mapping* tmp = resultMap;
		resultMap = MappingUtils::add(*resultMap, *thermalNoise);
		delete tmp;
	}

	// otherwise, iterate over all AirFrames (except exclude)
	// and sum up their receiving-power-mappings
	for (AirFrameVector::const_iterator it = airFrames.begin(); it != airFrames.end(); ++it) {
		// the vector should not contain pointers to 0
		assert (*it != 0);

        simtime_t ReceptionEnd = (*it)->getSignal().getReceptionEnd();
        if (ReceptionEnd > MaxReceptionEnd) {
            MaxReceptionEnd = ReceptionEnd;
        }
		// if iterator points to exclude (that includes the default-case 'exclude == 0')
		// then skip this AirFrame
		if ( *it == exclude ) {
			if (thermalNoise && exclude) {
				// suggested by David Eckhoff:
				// Instead of ignoring the want-to-receive AirFrame when
				// building up the NoiseMap i add the thermalNoise for the time and
				// frequencies of this airframe. There's probably a better way to do it but
				// i did it like this:
				const ConstMapping *const recvPowerMap = (*it)->getSignal().getReceivingPower();

				if (recvPowerMap) {
                    EV_DEBUG << "Adding mapping of Airframe with ID " << (*it)->getId()
                              << ". Starts at "  << SIMTIME_STR((*it)->getSignal().getReceptionStart())
                              << " and ends at " << SIMTIME_STR((*it)->getSignal().getReceptionEnd()) << endl;

					Mapping* rcvPowerPlusThermalNoise = MappingUtils::add(      *recvPowerMap,             *thermalNoise );
					Mapping* resultMapTmp             = MappingUtils::subtract( *rcvPowerPlusThermalNoise, *recvPowerMap );
					Mapping* resultMapNew             = MappingUtils::add(      *resultMap,                *resultMapTmp );

					delete rcvPowerPlusThermalNoise;
					delete resultMapTmp;

					delete resultMap;
					resultMap    = resultMapNew;
					resultMapNew = NULL;
				}
			}
			continue;
		}

		// otherwise get the Signal and its receiving-power-mapping
		DetailedRadioSignal& signal = (*it)->getSignal();

		// backup pointer to result map
		// Mapping* resultMapOld = resultMap;

		// add the Signal's receiving-power-mapping to resultMap in [start, end],
		// the operation Mapping::add returns a pointer to a new Mapping

		const ConstMapping *const recvPowerMap = signal.getReceivingPower();
		assert(recvPowerMap);

		EV_DEBUG << "Adding mapping of Airframe with ID " << (*it)->getId()
		          << ". Starts at "  << SIMTIME_STR(signal.getReceptionStart())
		          << " and ends at " << SIMTIME_STR(signal.getReceptionEnd()) << endl;

		Mapping* resultMapNew = MappingUtils::add( *recvPowerMap, *resultMap, Argument::MappedZero );

		// discard old mapping
		delete resultMap;
		resultMap    = resultMapNew;
		resultMapNew = NULL;
	}

	return std::make_pair(resultMap, MaxReceptionEnd);
}

void BaseDecider::finish()
{
    if (phy) {
        // record scalars through the interface to the PHY-Layer
        phy->recordScalar("nbFramesWithInterference"          , nbFramesWithInterference);
        phy->recordScalar("nbFramesWithoutInterference"       , nbFramesWithoutInterference);
        phy->recordScalar("nbFramesWithInterferencePartial"   , nbFramesWithInterferencePartial);
        phy->recordScalar("nbFramesWithoutInterferencePartial", nbFramesWithoutInterferencePartial);
        phy->recordScalar("nbFramesWithInterferenceDropped"   , nbFramesWithInterferenceDropped);
        phy->recordScalar("nbFramesWithoutInterferenceDropped", nbFramesWithoutInterferenceDropped);
    }
    Decider::finish();
}
