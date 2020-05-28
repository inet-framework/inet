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
}}

class Ipv6ExtHeaderTagBase extends TagBase
{
    Ipv6ExtensionHeader *extensionHeader[] @owned; // array of extension headers, subclassed from ~IPv6ExtensionHeader   //TODO IPv6ExtensionHeadersTag
}

cplusplus(Ipv6ExtHeaderTagBase) {{
  public:
    /**
     * Remove the first extension header and return it.
     */
    Ipv6ExtensionHeader *removeFirstExtensionHeader();
}}

class Ipv6ExtHeaderReq extends Ipv6ExtHeaderTagBase
{
}

class Ipv6ExtHeaderInd extends Ipv6ExtHeaderTagBase
{
}
