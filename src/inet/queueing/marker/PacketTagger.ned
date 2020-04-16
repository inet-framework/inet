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

package inet.queueing.marker;

import inet.queueing.base.PacketTaggerBase;
import inet.queueing.contract.IPacketMarker;

//
// This module attaches various request tags to packets.
//
simple PacketTagger extends PacketTaggerBase like IPacketMarker
{
    parameters:
        string filterClass = default("inet::queueing::AnyPacketFilter"); // determines which packets are tagged
        @class(PacketTagger);
}
