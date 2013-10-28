#include "BreakpointPathlossModel.h"

#include "DetailedRadioFrame.h"

bool BreakpointPathlossModel::initFromMap(const ParameterMap& params) {
    ParameterMap::const_iterator it;
    bool                         bInitSuccess = true;

    if ((it = params.find("seed")) != params.end()) {
        srand( ParameterMap::mapped_type(it->second).longValue() );
    }
    if ((it = params.find("alpha1")) != params.end()) {
        alpha1 = ParameterMap::mapped_type(it->second).doubleValue();
    }
    else {
        bInitSuccess = false;
        opp_warning("No alpha1 defined in config.xml for BreakpointPathlossModel!");
    }
    if ((it = params.find("alpha2")) != params.end()) {
        alpha2 = ParameterMap::mapped_type(it->second).doubleValue();
    }
    else {
        bInitSuccess = false;
        opp_warning("No alpha1 defined in config.xml for BreakpointPathlossModel!");
    }
    if ((it = params.find("L01")) != params.end()) {
        PL01      = ParameterMap::mapped_type(it->second).doubleValue();
        PL01_real = pow(10, PL01/10);
    }
    else {
        bInitSuccess = false;
        opp_warning("No L01 defined in config.xml for BreakpointPathlossModel!");
    }
    if ((it = params.find("L02")) != params.end()) {
        PL02      = ParameterMap::mapped_type(it->second).doubleValue();
        PL02_real = pow(10, PL02/10);
    }
    else {
        bInitSuccess = false;
        opp_warning("No L02 defined in config.xml for BreakpointPathlossModel!");
    }
    if ((it = params.find("breakpointDistance")) != params.end()) {
        breakpointDistance = ParameterMap::mapped_type(it->second).doubleValue();
    }
    else {
        bInitSuccess = false;
        opp_warning("No breakpointDistance defined in config.xml for BreakpointPathlossModel!");
    }
    if ((it = params.find("useTorus")) != params.end()) {
        useTorus = ParameterMap::mapped_type(it->second).boolValue();
    }
    else {
        bInitSuccess = false;
        opp_warning("No useTorus defined in config.xml for SimplePathlossModel!");
    }
    if ((it = params.find("PgsX")) != params.end()) {
        playgroundSize.x = ParameterMap::mapped_type(it->second).doubleValue();
    }
    else {
        bInitSuccess = false;
        opp_warning("No PgsX defined in config.xml for SimplePathlossModel!");
    }
    if ((it = params.find("PgsY")) != params.end()) {
        playgroundSize.y = ParameterMap::mapped_type(it->second).doubleValue();
    }
    else {
        bInitSuccess = false;
        opp_warning("No PgsY defined in config.xml for SimplePathlossModel!");
    }
    if ((it = params.find("PgsZ")) != params.end()) {
        playgroundSize.z = ParameterMap::mapped_type(it->second).doubleValue();
    }
    else {
        bInitSuccess = false;
        opp_warning("No PgsZ defined in config.xml for SimplePathlossModel!");
    }
    if ((it = params.find("carrierFrequency")) != params.end()) {
        carrierFrequency = ParameterMap::mapped_type(it->second).doubleValue();
    }
    else {
        bInitSuccess = false;
        opp_warning("No carrierFrequency defined in config.xml for SimplePathlossModel!");
    }

    return AnalogueModel::initFromMap(params) && bInitSuccess;
}

void BreakpointPathlossModel::filterSignal(DetailedRadioFrame* frame, const Coord& sendersPos, const Coord& receiverPos) {
    DetailedRadioSignal& signal = frame->getSignal();

	/** Calculate the distance factor */
	double distance = useTorus ? receiverPos.sqrTorusDist(sendersPos, playgroundSize)
								  : receiverPos.sqrdist(sendersPos);
	distance = sqrt(distance);
	EV_DEBUG << "distance is: " << distance << endl;

	if(distance <= 1.0) {
		//attenuation is negligible
		return;
	}

	double attenuation = 1;
	// PL(d) = PL0 + 10 alpha log10 (d/d0)
	// 10 ^ { PL(d)/10 } = 10 ^{PL0 + 10 alpha log10 (d/d0)}/10
	// 10 ^ { PL(d)/10 } = 10 ^ PL0/10 * 10 ^ { 10 log10 (d/d0)^alpha }/10
	// 10 ^ { PL(d)/10 } = 10 ^ PL0/10 * 10 ^ { log10 (d/d0)^alpha }
	// 10 ^ { PL(d)/10 } = 10 ^ PL0/10 * (d/d0)^alpha
	if(distance < breakpointDistance) {
		attenuation = attenuation * PL01_real;
		attenuation = attenuation * pow(distance, alpha1);
	} else {
		attenuation = attenuation * PL02_real;
		attenuation = attenuation * pow(distance/breakpointDistance, alpha2);
	}
	attenuation = 1/attenuation;
	EV_DEBUG << "attenuation is: " << attenuation << endl;

	if (cLogLevel::globalRuntimeLoglevel <= LOGLEVEL_DEBUG) {
	  pathlosses.record(10*log10(attenuation)); // in dB
	}

	//const DimensionSet& domain = DimensionSet::timeDomain;
	Argument arg;	// default constructor initializes with a single dimension, time, and value 0 (offset from signal start)
	TimeMapping<Linear>* attMapping = new TimeMapping<Linear> ();	// mapping performs a linear interpolation from our single point -> constant
	attMapping->setValue(arg, attenuation);

	/* at last add the created attenuation mapping to the signal */
	signal.addAttenuation(attMapping);
}
