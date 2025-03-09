//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.transceiver.contract;

//
// Defines the interface for packet receiver modules that convert signals from
// the physical medium into packets for upper protocol layers. Provides the
// foundation for different signal reception strategies.
//
// Operation:
// - Receives signals from the physical medium
// - Processes signals according to specific reception strategy
// - Converts signals into packets for upper layer processing
// - Handles timing and synchronization aspects of reception
// - Provides statistics about reception performance
//
// Particularly useful for:
// - Implementing physical layer reception in protocol stacks
// - Creating different signal reception strategies
// - Building modular network interfaces
// - Supporting various transmission technologies
// - Simulating realistic signal reception behavior
//
// @see ~IPacketTransmitter, ~PacketReceiverBase
//
moduleinterface IPacketReceiver
{
    parameters:
        @display("i=block/wrx");
    gates:
        input in;
        output out;
}
