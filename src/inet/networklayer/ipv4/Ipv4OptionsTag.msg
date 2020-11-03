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
import inet.networklayer.ipv4.Ipv4Header;

namespace inet;

class Ipv4OptionsTagBase extends TagBase
{
    Ipv4Option *option[] @owned;
}

//
// Determines the source and destination network addresses for an outgoing packet.
//
class Ipv4OptionsReq extends Ipv4OptionsTagBase
{
}

//
// Specifies the source and destination network addresses for an incoming packet.
//
class Ipv4OptionsInd extends Ipv4OptionsTagBase
{
}

