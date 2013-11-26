#include "DeciderUWBIRED.h"

#include "PhyLayerUWBIR.h"
#include "AirFrameUWBIR_m.h"
#include "DeciderResultUWBIR.h"
#include "MiXiMAirFrame.h"

using std::map;
using std::vector;
using std::pair;

const double          DeciderUWBIRED::noiseVariance        = 101.085E-12; // P=-116.9 dBW // 404.34E-12;   v²=s²=4kb T R B (T=293 K)
const double          DeciderUWBIRED::peakPulsePower       = 1.3E-3; //1.3E-3 W peak power of pulse to reach  0dBm during burst; // peak instantaneous power of the transmitted pulse (A=0.6V) : 7E-3 W. But peak limit is 0 dBm
const simsignalwrap_t DeciderUWBIRED::catUWBIRPacketSignal = simsignalwrap_t(MIXIM_SIGNAL_UWBIRPACKET_NAME);

DeciderUWBIRED::DeciderUWBIRED( DeciderToPhyInterface* phy
              , double                 sensitivity
              , int                    myIndex
              , bool                   debug )
    : BaseDecider(phy, sensitivity, myIndex, debug)
    , trace(false), stats(false)
    , nbFailedSyncs(0), nbSuccessfulSyncs(0)
    , nbSymbols(0), allThresholds(0)
    , vsignal2(0), pulseSnrs(0)
    , syncThreshold(false)
    , syncAlwaysSucceeds(false)
    , alwaysFailOnDataInterference(true)
    , packet()
    , receivedPulses()
    , syncThresholds()
    , uwbiface(dynamic_cast<PhyLayerUWBIR*>(phy))
    , nbCancelReceptions(0)
    , nbFinishTrackingFrames(0)
{
    assert(uwbiface != NULL);
    receivedPulses.setName("receivedPulses");
    syncThresholds.setName("syncThresholds");
}

bool DeciderUWBIRED::initFromMap(const ParameterMap& params) {
    bool                         bInitSuccess = true;
    ParameterMap::const_iterator it           = params.find("syncThreshold");
    if(it != params.end()) {
        syncThreshold = ParameterMap::mapped_type(it->second).doubleValue();
    }
    else {
        bInitSuccess = false;
        opp_warning("No syncThreshold defined in config.xml for DeciderUWBIREDSync!");
    }
    it = params.find("stats");
    if(it != params.end()) {
        stats = ParameterMap::mapped_type(it->second).boolValue();
    }
    else {
        bInitSuccess = false;
        opp_warning("No stats defined in config.xml for DeciderUWBIREDSync!");
    }
    it = params.find("trace");
    if(it != params.end()) {
        trace = ParameterMap::mapped_type(it->second).boolValue();
    }
    else {
        bInitSuccess = false;
        opp_warning("No trace defined in config.xml for DeciderUWBIREDSync!");
    }
    it = params.find("syncAlwaysSucceeds");
    if(it != params.end()) {
        syncAlwaysSucceeds = ParameterMap::mapped_type(it->second).boolValue();
    }
    else {
        bInitSuccess = false;
        opp_warning("No syncAlwaysSucceeds defined in config.xml for DeciderUWBIREDSync!");
    }
    it = params.find("alwaysFailOnDataInterference");
    if(it != params.end()) {
        alwaysFailOnDataInterference = ParameterMap::mapped_type(it->second).boolValue();
    }
    else {
        alwaysFailOnDataInterference = false;
    }

    catUWBIRPacketSignal.initialize();

    return BaseDecider::initFromMap(params) && bInitSuccess;
}

void DeciderUWBIRED::cancelProcessSignal() {
	BaseDecider::cancelProcessSignal();
	++nbCancelReceptions;
}

