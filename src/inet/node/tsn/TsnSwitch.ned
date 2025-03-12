//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.node.tsn;

import inet.node.ethernet.EthernetSwitch;

//
// An Ethernet switch with Time-Sensitive Networking (TSN) capabilities. It
// extends the standard EthernetSwitch with various TSN features defined in the
// IEEE 802.1 standards suite for deterministic, low-latency communication over
// Ethernet networks.
//
// This module provides a comprehensive implementation of TSN features, all of which are
// optional and can be enabled or disabled individually. The switch supports time synchronization,
// traffic filtering and shaping, stream redundancy, frame preemption, and cut-through switching.
// These features can be combined to meet the requirements of different industrial, automotive,
// and professional audio/video applications.
//
// Key features:
// - IEEE 802.1AS time synchronization
// - IEEE 802.1Qci per-stream filtering and policing for traffic control
// - IEEE 802.1Q traffic shaping
// - IEEE 802.1CB frame replication and elimination for seamless redundancy
// - IEEE 802.1Qbu frame preemption for reduced latency of critical traffic
// - Cut-through switching for minimal forwarding latency
//
// @see ~TsnClock, ~TsnDevice
//
module TsnSwitch extends EthernetSwitch
{
    parameters:
        bool hasTimeSynchronization = default(false); // Enable IEEE 802.1 AS time synchronization
        bool hasIngressTrafficFiltering = default(false); // Enable IEEE 802.1 Qci ingress per-stream filtering
        bool hasEgressTrafficShaping = default(false); // Enable IEEE 802.1 egress traffic shaping (credit based shaping, time aware shaping, asynchronous shaping)
        bool hasStreamRedundancy = default(false); // Enable IEEE 802.1 CB frame replication and elimination
        bool hasIncomingStreams = default(false); // Enable IEEE 802.1 stream decoding
        bool hasOutgoingStreams = default(false); // Enable IEEE 802.1 stream identification and stream encoding
        bool hasFramePreemption = default(false); // Enable IEEE 802.1 Qbu frame preemption
        hasGptp = default(hasTimeSynchronization); // Enable gPTP protocol
        gptp.gptpNodeType = default("BRIDGE_NODE"); // Configure gPTP bridge node type
        gptp.slavePort = default("eth0"); // Configure default gPTP bridge slave port
        clock.typename = default(hasTimeSynchronization ? "SettableClock" : ""); // Enable explicit local clock model when time synchronization is enabled
        ethernet.typename = default("EthernetLayer"); // Use Ethernet protocol layer outside of network interfaces
        eth[*].typename = default(hasCutthroughSwitching ? "EthernetCutthroughInterface" : "LayeredEthernetInterface"); // Switch to modular Ethernet interface
        eth[*].macLayer.typename = default(hasFramePreemption ? "EthernetPreemptingMacLayer" : "EthernetMacLayer");
        eth[*].macLayer.queue.typename = default(hasEgressTrafficShaping ? "Ieee8021qTimeAwareShaper" : "PacketQueue"); // Use compound priority queue having multiple subqueues controlled by separate gates when egress traffic shaping is enabled
        eth[*].phyLayer.typename = default(hasCutthroughSwitching ? "EthernetStreamThroughPhyLayer" : (hasFramePreemption ? "EthernetPreemptingPhyLayer" : "EthernetPhyLayer")); // Use packet streaming when cut-through switching is enabled
        bridging.typename = default("BridgingLayer"); // Switch to modular bridging
        bridging.directionReverser.cutthroughBarrier.typename = default(hasCutthroughSwitching ? "EthernetCutthroughBarrier" : ""); // Enable cut-through barrier when cut-through switching is enabled
        bridging.streamIdentifier.typename = default(hasOutgoingStreams || hasStreamRedundancy ? "StreamIdentifierLayer" : ""); // Enable stream identification when stream redundancy is enabled
        bridging.streamRelay.typename = default(hasStreamRedundancy ? "StreamRelayLayer" : ""); // Enable stream merging and stream splitting when stream redundancy is enabled
        bridging.streamFilter.typename = default(hasIngressTrafficFiltering ? "StreamFilterLayer" : ""); // Enable stream filtering when ingress per-stream filtering is enabled
        bridging.streamFilter.ingress.typename = default(hasIngressTrafficFiltering ? "SimpleIeee8021qFilter" : ""); // Use 802.1 Qci ingress filtering when ingress per-stream filtering is enabled
        bridging.streamCoder.typename = default(hasIncomingStreams || hasOutgoingStreams || hasIngressTrafficFiltering || hasStreamRedundancy ? "StreamCoderLayer" : ""); // Enable stream encoding/decoding when stream redundancy is enabled
        bridging.interfaceRelay.learner.typename = default(hasIncomingStreams || hasOutgoingStreams ? "" : "RelayInterfaceLearner"); // Disable learning MAC addresses in the MAC forwarding table because it would change static TSN stream configuration
        ieee8021q.typename = default("Ieee8021qProtocol");
        ieee8021r.typename = default("Ieee8021rProtocol");
}
