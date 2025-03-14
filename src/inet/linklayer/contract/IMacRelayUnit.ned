//
// Copyright (C) 2003 Andras Varga; CTIE, Monash University, Australia
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.linklayer.contract;

import inet.protocolelement.contract.IProtocolLayer;

//
// Module interface for Ethernet switch functionality that handles MAC address learning
// and frame forwarding between different ports. Relay units maintain a mapping between
// destination MAC addresses and output ports, allowing frames to be forwarded only to
// the appropriate ports rather than flooding all ports.
//
// Functions:
// - Relays frames based on destination MAC addresses (dest-->port mapping)
// - Models finite buffer size (drops) and processing power (latency, queuing)
// - Supports address learning through a dynamic lookup table
// - Handles broadcast/multicast by flooding frames to all ports except the incoming one
// - Supports aging of MAC table entries to handle topology changes
//
// Concrete implementations add capacity and performance aspects to the model, such as
// the number of frames processed per second and the amount of memory available in the switch.
//
// @see MacRelayUnit, Ieee8021dRelay, MrpRelay
//
moduleinterface IMacRelayUnit extends IProtocolLayer
{
    parameters:
        @display("i=block/switch");
}
