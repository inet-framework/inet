/*
 * Decider80211MultiChannel.h
 *
 *  Created on: Mar 22, 2011
 *      Author: karl
 */

#ifndef DECIDER80211MULTICHANNEL_H_
#define DECIDER80211MULTICHANNEL_H_

#include "DetailedRadioFrame.h"
#include "Decider80211.h"

/**
 * @brief Extends Decider80211 by multi channel support.
 *
 * Filters processing of AirFrames depending on the channel they were sent on
 * and the channel this deciders radio is currently set to. All AirFrames
 * on another channel than the currently used one are ignored totally, they are
 * neither received nor considered as interference.
 * If the channel changed during the reception of an AirFrame the AirFrame is
 * considered lost, independent from the size of the part which was lost due to
 * channel change.
 *
 * NOTE: This decider does not model interference between adjacent channels!
 *
 * @author Karl Wessel
 */
class INET_API Decider80211MultiChannel: public Decider80211
{
protected:
	/**
	 * @brief Checks if the passed completed AirFrame was received correctly.
	 *
	 * Returns the result as a DeciderResult.
	 * If the channel changed during transmission the AirFrame is considered
	 * broken.
	 *
	 * @return	The result of the decider for the passed AirFrame.
	 */
	virtual DeciderResult* createResult(const DetailedRadioFrame* frame) const;

public:
	/** @brief Standard Decider constructor.
	 */
	Decider80211MultiChannel( DeciderToPhyInterface* phy
	                        , double                 sensitivity
	                        , int                    myIndex)
		: Decider80211(phy, sensitivity, myIndex)
	{}

	virtual ~Decider80211MultiChannel();

	/**
	 * @brief Called by phy layer to indicate that the channel this radio
	 * currently listens to has changed.
	 *
	 * Sub-classing deciders which support multiple channels should override
	 * this method to handle the effects of channel changes on ongoing
	 * receptions.
	 *
	 * @param newChannel The new channel the radio has changed to.
	 */
	virtual void channelChanged(int newChannel);


};

#endif /* DECIDER80211MULTICHANNEL_H_ */
