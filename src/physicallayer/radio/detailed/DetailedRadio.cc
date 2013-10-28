/*
 * DetailedRadio.cc
 *
 *  Created on: 11.02.2009
 *      Author: karl
 */

#include "DetailedRadio.h"

#include "./Decider80211.h"
#include "Decider802154Narrow.h"
#include "SimplePathlossModel.h"
#include "BreakpointPathlossModel.h"
#include "LogNormalShadowing.h"
#include "SNRThresholdDecider.h"
#include "JakesFading.h"
#include "PERModel.h"
#include "BaseConnectionManager.h"

Define_Module(DetailedRadio);

AnalogueModel* DetailedRadio::getAnalogueModelFromName(const std::string& name, ParameterMap& params) const {
    std::string sParamName("");
    std::string sCcParamName("");
    double      dDefault = 0.0;

    sCcParamName = sParamName = "carrierFrequency"; dDefault = 2.412e+9;
    if (params.count(sParamName.c_str()) == 0) {
        if (cc->hasPar(sCcParamName.c_str())) {
            params[sParamName.c_str()] = ParameterMap::mapped_type(sParamName.c_str()).setDoubleValue(cc->par(sCcParamName.c_str()).doubleValue());
        }
        else {
            params[sParamName.c_str()] = ParameterMap::mapped_type(sParamName.c_str()).setDoubleValue(dDefault);
        }
    }
    else if(cc->hasPar(sCcParamName.c_str()) && params[sParamName.c_str()].doubleValue() < cc->par(sCcParamName.c_str()).doubleValue()) {
        // throw error
        opp_error("DetailedRadio::getAnalogueModelFromName(): %s can't be smaller than specified in ConnectionManager. Please adjust your config.xml file accordingly", sParamName.c_str() );
    }

    sCcParamName = sParamName = "alpha"; dDefault = 3.5;
    if (params.count(sParamName.c_str()) == 0) {
        if (cc->hasPar(sCcParamName.c_str())) {
            params[sParamName.c_str()] = ParameterMap::mapped_type(sParamName.c_str()).setDoubleValue(cc->par(sCcParamName.c_str()).doubleValue());
        }
        else {
            params[sParamName.c_str()] = ParameterMap::mapped_type(sParamName.c_str()).setDoubleValue(dDefault);
        }
    }
    else if(cc->hasPar(sCcParamName.c_str()) && params[sParamName.c_str()].doubleValue() < cc->par(sCcParamName.c_str()).doubleValue()) {
        // throw error
        opp_error("DetailedRadio::getAnalogueModelFromName(): %s can't be smaller than specified in ConnectionManager. Please adjust your config.xml file accordingly", sParamName.c_str() );
    }

    sCcParamName = "alpha"; sParamName = "alpha1";
    if (params.count(sParamName.c_str()) == 0) {
        if (cc->hasPar(sCcParamName.c_str())) {
            params[sParamName.c_str()] = ParameterMap::mapped_type(sParamName.c_str()).setDoubleValue(cc->par(sCcParamName.c_str()).doubleValue());
        }
    }
    else if(cc->hasPar(sCcParamName.c_str()) && params[sParamName.c_str()].doubleValue() < cc->par(sCcParamName.c_str()).doubleValue()) {
        // throw error
        opp_error("DetailedRadio::getAnalogueModelFromName(): %s can't be smaller than specified in ConnectionManager. Please adjust your config.xml file accordingly", sParamName.c_str() );
    }
    sCcParamName = "alpha"; sParamName = "alpha2";
    if (params.count(sParamName.c_str()) == 0) {
        if (cc->hasPar(sCcParamName.c_str())) {
            params[sParamName.c_str()] = ParameterMap::mapped_type(sParamName.c_str()).setDoubleValue(cc->par(sCcParamName.c_str()).doubleValue());
        }
    }
    else if(cc->hasPar(sCcParamName.c_str()) && params[sParamName.c_str()].doubleValue() < cc->par(sCcParamName.c_str()).doubleValue()) {
        // throw error
        opp_error("DetailedRadio::getAnalogueModelFromName(): %s can't be smaller than specified in ConnectionManager. Please adjust your config.xml file accordingly", sParamName.c_str() );
    }

    sParamName = "useTorus";
    params[sParamName.c_str()] = ParameterMap::mapped_type(sParamName.c_str()).setBoolValue(cc->getUseTorus());
    sParamName = "PgsX";
    params[sParamName.c_str()] = ParameterMap::mapped_type(sParamName.c_str()).setDoubleValue(cc->getPgs().x);
    sParamName = "PgsY";
    params[sParamName.c_str()] = ParameterMap::mapped_type(sParamName.c_str()).setDoubleValue(cc->getPgs().y);
    sParamName = "PgsZ";
    params[sParamName.c_str()] = ParameterMap::mapped_type(sParamName.c_str()).setDoubleValue(cc->getPgs().z);

	if (name == "SimplePathlossModel") {
		return createAnalogueModel<SimplePathlossModel>(params);
	}
	if (name == "LogNormalShadowing") {
		return createAnalogueModel<LogNormalShadowing>(params);
	}
	if (name == "JakesFading") {
		return createAnalogueModel<JakesFading>(params);
	}
	if(name == "BreakpointPathlossModel") {
		return createAnalogueModel<BreakpointPathlossModel>(params);
	}
	if(name == "PERModel") {
		return createAnalogueModel<PERModel>(params);
	}
	return BasePhyLayer::getAnalogueModelFromName(name, params);
}

Decider* DetailedRadio::getDeciderFromName(const std::string& name, ParameterMap& params) {
    params["recordStats"] = cMsgPar("recordStats").setBoolValue(recordStats);

	if(name == "Decider80211") {
		protocolId = IEEE_80211;
		return createDecider<Decider80211>(params);
	}
	if(name == "SNRThresholdDecider"){
		protocolId = GENERIC;
		return createDecider<SNRThresholdDecider>(params);
	}
	if(name == "Decider802154Narrow") {
		protocolId = IEEE_802154_NARROW;
		return createDecider<Decider802154Narrow>(params);
	}

	return BasePhyLayer::getDeciderFromName(name, params);
}
