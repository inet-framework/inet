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

package inet.protocol.service;

import inet.protocol.classifier.DynamicClassifier;
import inet.protocol.fragmentation.FragmentNumberHeaderBasedDefragmenter;
import inet.queueing.common.PacketMultiplexer;

module PeerService
{
    parameters:
        int numDefragmenter = default(0);
        defragmenter[*].deleteSelf = true;
        @display("i=block/routing");
    gates:
        input in;
        output out;
    submodules:
        multiplexer: PacketMultiplexer {
            @display("p=150,100");
        }
        defragmenter[numDefragmenter]: FragmentNumberHeaderBasedDefragmenter {
            @display("p=150,200,row,100");
        }
        classifier: DynamicClassifier {
            @display("p=150,300");
        }
    connections:
        in --> classifier.in;
        for i=0..numDefragmenter-1 {
            classifier.out++ --> multiplexer.in++;
        }
        multiplexer.out --> out;
}

module MacService
{
    parameters:
        int numPeers = default(0);
        @display("i=block/routing");
    gates:
        input in;
        output out;
    submodules:
        multiplexer: PacketMultiplexer {
            @display("p=150,100");
        }
        peer[numPeers]: PeerService {
            @display("p=150,200,row,100");
        }
        classifier: DynamicClassifier {
            @display("p=150,300");
        }
    connections:
        in --> classifier.in;
        for i=0..numPeers - 1 {
            classifier.out++ --> multiplexer.in++;
        }
        multiplexer.out --> out;
}
