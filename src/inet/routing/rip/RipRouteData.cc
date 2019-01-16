//
// Copyright (C) 2014 OpenSim Ltd.
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

