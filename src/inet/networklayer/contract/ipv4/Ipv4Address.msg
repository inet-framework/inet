//
// Copyright (C) 2017 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this program; if not, see <http://www.gnu.org/licenses/>.
//
// Author: Zoltan Bojthe
//

import inet.common.INETDefs;

cplusplus {{
#include "inet/networklayer/contract/ipv4/Ipv4Address.h"
}}

namespace inet;

cplusplus {{
typedef std::vector<Ipv4Address> Ipv4AddressVector;
}}


class Ipv4Address
{
    @existingClass;
    @opaque;
    @toString(.str());
    @fromString(Ipv4Address($));
}

class Ipv4AddressVector
{
   @existingClass;
   //FIXME tostring, ...
}

