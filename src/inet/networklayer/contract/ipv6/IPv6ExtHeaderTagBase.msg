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


cplusplus {{
#include "inet/common/INETDefs.h"
}}

import inet.common.TagBase;

namespace inet;

class noncobject IPv6ExtensionHeaderPtr;

cplusplus {{
class Ipv6ExtensionHeader;
typedef Ipv6ExtensionHeader *IPv6ExtensionHeaderPtr;
std::ostream& operator<<(std::ostream& os, IPv6ExtensionHeaderPtr eh);

#ifndef WITH_IPv6
inline std::ostream& operator<<(std::ostream& os, IPv6ExtensionHeaderPtr eh) {return os;}
#endif
}}


class IPv6ExtHeaderTagBase
{
    @customize(true);
    abstract IPv6ExtensionHeaderPtr extensionHeader[]; // array of extension headers, subclassed from ~Ipv6ExtensionHeader   //TODO IPv6ExtensionHeadersTag
}
