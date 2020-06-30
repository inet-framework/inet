//
// Copyright (C) OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see http://www.gnu.org/licenses/.
//

package inet.protocol.ethernet;

import inet.common.MessageDispatcher;
import inet.protocol.common.ProtocolChecker;
import inet.protocol.ieee8022.IIeee8022LlcChecker;
import inet.protocol.ieee8022.IIeee8022LlcInserter;
import inet.protocol.ieee8022.IIeee8022SnapChecker;
import inet.protocol.ieee8022.IIeee8022SnapInserter;
import inet.queueing.common.PacketMultiplexer;
import inet.queueing.contract.IPacketClassifier;

module Ieee8022LlcLayer like IIeee8022LlcLayer
{
    parameters:
        @display("i=block/layer");
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input lowerLayerIn;
        output lowerLayerOut;
    submodules:
        classifier: <default("PacketClassifier")> like IPacketClassifier {
            @display("p=200,100");
        }
        snapInserter: <default("Ieee8022SnapInserter")> like IIeee8022SnapInserter if typename != "" {
            @display("p=100,200");
        }
        m1: PacketMultiplexer {
            @display("p=100,300");
        }
        llcInserter: <default("Ieee8022LlcInserter")> like IIeee8022LlcInserter if typename != "" {
            @display("p=100,400");
        }
        m2: PacketMultiplexer {
            @display("p=200,500");
        }
        protocolChecker: ProtocolChecker {
            @display("p=500,100");
        }
        llcChecker: <default("Ieee8022LlcChecker")> like IIeee8022LlcChecker {
            @display("p=400,400");
        }
        snapChecker: <default("Ieee8022SnapChecker")> like IIeee8022SnapChecker {
            @display("p=600,400");
        }
        dp: MessageDispatcher {
            @display("p=500,300;b=100,5,,,,1");
        }
    connections:
        upperLayerIn --> { @display("m=n"); } --> classifier.in;
        classifier.out++ --> snapInserter.in if exists(snapInserter);
        classifier.out++ --> m1.in++;
        snapInserter.out --> m1.in++ if exists(snapInserter);
        m1.out --> llcInserter.in;
        classifier.out++ --> m2.in++ if exists(llcInserter);
        llcInserter.out --> m2.in++ if exists(llcInserter);
        m2.out --> { @display("m=s"); } --> lowerLayerOut;

        lowerLayerIn --> { @display("m=s"); } --> dp.in++;
        dp.out++ --> llcChecker.in;
        dp.out++ --> snapChecker.in;
        llcChecker.out --> dp.in++;
        snapChecker.out --> dp.in++;
        dp.out++ --> protocolChecker.in;
        protocolChecker.out --> { @display("m=n"); } --> upperLayerOut;
}
