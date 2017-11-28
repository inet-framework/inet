//
// Copyright (C) 2005 Andras Varga
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


import inet.common.INETDefs;
import inet.common.TagBase;
import inet.networklayer.ipv6.Ipv6Header;

namespace inet;

cplusplus {{
std::ostream& operator<<(std::ostream& os, Ipv6ExtensionHeader *eh);

#ifndef WITH_IPv6
class Ipv6ExtensionHeader {}
inline std::ostream& operator<<(std::ostream& os, Ipv6ExtensionHeader* eh) {return os;}
#endif
}}

class Ipv6ExtHeaderTagBase extends cObject
{
    @customize;
    abstract Ipv6ExtensionHeader *extensionHeader[]; // array of extension headers, subclassed from ~IPv6ExtensionHeader   //TODO IPv6ExtensionHeadersTag
}

