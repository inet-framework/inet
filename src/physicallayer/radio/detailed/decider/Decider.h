/* -*- mode:c++ -*- ********************************************************
 *
 *
 *
 */

#ifndef DECIDER_H_
#define DECIDER_H_

#include <omnetpp.h>

#include "INETDefs.h"
#include "DeciderToPhyInterface.h"
#include "ChannelSenseRequest_m.h"
#include "ChannelState.h"

/**
 * @brief A class to represent the result of a processed packet (that is not
 * noise) by the Decider.
 *
 * It stores information (i.e. basically whether a packet has been received
 * correctly) for the MACLayer that is handed up to the MACLayer by the PhyLayer
 * together with the received packet. (see also DeciderToPhyInterface)
 *
 * You can subclass DeciderResult to create a more detailed result.
 *
 * @ingroup decider
 */
class INET_API DeciderResult
{
protected:
	/** Stores if the AirFrame for this result was received correct.*/
	bool isCorrect;
public:
	virtual ~DeciderResult() {}

	/**
	 * @brief Initializes the DeciderResult with the passed bool, or true
	 * if omitted.
	 */
	DeciderResult(bool isCorrect = true):
		isCorrect(isCorrect) {}

	/**
	 * @brief A Function that returns a very basic result about the Signal.
	 */
	 virtual bool isSignalCorrect() const;

};

/**
 * @brief The basic Decider class
 *
 * The Deciders tasks are:
 * 	1.	decide which packets should be handed up to the MAC Layer (primary task)
 * 	2.	decide whether the channel is busy/idle at a time point or
 * 		during a time interval (channel sensing)
 *
 * BasePhyLayer hands every receiving AirFrame several times to the
 * "processSignal()"-function and is returned a time point when to do so again.
 *
 * @ingroup decider
 */
class INET_API Decider
{
public:
	/** @brief simtime that tells the Phy-Layer not to pass an AirFrame again */
	static const_simtime_t notAgain;
protected:
	/** @brief A pointer to the physical layer of this Decider. */
	DeciderToPhyInterface* const phy;

	/** @brief Defines what an AirFrameVector shall be here */
	typedef DeciderToPhyInterface::AirFrameVector AirFrameVector;

	/**
	 * @brief Used at initialisation to pass the parameters
	 * to the AnalogueModel and Decider
	 */
	typedef DeciderToPhyInterface::ParameterMap ParameterMap;

private:
	/** @brief Copy constructor is not allowed.
	 */
	Decider(const Decider&);
	/** @brief Assignment operator is not allowed.
	 */
	Decider& operator=(const Decider&);

public:

	/**
	 * @brief Initializes the Decider with a pointer to its PhyLayer
	 */
	Decider(DeciderToPhyInterface* phy);

	/** @brief Initialize the decider from XML map data.
	 *
	 * This method should be defined for generic decider initialization.
	 *
	 * @param params The parameter map which was filled by XML reader.
	 *
	 * @return true if the initialization was successfully.
	 */
	virtual bool initFromMap(const ParameterMap&) { return true; }

	virtual ~Decider() {}

	/**
	 * @brief This function processes a AirFrame given by the PhyLayer and
	 * returns the time point when Decider wants to be given the AirFrame again.
	 */
	virtual simtime_t processSignal(DetailedRadioFrame* frame);

	/** @brief Cancels processing a AirFrame.
	 */
	virtual void cancelProcessSignal() {}

	/**
	 * @brief A function that returns information about the channel state
	 *
	 * It is an alternative for the MACLayer in order to obtain information
	 * immediately (in contrast to sending a ChannelSenseRequest,
	 * i.e. sending a cMessage over the OMNeT-control-channel)
	 */
	virtual ChannelState getChannelState() const;

	/**
	 * @brief This function is called by the PhyLayer to hand over a
	 * ChannelSenseRequest.
	 *
	 * The MACLayer is able to send a ChannelSenseRequest to the PhyLayer
	 * that calls this function with it and is returned a time point when to
	 * re-call this function with the specific ChannelSenseRequest.
	 *
	 * The Decider puts the result (ChannelState) to the ChannelSenseRequest
	 * and "answers" by calling the "sendControlMsg"-function on the
	 * DeciderToPhyInterface, i.e. telling the PhyLayer to send it back.
	 */
	virtual simtime_t handleChannelSenseRequest(ChannelSenseRequest* request);

	/**
	 * @brief Method to be called by an OMNeT-module during its own finish(),
	 * to enable a decider to do some things.
	 */
	virtual void finish() {}

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
	virtual void channelChanged(int /*newChannel*/) {}

};


#endif /*DECIDER_H_*/
