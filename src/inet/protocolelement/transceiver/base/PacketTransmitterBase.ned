//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.transceiver.base;

import inet.queueing.base.PacketProcessorBase;

//
// Base module for packet transmitters that provides common functionality for
// converting packets into signals for the physical medium. Handles timing,
// statistics, and basic signal generation.
//
// Operation:
// - Receives packets from upper protocol layers
// - Processes packets at a configurable datarate
// - Tracks transmission start and end events
// - Collects comprehensive transmission statistics
// - Provides foundation for specialized transmitter implementations
//
// @see ~PacketReceiverBase, ~StreamingTransmitterBase, ~IPacketTransmitter
//
simple PacketTransmitterBase extends PacketProcessorBase
{
    parameters:
        string clockModule = default(""); // Relative path of a module that implements IClock; optional
        volatile double datarate @unit(bps);
        @class(PacketTransmitterBase);
        @display("i=block/wtx");
        @signal[transmissionStarted](type=inet::physicallayer::Signal);
        @signal[transmissionEnded](type=inet::physicallayer::Signal);
        @statistic[transmitting](title="transmitting state"; type=int; source=count(transmissionStarted) - count(transmissionEnded); record=vector; interpolationmode=sample-hold);
        @statistic[throughput](title="throughput"; unit=bps; source=throughput(transmissionEnded); record=vector,histogram; interpolationmode=linear);
        @statistic[utilization](title="utilization"; unit=%; source=100 * utilization(count(transmissionStarted) - count(transmissionEnded)); record=vector,histogram; interpolationmode=linear);
        @statistic[propagationTime](title="propagation time"; unit=s);
        @statistic[transmissionTime](title="transmission time"; unit=s; source=packetDuration(transmissionEnded); record=vector,histogram; interpolationmode=none);
        @statistic[flowTransmissionTime](title="flow transmission time"; unit=s; source=packetDuration(demuxFlow(transmissionEnded)); record=vector,histogram; interpolationmode=none);
    gates:
        input in;
        output out;
}
