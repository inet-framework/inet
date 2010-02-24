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
import inet.nodes.ethernet.EtherSwitch;
//import inet.linklayer.ethernet.EtherMAC;
//import inet.linklayer.ethernet.EthernetInterface;


module Onu
{
    parameters:
        @node();
        @display("bgb=188,228;bgp=2,2");
        int lambda; // wavelength assigned to this ONU
        int numUsersPerOnu; // how many users in this ONU
        int queueSize @unit(bit); // size of FIFO queue in the Mac [bit]
    gates:
        //        output fromMacToMonitor;
        inout pong;	/// PON I/F (i.e., toward AWG)
        inout ethg[numUsersPerOnu];
    submodules:
        mac: HybridPonMac {
            parameters:
                //                @display("i=block/subqueue;p=114,166");
                @display("i=block/subqueue;p=92,164");
                queueSize = queueSize;
        }
        switch: EtherSwitch {
            parameters:
                @display("p=92,59");
        }
    //        eth[sizeof(ethGate)]: EtherMAC {
    //            parameters:
    //                promiscuous = true;
    //                queueModule = "";
    //                @display("p=70,150,row;q=queue");
    //        }        
    //        eth[sizeof(ethGate)]: EtherMac {
    //            parameters:
    //                @display("p=145,257,row,110;q=l2queue");
    //        }
    connections:
        //        mac.toMonitor --> fromMacToMonitor;
        pong <--> mac.pong;
        for i=0..numUsersPerOnu-1 {
            ethg[i] <--> switch.ethg[i];
        }
        mac.ethg <--> switch.ethg[numUserPerOnu];
}
