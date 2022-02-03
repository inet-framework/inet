//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.transceiver.base;

import inet.queueing.base.PacketProcessorBase;

simple PacketReceiverBase extends PacketProcessorBase
{
    parameters:
        double datarate @unit(bps);
        @display("i=block/wrx");
        @signal[receptionStarted](type=inet::physicallayer::Signal);
        @signal[receptionEnded](type=inet::physicallayer::Signal);
// TODO        @statistic[receiving](title="transmitting"; type=int; source=count(receptionStarted) - count(receptionEnded); record=vector; interpolationmode=sample-hold);
        @statistic[throughput](title="throughput"; unit=bps; source=throughput(receptionEnded); record=vector,histogram; interpolationmode=linear);
// TODO        @statistic[utilization](title="utilization"; unit=%; source=100 * utilization(count(receptionStarted) - count(receptionEnded)); record=vector,histogram; interpolationmode=linear);
        @statistic[propagationTime](title="propagation time"; unit=s);
        @statistic[receptionTime](title="reception time"; unit=s; source=packetDuration(receptionEnded); record=vector,histogram; interpolationmode=none);
        @statistic[flowReceptionTime](title="flow reception time"; unit=s; source=packetDuration(demuxFlow(receptionEnded)); record=vector,histogram; interpolationmode=none);
    gates:
        input in;
        output out;
}

