//
// Copyright (C) 2012 Opensim Ltd.
// Author: Tamas Borbely
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
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

package inet.linklayer.ethernet;

import inet.queueing.base.PacketClassifierBase;
import inet.queueing.contract.IPacketClassifier;

//
// Classifier that forwards Ethernet PAUSE frames
// to the pauseOut gate, and other frames to the defaultOut
// gate.
//
// This component can be used to build a QoS queue that gives
// the PAUSE frames the highest priority.
//
simple EtherFrameClassifier extends PacketClassifierBase like IPacketClassifier
{
    parameters:
        @class(EtherFrameClassifier);
        @display("i=block/classifier");
}

