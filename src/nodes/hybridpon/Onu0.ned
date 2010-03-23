///
/// @file   Onu0.ned
/// @author Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
/// @date   Feb/25/2010
///
/// @brief  Defines a hybrid TDM/WDM-PON ONU module.
///
/// @remarks Copyright (C) 2010 Kyeong Soo (Joseph) Kim. All rights reserved.
///
/// @remarks This software is written and distributed under the GNU General
///          Public License Version 2 (http://www.gnu.org/licenses/gpl-2.0.html).
///          You must not remove this notice, or any other, from this software.
///


package inet.nodes.hybridpon;

import inet.linklayer.etherswitch.MACRelayUnit;
import inet.linklayer.ethernet.EtherMAC;
import inet.linklayer.hybridpon.OnuMacLayer;
import inet.linklayer.hybridpon.OnuWdmLayer;

module Onu0
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
        etherMac[sizeof(ethg)]: EtherMAC {
            parameters:
                promiscuous = true;
                queueModule = "";
                @display("p=69,51,row;q=queue");
        }
        mac: OnuMacLayer {
            parameters:
                @display("p=69,213");
        }
        wdm: OnuWdmLayer {
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
