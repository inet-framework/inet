//
// Copyright (C) 2005 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/networklayer/ipv6/Ipv6Header.h"

namespace inet {

short Ipv6Header::getDscp() const
{
    return (trafficClass & 0xfc) >> 2;
}

void Ipv6Header::setDscp(short dscp)
{
    setTrafficClass(((dscp & 0x3f) << 2) | (trafficClass & 0x03));
}

short Ipv6Header::getEcn() const
{
    return trafficClass & 0x03;
}

void Ipv6Header::setEcn(short ecn)
{
    setTrafficClass((trafficClass & 0xfc) | (ecn & 0x03));
}

} // namespace inet

