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

package inet.linklayer.ieee8021r;

import inet.queueing.base.PacketFlowBase;

simple Ieee8021rTagEpdHeaderInserter extends PacketFlowBase like IIeee8021rTagEpdHeaderInserter
{
    parameters:
        string nextProtocol = default("ieee8021qctag");
        @class(Ieee8021rTagEpdHeaderInserter);
        @display("i=block/inserter");
}

