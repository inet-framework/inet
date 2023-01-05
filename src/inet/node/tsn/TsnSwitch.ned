//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.node.tsn;

import inet.node.ethernet.EthernetSwitch;

//
// This module represents a Time-Sensitive Networking (TSN) switch that supports
// time synchronization, per-stream filtering and policing, scheduling and traffic
// shaping, frame replication and elimination, frame preemption and cut-through
// switching for Ethernet networks. All TSN features are optional and they can
// be combined with other Ethernet features.
//
// TODO move cut-through switching from EthernetSwitch to TsnSwitch?
module TsnSwitch extends EthernetSwitch
{
    parameters:
        bool hasTimeSynchronization = default(false); // enable IEEE 802.1 AS time synchronization
        bool hasIngressTrafficFiltering = default(false); // enable IEEE 802.1 Qci ingress per-stream filtering
        bool hasEgressTrafficShaping = default(false); // enable IEEE 802.1 egress traffic shaping (credit based shaping, time aware shaping, asynchronous shaping)
        bool hasStreamRedundancy = default(false); // enable IEEE 802.1 CB frame replication and elimination
        bool hasIncomingStreams = default(false); // enable IEEE 802.1 stream decoding
        bool hasOutgoingStreams = default(false); // enable IEEE 802.1 stream identification and stream encoding
        bool hasFramePreemption = default(false); // enable IEEE 802.1 Qbu frame preemption
        hasGptp = default(hasTimeSynchronization); // enable gPTP protocol
        gptp.gptpNodeType = default("BRIDGE_NODE"); // configure gPTP bridge node type
        gptp.slavePort = default("eth0"); // configure default gPTP bridge slave port
        clock.typename = default(hasTimeSynchronization ? "SettableClock" : ""); // enable explicit local clock model when time synchronization is enabled
        ethernet.typename = default("EthernetLayer"); // use Ethernet protocol layer outside of network interfaces
        eth[*].typename = default(hasCutthroughSwitching ? "EthernetCutthroughInterface" : "LayeredEthernetInterface"); // switch to modular Ethernet interface
        eth[*].macLayer.typename = default(hasFramePreemption ? "EthernetPreemptingMacLayer" : "EthernetMacLayer");
        eth[*].macLayer.queue.typename = default(hasEgressTrafficShaping ? "Ieee8021qTimeAwareShaper" : "PacketQueue"); // use compound priority queue having multiple subqueues controlled by separate gates when egress traffic shaping is enabled
        eth[*].phyLayer.typename = default(hasCutthroughSwitching ? "EthernetStreamThroughPhyLayer" : (hasFramePreemption ? "EthernetPreemptingPhyLayer" : "EthernetPhyLayer")); // use packet streaming when cut-through switching is enabled
        bridging.typename = default("BridgingLayer"); // switch to modular bridging
        bridging.directionReverser.cutthroughBarrier.typename = default(hasCutthroughSwitching ? "EthernetCutthroughBarrier" : ""); // enable cut-through barrier when cut-through switching is enabled
        bridging.streamIdentifier.typename = default(hasOutgoingStreams || hasStreamRedundancy ? "StreamIdentifierLayer" : ""); // enable stream identification when stream redundancy is enabled
        bridging.streamRelay.typename = default(hasStreamRedundancy ? "StreamRelayLayer" : ""); // enable stream merging and stream splitting when stream redundancy is enabled
        bridging.streamFilter.typename = default(hasIngressTrafficFiltering ? "StreamFilterLayer" : ""); // enable stream filtering when ingress per-stream filtering is enabled
        bridging.streamFilter.ingress.typename = default(hasIngressTrafficFiltering ? "SimpleIeee8021qFilter" : ""); // use 802.1 Qci ingress filtering when ingress per-stream filtering is enabled
        bridging.streamCoder.typename = default(hasIncomingStreams || hasOutgoingStreams || hasIngressTrafficFiltering || hasStreamRedundancy ? "StreamCoderLayer" : ""); // enable stream endocing/decoding when stream redundancy is enabled
        llc.typename = default("");
}
