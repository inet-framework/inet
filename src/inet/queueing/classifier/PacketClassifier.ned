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

package inet.queueing.classifier;

import inet.queueing.base.PacketClassifierBase;
import inet.queueing.contract.IPacketClassifier;

//
// This module connects one packet producer to multiple packet consumers. It
// can be pushed with packets from the connected packet producer. When this
// happens, the classifier pushes the packet to one of its connected packet
// consumers based on the configured packet classifier function. The packet
// classifier function takes a packet and returns an integer which in turn
// is used for determining the output gate.
//
simple PacketClassifier extends PacketClassifierBase like IPacketClassifier
{
    parameters:
        string classifierClass; // determines the output queue of packets, the parameter must be the name of a C++ class which implements the IPacketClassifierFunction interface and is registered via Register_Class
        @class(PacketClassifier);
}
