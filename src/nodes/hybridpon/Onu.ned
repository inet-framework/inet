// $Id$
//------------------------------------------------------------------------------
//	Onu.ned --
//
//	This file defines a PON ONU module.
//
//	Copyright (C) 2009 Kyeong Soo (Joseph) Kim
//------------------------------------------------------------------------------


package inet.nodes.hybridpon;

import inet.nodes.ethernet.EtherSwitch;
//import inet.linklayer.ethernet.EtherMAC;
//import inet.linklayer.ethernet.EthernetInterface;
import inet.linklayer.hybridpon.HybridPonMac;
import inet.linklayer.hybridpon.OnuWdmLayer;

module Onu
{
    parameters:
        @node();
        @display("i=device/smallrouter;bgb=188,228;bgp=2,2");
        int lambda; // WDM channel assigned to this ONU
        int numUsersPerOnu; // how many users in this ONU
        int queueSize @unit(bit); // size of FIFO queue in the Mac [bit]
    gates:
        inout ethg[numUsersPerOnu];	// User Network I/F
        inout pong;					// PON I/F (i.e., toward AWG)
        //        output fromMacToMonitor;
    submodules:
        switch: EtherSwitch {
            parameters:
                @display("p=92,59");
        }
        mac: HybridPonMac {
            parameters:
                @display("p=92,124");
                queueSize = queueSize;
        }
        wdm: OnuWdmLayer {
            parameters:
                @display("p=92,185");
        }
    //        etherSwitch: EtherSwitch {
    //            @display("p=92,60");
    //        }
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
        for i=0..numUsersPerOnu-1 {
            ethg++ <--> switch.ethg++;
        }
        switch.ethg[numUserPerOnu] <--> mac.ethg;
        mac.wdmg <--> wdm.demuxg;
        wdm.muxg <--> pong;
}
