//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.contract;

//
// Defines the interface for all wireless network interfaces in the INET framework.
// This interface extends the base network interface with specific requirements
// for radio-based communication over wireless media.
//
// Wireless interfaces are characterized by:
// - Radio signal reception through the 'radioIn' gate
// - Shared medium access requiring specialized protocols
// - Variable reliability based on signal strength, interference, and mobility
// - Support for features like channel selection, power control, and rate adaptation
//
// Common implementations include WiFi (IEEE 802.11) interfaces, various
// wireless sensor network interfaces (IEEE 802.15.4, B-MAC, X-MAC, L-MAC),
// cellular network interfaces, and other radio-based communication technologies.
//
// @see ~INetworkInterface, ~IWiredInterface, ~WirelessInterface
//
moduleinterface IWirelessInterface extends INetworkInterface
{
    parameters:
        @display("i=block/ifcard");
    gates:
        input radioIn @labels(IWirelessSignal);
}
