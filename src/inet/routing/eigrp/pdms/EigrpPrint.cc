//
// Copyright (C) 2009 - today Brno University of Technology, Czech Republic
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
/**
 * @file EigrpPrint.cc
 * @author Jan Zavrel (honza.zavrel96@gmail.com)
 * @author Vit Rek (rek@kn.vutbr.cz)
 * @author Vladimir Vesely (ivesely@fit.vutbr.cz)
 * @copyright Brno University of Technology (www.fit.vutbr.cz) under GPLv3
 * @date 6. 11. 2014
 * @brief EIGRP Print functions
 * @detail File contains useful functions for printing EIGRP informations (CUT + PASTE from EigrpIpv4Pdm.cc)
 */

#include "inet/routing/eigrp/pdms/EigrpPrint.h"

namespace inet {
namespace eigrp {

// User messages
const char *UserMsgs[] =
{
    // M_OK
    "OK",
    // M_UPDATE_SEND
    "Update",
    // M_REQUEST_SEND
    "Request",
    // M_QUERY_SEND
    "Query",
    // M_REPLY_SEND
    "Reply",
    // M_HELLO_SEND
    "Hello",
    // M_DISABLED_ON_IF
    "EIGRP process isn't enabled on interface",
    // M_NEIGH_BAD_AS
    "AS number is different",
    // M_NEIGH_BAD_KVALUES
    "K-value mismatch",
    // M_NEIGH_BAD_SUBNET
    "Not on the common subnet",
    // M_SIAQUERY_SEND
    "Send SIA Query message",
    // M_SIAREPLY_SEND
    "Send SIA Reply message",
};
}; // end of namespace eigrp

std::ostream& operator<<(std::ostream& os, const eigrp::EigrpNetwork<Ipv4Address>& network)
{
    os << "Address:" << network.getAddress() << " Mask:" << network.getMask();
    return os;
}

std::ostream& operator<<(std::ostream& os, const eigrp::EigrpNetwork<Ipv6Address>& network)
{
    os << "Address:" << network.getAddress() << " Mask: /" << getNetmaskLength(network.getMask());
    return os;
}

std::ostream& operator<<(std::ostream& os, const EigrpKValues& kval)
{
    os << "K1:" << kval.K1 << " K2:" << kval.K2 << " K3:" << kval.K3;
    os << "K4:" << kval.K4 << " K5:" << kval.K5 << " K6:" << kval.K6;
    return os;
}

std::ostream& operator<<(std::ostream& os, const EigrpStub& stub)
{
    if (stub.connectedRt) os << "connected ";
    if (stub.leakMapRt) os << "leak-map ";
    if (stub.recvOnlyRt) os << "recv-only ";
    if (stub.redistributedRt) os << "redistrib ";
    if (stub.staticRt) os << "static ";
    if (stub.summaryRt) os << "summary";
    return os;
}

} // namespace inet

