// $Id$
//-------------------------------------------------------------------------------
//	HybridPonMac.ned --
//
//	This file defines a HybridPonMac module in a PON ONU.
//
//	Copyright (C) 2009 Kyeong Soo (Joseph) Kim
//-------------------------------------------------------------------------------


package inet.linklayer.pon;


simple HybridPonMac
{
    parameters:
        int queueSize;        // size of FIFO queue
    gates:
//        output toMonitor;				// to monitor
		inout	pong;					// to and from PON (i.e., splitter)
		in		fromUni[numUnisPerOnu];	// from users (through UNIs)
		out		toUni[numUnisPerOnu];	// to users (through UNIs)
}
