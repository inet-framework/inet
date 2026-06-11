//
// Copyright (C) 2007
// Faqir Zarrar Yousaf, Communication Networks Institute, University of Dortmund, Germany.
// Christian Bauer, Institute of Communications and Navigation, German Aerospace Center (DLR)
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/networklayer/ipv6/Mipv6InterfaceData.h"

#include "inet/networklayer/ipv6/Ipv6InterfaceData.h"

namespace inet {

Mipv6InterfaceData::Mipv6InterfaceData()
    : InterfaceProtocolData(NetworkInterface::F_MIPV6_DATA)
{
    hostConstants.initialBindAckTimeout = MIPv6_INITIAL_BINDACK_TIMEOUT;
    hostConstants.maxBindAckTimeout = MIPv6_MAX_BINDACK_TIMEOUT;
    hostConstants.initialBindAckTimeoutFirst = MIPv6_INITIAL_BINDACK_TIMEOUT_FIRST;
    hostConstants.maxRRBindingLifeTime = MIPv6_MAX_RR_BINDING_LIFETIME;
    hostConstants.maxHABindingLifeTime = MIPv6_MAX_HA_BINDING_LIFETIME;
    hostConstants.maxTokenLifeTime = MIPv6_MAX_TOKEN_LIFETIME;
}

std::ostream& operator<<(std::ostream& os, const Mipv6InterfaceData::HomeNetworkInfo& homeNetInfo)
{
    os << "HoA of MN:" << homeNetInfo.HoA << " HA Address: " << homeNetInfo.homeAgentAddr
       << " Home Network Prefix: " << homeNetInfo.prefix /*.prefix()*/;
    return os;
}

void Mipv6InterfaceData::updateHomeNetworkInfo(const Ipv6Address& hoa, const Ipv6Address& ha, const Ipv6Address& prefix, const int prefixLength)
{
    EV_INFO << "\n++++++ Updating the Home Network Information \n";
    homeInfo.HoA = hoa;
    homeInfo.homeAgentAddr = ha;
    homeInfo.prefix = prefix;

    // check if we already have a HoA on this interface
    // if not, then we create one
    auto ipv6Data = getNetworkInterface()->getProtocolDataForUpdate<Ipv6InterfaceData>();
    if (ipv6Data->getGlobalAddress(Ipv6InterfaceData::HoA) == Ipv6Address::UNSPECIFIED_ADDRESS)
        ipv6Data->assignAddress(hoa, false, SIMTIME_ZERO, SIMTIME_ZERO, true);
}

} // namespace inet
