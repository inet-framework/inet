/*
 * Decider80211.h
 *
 *  Created on: 11.02.2009
 *      Author: karl wessel
 */

#ifndef DECIDER80211_H_
#define DECIDER80211_H_

#include "INETDefs.h"
#include "BaseDecider.h"
#include "MappingBase.h"

/**
 * @brief Decider for the 802.11 modules
 *
 * Depending on the minimum of the snr included in the PhySDU this
 * module computes a bit error probability. The header (1 Mbit/s) is
 * always modulated with DBQPSK. The PDU is normally modulated either
 * with DBPSK (1 and 2 Mbit/s) or CCK (5.5 and 11 Mbit/s). CCK is not
 * easy to model, therefore it is modeled as DQPSK with a 16-QAM for
 * 5.5 Mbit/s and a 256-QAM for 11 Mbit/s.
 *
 *
 * @ingroup decider
 * @ingroup ieee80211
 * @author Marc L�bbers, David Raguin, Karl Wessel(port for MiXiM)
 */
class INET_API Decider80211: public BaseDecider {
public:
	/** @brief Control message kinds used by this Decider.*/
	enum Decider80211ControlKinds {
		NOTHING = 22100,
		/** @brief Packet lost due to collision.*/
		COLLISION,
		/** @brief Sub-classing deciders should begin their own kinds at this value.*/
		LAST_DECIDER_80211_CONTROL_KIND
	};
protected:
	/** @brief threshold value for checking a SNR-map (SNR-threshold)*/
	double snrThreshold;

	/** @brief The center frequency on which the decider listens for signals */
	double centerFrequency;

protected:
	/** @brief The lower band frequency at given time point.
	 */
	virtual Argument getLowerBandFrequency(simtime_t_cref pTimePoint) const {
	    Argument       argTimeFrequ(DimensionSet::timeFreqDomain);

	    argTimeFrequ.setTime(pTimePoint);
	    argTimeFrequ.setArgValue(Dimension::frequency, centerFrequency - 11e6);

	    return argTimeFrequ;
	}
	/** @brief The upper band frequency at given time point.
	 */
	virtual Argument getUpperBandFrequency(simtime_t_cref pTimePoint) const {
		Argument       argTimeFrequ(DimensionSet::timeFreqDomain);

		argTimeFrequ.setTime(pTimePoint);
		argTimeFrequ.setArgValue(Dimension::frequency, centerFrequency + 11e6);

		return argTimeFrequ;
	}

	/**
	 * @brief Checks if the passed completed AirFrame was received correctly.
	 *
	 * Returns the result as a DeciderResult
	 *
	 * @return	The result of the decider for the passed AirFrame.
	 */
	virtual DeciderResult* createResult(const DetailedRadioFrame* frame) const;

	/**
	 * @brief Calculates the receive power of given frame.
	 */
	virtual double getFrameReceivingPower(DetailedRadioFrame* frame) const;

	/** @brief computes if packet is ok or has errors*/
	virtual bool packetOk(double snirMin, int lengthMPDU, double bitrate) const;

	/**
	 * @brief Calculates the RSSI value for the passed interval.
	 *
	 * This method is called by BaseDecider when it answers a ChannelSenseRequest
	 * or calculates the current channel state.
	 *
	 * Returns the maximum RSSI value inside the passed time
	 * interval and the channel the Decider currently listens to. The second value is the
	 * maximum reception end of all air frames in passed time interval.
	 */
	virtual channel_sense_rssi_t calcChannelSenseRSSI(simtime_t_cref start, simtime_t_cref end) const;

public:
	/** @brief Standard Decider constructor.
	 */
	Decider80211( DeciderToPhyInterface* phy
                , double                 sensitivity
                , int                    myIndex);

	/** @brief Initialize the decider from XML map data.
	 *
	 * This method should be defined for generic decider initialization.
	 *
	 * @param params The parameter map which was filled by XML reader.
	 *
	 * @return true if the initialization was successfully.
	 */
	virtual bool initFromMap(const ParameterMap& params);

	virtual ~Decider80211() {};
};

#endif /* DECIDER80211_H_ */
