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

package inet.protocolelement.transceiver;

import inet.protocolelement.transceiver.base.PacketTransmitterBase;
import inet.protocolelement.transceiver.contract.IPacketTransmitter;

//
// This module receives packets from the upper layer as a whole. It also sends
// signals to the transmission medium (wire) as a whole.
//
simple PacketTransmitter extends PacketTransmitterBase like IPacketTransmitter
{
    parameters:
        @class(PacketTransmitter);
}
