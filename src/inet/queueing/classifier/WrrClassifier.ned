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
// This module implements weighted round-robin classifier.
//
// There is a weight associated with each output gate.
// Initially each output gate gets as many tokens as their
// weight. When a packet is pushed then the first output
// gate is selected that has a packet and enough tokens,
// and one token is consumed. When there is not enough
// tokens to classify a packet, then each token bucket
// is filled up.
//
simple WrrClassifier extends PacketClassifierBase like IPacketClassifier
{
    parameters:
        string weights;
        @class(WrrClassifier);
}
