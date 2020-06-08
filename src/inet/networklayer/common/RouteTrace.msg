//
// Copyright (C) 2019 Universidad de Malaga
// Author: Alfonso Ariza
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
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

import inet.common.INETDefs;
import inet.networklayer.common.L3Address;
import inet.common.TagBase;

namespace inet;

cplusplus {{
typedef std::vector<L3Address> L3AddressVector;
typedef std::vector<unsigned int> FlagVector;
const unsigned int FLAG_STABLE = 0x1;
}}

class L3AddressVector
{
   @existingClass;
}

class FlagVector
{
   @existingClass;
}

class RouteTraceTag extends TagBase {
    L3Address src;
    L3Address dst;
    unsigned int flagSrc = 0;
    unsigned int flagDst = 0;
    L3AddressVector route;
    FlagVector flags;
}

