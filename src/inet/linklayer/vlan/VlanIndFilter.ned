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

package inet.linklayer.vlan;

import inet.queueing.base.PacketFilterBase;
import inet.queueing.contract.IPacketFilter;

//
// This module filters out packets based on the attached VlanInd tag.
//
simple VlanIndFilter extends PacketFilterBase like IPacketFilter
{
    parameters:
        string interfaceTableModule;
        object acceptedVlanIds = default({}); // maps network interface name to a list of accepted VLAN ids, '*' as interface name matches all interfaces, -1 VLAN id matches if no tag is present
        @class(VlanIndFilter);
        @display("i=block/filter");
}

