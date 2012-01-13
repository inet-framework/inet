/*
 *  Copyright (C) 2005 Mohamed Louizi
 *  Copyright (C) 2006,2007 Christoph Sommer <christoph.sommer@informatik.uni-erlangen.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "DYMO_RoutingEntry.h"
#include "DYMO.h"

DYMO_RoutingEntry::DYMO_RoutingEntry(DYMO* dymo) :
    routeNextHopInterface(NULL),
    routeAgeMin(dymo, "routeAgeMin"),
    routeAgeMax(dymo, "routeAgeMax"),
    routeNew(dymo, "routeNew"),
    routeUsed(dymo, "routeUsed"),
    routeDelete(dymo, "routeDelete"),
    dymo(dymo)
{
}

DYMO_RoutingEntry::~DYMO_RoutingEntry()
{
}

std::ostream& operator<<(std::ostream& os, const DYMO_RoutingEntry& o)
{
    os << "[ ";
    os << "destination address: " << o.routeAddress;

    cModule* router = o.dymo->getRouterByAddress(o.routeAddress);
    os << " (" << (router ? router->getFullName() : "?") << ")";

    os << ", ";
    os << "sequence number: " << o.routeSeqNum;
    os << ", ";
    os << "next hop address: " << o.routeNextHopAddress;
    os << ", ";
    os << "next hop interface: " << ((o.routeNextHopInterface) ? (o.routeNextHopInterface->getName()) : "unknown");
    os << ", ";
    os << "broken: " << o.routeBroken;
    os << ", ";
    os << "distance metric: " << o.routeDist;
    os << ", ";
    os << "prefix: " << o.routePrefix;
    os << ", ";
    os << "ROUTE_AGE_MIN: " << o.routeAgeMin;
    os << ", ";
    os << "ROUTE_AGE_MAX: " << o.routeAgeMax;
    os << ", ";
    os << "ROUTE_NEW: " << o.routeNew;
    os << ", ";
    os << "ROUTE_USED: " << o.routeUsed;
    os << ", ";
    os << "ROUTE_DELETE: " << o.routeDelete;
    os << " ]";

    return os;
};
