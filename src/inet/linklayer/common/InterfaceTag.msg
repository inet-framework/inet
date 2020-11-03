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

import inet.common.INETDefs;
import inet.common.TagBase;

namespace inet;

class InterfaceTagBase extends TagBase
{
    int interfaceId = -1; // a locally unique (network node) identifier that determines the interface registered in the ~InterfaceTable
}

//
// Determines the outgoing interface where the packet should leave the network node.
//
class InterfaceReq extends InterfaceTagBase
{
}

//
// Specifies the incoming interface where the packet has arrived at the network node.
//
class InterfaceInd extends InterfaceTagBase
{
}
