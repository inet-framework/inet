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

#include "IPv6Route.h"

#include "IPv6RoutingTable.h"

std::string IPv6Route::info() const
{
    std::stringstream out;
    out << getDestPrefix() << "/" << getPrefixLength() << " --> ";
    out << "if=" << getInterfaceId() << " next hop:" << getNextHop(); // FIXME try printing interface name
    out << " " << routeSrcName(getSrc());
    if (getExpiryTime()>0)
        out << " exp:" << getExpiryTime();
    return out.str();
}

std::string IPv6Route::detailedInfo() const
{
    return std::string();
}

const char *IPv6Route::routeSrcName(RouteSrc src)
{
    switch (src)
    {
        case FROM_RA:         return "FROM_RA";
        case OWN_ADV_PREFIX:  return "OWN_ADV_PREFIX";
        case STATIC:          return "STATIC";
        case ROUTING_PROT:    return "ROUTING_PROT";
        default:              return "???";
    }
}

void IPv6Route::changed(int fieldCode)
{
    if (_rt)
        _rt->routeChanged(this, fieldCode);
}

