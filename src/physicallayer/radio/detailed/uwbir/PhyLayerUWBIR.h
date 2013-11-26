/* -*- mode:c++ -*- ********************************************************
 * file:        PhyLayerUWBIR.h
 *
 * author:      Jerome Rousselot <jerome.rousselot@csem.ch>
 *
 * copyright:   (C) 2008 Centre Suisse d'Electronique et Microtechnique (CSEM) SA
 * 				Systems Engineering
 *              Wireless Embedded Systems
 *              Jaquet-Droz 1, CH-2002 Neuchatel, Switzerland.
 *
 *
 *              This program is free software; you can redistribute it
 *              and/or modify it under the terms of the GNU General Public
 *              License as published by the Free Software Foundation; either
 *              version 2 of the License, or (at your option) any later
 *              version.
 *              For further information see file COPYING
 *              in the top level directory
 * description: this physical layer models an ultra wideband impulse radio channel.
 * acknowledgment: this work was supported (in part) by the National Competence
 * 			    Center in Research on Mobile Information and Communication Systems
 * 				NCCR-MICS, a center supported by the Swiss National Science
 * 				Foundation under grant number 5005-67322.
 ***************************************************************************/
//
// A physical layer that models an Ultra Wideband Impulse Radio wireless communication system.
//
// This class loads channel models and delivers frames to an UWB Decider. It is independent of the modulation technique,
// as long as the frames are represented using the same approach as in IEEE802154A.h (Maximum Pulse Amplitude Estimation).
//
// Several channel models are possible: Ghassemzadeh-LOS, Ghassemadeh-NLOS (see UWBIRStochasticPathlossModel.h)
// and the IEEE 802.15.4A UWB channel models that use the default power delay profile (see UWBIRIEEE802154APathlossModel.h).
//
// Currently, an energy detection receiver is modeled in UWBIRED.h.
// Several synchronization logics have been implemented in derived classes:
// see DeciderUWBIREDSync.h and and DeciderUWBIREDSyncOnAddress.h.
//
// To add a novel receiver (e.g. coherent demodulation), either derive UWBIRED or write a new one,
// then add functionality in this module to load the new decider.
// The same procedure applies for new channel models.
//
// To change the modulation, see UWBIRMac.h, IEEE802154A.h and UWBIRED.h.
//
// To implement optional modes of IEEE802154A, see IEEE802154A.h.
//
// Citation of the following publication is appreciated if you use the MiXiM UWB PHY model
// for a publication of your own.
// J. Rousselot, J.-D. Decotignie, An ultra-wideband impulse radio PHY
// layer model for network simulation. SIMULATION January 2011 vol. 87 no. 1-2 82-112.
//
// For more information, see also:
//
// [1] J. Rousselot, J.-D. Decotignie, An ultra-wideband impulse radio PHY
// layer model for network simulation. SIMULATION January 2011 vol. 87 no.
// 1-2 82-112. http://dx.doi.org/10.1177/0037549710377767
// [2] J. Rousselot, Ultra Low Power Communication Protocols for UWB
// Impulse Radio Wireless Sensor Networks. EPFL Thesis 4720, 2010.
// http://infoscience.epfl.ch/record/147987
// [3]  A High-Precision Ultra Wideband Impulse Radio Physical Layer Model
// for Network Simulation, Jérôme Rousselot, Jean-Dominique Decotignie,
// Second International Omnet++ Workshop,Simu'TOOLS, Rome, 6 Mar 09.
// http://portal.acm.org/citation.cfm?id=1537714
//
#ifndef UWBIR_PHY_LAYER_H
#define UWBIR_PHY_LAYER_H

#include "MiXiMDefs.h"
#include "PhyLayerBattery.h"
#include "RadioUWBIR.h"
#include "HostState.h"

class DeciderUWBIREDSyncOnAddress;
class DeciderUWBIREDSync;

#include "DeciderUWBIRED.h"

#if (OMNETPP_VERSION >= 0x0402)
	typedef cNEDValue 				  t_dynamic_expression_value;
#else
	typedef cDynamicExpression::Value t_dynamic_expression_value;
#endif

/**
 * @brief Physical layer that models an Ultra Wideband Impulse Radio wireless communication system.
 *
 * This class loads channel models and deliver frames to an UWB Decider. It is independent of the modulation technique,
 * as long as the frames are represented using the same approach as in IEEE802154A.h (Maximum Pulse Amplitude Estimation).
 *
 * Several channel models are possible: Ghassemzadeh-LOS, Ghassemadeh-NLOS (see UWBIRStochasticPathlossModel.h)
 * and the IEEE 802.15.4A UWB channel models that use the default power delay profile (see UWBIRIEEE802154APathlossModel.h).
 *
 * Currently, an energy detection receiver is modeled in UWBIRED.h.
 * Several synchronization logics have been implemented in derived classes:
 * see DeciderUWBIREDSync.h and and DeciderUWBIREDSyncOnAddress.h.
 *
 * If you want to add a novel receiver (e.g. coherent demodulation), either derive UWBIRED or write your own,
 * then add functionality in this module to load your decider.
 * The same apply for new channel models.
 *
 * To change the modulation, refer to UWBIRMac.h, IEEE802154A.h and UWBIRED.h.
 * To implement optional modes of IEEE802154A, refer to IEEE802154A.h.
 *
 * Refer to the following publications for more information:
 * [1] An Ultra Wideband Impulse Radio PHY Layer Model for Network Simulation,
 * J. Rousselot, J.-D. Decotignie, Simulation: Transactions of the Society
 * for Computer Simulation, 2010 (to appear).
 * [2] A High-Precision Ultra Wideband Impulse Radio Physical Layer Model
 * for Network Simulation, Jérôme Rousselot, Jean-Dominique Decotignie,
 * Second International Omnet++ Workshop,Simu'TOOLS, Rome, 6 Mar 09.
 * http://portal.acm.org/citation.cfm?id=1537714
 *
 * @ingroup ieee802154a
 * @ingroup phyLayer
 * @ingroup power
 */
