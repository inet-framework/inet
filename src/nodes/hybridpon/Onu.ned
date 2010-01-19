// $Id$
//------------------------------------------------------------------------------
//	Onu.ned --
//
//	This file defines a PON ONU module.
//
//	Copyright (C) 2009 Kyeong Soo (Joseph) Kim
//------------------------------------------------------------------------------


package inet.nodes.hybridpon;

import inet.linklayer.hybridpon.HybridPonMac;
import inet.linklayer.ethernet.EthernetInterface;


module Onu
{
    parameters:
		@node();
        @display("bgb=212,228;bgp=2,2");
        int lambda; // wavelength assigned to this ONU
        int numUsersPerOnu; // how many users in this ONU
        int queueSize @unit(bit); // size of FIFO queue in the Mac [bit]
    gates:
        output fromMacToMonitor;
		inout ponGate;	// to and from PON (i.e., splitter)
		inout ethGate[numUsersPerOnu];
    submodules:
        mac: HybridPonMac {
            parameters:
                @display("i=block/subqueue;p=114,166");
                queueSize = queueSize;
			gates:
				fromUni[sizeof(ethg)];
				toUni[sizepf(ethg)];
        }
        eth[sizeof(ethg)]: EthernetMac {
            parameters:
//                @display("p=145,257,row,110;q=l2queue");
        }
    connections:
        mac.toMonitor --> fromMacToMonitor;
		ponGate <--> mac.ponGate
        for i=0..sizeof(ethg)-1 {
            ethg[i] <--> eth[i].phys;
            eth[i].netwOut --> mac.fromUni[i];
            eth[i].netwIn <-- mac.toUni[i];
        }
}
