//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/routing/rip/RipRouteData.h"

namespace inet {

std::string RipRoute::str() const
{
    std::stringstream out;
    if (dest.isUnspecified())
        out << "0.0.0.0";
    else
        out << dest;
    out << "/" << prefixLength;
    out << " gw: ";
    if (nextHop.isUnspecified())
        out << "*  ";
    else
        out << nextHop << "  ";

    out << "metric:" << metric << " ";

    out << "if:";
    if (!ie)
        out << "*  ";
    else
        out << ie->getInterfaceName() << "  ";

    out << "from:" << from << " ";
    out << "lastUpdate:" << lastUpdateTime << "s  ";
    out << "changed:" << changed << " ";
    out << "tag:" << tag << " ";
    out << "lastInvalid:" << lastInvalid << "s  ";

    switch (type) {
        case RIP_ROUTE_INTERFACE:
            out << "INTERFACE";
            break;

        case RIP_ROUTE_STATIC:
            out << "STATIC";
            break;

        case RIP_ROUTE_DEFAULT:
            out << "DEFAULT";
            break;

        case RIP_ROUTE_RTE:
            out << "RTE";
            break;

        case RIP_ROUTE_REDISTRIBUTE:
            out << "REDISTRIBUTE";
            break;
    }

    return out.str();
}

} // namespace inet

