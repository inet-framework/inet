//
// Copyright (C) 2012 Kyeong Soo (Joseph) Kim. All rights reserved.
//
// This software is written and distributed under the GNU General
// Public License Version 2 (http://www.gnu.org/licenses/gpl-2.0.html).
// You must not remove this notice, or any other, from this software.
//


package inet.nodes.hybridpon;

import inet.linklayer.etherswitch.MACRelayUnit;
import inet.linklayer.ethernet.EtherMAC3;
import inet.linklayer.hybridpon.IOltScheduler2;
import inet.linklayer.hybridpon.WdmLayer;

//
// Model of a hybrid TDM/WDM-passive optical network (PON) optical line
// terminal (OLT) providing Ethernet service network interfaces (SNIs)
// with switching capability (through MACRelayUnit).
//
// Note that the Ethernet interfaces (provided by EtherMAC2) support
// full duplex mode only.
//
// @author Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
// @date   2012-07-09
//  
module Olt2
{
    parameters:
        @node();
        @labels(node,hybridpon-node);
        @display("i=device/router;bgb=145,326");
        int numReceivers = default(1);	// number of receivers
        int numTransmitters = default(2);	// number of transmitters
        string schType = default("OltScheduler2MCDRR");	// type of IOltScheduler
														// (algorithm and operation mode)
        string relayUnitType = default("MACRelayUnitNP"); // type of MACRelayUnit; currently possible
                                                          // values are MACRelayUnitNP and MACRelayUnitPP														
    gates:
        inout ethg[] @labels(EtherFrame-conn);	// service network interface (i.e., with metro/backbone)
        inout phyg[] @labels(OpticalFrame-conn);	// physical interface (i.e., with optical fiber)
    submodules:
        relayUnit: <relayUnitType> like MACRelayUnit {
            parameters:
                @display("p=73,122");
            gates:
                lowerLayerIn[sizeof(ethg)+sizeof(phyg)];
                lowerLayerOut[sizeof(ethg)+sizeof(phyg)];
        }
        etherMac[sizeof(ethg)]: EtherMAC3 {
            parameters:
                promiscuous = true;
                queueModule = "";
                @display("p=73,46,row;q=queue");
        }
        scheduler: <schType> like IOltScheduler2 {
            parameters:
                @display("p=73,209");
            gates:
                ethg[sizeof(phyg)];
                wdmg[sizeof(phyg)];
        }
        wdm[sizeof(phyg)]: WdmLayer {
            parameters:
                @display("p=73,274");
                ch = index;
            gates:
                pong;
                phyg;
        }
    connections:
        for i=0..sizeof(ethg)-1 {
            etherMac[i].upperLayerIn <-- relayUnit.lowerLayerOut++;
            etherMac[i].upperLayerOut --> relayUnit.lowerLayerIn++;
            ethg++ <--> etherMac[i].phys;
        }
        for i=0..sizeof(phyg)-1 {
            relayUnit.lowerLayerOut++ --> scheduler.ethg$i++;
            relayUnit.lowerLayerIn++ <-- scheduler.ethg$o++;
        	scheduler.wdmg++ <--> wdm[i].pong;
            wdm[i].phyg <--> phyg++;
        }
}
