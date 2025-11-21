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
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

package inet.node.tsn;

import inet.linklayer.contract.IIeee8021qLayer;
import inet.node.wireless.AccessPoint;
import ned.IdealChannel;

module TsnAccessPoint extends AccessPoint
{
    parameters:
        @defaultStatistic("gateState:vector"; module="wlan[0].macLayer.queue.transmissionGate[0]");
    //        bool hasTimeSynchronization = default(false); // enable IEEE 802.1 AS time synchronization
    //        bool hasIngressTrafficFiltering = default(false); // enable IEEE 802.1 Qci ingress per-stream filtering
    //        bool hasEgressTrafficShaping = default(false); // enable IEEE 802.1 egress traffic shaping (credit based shaping, time aware shaping, asynchronous shaping)
    //        bool hasStreamRedundancy = default(false); // enable IEEE 802.1 CB frame replication and elimination
    //        bool hasIncomingStreams = default(false); // enable IEEE 802.1 stream decoding
    //        bool hasOutgoingStreams = default(false); // enable IEEE 802.1 stream identification and stream encoding
    //        bool hasFramePreemption = default(false); // enable IEEE 802.1 Qbu frame preemption
    //        //hasGptp = default(hasTimeSynchronization); // enable gPTP protocol
    //        //gptp.gptpNodeType = default("BRIDGE_NODE"); // configure gPTP bridge node type
    //        //gptp.slavePort = default("eth0"); // configure default gPTP bridge slave port
    //        clock.typename = default(hasTimeSynchronization ? "SettableClock" : ""); // e
    //		wiFiBridgingLayer.streamIdentifier.typename = default(hasOutgoingStreams || hasStreamRedundancy ? "StreamIdentifierLayer" : "");
    //        bridging.typename = default("BridgingLayer"); // switch to modular bridging
    //        bridging.directionReverser.cutthroughBarrier.typename = default(hasCutthroughSwitching ? "EthernetCutthroughBarrier" : ""); // enable cut-through barrier when cut-through switching is enabled
    //        bridging.streamIdentifier.typename = default(hasOutgoingStreams || hasStreamRedundancy ? "StreamIdentifierLayer" : ""); // enable stream identification when stream redundancy is enabled
    //        bridging.streamRelay.typename = default(hasStreamRedundancy ? "StreamRelayLayer" : ""); // enable stream merging and stream splitting when stream redundancy is enabled
    //        bridging.streamFilter.typename = default(hasIngressTrafficFiltering ? "StreamFilterLayer" : ""); // enable stream filtering when ingress per-stream filtering is enabled
    //        bridging.streamFilter.ingress.typename = default(hasIngressTrafficFiltering ? "SimpleIeee8021qFilter" : ""); // use 802.1 Qci ingress filtering when ingress per-stream filtering is enabled
    //        bridging.streamCoder.typename = default(hasIncomingStreams || hasOutgoingStreams || hasIngressTrafficFiltering || hasStreamRedundancy ? "StreamCoderLayer" : ""); // enable stream endocing/decoding when stream redundancy is enabled 
    //        
    gates:
        inout virtual[numWlanInterfaces] @labels(Wireless-conn);
    submodules:
        iIeee8021q: <default("Ieee8021qProtocol")> like IIeee8021qLayer {
            @display("p=882,283");
        }
    connections:
        //bl.out++ --> IdealChannel --> iIeee8021q.upperLayerIn;
        li.out++ --> IdealChannel --> iIeee8021q.lowerLayerIn;
        iIeee8021q.upperLayerOut --> IdealChannel --> bl.in++;
        iIeee8021q.lowerLayerOut --> IdealChannel --> li.in++;
        bl.out++ --> IdealChannel --> iIeee8021q.upperLayerIn;
}
