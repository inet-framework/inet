//
// Copyright (C) 2005 Andras Varga
// Copyright (C) 2005 Wei Yang, Ng
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

#include "inet/networklayer/ipv6/IPv6Route.h"
#include "inet/networklayer/ipv6/IPv6RoutingTable.h"

#include "IPv6RoutingTable.h"

namespace inet {

Register_Abstract_Class(IPv6Route);

std::string IPv6Route::info() const
{
    std::stringstream out;
    out << getDestPrefix() << "/" << getPrefixLength() << " --> ";
    out << "if:" << (_interfacePtr ? _interfacePtr->getName() : "*  ");
    out << " next hop:" << getNextHop();
    out << " " << IRoute::sourceTypeName(getSourceType());
    if (getExpiryTime() > 0)
        out << " exp:" << getExpiryTime();
    return out.str();
}

std::string IPv6Route::detailedInfo() const
{
    return std::string();
}

void IPv6Route::changed(int fieldCode)
{
    if (_rt)
        _rt->routeChanged(this, fieldCode);
}

IRoutingTable *IPv6Route::getRoutingTableAsGeneric() const
{
    return getRoutingTable();
}

} // namespace inet

