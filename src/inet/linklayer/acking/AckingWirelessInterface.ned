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
// Provides a highly abstracted wireless network interface that uses a simplified
// MAC protocol with optional acknowledgments. This interface is designed for
// scenarios where physical and data link layer effects can be idealized or
// completely ignored.
//
// The interface creates a perfect transmission environment where:
// - All nodes within transmission range receive frames perfectly
// - Nodes outside the transmission range receive nothing
// - No interference, fading, or other physical layer effects are modeled
// - Optional acknowledgments provide basic reliability
//
// This abstraction is particularly valuable for:
// - Testing and debugging higher layer protocols (especially routing protocols)
// - Educational scenarios where lower layer complexity would be distracting
// - Rapid prototyping of wireless network applications
// - Focusing on network layer behavior without physical/link layer interference
//
// The transmission range is the key parameter that determines connectivity.
// It is configured through the radio module (typically ~UnitDiskRadio) and
// the associated ~UnitDiskRadioMedium, which must be present in the network.
//
// @see ~AckingMac, ~UnitDiskRadio, ~UnitDiskRadioMedium
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
        radio: <default("GenericRadio")> like IRadio {
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
