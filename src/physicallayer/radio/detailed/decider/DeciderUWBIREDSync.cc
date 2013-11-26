#include "DeciderUWBIREDSync.h"

#include "MiXiMAirFrame.h"

bool DeciderUWBIREDSync::initFromMap(const ParameterMap& params) {
    bool                         bInitSuccess = true;
    ParameterMap::const_iterator it           = params.find("syncMinDuration");
    if(it != params.end()) {
        tmin = ParameterMap::mapped_type(it->second).doubleValue();
    }
    else {
        bInitSuccess = false;
        opp_warning("No syncMinDuration defined in config.xml for DeciderUWBIREDSync!");
    }
    return DeciderUWBIRED::initFromMap(params) && bInitSuccess;
}

bool DeciderUWBIREDSync::attemptSync(const airframe_ptr_t frame) {
    AirFrameVector syncVector;

    // Retrieve all potentially colliding airFrames
	getChannelInfo(frame->getSignal().getReceptionStart(), frame->getSignal().getReceptionStart()+IEEE802154A::mandatory_preambleLength, syncVector);
	assert(syncVector.size() != 0);

	if (syncVector.size() == 1) {
		return evaluateEnergy(frame, syncVector);
	}

	bool synchronized = false;
	AirFrameVector::iterator it = syncVector.begin();
	bool search = true;
	simtime_t latestSyncStart = frame->getSignal().getReceptionStart() + IEEE802154A::mandatory_preambleLength - tmin;
	airframe_ptr_t af = syncVector.front();
	Signal & aSignal = af->getSignal();

	while(search &&
			!(aSignal.getReceptionStart() == frame->getSignal().getReceptionStart() &&
					aSignal.getDuration() == frame->getSignal().getDuration())) {
		if(aSignal.getReceptionEnd() > latestSyncStart) {
			// CASE: the end of one of the previous signals goes too far
			// and prevents synchronizing on the current frame.
			search = false;
			break;
		}
		it++;
		af = *it;
		aSignal = af->getSignal();
	}

	if(search && it != syncVector.end()) {
		// sync is possible but there is a frame beginning after our sync start
		Signal & nextSignal = (*it)->getSignal();
		if(nextSignal.getReceptionStart() <
				aSignal.getReceptionEnd() + tmin) {
			// CASE: sync is not possible because next frame starts too early
			search = false;
		}
	}

	if(search) {
		// the signal is long enough. Now evaluate its energy
		synchronized = evaluateEnergy(frame, syncVector);
	}

	return synchronized;
};

bool DeciderUWBIREDSync::evaluateEnergy(const airframe_ptr_t frame, const AirFrameVector& syncVector) const {
	// Assumption: channel coherence time > signal duration
	// Thus we can simply sample the first pulse of the received signal
	const ConstMapping *const rxPower = frame->getSignal().getReceivingPower();
	Argument                  argSync;

	argSync.setTime(frame->getSignal().getReceptionStart() + IEEE802154A::tFirstSyncPulseMax);
	// We could retrieve the pathloss through s->getAttenuation() but we must be careful:
	// maybe the pathloss is not the only analogue model (e.g. RSAMAnalogueModel)
	// If we get the pathloss, we can compute Eb/N0: Eb=1E-3*pathloss if we are at peak power
	double signalPower = sqrt(pow(rxPower->getValue(argSync), 2)*0.5*peakPulsePower / 10); // de-normalize, take half, and 10 dB losses
	double noisePower  = pow(getNoiseValue(), 2)/ 50;
	if(signalPower / noisePower > syncThreshold) {
		return true;
	}
	return false;
};



