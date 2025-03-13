//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee8021q;

import inet.queueing.contract.IPacketBuffer;
import inet.queueing.contract.IPacketClassifier;
import inet.queueing.contract.IPacketGate;
import inet.queueing.contract.IPacketQueue;
import inet.queueing.contract.IPacketScheduler;
import inet.queueing.gate.GateControlList;
import inet.queueing.queue.CompoundPacketQueueBase;

//
// Implements the IEEE 802.1Q Time-Aware Shaper (TAS) for Time-Sensitive Networking
// (TSN) applications. This compound module provides deterministic packet scheduling
// based on a global time reference, enabling precise control over traffic timing
// for applications with strict latency requirements.
//
// The Time-Aware Shaper is a key component of IEEE 802.1Q TSN extensions that
// enables deterministic communication by:
// - Classifying incoming traffic into multiple traffic classes
// - Queuing packets in separate queues based on their traffic class
// - Applying transmission selection algorithms to each queue
// - Controlling transmission gates according to a predefined schedule
// - Prioritizing traffic classes during transmission selection
//
// Key components of the module:
// - Traffic classifier: Assigns incoming packets to appropriate traffic classes
// - Per-class queues: Store packets for each traffic class separately
// - Transmission selection algorithms: Apply class-specific traffic shaping
// - Transmission gates: Control when each traffic class can transmit
// - Transmission selection: Prioritizes traffic from enabled gates
//
// The module can be configured with an optional Gate Control List (GCL) that
// defines the precise timing for opening and closing transmission gates. This
// enables time-triggered communication with minimal jitter and deterministic
// latency, which is essential for applications like industrial control systems,
// automotive networks, and professional audio/video systems.
//
// @see ~Ieee8021qCreditBasedShaper, ~PcpTrafficClassClassifier
//
module Ieee8021qTimeAwareShaper extends CompoundPacketQueueBase
{
    parameters:
        int numTrafficClasses = default(8); // Number of traffic categories
        double bitrate @unit(bps);
        bool hasGcl = default(false);
        *.bitrate = default(this.bitrate);
        *.extraLength = default(64b); // Overhead for Ethernet PHY header
    submodules:
        buffer: <default("")> like IPacketBuffer if typename != "" {
            parameters:
                @display("p=100,225");
        }
        gateControlList: GateControlList if hasGcl {
            parameters:
                @display("p=100,350");
        }
        classifier: <default("PcpTrafficClassClassifier")> like IPacketClassifier {
            parameters:
                @display("p=100,100");
        }
        queue[numTrafficClasses]: <default("PacketQueue")> like IPacketQueue {
            parameters:
                bufferModule = default(exists(parent.buffer) ? "^.buffer" : "");
                @display("p=300,100,column,125");
        }
        transmissionSelectionAlgorithm[numTrafficClasses]: <default("")> like IPacketGate {
            parameters:
                @display("p=500,100,column,125");
        }
        transmissionGate[numTrafficClasses]: <default("PeriodicGate")> like IPacketGate {
            parameters:
                @display("p=700,100,column,125");
        }
        transmissionSelection: <default("PriorityScheduler")> like IPacketScheduler {
            parameters:
                reverseOrder = default(true);
                @display("p=900,100");
        }
    connections:
        in --> { @display("m=w"); } --> classifier.in;
        for i = 0 .. numTrafficClasses - 1 {
            classifier.out++ --> queue[i].in;
            queue[i].out --> transmissionSelectionAlgorithm[i].in;
            transmissionSelectionAlgorithm[i].out --> transmissionGate[i].in;
            transmissionGate[i].out --> transmissionSelection.in++;
        }
        transmissionSelection.out --> { @display("m=e"); } --> out;
}
