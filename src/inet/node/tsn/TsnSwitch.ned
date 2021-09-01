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

import inet.node.ethernet.EthernetSwitch;

module TsnSwitch extends EthernetSwitch
{
    parameters:
        bool hasTimeSynchronization = default(true); // enable 802.1 AS time synchronization
        bool hasIngressTrafficFiltering = default(true); // enable 802.1 Qci ingress traffic filtering
        bool hasEgressTrafficShaping = default(true); // enable 802.1 egress traffic shaping (credit based shaping, time aware shaping, asynchronous shaping) 
        bool hasStreamRedundancy = default(true); // enable 802.1 CB stream redundancy
        hasGptp = default(hasTimeSynchronization); // enable gPTP protocol
        gptp.gptpNodeType = default("BRIDGE_NODE"); // configure gPTP bridge node type
        gptp.slavePort = default("eth0"); // configure default gPTP bridge slave port
        clock.typename = default(hasTimeSynchronization ? "SettableClock" : ""); // enable explicit local clock model when time synchronization is enabled
        ethernet.typename = default("EthernetLayer"); // use Ethernet protocol layer outside of network interfaces
        eth[*].typename = default("LayeredEthernetInterface"); // switch to modular Ethernet interface
        eth[*].macLayer.queue.typename = default(hasEgressTrafficShaping ? "GatingPriorityQueue" : "PacketQueue"); // use compound priority queue having multiple subqueues controlled by separate gates when egress traffic shaping is enabled
        bridging.typename = default("BridgingLayer"); // switch to modular bridging
        bridging.streamIdentifier.typename = default(hasStreamRedundancy ? "StreamIdentifierLayer" : ""); // enable stream identification when stream redundancy is enabled
        bridging.streamRelay.typename = default(hasStreamRedundancy ? "StreamRelayLayer" : ""); // enable stream merging and stream splitting when stream redundancy is enabled
        bridging.streamFilter.typename = default(hasIngressTrafficFiltering ? "StreamFilterLayer" : ""); // enable stream filtering when ingress traffic filtering is enabled
        bridging.streamFilter.ingress.typename = default(hasIngressTrafficFiltering ? "SimpleIeee8021qciFilter" : ""); // use 802.1 Qci ingress filtering when ingress traffic filtering is enabled
        bridging.streamCoder.typename = default(hasStreamRedundancy ? "StreamCoderLayer" : ""); // enable stream endocing/decoding when stream redundancy is enabled
        llc.typename = default("");
}