simtime_t DeciderUWBIRED::processSignalHeader(airframe_ptr_t frame) {

	int currState = uwbiface->getRadioState();

	if (currentSignal.isProcessing() && currState == RadioUWBIR::SYNC) {
		// We are not tracking a signal currently.
		// Can we synchronize on this one ?

		bool isSyncSignalHigherThanThreshold;
		if(syncAlwaysSucceeds) {
			isSyncSignalHigherThanThreshold = true;
		} else {
			isSyncSignalHigherThanThreshold = attemptSync(frame);
		}

		packet.setNbSyncAttempts(packet.getNbSyncAttempts() + 1);

		if(isSyncSignalHigherThanThreshold) {
			++nbSuccessfulSyncs;
			uwbiface->switchRadioToRX();
			packet.setNbSyncSuccesses(packet.getNbSyncSuccesses() + 1);
			// notify MAC layer through PHY layer
			cMessage* syncSuccessfulNotification = new cMessage("Ctrl_PHY2MAC_Sync_Success", SYNC_SUCCESS);
			phy->sendControlMsgToMac(syncSuccessfulNotification);

			// in any case, look at that frame again when it is finished
			// store this frame as signal to receive and set state
			setSignalState(frame, getNextSignalState(EXPECT_HEADER));
		} else {
			++nbFailedSyncs;
			cMessage* syncFailureNotification = new cMessage("Ctrl_PHY2MAC_Sync_Failure", SYNC_FAILURE);
			phy->sendControlMsgToMac(syncFailureNotification);

			currentSignal.finishProcessing();
		}
		uwbiface->emit(catUWBIRPacketSignal, &packet);

	}

	return frame->getSignal().getReceptionEnd();
}


simtime_t DeciderUWBIRED::getNextSignalHandleTime(const airframe_ptr_t frame) const {
    if (frame != currentSignal.first)
        return BaseDecider::getNextSignalHandleTime(frame);

    switch(getSignalState(frame)) {
        case NEW:           return BaseDecider::getNextSignalHandleTime(frame); break;
        case EXPECT_HEADER: {
            simtime_t tHandleTime = frame->getSignal().getReceptionStart() + IEEE802154A::mandatory_preambleLength;
            if (tHandleTime < frame->getSignal().getReceptionEnd())
                return tHandleTime;
            return MappingUtils::pre(frame->getSignal().getReceptionEnd());
        } break;
        default: break;
    }
    return BaseDecider::getNextSignalHandleTime(frame);
}

bool DeciderUWBIRED::attemptSync(const airframe_ptr_t frame) {
	if (currentSignal.getInterferenceCnt() > 0)
		return false; // do not accept interferers

	double snrValue;
	const ConstMapping *const power = frame->getSignal().getReceivingPower();
	ConstMappingIterator*     mIt   = power->createConstIterator();

	AirFrameVector syncVector;
	// Retrieve all potentially colliding airFrames
	getChannelInfo(frame->getSignal().getReceptionStart(), simTime(), syncVector);

	if(syncVector.size() > 1) {
		// do not accept interferers
		return false;
	}
	Argument posFirstPulse(IEEE802154A::tFirstSyncPulseMax + frame->getSignal().getReceptionStart());
	mIt->jumpTo(posFirstPulse);
	snrValue = fabs(mIt->getValue()/getNoiseValue());
	syncThresholds.record(snrValue);
	if(snrValue > syncThreshold) {
		return true;
	} else {
		return false;
	}
}

DeciderResult* DeciderUWBIRED::createResult(const airframe_ptr_t frame) const {
	if (currentSignal.first == frame) {
		++nbFinishTrackingFrames;
		vector<bool>*           receivedBits   = new vector<bool>();
		AirFrameUWBIR*          frameuwb       = check_and_cast<AirFrameUWBIR*>(frame);
		std::pair<bool, double> pairCorrectSnr = decodePacket(frame, receivedBits, frameuwb->getCfg());
		// we cannot compute bit error rate here
		// so we send the packet to the MAC layer which will compare receivedBits
		// with the actual bits sent (stored in the encapsulated UWBIRMacPkt object).
		DeciderResultUWBIR * result = new DeciderResultUWBIR(pairCorrectSnr.first && !frame->hasBitError(), receivedBits, pairCorrectSnr.second);

		uwbiface->switchRadioToSync();
		return result;
	}
	return NULL;
}

