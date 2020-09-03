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
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//

package inet.protocol.transceiver;

import inet.protocol.transceiver.base.StreamingTransmitterBase;
import inet.protocol.transceiver.contract.IPacketTransmitter;

//
// This module receives packets from the upper layer as a whole. It sends signals
// to the transmission medium (wire) as a stream, it sends the signal start and
// signal end separately.
//
simple StreamingTransmitter extends StreamingTransmitterBase like IPacketTransmitter
{
    parameters:
        @class(StreamingTransmitter);
}

