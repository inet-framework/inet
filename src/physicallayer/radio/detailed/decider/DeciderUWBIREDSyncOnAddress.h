/*
 * DeciderUWBIREDSyncOnAddress.cc
 * Author: Jerome Rousselot <jerome.rousselot@csem.ch>
 * Copyright: (C) 2008-2010 Centre Suisse d'Electronique et Microtechnique (CSEM) SA
 *              Wireless Embedded Systems
 *              Jaquet-Droz 1, CH-2002 Neuchatel, Switzerland.
 */

#ifndef UWBIREDSYNCONADDRESS_H_
#define UWBIREDSYNCONADDRESS_H_

#include "MiXiMDefs.h"
#include "SimpleAddress.h"
#include "DeciderUWBIRED.h"

class Signal;
class PhyLayerUWBIR;

/**
 * @brief  this Decider models a non-coherent energy-detection receiver
 *
 * that that always synchronizes successfully on frames coming from
 * a particular node. It can be used to approximate quasi-orthogonal UWB
 * modulations and private time hopping sequences (you should also configure
 * the Decider adequately). It can also be used to study the robustness to
 * interference of the data segment while neglecting interference on the
 * sync preamble.
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
 * @ingroup ieee802154a
 * @ingroup decider
*/

class MIXIM_API DeciderUWBIREDSyncOnAddress: public DeciderUWBIRED {
private:
	/** @brief Copy constructor is not allowed.
	 */
	DeciderUWBIREDSyncOnAddress(const DeciderUWBIREDSyncOnAddress&);
	/** @brief Assignment operator is not allowed.
	 */
	DeciderUWBIREDSyncOnAddress& operator=(const DeciderUWBIREDSyncOnAddress&);

public:
	DeciderUWBIREDSyncOnAddress( DeciderToPhyInterface* phy
	                           , double                 sensitivity
	                           , int                    myIndex
	                           , bool                   debug )
		: DeciderUWBIRED(phy, sensitivity, myIndex, debug)
		, syncAddress()
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

	virtual bool attemptSync(const airframe_ptr_t frame);

protected:
	LAddress::L2Type syncAddress;
};

#endif /* UWBIREDSYNCONADDRESS_H_ */
