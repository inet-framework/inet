//
// Copyright (C) 2005 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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
        case EIGRP:
            return "EIGRP";
        case LISP:
            return "LISP";
        case BABEL:
            return "BABEL";
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

        case PIM_DM:
            return "PIM-DM";

        case PIM_SM:
            return "PIM-SM";

        default:
            return "???";
    }
}

} // namespace inet

