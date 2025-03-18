//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.protocolelement.transceiver.base;

import inet.queueing.base.PacketProcessorBase;

//
// Base module for packet transmitter implementations. Provides common functionality
// for converting packets to signals and transmitting them over a channel. Tracks
// transmission statistics such as throughput, utilization, propagation time, and
// transmission time. Supports clock synchronization and lifecycle operations.
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
        @statistic[transmitting](title="transmitting state"; type=enum; enum=IDLE, TRANSMITTING; source=count(transmissionStarted) - count(transmissionEnded); record=vector; interpolationmode=sample-hold);
        @statistic[throughput](title="throughput"; unit=bps; source=throughput(transmissionEnded); record=vector,histogram; interpolationmode=linear);
        @statistic[utilization](title="utilization"; unit=%; source=100 * utilization(count(transmissionStarted) - count(transmissionEnded)); record=vector,histogram; interpolationmode=linear);
        @statistic[propagationTime](title="propagation time"; unit=s);
        @statistic[transmissionTime](title="transmission time"; unit=s; source=packetDuration(transmissionEnded); record=vector,histogram; interpolationmode=none);
        @statistic[flowTransmissionTime](title="flow transmission time"; unit=s; source=packetDuration(demuxFlow(transmissionEnded)); record=vector,histogram; interpolationmode=none);
    gates:
        input in;
        output out;
}
