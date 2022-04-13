//
// Copyright (C) 2006 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee802154;

import inet.queueing.contract.IPacketQueue;
import inet.linklayer.acking.AckingMac;
import inet.linklayer.contract.IWirelessInterface;
import inet.networklayer.common.NetworkInterface;
import inet.physicallayer.wireless.common.contract.packetlevel.IRadio;

//
// This module implements an IEEE 802.15.4 UWB-IR network interface.
//
module Ieee802154UwbIrInterface extends NetworkInterface like IWirelessInterface
{
    parameters:
        @class(NetworkInterface);
        string interfaceTableModule;
        *.interfaceTableModule = default(absPath(this.interfaceTableModule));
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input radioIn @labels(IWirelessSignal);
    submodules:
        queue: <default("DropTailQueue")> like IPacketQueue {
            parameters:
                packetCapacity = default(100);
                @display("p=200,100;q=l2queue");
        }
        mac: AckingMac {
            parameters:
                @display("p=300,100");
        }
        radio: <default("Ieee802154UwbIrRadio")> like IRadio {
            parameters:
                @display("p=300,200");
        }
    connections:
        upperLayerIn --> { @display("m=n"); } --> queue.in;
        queue.out --> mac.upperLayerIn;
        mac.lowerLayerOut --> radio.upperLayerIn;

        radioIn --> { @display("m=s"); } --> radio.radioIn;
        radio.upperLayerOut --> mac.lowerLayerIn;
        mac.upperLayerOut --> { @display("m=n"); } --> upperLayerOut;
}