class MIXIM_API PhyLayerUWBIR : public PhyLayerBattery
{
	friend class DeciderUWBIRED;
private:
	/** @brief Copy constructor is not allowed.
	 */
	PhyLayerUWBIR(const PhyLayerUWBIR&);
	/** @brief Assignment operator is not allowed.
	 */
	PhyLayerUWBIR& operator=(const PhyLayerUWBIR&);

public:
	PhyLayerUWBIR()
		: PhyLayerBattery()
		, uwbradio(NULL)
		, syncCurrent(0)
	{}

	virtual void finish();

	// this function allows to include common xml documents for ned parameters as ned functions
	static t_dynamic_expression_value ghassemzadehNLOSFunc(cComponent */*context*/, t_dynamic_expression_value argv[] __attribute__((unused)), int /*argc*/) {
		const char * ghassemzadehnlosxml =
			"<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
			"<root>"
				"<AnalogueModels>"
					"<AnalogueModel type=\"UWBIRStochasticPathlossModel\"><parameter name=\"PL0\" type=\"double\" value=\"-51\"/>"
					      "<parameter name=\"mu_gamma\" type=\"double\" value=\"3.5\"/>"
						"<parameter name=\"sigma_gamma\" type=\"double\" value=\"0.97\"/>"
						"<parameter name=\"mu_sigma\" type=\"double\" value=\"2.7\"/>"
						"<parameter name=\"sigma_sigma\" type=\"double\" value=\"0.98\"/>"
						"<parameter name=\"isEnabled\" type=\"bool\" value=\"true\"/>"
						"<parameter name=\"shadowing\" type=\"bool\" value=\"true\"/>"
					"</AnalogueModel>"
				"</AnalogueModels>"
			"</root>";
		cXMLParImpl xmlParser;
		xmlParser.parse(ghassemzadehnlosxml);  // from char* to xml
		t_dynamic_expression_value parameters(xmlParser.xmlValue(NULL)); // from xml to Value
		return parameters;
	}
	typedef t_dynamic_expression_value (*fptr) (cComponent *context, t_dynamic_expression_value argv[], int argc);
	static fptr ghassemzadehNLOSFPtr;
	//static t_dynamic_expression_value (*ghassemzadehNLOSFPtr) (cComponent *context, t_dynamic_expression_value argv[], int argc);

protected:
	virtual airframe_ptr_t encapsMsg(cPacket *msg);

	/**
	 * @brief Creates and returns an instance of the AnalogueModel with the
	 *        specified name.
	 *
	 * Is able to initialize the following AnalogueModels:
	 * - UWBIRStochasticPathlossModel
	 * - UWBIRIEEE802154APathlossModel
	 */
	virtual AnalogueModel* getAnalogueModelFromName(const std::string& name, ParameterMap& params) const;

	/**
	 * @brief Creates and returns an instance of the decider with the specified
	 *        name.
	 *
	 * Is able to initialize directly the following decider:
	 * - DeciderUWBIREDSyncOnAddress
	 * - DeciderUWBIREDSync
	 * - DeciderUWBIRED
	 */
	virtual Decider*    getDeciderFromName(const std::string& name, ParameterMap& params);
	virtual MiximRadio* initializeRadio() const;

	RadioUWBIR* uwbradio;

	virtual void switchRadioToRX() {
		Enter_Method_Silent();
		uwbradio->startReceivingFrame(simTime());
		setRadioCurrent(radio->getCurrentState());
	}

	virtual void switchRadioToSync() {
		Enter_Method_Silent();
		uwbradio->finishReceivingFrame(simTime());
		setRadioCurrent(radio->getCurrentState());
	}

	/** @brief The different currents in mA.*/
	double syncCurrent;

	/**
	 * @brief Defines the power consuming activities (accounts) of
	 * the NIC. Should be the same as defined in the decider.
	 */
	enum Activities {
		SLEEP_ACCT=0,
		RX_ACCT,  		//1
		TX_ACCT,  		//2
		SWITCHING_ACCT, //3
		SYNC_ACCT,		//4
	};

	enum ProtocolIds {
		IEEE_802154_UWB = 3200,
	};

	/** @brief Updates the actual current drawn for the passed state.*/
	virtual void setRadioCurrent(int rs);

	/** @brief Updates the actual current drawn for switching between
	 * the passed states.*/
	virtual void setSwitchingCurrent(int from, int to);

public:
	virtual void initialize(int stage);

	/**
	 * @brief Captures radio switches to adjust power consumption.
	 */
	virtual simtime_t setRadioState(int rs);

	/**
	 * @brief Returns the true if the radio is in RX state.
	 */
	virtual bool isRadioInRX() const;
};

#endif
