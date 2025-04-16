//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.flow;

import inet.queueing.base.PacketFlowBase;
import inet.queueing.contract.IPacketFlow;

//
// Initiates packet flow measurements by adding time tags to selected packets. 
// Supports measuring various timing aspects including elapsed time, delaying time,
// queueing time, processing time, transmission time, and propagation time.
// Can be configured to measure specific packet regions and to filter which
// packets are measured.
//
simple FlowMeasurementStarter extends PacketFlowBase like IPacketFlow
{
    parameters:
        object packetFilter = default("*"); // Specifies which packets are considered, matches all packets by default
        int offset @unit(b) = default(0b); // Designates the packet part to be measured
        int length @unit(b) = default(-1b); // Designates the packet part to be measured
        string flowName = default(""); // Use packet flow to separate measurement from other measurements
        string measure = default("not packetEvent"); // Match expression for a combination of: packetEvent, elapsedTime, delayingTime, queueingTime, processingTime, transmissionTime, propagationTime
        @class(FlowMeasurementStarter);
        @display("i=block/timer");
        @signal[packetFlowStarted](type=inet::Packet);
}
