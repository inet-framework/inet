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

#include "inet/networklayer/contract/ipv6/IPv6ControlInfo.h"

#ifdef WITH_IPv6
#include "inet/networklayer/ipv6/IPv6Datagram.h"
#endif // ifdef WITH_IPv6

namespace inet {

void IPv6ControlInfo::copy(const IPv6ControlInfo& other)
{
}

IPv6ControlInfo& IPv6ControlInfo::operator=(const IPv6ControlInfo& other)
{
    if (this == &other)
        return *this;
    clean();
    IPv6ControlInfo_Base::operator=(other);
    copy(other);
    return *this;
}

void IPv6ControlInfo::clean()
{
}

IPv6ControlInfo::~IPv6ControlInfo()
{
    clean();
}

} // namespace inet

