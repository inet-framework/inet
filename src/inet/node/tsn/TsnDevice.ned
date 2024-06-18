//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.node.tsn;

import inet.applications.contract.IApp;
import inet.node.inet.StandardHost;

//
// This module represents a Time-Sensitive Networking (TSN) hardware end device
// that supports time synchronization, per-stream filtering and policing, scheduling
// and traffic shaping, frame replication and elimination, frame preemption for
// Ethernet networks. All TSN features are optional and they can be combined with
// other Ethernet features.
//
module TsnDevice extends StandardHost
{
    parameters:
        bool hasTimeSynchronization = default(false); // Enable IEEE 802.1 AS time synchronization
        bool hasIngressTrafficFiltering = default(false); // Enable IEEE 802.1 Qci ingress per-stream filtering
        bool hasEgressTrafficFiltering = default(false); // Enable IEEE 802.1 Qci egress per-stream filtering
        bool hasEgressTrafficShaping = default(false); // Enable IEEE 802.1 egress traffic shaping (credit based shaping, time-aware shaping, asynchronous shaping)
        bool hasStreamRedundancy = default(false); // Enable IEEE 802.1 CB frame replication and elimination
        bool hasIncomingStreams = default(false); // Enable IEEE 802.1 stream decoding
        bool hasOutgoingStreams = default(false); // Enable IEEE 802.1 stream identification and stream encoding
        bool hasFramePreemption = default(false); // Enable IEEE 802.1 Qbu frame preemption
        bool hasCutthroughSwitching = default(false); // Enable cut-through switching support
        bool hasBridging = default(hasIncomingStreams || hasOutgoingStreams || hasStreamRedundancy || hasIngressTrafficFiltering || hasEgressTrafficFiltering);
        clock.typename = default(hasTimeSynchronization ? "PiServoClock" : ""); // Enable explicit local clock model
        ethernet.typename = default("EthernetLayer"); // Use Ethernet protocol layer outside of network interfaces
        eth[*].typename = default("LayeredEthernetInterface"); // Switch to modular Ethernet interface
        eth[*].macLayer.typename = default(hasFramePreemption ? "EthernetPreemptingMacLayer" : "EthernetMacLayer");
        eth[*].macLayer.queue.typename = default(hasEgressTrafficShaping ? "Ieee8021qTimeAwareShaper" : (hasFramePreemption ? "" : "PacketQueue")); // Use priority queue having multiple subqueues controlled by separate gates
        eth[*].phyLayer.typename = default(hasCutthroughSwitching ? "EthernetStreamThroughPhyLayer" : (hasFramePreemption ? "EthernetPreemptingPhyLayer" : (hasTimeSynchronization ? "EthernetStreamingPhyLayer" : "EthernetPhyLayer"))); // use packet streaming when cut-through switching is enabled
        bridging.typename = default(hasBridging ? "BridgingLayer" : ""); // Switch to modular bridging
        bridging.interfaceRelay.typename = default(""); // Disable frame relaying
        bridging.streamIdentifier.typename = default(hasOutgoingStreams || hasStreamRedundancy ? "StreamIdentifierLayer" : ""); // Enable stream identification when stream redundancy is enabled
        bridging.streamIdentifier.identifier.hasSequenceNumbering = default(hasStreamRedundancy); // Enable sequence numbering if stream redundancy is enabled
        bridging.streamRelay.typename = default(hasStreamRedundancy ? "StreamRelayLayer" : ""); // Enable stream merging and stream splitting when stream redundancy is enabled
        bridging.streamFilter.typename = default(hasIngressTrafficFiltering || hasEgressTrafficFiltering ? "StreamFilterLayer" : ""); // Enable stream filtering when ingress or egress per-stream filtering is enabled
        bridging.streamFilter.ingress.typename = default(hasIngressTrafficFiltering ? "SimpleIeee8021qFilter" : ""); // Use IEEE 802.1 Qci ingress filter when ingress per-stream filtering is enabled
        bridging.streamFilter.egress.typename = default(hasEgressTrafficFiltering ? "SimpleIeee8021qFilter" : ""); // Use IEEE 802.1 Qci egress filter when egress per-stream filtering is enabled
        bridging.streamCoder.typename = default(hasIncomingStreams || hasOutgoingStreams || hasStreamRedundancy ? "StreamCoderLayer" : ""); // Enable stream encoding/decoding when stream redundancy is enabled
        ieee8021r.typename = default(hasStreamRedundancy ? "Ieee8021rProtocol" : "");
        ieee8021q.typename = default(hasIncomingStreams || hasOutgoingStreams || hasStreamRedundancy ? "Ieee8021qProtocol" : "");
        @display("i=device/card"); // Change icon to emphasize hardware device
    submodules:
        gptp: <default("Gptp")> like IApp if hasTimeSynchronization {
            gptpNodeType = default("SLAVE_NODE");
            slavePort = default("eth0");
            masterPorts = default([]);
            @display("p=700,75");
        }
    connections:
        if hasTimeSynchronization {
            gptp.socketOut --> at.in++;
            at.out++ --> gptp.socketIn;
        }
}
