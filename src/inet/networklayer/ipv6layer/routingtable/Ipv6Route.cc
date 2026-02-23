//
// Copyright (C) 2005 OpenSim Ltd.
// Copyright (C) 2005 Wei Yang, Ng
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/networklayer/ipv6/Ipv6Route.h"

#include "inet/networklayer/ipv6/Ipv6RoutingTable.h"

namespace inet {

Register_Abstract_Class(Ipv6Route);

const char *inet::Ipv6Route::getSourceTypeAbbreviation() const {
    switch (_sourceType) {
        case IFACENETMASK:
            return "C";
        case MANUAL:
            return getDestPrefix().isUnspecified() ? "S*" : "S";
        case ROUTER_ADVERTISEMENT:
            return "ra";
        case RIP:
            return "R";
        case OSPF:
            return "O";
        case BGP:
            return "B";
        case EIGRP:
            return getAdminDist() < Ipv6Route::dEIGRPExternal ? "D" : "D EX";
        case LISP:
            return "l";
        case BABEL:
            return "ba";
        case ODR:
            return "o";
        default:
            return "???";
    }
}

std::string Ipv6Route::str() const
{
    std::stringstream out;
    out << getSourceTypeAbbreviation();
    if (getDestPrefix().isUnspecified())
        out << " ::";
    else
        out << " " << getDestPrefix();
    out << "/" << getPrefixLength();
    if (getNextHop().isUnspecified()) {
        out << " is directly connected";
    }
    else {
        out << " [" << getAdminDist() << "/" << getMetric() << "]";
        out << " via ";
        out << getNextHop();
    }
    out << ", " << getInterface()->getInterfaceName();
    return out.str();
}

std::string Ipv6Route::detailedInfo() const
{
    return std::string();
}

void Ipv6Route::changed(int fieldCode)
{
    if (_rt)
        _rt->routeChanged(this, fieldCode);
}

IRoutingTable *Ipv6Route::getRoutingTableAsGeneric() const
{
    return getRoutingTable();
}

} // namespace inet

