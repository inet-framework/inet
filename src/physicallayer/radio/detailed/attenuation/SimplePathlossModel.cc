#include "SimplePathlossModel.h"

#include "DetailedRadioFrame.h"

SimplePathlossConstMapping::SimplePathlossConstMapping(const DimensionSet& dimensions,
                                                       SimplePathlossModel* model,
                                                       const double distFactor)
	: SimpleConstMapping(dimensions)
	, distFactor(distFactor)
	, model(model)
	, hasFrequency(dimensions.hasDimension(Dimension::frequency))
{ }

SimplePathlossConstMapping::SimplePathlossConstMapping(const SimplePathlossConstMapping& o)
	: SimpleConstMapping(o)
	, distFactor(o.distFactor)
	, model(o.model)
	, hasFrequency(o.hasFrequency)
{ }

double SimplePathlossConstMapping::getValue(const Argument& pos) const
{
	double freq = model->carrierFrequency;
	if(hasFrequency) {
		assert(pos.hasArgVal(Dimension::frequency));
		freq = pos.getArgValue(Dimension::frequency);
	}
	double wavelength = SPEED_OF_LIGHT / freq;
	return (wavelength * wavelength) * distFactor;
}

bool SimplePathlossModel::initFromMap(const ParameterMap& params) {
    ParameterMap::const_iterator it;
    bool                         bInitSuccess = true;

    if ((it = params.find("seed")) != params.end()) {
        srand( ParameterMap::mapped_type(it->second).longValue() );
    }
    if ((it = params.find("alpha")) != params.end()) {
        pathLossAlphaHalf = ParameterMap::mapped_type(it->second).doubleValue() * .5;
    }
    else {
        bInitSuccess = false;
        opp_warning("No alpha defined in config.xml for SimplePathlossModel!");
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

void SimplePathlossModel::filterSignal(DetailedRadioFrame* frame, const Coord& sendersPos, const Coord& receiverPos)
{
    DetailedRadioSignal& signal = frame->getSignal();

	/** Calculate the distance factor */
	double sqrDistance = useTorus ? receiverPos.sqrTorusDist(sendersPos, playgroundSize)
								  : receiverPos.sqrdist(sendersPos);

	EV_DEBUG << "sqrdistance is: " << sqrDistance << endl;

	if(sqrDistance <= 1.0) {
		//attenuation is negligible
		return;
	}

	// wavelength in meters (this is only used for debug purposes here
	// the actual effect of the wavelength on the attenuation is
	// calculated in SimplePathlossConstMappings "getValue()" method).
	double wavelength = SPEED_OF_LIGHT / carrierFrequency;
	EV_DEBUG << "wavelength is: " << wavelength << endl;

	// the part of the attenuation only depending on the distance
	double distFactor = pow(sqrDistance, -pathLossAlphaHalf) / (16.0 * M_PI * M_PI);
	EV_DEBUG << "distance factor is: " << distFactor << endl;

	//is our signal to attenuate defined over frequency?
	bool hasFrequency = signal.getTransmissionPower()->getDimensionSet().hasDimension(Dimension::frequency);
	EV_DEBUG << "Signal contains frequency dimension: " << (hasFrequency ? "yes" : "no") << endl;

	const DimensionSet& domain = hasFrequency ? DimensionSet::timeFreqDomain : DimensionSet::timeDomain;

	//create the Attenuation mapping which takes the distance factor as parameter
	//to calculate the attenuation from this and the frequency used for the transmission
	//see the classes "getValue()" for more
	SimplePathlossConstMapping* attMapping = new SimplePathlossConstMapping(
													domain,
													this,
													distFactor);

	/* at last add the created attenuation mapping to the signal */
	signal.addAttenuation(attMapping);
}

double SimplePathlossModel::calcPathloss(const Coord& receiverPos, const Coord& sendersPos)
{
	/*
	 * maybe we can reuse an already calculated value for the square-distance
	 * at this point.
	 *
	 */
	double sqrdistance = 0.0;

	if (useTorus)
	{
		sqrdistance = receiverPos.sqrTorusDist(sendersPos, playgroundSize);
	} else
	{
		sqrdistance = receiverPos.sqrdist(sendersPos);
	}

	EV_DEBUG << "sqrdistance is: " << sqrdistance << endl;

	double attenuation = 1.0;
	// wavelength in metres
	double wavelength = SPEED_OF_LIGHT / carrierFrequency;

	EV_DEBUG << "wavelength is: " << wavelength << endl;

	if (sqrdistance > 1.0)
	{
		attenuation = (wavelength * wavelength) / (16.0 * M_PI * M_PI)
						* (pow(sqrdistance, -1.0*pathLossAlphaHalf));
	}

	EV_DEBUG << "attenuation is: " << attenuation << endl;

	return attenuation;
}
