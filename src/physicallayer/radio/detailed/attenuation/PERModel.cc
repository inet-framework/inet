#include "PERModel.h"

#include "DetailedRadioFrame.h"

bool PERModel::initFromMap(const ParameterMap& params) {
    ParameterMap::const_iterator it;
    bool                         bInitSuccess = true;

    if ((it = params.find("seed")) != params.end()) {
        srand( ParameterMap::mapped_type(it->second).longValue() );
    }
    if ((it = params.find("packetErrorRate")) != params.end()) {
        packetErrorRate = ParameterMap::mapped_type(it->second).doubleValue();
    }
    else {
        bInitSuccess = false;
        opp_warning("No packetErrorRate defined in config.xml for PERModel!");
    }

    return AnalogueModel::initFromMap(params) && bInitSuccess;
}

void PERModel::filterSignal(DetailedRadioFrame* frame, const Coord& /*sendersPos*/, const Coord& /*receiverPos*/) {
    DetailedRadioSignal&   signal = frame->getSignal();
	//simtime_t start  = signal.getReceptionStart();
	//simtime_t end    = signal.getReceptionEnd();

	double attenuationFactor = 1;  // no attenuation
	if(packetErrorRate > 0 && uniform(0, 1) < packetErrorRate) {
		attenuationFactor = 0;  // absorb all energy so that the receveir cannot receive anything
	}

	TimeMapping<Linear>* attMapping = new TimeMapping<Linear> ();
	Argument arg;
	attMapping->setValue(arg, attenuationFactor);
	signal.addAttenuation(attMapping);
}



