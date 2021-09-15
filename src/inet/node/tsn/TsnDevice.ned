//
// Copyright (C) 2020 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

package inet.node.tsn;

import inet.applications.contract.IApp;
import inet.node.inet.StandardHost;

module TsnDevice extends StandardHost
{
    parameters:
        bool hasTimeSynchronization = default(true); // enable 802.1 AS time synchronization
        bool hasIngressTrafficFiltering = default(false); // enable 802.1 Qci ingress traffic filtering
        bool hasEgressTrafficShaping = default(false); // enable 802.1 egress traffic shaping (credit based shaping, time aware shaping, asynchronous shaping) 
        bool hasStreamRedundancy = default(false); // enable 802.1 CB stream redundancy
        bool hasOutputStreams = default(true); // enable 802.1 stream identification and stream encoding
        bool hasFramePreemption = default(false); // enable 802.1 Qbu frame preemption
        bool hasCutthroughSwitching = default(false); // enable cut-through switching support
        clock.typename = default(hasTimeSynchronization ? "SettableClock" : ""); // enable explicit local clock model
        ethernet.typename = default("EthernetLayer"); // use Ethernet protocol layer outside of network interfaces
        eth[*].typename = default("LayeredEthernetInterface"); // switch to modular Ethernet interface
        eth[*].macLayer.typename = default(hasFramePreemption ? "EthernetPreemptingMacLayer" : "EthernetMacLayer");
        eth[*].macLayer.queue.typename = default(hasEgressTrafficShaping ? "GatingPriorityQueue" : (hasFramePreemption ? "" : "PacketQueue")); // use priority queue having multiple subqueues controlled by separate gates
        eth[*].phyLayer.typename = default(hasCutthroughSwitching ? "EthernetStreamingPhyLayer" : (hasFramePreemption ? "EthernetPreemptingPhyLayer" : "EthernetPhyLayer")); // use packet streaming when cut-through switching is enabled
        bridging.typename = default(hasOutputStreams || hasStreamRedundancy || hasIngressTrafficFiltering ? "BridgingLayer" : ""); // switch to modular bridging
        bridging.interfaceRelay.typename = default(""); // disable frame relaying
        bridging.streamIdentifier.typename = default(hasOutputStreams || hasStreamRedundancy ? "StreamIdentifierLayer" : ""); // enable stream identification when stream redundancy is enabled
        bridging.streamRelay.typename = default(hasStreamRedundancy ? "StreamRelayLayer" : ""); // enable stream merging and stream splitting when stream redundancy is enabled
        bridging.streamFilter.typename = default(hasIngressTrafficFiltering ? "StreamFilterLayer" : ""); // enable stream filtering when ingress traffic filtering is enabled
        bridging.streamFilter.ingress.typename = default(hasIngressTrafficFiltering ? "SimpleIeee8021qciFilter" : ""); // use 802.1 Qci ingress filtering when ingress traffic filtering is enabled
        bridging.streamCoder.typename = default(hasOutputStreams || hasStreamRedundancy ? "StreamCoderLayer" : ""); // enable stream endocing/decoding when stream redundancy is enabled
        ieee8021r.typename = default(hasStreamRedundancy ? "Ieee8021rProtocol" : "");
        ieee8021q.typename = default(hasOutputStreams || hasStreamRedundancy ? "Ieee8021qProtocol" : "");
        llc.typename = default("Ieee8022Llc");
        @display("i=device/card"); // change icon to emphasise hardware device
    submodules:
        gptp: <default("Gptp")> like IApp if hasTimeSynchronization {
            gptpNodeType = default("SLAVE_NODE");
            slavePort = default("eth0");
            masterPorts = [];
            @display("p=700,75");
        }
    connections:
        if hasTimeSynchronization {
            gptp.socketOut --> at.in++;
            at.out++ --> gptp.socketIn;
        }
}
