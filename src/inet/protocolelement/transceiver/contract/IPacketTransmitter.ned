//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.transceiver.contract;

//
// Defines the interface for packet transmitter modules that convert packets
// from upper protocol layers into signals for the physical medium. Provides
// the foundation for different signal transmission strategies.
//
// Operation:
// - Receives packets from upper protocol layers
// - Processes packets according to specific transmission strategy
// - Converts packets into signals for the physical medium
// - Handles timing and synchronization aspects of transmission
// - Provides statistics about transmission performance
//
// @see ~IPacketReceiver, ~PacketTransmitterBase
//
moduleinterface IPacketTransmitter
{
    parameters:
        @display("i=block/wtx");
    gates:
        input in;
        output out;
}
