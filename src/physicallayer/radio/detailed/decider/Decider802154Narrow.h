/*
 * Decider80211.h
 *
 *  Created on: 11.02.2009
 *      Author: karl wessel
 */

#ifndef DECIDER802154NARROW_H_
#define DECIDER802154NARROW_H_

#include "INETDefs.h"
#include "BaseDecider.h"

/**
 * @brief Decider for the 802.15.4 Narrow band module
 *
 * @ingroup decider
 * @ingroup ieee802154
 * @author Jerome Rousselot, Amre El-Hoiydi, Marc Loebbers, Karl Wessel (port for MiXiM)
 */
class INET_API Decider802154Narrow: public BaseDecider {
public:
	enum Decider802154NarrowControlKinds {
		RECEPTION_STARTED=LAST_BASE_DECIDER_CONTROL_KIND,
		LAST_DECIDER802154NARROW_CONTROL_KIND
	};
protected:
	/** @brief Start Frame Delimiter length in bits. */
	int sfdLength;

	/** @brief Minimum bit error rate. If SNIR is high, computed ber could be
		higher than maximum radio performance. This value is an upper bound to
		the performance. */
	double BER_LOWER_BOUND;

	/** @brief modulation type */
	std::string modulation;

	/** log minimum snir values of dropped packets */
	cOutVector snirDropped;

	/** log minimum snir values of received packets */
	mutable cOutVector snirReceived;


	/** log snr value each time we enter getBERFromSNR */
	mutable cOutVector snrlog;

	/** log ber value each time we enter getBERFromSNR */
	mutable cOutVector berlog;

protected:
	/**
	 * @brief Returns the next signal state (END, HEADER, NEW).
	 *
	 * @param CurState The current signal state.
	 * @return The next signal state.
	 */
	virtual eSignalState getNextSignalState(eSignalState CurState) const {
		switch(CurState) {
			case NEW:           return EXPECT_HEADER;                             break;
			default:            return BaseDecider::getNextSignalState(CurState); break;
		}
		return BaseDecider::getNextSignalState(CurState);
	}

	virtual simtime_t processSignalHeader(DetailedRadioFrame* frame);


	/** @brief Creates the DeciderResult from frame.
	 *
	 * @param frame The processed frame.
	 * @return The result for frame.
	 */
	virtual DeciderResult* createResult(const DetailedRadioFrame* frame) const;

	double getBERFromSNR(double snr) const;

	bool   syncOnSFD(DetailedRadioFrame* frame) const;

	double evalBER(DetailedRadioFrame* frame) const;

	bool recordStats;

public:

	/** @brief Helper function to compute BER from SNR using analytical formulas */
	static double n_choose_k(int n, int k);

	/** @brief Standard Decider constructor.
	 */
	Decider802154Narrow( DeciderToPhyInterface* phy
	                   , double                 sensitivity
	                   , int                    myIndex)
	    : BaseDecider(phy, sensitivity, myIndex)
	    , sfdLength(0)
	    , BER_LOWER_BOUND(0)
	    , modulation("")
	    , snirDropped()
	    , snirReceived()
	    , snrlog()
	    , berlog()
	    , recordStats(false)
	{
		snirDropped.setName("snirDropped");
		snirReceived.setName("snirReceived");
		berlog.setName("berlog");
		snrlog.setName("snrlog");
	}

	/** @brief Initialize the decider from XML map data.
	 *
	 * This method should be defined for generic decider initialization.
	 *
	 * @param params The parameter map which was filled by XML reader.
	 *
	 * @return true if the initialization was successfully.
	 */
	virtual bool initFromMap(const ParameterMap& params);

	virtual ~Decider802154Narrow() {};
};

#endif /* DECIDER80211_H_ */
