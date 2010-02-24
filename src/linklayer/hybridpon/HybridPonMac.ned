// $Id$
//-------------------------------------------------------------------------------
//	HybridPonMac.ned --
//
//	This file defines a HybridPonMac module in a PON ONU.
//
//	Copyright (C) 2009 Kyeong Soo (Joseph) Kim
//-------------------------------------------------------------------------------


package inet.linklayer.hybridpon;


simple HybridPonMac
{
    parameters:
        @display("i=block/subqueue;p=92,124");
        int queueSize;        // size of FIFO queue
    gates:
        //        output toMonitor;				// to monitor
        // I/F with Ethernet switch
        inout ethg;	// I/F with Ethernet switch
        // I/F with WDM Optical Layer
        inout wdmg;	// I/F with WDM Optical Layer
}
