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

import inet.queueing.base.PacketMarkerBase;
import inet.queueing.contract.IPacketMarker;

//
// This module attaches a ~LabelsTag to packets based on the configured filters.
// For each matching filter a new label is attached.
//
simple PacketLabeler extends PacketMarkerBase like IPacketMarker
{
    parameters:
        string filterClasses = default(""); // determines the packets which are marked, the parameter must be the name of a C++ class which implements the IPacketFilterFunction interface and is registered via Register_Class
        string labels = default(""); // a space separated list of labels
        @display("i=block/star");
        @class(PacketLabeler);
}
