//
// Copyright (C) 2014 Florian Meier
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.linklayer.ieee802154;

import inet.linklayer.contract.IWirelessInterface;
import inet.networklayer.common.NetworkInterface;
import inet.physicallayer.wireless.common.contract.packetlevel.IRadio;
import inet.queueing.contract.IPacketQueue;


//
// Implements a complete IEEE 802.15.4 narrowband network interface for low-rate
// wireless personal area networks (LR-WPANs). This module integrates all the
// necessary components to provide a standards-compliant interface for wireless
// sensor networks and other low-power applications.
//
// The IEEE 802.15.4 narrowband interface operates in various frequency bands
// (868 MHz, 915 MHz, and 2.4 GHz) with data rates ranging from 20 kbps to 250 kbps.
// It is designed for low-power, low-cost, and low-complexity wireless communication
// in applications such as wireless sensor networks, home automation, and IoT devices.
//
// The interface consists of three main components:
// - Queue: Buffers packets from upper layers before transmission
// - MAC: Implements the IEEE 802.15.4 narrowband MAC protocol with CSMA/CA
// - Radio: Provides the physical layer functionality for wireless transmission
//
// This module can be used in any network node that requires IEEE 802.15.4
// narrowband communication capabilities. It connects to the upper network layers
// and provides a standard wireless interface implementation.
//
// @see ~Ieee802154NarrowbandMac, ~Ieee802154Mac, ~IWirelessInterface
//
module Ieee802154NarrowbandInterface extends NetworkInterface like IWirelessInterface
{
    parameters:
        @class(NetworkInterface);
        string interfaceTableModule;
        string energySourceModule = default("");
        *.interfaceTableModule = default(absPath(this.interfaceTableModule));
        *.energySourceModule = default(absPath(this.energySourceModule));
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
        mac: Ieee802154NarrowbandMac {
            parameters:
                @display("p=300,100");
        }
        radio: <default("Ieee802154NarrowbandRadio")> like IRadio {
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
