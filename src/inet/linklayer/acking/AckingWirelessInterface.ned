//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.acking;

import inet.queueing.contract.IPacketQueue;
import inet.linklayer.contract.IWirelessInterface;
import inet.networklayer.common.NetworkInterface;
import inet.physicallayer.wireless.common.contract.packetlevel.IRadio;

//
// This module implements a highly abstracted wireless network interface (NIC)
// that uses a trivial MAC protocol. It offers simplicity for scenarios where
// Layer 1 and 2 effects can be completely ignored, for example testing the
// basic functionality of a wireless ad-hoc routing protocol.
//
// The most important parameter this model accepts is the transmission range.
// When a radio transmits a frame, all other radios within transmission range
// will receive the frame correctly, and radios that are out of range will not be
// affected at all.
//
// This module requires an ~UnitDiskRadioMedium instance in the network.
//
module AckingWirelessInterface extends NetworkInterface like IWirelessInterface
{
    parameters:
        string interfaceTableModule;
        string energySourceModule = default("");
        double bitrate @unit(bps);
        *.interfaceTableModule = default(absPath(this.interfaceTableModule));
        *.energySourceModule = default(absPath(this.energySourceModule));
        **.bitrate = this.bitrate;
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input radioIn @labels(WirelessSignal);
    submodules:
        queue: <default("DropTailQueue")> like IPacketQueue {
            parameters:
                @display("p=200,100;q=l2queue");
        }
        mac: AckingMac {
            parameters:
                @display("p=300,100");
        }
        radio: <default("UnitDiskRadio")> like IRadio {
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

