/*
 * RadioUWBIR.h
 * Author: Jerome Rousselot <jerome.rousselot@csem.ch>
 * Copyright: (C) 2008-2010 Centre Suisse d'Electronique et Microtechnique (CSEM) SA
 *              Wireless Embedded Systems
 *              Jaquet-Droz 1, CH-2002 Neuchatel, Switzerland.
 */

#ifndef UWBIRRADIO_H_
#define UWBIRRADIO_H_

#include "MiXiMDefs.h"
#include "PhyUtils.h"
#include "PhyLayerUWBIR.h"

/**
 * @brief This class extends the basic radio model.
 *
 * It adds a SYNC state before reception.
 * The decider tells the uwb phy layer when it locks on a frame, and the uwb phy layer
 * then sets the uwb radio state into RX mode.
 * This is done through a private method so that the MAC can not change these states.
 * This is why this class is friend with PhyLayerUWBIR.
 *
 * @ingroup ieee802154a
 * @ingroup phyLayer
 */

class MIXIM_API RadioUWBIR: public MiximRadio {
	friend class PhyLayerUWBIR;

public:

	enum UWBIRRadioStates {
		/* receiving state*/
		 SYNC = MiximRadio::NUM_RADIO_STATES,
		 UWBIR_NUM_RADIO_STATES
	};

	/* Static factory method (see Radio class in PhyUtils.h) */
	static RadioUWBIR* createNewUWBIRRadio(bool                       recordStats,
	                                       int                        initialState,
	      	                               Argument::mapped_type_cref minAtt       = Argument::MappedOne,
	      	                               Argument::mapped_type_cref maxAtt       = Argument::MappedZero)
	{
		return new RadioUWBIR(RadioUWBIR::UWBIR_NUM_RADIO_STATES,
						 recordStats,
						 initialState,
						 minAtt, maxAtt);
	}

	/**
	 * @brief This switchTo method only accepts three states to switch to:
	 * reception, transmission and sleep.
	 */

	virtual simtime_t switchTo(int newState, simtime_t_cref now) {
		// state must be one of sleep, receive or transmit (not sync)
		//assert(newState != Radio::SYNC);
		if(newState == state || (newState == RadioUWBIR::RX && state == RadioUWBIR::SYNC)) {
			return -1; // nothing to do
		} else {
			if(newState == RadioUWBIR::RX) {
				// prevent entering "frame reception" immediately
				newState = RadioUWBIR::SYNC;
			}
			return reallySwitchTo(newState, now);
		}
	}

	virtual simtime_t reallySwitchTo(int newState, simtime_t_cref now) {
		// set the nextState to the newState and the current state to SWITCHING
		nextState = newState;
		int lastState = state;
		state = RadioUWBIR::SWITCHING;
		radioStates.record(state);
		// make entry to RSAM
		makeRSAMEntry(now, state);

		// return matching entry from the switch times matrix
		return swTimes[lastState][nextState];
	}

protected:

	RadioUWBIR(int numRadioStates,bool recordStats, int initialState, Argument::mapped_type_cref minAtt = Argument::MappedOne, Argument::mapped_type_cref maxAtt = Argument::MappedZero)
	:MiximRadio(numRadioStates, recordStats, initialState, minAtt, maxAtt) {	}

	virtual Argument::mapped_type_cref mapStateToAtt(int state)
	{
		if (state == RadioUWBIR::RX || state == RadioUWBIR::SYNC) {
			return minAtt;
		} else {
			return maxAtt;
		}
	}

private:
	/**
	 * @brief Called by the decider through the phy layer to announce that
	 * the radio has locked on a frame and is attempting reception.
	 */
	virtual void startReceivingFrame(simtime_t_cref now) {
		assert(state == RadioUWBIR::SYNC);
		state = RadioUWBIR::SWITCHING;
		nextState = RadioUWBIR::RX;
		endSwitch(now);
	}
	/**
		 * @brief Called by the decider through the phy layer to announce that
		 * the radio has finished receiving a frame and is attempting to
		 * synchronize on incoming frames.
		 */
	virtual void finishReceivingFrame(simtime_t_cref now) {
		assert(state == RadioUWBIR::RX);
		state = RadioUWBIR::SWITCHING;
		nextState = RadioUWBIR::SYNC;
		endSwitch(now);
	}

};

#endif /* UWBIRRADIO_H_ */
