//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.flow;

import inet.queueing.base.PacketFlowBase;
import inet.queueing.contract.IPacketFlow;

simple FlowMeasurementStarter extends PacketFlowBase like IPacketFlow
{
    parameters:
        object packetFilter = default("*"); // which packets are considered, matches all packets by default
        int offset @unit(b) = default(0b); // designates the packet part to be measured
        int length @unit(b) = default(-1b); // designates the packet part to be measured
        string flowName = default(""); // use packet flow to separate measurement from other measurements
        string measure = default("*"); // match expression for a combination of: packetEvent, elapsedTime, delayingTime, queueingTime, processingTime, transmissionTime, propagationTime
        @class(FlowMeasurementStarter);
        @display("i=block/timer");
        @signal[packetFlowStarted](type=inet::Packet);
}