/*
 * @brief Returns false if the packet is incorrect. If true,
 * the MAC layer must still compare bit values to validate the frame.
 */
std::pair<bool, double> DeciderUWBIRED::decodePacket(const airframe_ptr_t frame, vector<bool> * receivedBits, const IEEE802154A::config& cfg) const {

	simtime_t now, offset;
	simtime_t aSymbol, shift, burst;

	double packetSNIR    = 0;

	const Signal&              FrameSignal = frame->getSignal();
	const ConstMapping*        signalPower = NULL;
	AirFrameVector             airFrameVector;
	// Retrieve all potentially colliding airFrames
	getChannelInfo(FrameSignal.getReceptionStart(), FrameSignal.getReceptionEnd(), airFrameVector);

	for (AirFrameVector::const_iterator airFrameIter = airFrameVector.begin(); airFrameIter != airFrameVector.end(); ++airFrameIter) {
		Signal&                   aSignal   = (*airFrameIter)->getSignal();
		const ConstMapping *const currPower = aSignal.getReceivingPower();
		if (  aSignal.getReceptionStart() == FrameSignal.getReceptionStart()
		   && aSignal.getDuration()       == FrameSignal.getDuration()) {
			signalPower = currPower;
		}
	}

	// times are absolute
	offset  = FrameSignal.getReceptionStart() + cfg.preambleLength;
	shift   = cfg.shift_duration;
	aSymbol = cfg.data_symbol_duration;
	burst   = cfg.burst_duration;
	now     = offset + cfg.pulse_duration / 2;
	std::pair<double, double> energyZero, energyOne;
	IEEE802154A::setConfig(cfg);

	// debugging information (start)
	if (trace && signalPower != NULL) {
		ConstMappingIterator *const mIt = signalPower->createConstIterator();
		while(mIt->inRange()){
			receivedPulses.recordWithTimestamp(mIt->getPosition().getTime(), mIt->getValue());

			if(!mIt->hasNext())
				break;

			mIt->next();
		}
		/*mIt->jumpToBegin();
		while (mIt->hasNext()) {
			receivedPulses.recordWithTimestamp(mIt->getPosition().getTime(), mIt->getValue());
			mIt->next();
		}*/
		delete mIt;
	}
	// debugging information (end)

	int symbol;
	// Loop to decode each bit value
	for (symbol = 0; cfg.preambleLength + symbol * aSymbol < FrameSignal.getDuration(); symbol++) {

//		int hoppingPos = IEEE802154A::getHoppingPos(symbol);
		int decodedBit;

		if (stats) {
			nbSymbols = nbSymbols + 1;
		}

		// sample in window zero
		now = now + IEEE802154A::getHoppingPos(symbol)*cfg.burst_duration;
		energyZero = integrateWindow(symbol, now, burst, airFrameVector, signalPower, frame, cfg);
		// sample in window one
		now = now + shift;
		energyOne  = integrateWindow(symbol, now, burst, airFrameVector, signalPower, frame, cfg);

		if (energyZero.second > energyOne.second) {
		  decodedBit = 0;
		  packetSNIR = packetSNIR + energyZero.first;
	    } else {
	      decodedBit = 1;
		  packetSNIR = packetSNIR + energyOne.first;
	    }


		receivedBits->push_back(static_cast<bool>(decodedBit));
		//packetSamples = packetSamples + 16; // 16 EbN0 evaluations per bit

		now = offset + (symbol + 1) * aSymbol + cfg.pulse_duration / 2;

	}
	symbol = symbol + 1;

	bool isCorrect = true;
	if(airFrameVector.size() > 1 && alwaysFailOnDataInterference) {
		isCorrect = false;
	}
	packetSNIR = packetSNIR / symbol;

	double snrLastPacket = 10*log10(packetSNIR);  // convert to dB
	airFrameVector.clear();

	return std::make_pair(isCorrect, snrLastPacket);
}

