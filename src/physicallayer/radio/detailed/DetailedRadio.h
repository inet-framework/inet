/*
 * PhyLayer.h
 *
 *  Created on: 11.02.2009
 *      Author: karl wessel
 */

#ifndef PHYLAYER_H_
#define PHYLAYER_H_

#include "BasePhyLayer.h"

/**
 * @brief Provides initialisation for several AnalogueModels and Deciders
 * from modules directory.
 *
 * Knows the following AnalogueModels:
 * - SimplePathlossModel
 * - LogNormalShadowing
 * - JakesFading
 *
 * Knows the following Deciders
 * - Decider80211
 * - SNRThresholdDecider
 *
 * @ingroup phyLayer
 */
class INET_API DetailedRadio: public BasePhyLayer {
protected:
	enum ProtocolIds {
		IEEE_80211 = 12123,
		IEEE_802154_NARROW,
	};
	/**
	 * @brief Creates and returns an instance of the AnalogueModel with the
	 * specified name.
	 *
	 * Is able to initialize the following AnalogueModels:
	 * - SimplePathlossModel
	 * - LogNormalShadowing
	 * - JakesFading
	 * - BreakpointPathlossModel
	 * - PERModel
	 */
	virtual AnalogueModel* getAnalogueModelFromName(const std::string& name, ParameterMap& params) const;

	/**
	 * @brief Creates and returns an instance of the decider with the specified
	 *        name.
	 *
	 * Is able to initialize directly the following decider:
	 * - Decider80211
	 * - SNRThresholdDecider
	 * - Decider802154Narrow
	 */
	virtual Decider* getDeciderFromName(const std::string& name, ParameterMap& params);

};

#endif /* PHYLAYER_H_ */
