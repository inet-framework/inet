/*
 * DeciderResult80211.h
 *
 *  Created on: 04.02.2009
 *      Author: karl
 */

#ifndef DECIDERRESULT80211_H_
#define DECIDERRESULT80211_H_

#include "INETDefs.h"
#include "Decider.h"

/**
 * @brief Defines an extended DeciderResult for the 80211 protocol
 * which stores the bit-rate of the transmission.
 *
 * @ingroup decider
 * @ingroup ieee80211
 */
class INET_API DeciderResult80211 : public DeciderResult{
protected:
	/** @brief Stores the bit-rate of the transmission of the packet */
	double bitrate;

	/** @brief Stores the signal to noise ratio of the transmission */
	double snr;
public:

	/**
	 * @brief Initialises with the passed values.
	 *
	 * "bitrate" defines the bit-rate of the transmission of the packet.
	 */
	DeciderResult80211(bool isCorrect, double bitrate, double snr):
		DeciderResult(isCorrect), bitrate(bitrate), snr(snr) {}

	/**
	 * @brief Returns the bit-rate of the transmission of the packet.
	 */
	double getBitrate() const { return bitrate; }

	/**
	 * @brief Returns the signal to noise ratio of the transmission.
	 */
	double getSnr() const { return snr; }
};

#endif /* DECIDERRESULT80211_H_ */
