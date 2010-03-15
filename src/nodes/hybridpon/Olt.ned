///
/// @file   Olt.ned
/// @author Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
/// @date   Jan/21/2010
///
/// @brief  Defines a hybrid TDM/WDM-PON OLT module.
///
/// @remarks Copyright (C) 2009-2010 Kyeong Soo (Joseph) Kim. All rights reserved.
///
/// @remarks This software is written and distributed under the GNU General
///          Public License Version 2 (http://www.gnu.org/licenses/gpl-2.0.html).
///          You must not remove this notice, or any other, from this software.
///


package inet.nodes.hybridpon;

import inet.linklayer.etherswitch.MACRelayUnit;
import inet.linklayer.ethernet.EtherMAC;
import inet.linklayer.hybridpon.IOltScheduler;
import inet.linklayer.hybridpon.OltWdmLayer;

module Olt
{
    parameters:
        @node();
        @labels(node,hybridpon-node);
        @display("i=device/router;bgb=145,326");
        int numReceivers = default(1);	// number of receivers
        int numTransmitters = default(2);	// number of transmitters
        string schType = default("OltSchedulerSSSF");	// type of 'Scheduler'
														// (algorithm and operation mode)
        string relayUnitType = default("MACRelayUnitNP"); // type of the MACRelayUnit; currently possible
                                                          // values are MACRelayUnitNP and MACRelayUnitPP														
    gates:
        inout ethg[] @labels(EtherFrame-conn);	// Service Network I/F (i.e., with metro/backbone)
        inout pong[] @labels(OpticalFrame-conn);	// PON I/F (i.e., with AWG)
    submodules:
        relayUnit: <relayUnitType> like MACRelayUnit {
            parameters:
                @display("p=73,122");
            gates:
                lowerLayerIn[sizeof(ethg)+sizeof(pong)];
                lowerLayerOut[sizeof(ethg)+sizeof(pong)];
        }
        etherMac[sizeof(ethg)]: EtherMAC {
            parameters:
                promiscuous = true;
                queueModule = "";
                @display("p=73,46,row;q=queue");
        }
        scheduler: <schType> like IOltScheduler {
            parameters:
                @display("p=73,209");
            gates:
                ethg[sizeof(pong)];
                wdmg[sizeof(pong)];
        }
        wdm: OltWdmLayer {
            parameters:
                @display("p=73,274");
            gates:
                demuxg[sizeof(pong)];
                muxg[sizeof(pong)];
        }
    connections:
        for i=0..sizeof(ethg)-1 {
            etherMac[i].upperLayerIn <-- relayUnit.lowerLayerOut++;
            etherMac[i].upperLayerOut --> relayUnit.lowerLayerIn++;
            ethg++ <--> etherMac[i].phys;
        }
        for i=0..sizeof(pong)-1 {
            relayUnit.lowerLayerOut++ --> scheduler.ethg$i++;
            relayUnit.lowerLayerIn++ <-- scheduler.ethg$o++;
            scheduler.wdmg++ <--> wdm.demuxg++;
            wdm.muxg++ <--> pong++;
        }
}
