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

#include "inet/networklayer/contract/IRoute.h"

namespace inet {

const char *IRoute::sourceTypeName(SourceType sourceType)
{
    switch (sourceType) {
        case MANUAL:
            return "MANUAL";

        case IFACENETMASK:
            return "IFACENETMASK";

        case ROUTER_ADVERTISEMENT:
            return "FROM_RA";

        case OWN_ADV_PREFIX:
            return "OWN_ADV_PREFIX";

        case ICMP_REDIRECT:
            return "REDIRECT";

        case RIP:
            return "RIP";

        case OSPF:
            return "OSPF";

        case BGP:
            return "BGP";

        case ZEBRA:
            return "ZEBRA";

        case MANET:
            return "MANET";

        case MANET2:
            return "MANET2";

        case DYMO:
            return "DYMO";

        case AODV:
            return "AODV";

        default:
            return "???";
    }
}

const char *IMulticastRoute::sourceTypeName(SourceType sourceType)
{
    switch (sourceType) {
        case MANUAL:
            return "MANUAL";

        case DVMRP:
            return "DVRMP";

        case PIM_SM:
            return "PIM-SM";

        default:
            return "???";
    }
}

} // namespace inet

