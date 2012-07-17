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
import inet.linklayer.hybridpon.OnuMacLayer2;
import inet.linklayer.hybridpon.WdmLayer;

//
// A hybrid TDM/WDM-passive optical network (PON) optical network unit
// (ONU) equipped with its own light source (i.e., laser), providing
// Ethernet user network interfaces (UNIs) with switching capability
// (through MACRelayUnit).
//
// Note that the Ethernet interfaces (provided by EtherMAC2) support
// full duplex mode only.
//
// @author Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
// @date   2012-07-06
//
module Onu2
{
    parameters:
        @node();
        @labels(node,hybridpon-node);
        @display("i=device/smallrouter;bgb=138,318;bgp=2,2");
        string relayUnitType = default("MACRelayUnitNP"); // type of the MACRelayUnit; currently possible
                                                          // values are MACRelayUnitNP and MACRelayUnitPP
    gates:
        inout ethg[] @labels(EtherFrame-conn);	// user network interface
        inout phyg @labels(OpticalFrame-conn);	// physical interface (i.e., with optical fiber)
    submodules:
        relayUnit: <relayUnitType> like MACRelayUnit {
            parameters:
                @display("p=69,132");
            gates:
                lowerLayerIn[sizeof(ethg)+1];
                lowerLayerOut[sizeof(ethg)+1];
        }
        etherMac[sizeof(ethg)]: EtherMAC3 {
            parameters:
                promiscuous = true;
                queueModule = "";
                @display("p=69,51,row;q=queue");
        }
        mac: OnuMacLayer2 {
            parameters:
                @display("p=69,213");
        }
        wdm: WdmLayer {
            parameters:
                @display("p=69,273");
        }
    connections:
        for i=0..sizeof(ethg)-1 {
            etherMac[i].upperLayerIn <-- relayUnit.lowerLayerOut++;
            etherMac[i].upperLayerOut --> relayUnit.lowerLayerIn++;
            ethg++ <--> etherMac[i].phys;
        }
        relayUnit.lowerLayerOut++ --> mac.ethg$i;
        relayUnit.lowerLayerIn++ <-- mac.ethg$o;
        mac.wdmg <--> wdm.pong;
        wdm.phyg <--> phyg;
}
