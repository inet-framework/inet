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
        int queueSize;        // size of FIFO queue
    gates:
//        output toMonitor;				// to monitor
		inout	ponGate;					// to and from PON (i.e., splitter)
		input	fromUni[numUnisPerOnu];	// from users (through UNIs)
		output	toUni[numUnisPerOnu];	// to users (through UNIs)
}
