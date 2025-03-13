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
// Implements an IEEE 802.15.4 Ultra-Wideband Impulse Radio (UWB-IR) network
// interface for high-precision ranging and localization applications. This module
// provides a specialized interface for the UWB-IR physical layer option defined
// in the IEEE 802.15.4a-2007 amendment to the standard.
//
// The UWB-IR physical layer operates in frequency bands from 3.1 GHz to 10.6 GHz
// with very wide bandwidth signals (>500 MHz), enabling:
// - High-precision ranging and localization (centimeter-level accuracy)
// - Low power consumption due to short transmission bursts
// - Resistance to multipath fading and interference
// - Potential for high data rates over short distances
//
// Unlike the narrowband variant, this interface uses the ~AckingMac module for
// medium access control, which provides a simplified MAC implementation with
// basic acknowledgment capabilities. The interface consists of:
// - Queue: Buffers packets from upper layers before transmission
// - MAC: Implements a simplified MAC protocol with acknowledgments
// - Radio: Provides the UWB-IR physical layer functionality
//
// This module is particularly useful for simulating applications that require
// precise positioning, such as asset tracking, industrial automation, and
// healthcare monitoring.
//
// @see ~AckingMac, ~Ieee802154NarrowbandInterface, ~IWirelessInterface
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
