package org.mixim.modules.phy;

import org.mixim.base.phyLayer.BasePhyLayer;

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
// 1-2 82-112. <a href="http://dx.doi.org/10.1177/0037549710377767">http://dx.doi.org/10.1177/0037549710377767</a>.
//
// [2] J. Rousselot, Ultra Low Power Communication Protocols for UWB
// Impulse Radio Wireless Sensor Networks. EPFL Thesis 4720, 2010. 
// <a href="http://infoscience.epfl.ch/record/147987">http://infoscience.epfl.ch/record/147987</a>.
//
// [3] J. Rousselot, J.-D. Decotignie, A High-Precision Ultra Wideband Impulse Radio Physical Layer Model
// for Network Simulation. Second International Omnet++ Workshop,Simu'TOOLS, Rome, March 2009.
// <a href="http://portal.acm.org/citation.cfm?id=1537714">http://portal.acm.org/citation.cfm?id=1537714</a>.
//
// @author Jérôme Rousselot
simple PhyLayerUWBIR extends BasePhyLayer
{
    parameters:
        @class(PhyLayerUWBIR);
        @display("i=device/antennatower");
}

