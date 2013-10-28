#ifndef SNRTHRESHOLDDECIDER_H_
#define SNRTHRESHOLDDECIDER_H_

#include "INETDefs.h"
#include "BaseDecider.h"

/**
 * @brief Decider implementation which decides a signals
 * correctness by checking its SNR against a threshold.
 *
 * SNRThresholdDecider decides the channel state (idle/busy) at hand of the current
 * received total power level (independent from signal or noise).
 * If its above the threshold defined by the "busyThreshold" parameter
 * it considers the channel busy.
 * The RSSI value returned by this Decider for a ChannelSenseRequest
 * over time is always the RSSI value at the end of the sense.
 *
 * SNRThresholdDecider implements only instantaneous channel sensing
 * therefore it can only handle "UNTIL_IDLE" and "UNTIL_BUSY"
 * ChannelSenseRequests but not "UNTIL_TIMEOUT".
 *
 * Instantaneous channel sensing means SNRThresholdDecider is simplified in the way that
 * the channel does not has to be below the threshold for a certain
 * amount of time to be considered idle (how it would be in reality),
 * but immediately after the RSSI drops below the threshold the channel
 * is considered idle. This means "UNTIL_IDLE" and "UNTIL_BUSY" request
 * are also answered at the first moment the channel drops below or raises
 * above the threshold.
 * SNRThresholdDecider does not support "UNTIL_TIMEOUT" requests because
 * they are used to calculate the channel state over a time period
 * (by averaging over it for example) which is not consistent with using
 * instantaneous idle/busy changes.
 *
 * @ingroup decider
 */
class INET_API SNRThresholdDecider : public BaseDecider
{
protected:
	/** @brief Threshold value for checking a SNR-map (SNR-threshold).*/
	double snrThreshold;

	/** @brief The threshold rssi level above which the channel is considered busy.*/
	double busyThreshold;

protected:

	/**
	 * @brief Checks a mapping against a specific threshold (element-wise).
	 *
	 * @return	true	, if every entry of the mapping is above threshold
	 * 			false	, otherwise
	 *
	 *
	 */
	virtual bool checkIfAboveThreshold(Mapping* map, simtime_t_cref start, simtime_t_cref end) const;

	/**
	 * @brief Processes a received AirFrame.
	 *
	 * The SNR-mapping for the Signal is created and checked against the Deciders
	 * SNR-threshold. Depending on that the received AirFrame is either sent up
	 * to the MAC-Layer or dropped.
	 *
	 * @return	usually return a value for: 'do not pass it again'
	 */
	virtual DeciderResult* createResult(const DetailedRadioFrame* frame) const;

	/**
	 * @brief Returns point in time when the ChannelSenseRequest of the passed CSRInfo can be answered
	 * (e.g. because channel state changed or timeout is reached).
	 */
	virtual simtime_t canAnswerCSR(const CSRInfo& requestInfo) const;

	/**
	 * @brief Answers the ChannelSenseRequest (CSR) from the passed CSRInfo.
	 *
	 * Calculates the rssi value and the channel idle state and sends the CSR
	 * together with the result back to the mac layer.
	 */
	virtual void answerCSR(CSRInfo& requestInfo);

	/**
	 * @brief Returns whether the passed rssi value indicates a idle channel.
	 * @param rssi the channels rssi value to evaluate
	 * @return true if the channel should be considered idle
	 */
	bool isIdleRSSI(double rssi) const {
		return rssi <= busyThreshold;
	}

public:

	/**
	 * @brief Initializes a new SNRThresholdDecider
	 *
	 * @param phy 			- pointer to the deciders phy layer
	 * @param snrThreshold 	- the threshold (as fraction) above which
	 * 						  a signal is received correctly
	 * @param sensitivity 	- the sensitivity (in mW) of the physical layer
	 * @param busyThreshold - the rssi threshold (in mW) above which the
	 * 						  channel is considered idle
	 * @param myIndex 		- the index of the host of this deciders phy
	 * 						  (for debugging output)
	 */
	SNRThresholdDecider( DeciderToPhyInterface* phy
	                   , double                 sensitivity
	                   , int                    myIndex = -1)
		: BaseDecider(phy, sensitivity, myIndex)
		, snrThreshold(0)
		, busyThreshold(sensitivity)
	{}

	/** @brief Initialize the decider from XML map data.
	 *
	 * This method should be defined for generic decider initialization.
	 *
	 * @param params The parameter map which was filled by XML reader.
	 *
	 * @return true if the initialization was successfully.
	 */
	virtual bool initFromMap(const ParameterMap& params);

	virtual ~SNRThresholdDecider() {};

	/**
	 * @brief A function that returns information about the channel state
	 *
	 * It is an alternative for the MACLayer in order to obtain information
	 * immediately (in contrast to sending a ChannelSenseRequest,
	 * i.e. sending a cMessage over the OMNeT-control-channel)
	 */
	virtual ChannelState getChannelState() const;
};

#endif /* SNRTHRESHOLDDECIDER_H_ */