/*
 * @brief Returns a pair with as first value the SNIR (if the signal is not nul in this window, and 0 otherwise)
 * and as second value a "score" associated to this window. This score is equals to the sum for all
 * 16 pulse peak positions of the voltage measured by the receiver ADC.
 */
pair<double, double> DeciderUWBIRED::integrateWindow( int                        /*symbol*/
                                                    , simtime_t_cref             pNow
                                                    , simtime_t_cref             burst
                                                    , const AirFrameVector&      airFrameVector
                                                    , const ConstMapping *const  signalPower
                                                    , const airframe_ptr_t       /*frame*/
                                                    , const IEEE802154A::config& cfg) {
	std::pair<double, double>       energy = std::make_pair(0.0, 0.0); // first: stores SNIR, second: stores total captured window energy
	vector<ConstMapping*>::iterator mappingIter;
	Argument                        arg;
	simtime_t                       windowEnd = pNow + burst;

	// Triangular baseband pulses
	// we sample at each pulse peak
	// get the interpolated values of amplitude for each interferer
	// and add these to the peak with a random phase

	// we sample one point per pulse
	// caller has already set our time reference ("now") at the peak of the pulse
	for (simtime_t now = pNow; now < windowEnd; now += cfg.pulse_duration) {
		double signalValue      = 0; // electric field from tracked signal [V/m²]
		double resPower         = 0; // electric field at antenna = combination of all arriving electric fields [V/m²]
		double vEfield          = 0; // voltage at antenna caused by electric field Efield [V]
		double vmeasured        = 0; // voltage measured by energy-detector [V], including thermal noise
		double vmeasured_square = 0; // to the square [V²]
		double snir             = 0; // burst SNIR estimate
		double vThermalNoise    = 0; // thermal noise realization
		int    currSig          = 0;

		arg.setTime(now);
		// consider all interferers at this point in time
		for (AirFrameVector::const_iterator airFrameIter = airFrameVector.begin(); airFrameIter != airFrameVector.end(); ++airFrameIter) {
			Signal&                   aSignal   = (*airFrameIter)->getSignal();
			const ConstMapping *const currPower = aSignal.getReceivingPower();
			double                    measure   = currPower->getValue(arg)*peakPulsePower; //TODO: de-normalize (peakPulsePower should be in AirFrame or in Signal, to be set at run-time)
//			measure = measure * uniform(0, +1); // random point of Efield at sampling (due to pulse waveform and self interference)
			if (currPower == signalPower) {
				signalValue = measure*0.5; // we capture half of the maximum possible pulse energy to account for self  interference
				resPower    = resPower + signalValue;
			}
			else {
				// take a random point within pulse envelope for interferer
				resPower = resPower + measure * uniform(-1, +1);
			}
			++currSig;
		}

//		double attenuatedPower = resPower / 10; // 10 dB = 6 dB implementation loss + 5 dB noise factor
		vEfield          = sqrt(50*resPower); // P=V²/R
		// add thermal noise realization
		vThermalNoise    = getNoiseValue();
		vmeasured        = vEfield + vThermalNoise;
		vmeasured_square = pow(vmeasured, 2);

		// signal + interference + noise
		energy.second    = energy.second + vmeasured_square;  // collect this contribution

		// Now evaluates signal to noise ratio
		// signal converted to antenna voltage squared
		snir             = signalValue / 2.0217E-12;
		energy.first     = energy.first + snir;

	} // consider next point in time
	return energy;
}

ChannelState DeciderUWBIRED::getChannelState() const {
	return ChannelState(true, 0);  // channel is always "sensed" free
}

void DeciderUWBIRED::finish() {
    if (phy) {
        phy->recordScalar("avgThreshold",           getAvgThreshold());
        phy->recordScalar("nbSuccessfulSyncs",      nbSuccessfulSyncs);
        phy->recordScalar("nbFailedSyncs",          nbFailedSyncs);
        phy->recordScalar("nbCancelReceptions",     nbCancelReceptions);
        phy->recordScalar("nbFinishTrackingFrames", nbFinishTrackingFrames);
    }
	BaseDecider::finish();
}
