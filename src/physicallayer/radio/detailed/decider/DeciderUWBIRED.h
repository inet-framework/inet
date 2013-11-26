/*
 * DeciderUWBIRED.h
 * Author: Jerome Rousselot <jerome.rousselot@csem.ch>
 * Copyright: (C) 2008-2010 Centre Suisse d'Electronique et Microtechnique (CSEM) SA
 *              Wireless Embedded Systems
 *              Jaquet-Droz 1, CH-2002 Neuchatel, Switzerland.
 */

#ifndef _UWBIRENERGYDETECTIONDECIDERV2_H
#define	_UWBIRENERGYDETECTIONDECIDERV2_H

#include <vector>
#include <map>

#include "MiXiMDefs.h"
#include "Signal_.h"
#include "Mapping.h"
#include "BaseDecider.h"
#include "IEEE802154A.h"
#include "UWBIRPacket.h"
#include "MacToPhyInterface.h"

class UWBIRRadio;
class SimplifiedRadioFrame;

/**
 * @brief  This class implements a model of an energy detection receiver
 * that demodulates UWB-IR burst position modulation as defined
 * in the IEEE802154A standard (mandatory mode, high PRF).
 *
 *  The code modeling the frame clock synchronization is implemented
 *  in attemptSync(). Simply subclass this class and redefine attemptSync()
 *  if you wish to consider more sophisticated synchronization models.
 *
 *  To implement a coherent receiver, the easiest way to start is to copy-paste
 *  this code into a new class and rename it accordingly. Then, redefine
 *  decodePacket().
 *
 * Citation of the following publication is appreciated if you use the MiXiM UWB PHY model
 * for a publication of your own.
 * J. Rousselot, J.-D. Decotignie, An ultra-wideband impulse radio PHY
 * layer model for network simulation. SIMULATION January 2011 vol. 87 no. 1-2 82-112.
 *
 * For more information, see also:
 *
 * [1] J. Rousselot, J.-D. Decotignie, An ultra-wideband impulse radio PHY
 * layer model for network simulation. SIMULATION January 2011 vol. 87 no.
 * 1-2 82-112. http://dx.doi.org/10.1177/0037549710377767
 * [2] J. Rousselot, Ultra Low Power Communication Protocols for UWB
 * Impulse Radio Wireless Sensor Networks. EPFL Thesis 4720, 2010.
 * http://infoscience.epfl.ch/record/147987
 * [3]  A High-Precision Ultra Wideband Impulse Radio Physical Layer Model
 * for Network Simulation, Jérôme Rousselot, Jean-Dominique Decotignie,
 * Second International Omnet++ Workshop,Simu'TOOLS, Rome, 6 Mar 09.
 * http://portal.acm.org/citation.cfm?id=1537714
 *
 *
 * @ingroup ieee802154a
 * @ingroup decider
*/
class INET_API DeciderUWBIRED: public BaseDecider {
private:
	bool trace, stats;
	long nbFailedSyncs, nbSuccessfulSyncs;
	mutable double nbSymbols, allThresholds;
	double vsignal2, pulseSnrs;
protected:
	typedef std::map<Signal*, int> tSignalMap;
	double syncThreshold;
	bool syncAlwaysSucceeds;
	bool channelSensing;
	bool synced;
	bool alwaysFailOnDataInterference;
	UWBIRPacket packet;
	mutable cOutVector receivedPulses;
	cOutVector syncThresholds;
	UWBIRRadio* uwbiface;
	int nbCancelReceptions;
	mutable int nbFinishTrackingFrames;

	typedef ConcatConstMapping<std::multiplies<double> > MultipliedMapping;

public:
	/** @brief Signal for emitting UWBIR packets. */
	const static simsignalwrap_t catUWBIRPacketSignal;
	const static double          noiseVariance; // P=-116.9 dBW // 404.34E-12;   v²=s²=4kb T R B (T=293 K)
	const static double          peakPulsePower; //1.3E-3 W peak power of pulse to reach  0dBm during burst; // peak instantaneous power of the transmitted pulse (A=0.6V) : 7E-3 W. But peak limit is 0 dBm

	DeciderUWBIRED( DeciderToPhyInterface* phy
	              , double                 sensitivity
	              , int                    myIndex
	              , bool                   debug );

	/** @brief Initialize the decider from XML map data.
	 *
	 * This method should be defined for generic decider initialization.
	 *
	 * @param params The parameter map which was filled by XML reader.
	 *
	 * @return true if the initialization was successfully.
	 */
	virtual bool initFromMap(const ParameterMap& params);

	double getAvgThreshold() const {
		if (nbSymbols > 0)
			return allThresholds / nbSymbols;
		else
			return 0;
	};

	static double getNoiseValue() {
		 return normal(0, sqrt(noiseVariance));
	}

	/** @brief Cancels processing a AirFrame.
	 */
	virtual void cancelProcessSignal();

	virtual void finish();

	/**@brief Control message kinds specific to DeciderUWBIRED. Currently defines a
	 * message kind that informs the MAC of a successful SYNC event at PHY layer. */
	enum UWBIRED_CTRL_KIND {
	    SYNC_SUCCESS=MacToPhyInterface::LAST_BASE_PHY_KIND+1,
	    SYNC_FAILURE,
	    // add other control messages kinds here (from decider to mac, e.g. CCA)
	};

	// compatibility function to allow running MAC layers that depend on channel state information
	// from PHY layer. Returns last SNR
	virtual ChannelState getChannelState() const;

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
			default:            break;
		}
		return BaseDecider::getNextSignalState(CurState);
	}

	/**
	 * @brief Returns the next handle time for scheduler.
	 *
	 * @param frame The current frame which is in processing.
	 * @return The next scheduler handle time.
	 */
	virtual simtime_t getNextSignalHandleTime(const DetailedRadioFrame* frame) const;

	virtual simtime_t processSignalHeader(DetailedRadioFrame* frame);

	/**
	 * @brief Checks if the passed completed AirFrame was received correctly.
	 *
	 * Returns the result as a DeciderResult
	 *
	 * @return  The result of the decider for the passed AirFrame.
	 */
	virtual DeciderResult* createResult(const DetailedRadioFrame* frame) const;

	std::pair<bool, double> decodePacket(const DetailedRadioFrame* frame, std::vector<bool>* receivedBits, const IEEE802154A::config& cfg) const;

	virtual bool attemptSync(const DetailedRadioFrame* frame);

	// first value is energy from signal, other value is total window energy
	static
	std::pair<double, double> integrateWindow( int                        symbol
	                                         , simtime_t_cref             now
	                                         , simtime_t_cref             burst
	                                         , const AirFrameVector&      airFrameVector
	                                         , const ConstMapping *const  signalPower
	                                         , const DetailedRadioFrame*       frame
	                                         , const IEEE802154A::config& cfg);

private:
	/** @brief Copy constructor is not allowed.
	 */
	DeciderUWBIRED(const DeciderUWBIRED&);
	/** @brief Assignment operator is not allowed.
	 */
	DeciderUWBIRED& operator=(const DeciderUWBIRED&);

};

#endif	/* _UWBIRENERGYDETECTIONDECIDERV2_H */

