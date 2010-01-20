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
		inout	pong;	// PON I/F (i.e., toward AWG)
		inout	ethg;	// I/F Ethernet switch
//		input	fromUni[numUnisPerOnu];	// from users (through UNIs)
//		output	toUni[numUnisPerOnu];	// to users (through UNIs)
}
