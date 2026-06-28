//
// Copyright (C) 2009 - today Brno University of Technology, Czech Republic
// Copyright (C) OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
// Babel routing protocol (RFC 6126) ported from the ANSAINET project.
// Original authors: Vit Rek, Vladimir Vesely (Brno University of Technology).
//

#include "inet/routing/babel/BabelDefs.h"

namespace inet {
namespace babel {

namespace defval {
const L3Address MCASTG6 = Ipv6Address("ff02::1:6");
const L3Address MCASTG4 = Ipv4Address("224.0.0.111");
} // namespace defval

int getAeOfAddr(const L3Address& addr)
{
    if (addr.getType() == L3Address::IPv6)
        return isLinkLocal64(addr.toIpv6()) ? AE::LLIPv6 : AE::IPv6;
    return AE::IPv4;
}

int getAeOfPrefix(const L3Address& prefix)
{
    return prefix.getType() == L3Address::IPv6 ? AE::IPv6 : AE::IPv4;
}

std::string AF::toStr(int af)
{
    switch (af) {
        case AF::NONE: return "NONE";
        case AF::IPvX: return "IPvX";
        case AF::IPv4: return "IPv4";
        case AF::IPv6: return "IPv6";
        default: return "<unknown-af>";
    }
}

int AE::maxLen(int ae)
{
    switch (ae) {
        case AE::WILDCARD: return 0;
        case AE::IPv4: return 4;
        case AE::IPv6: return 16;
        case AE::LLIPv6: return 8;
        default: return -1;
    }
}

int AE::toAF(int ae)
{
    switch (ae) {
        case AE::IPv4: return AF::IPv4;
        case AE::IPv6: return AF::IPv6;
        case AE::LLIPv6: return AF::IPv6;
        default: return -1;
    }
}

std::string AE::toStr(int ae)
{
    switch (ae) {
        case AE::WILDCARD: return "WILDCARD";
        case AE::IPv4: return "IPv4";
        case AE::IPv6: return "IPv6";
        case AE::LLIPv6: return "LL-IPv6";
        default: return "<unknown-ae>";
    }
}

std::string tlvT::toStr(int tlvtype)
{
    switch (tlvtype) {
        case tlvT::PAD1: return "PAD1";
        case tlvT::PADN: return "PADN";
        case tlvT::ACKREQ: return "ACKREQ";
        case tlvT::ACK: return "ACK";
        case tlvT::HELLO: return "HELLO";
        case tlvT::IHU: return "IHU";
        case tlvT::ROUTERID: return "ROUTERID";
        case tlvT::NEXTHOP: return "NEXTHOP";
        case tlvT::UPDATE: return "UPDATE";
        case tlvT::ROUTEREQ: return "ROUTEREQ";
        case tlvT::SEQNOREQ: return "SEQNOREQ";
        default: return "<unknown-tlv>";
    }
}

std::string timerT::toStr(int timerT)
{
    switch (timerT) {
        case timerT::HELLO: return "BabelHelloT";
        case timerT::UPDATE: return "BabelUpdateT";
        case timerT::BUFFER: return "BabelBufferT";
        case timerT::BUFFERGC: return "BabelBufferGCT";
        case timerT::TOACKRESEND: return "BabelToAckResendT";
        case timerT::NEIGHHELLO: return "BabelNeighHelloT";
        case timerT::NEIGHIHU: return "BabelNeighIhuT";
        case timerT::ROUTEEXPIRY: return "BabelRouteExpiryT";
        case timerT::ROUTEBEFEXPIRY: return "BabelRouteBefExpiryT";
        case timerT::SOURCEGC: return "BabelSourceGCT";
        case timerT::SRRESEND: return "BabelSeqnoReqResendT";
        default: return "<unknown-timer-type>";
    }
}

void resetTimer(BabelTimer *timer, double delay)
{
    if (timer != nullptr) {
        cSimpleModule *owner = dynamic_cast<cSimpleModule *>(timer->isScheduled() ? timer->getSenderModule() : timer->getOwner());
        if (owner != nullptr)
            owner->scheduleAt(simTime() + delay, owner->cancelEvent(timer));
    }
}

void deleteTimer(BabelTimer **timer)
{
    if (timer != nullptr && *timer != nullptr) {
        cSimpleModule *owner = dynamic_cast<cSimpleModule *>((*timer)->isScheduled() ? (*timer)->getSenderModule() : (*timer)->getOwner());
        if (owner != nullptr) {
            owner->cancelAndDelete(*timer);
            *timer = nullptr;
        }
    }
}

std::string rid::str() const
{
    uint16_t groups[4] = {
        uint16_t(id[0] >> 16), uint16_t(id[0] & 0xffff), uint16_t(id[1] >> 16), uint16_t(id[1] & 0xffff)
    };

    std::stringstream s;
    s << std::hex;
    s << std::setw(4) << std::setfill('0') << groups[0] << ":";
    s << std::setw(4) << std::setfill('0') << groups[1] << ":";
    s << std::setw(4) << std::setfill('0') << groups[2] << ":";
    s << std::setw(4) << std::setfill('0') << groups[3];
    return s.str();
}

std::string routeDistance::str() const
{
    std::stringstream s;
    s << "(" << seqno << ", " << metric << ")";
    return s.str();
}

template<>
netPrefix<Ipv4Address>::netPrefix()
{
    addr = Ipv4Address();
    len = 0;
}

template<>
netPrefix<Ipv6Address>::netPrefix()
{
    addr = Ipv6Address();
    len = 0;
}

template<>
netPrefix<L3Address>::netPrefix()
{
    addr = L3Address(Ipv6Address()); // default L3Address is IPv4 -> force IPv6
    len = 0;
}

template<>
void netPrefix<Ipv4Address>::set(Ipv4Address a, uint8_t plen)
{
    ASSERT(plen <= 32);
    addr = a.doAnd(Ipv4Address::makeNetmask(plen));
    len = plen;
}

template<>
void netPrefix<Ipv6Address>::set(Ipv6Address a, uint8_t plen)
{
    ASSERT(plen <= 128);
    addr.set(0, 0, 0, 0);
    addr.setPrefix(a, plen);
    len = plen;
}

template<>
void netPrefix<L3Address>::set(L3Address a, uint8_t plen)
{
    ASSERT(plen <= 128);
    if (a.getType() == L3Address::IPv6)
        addr.set(Ipv6Address().setPrefix(a.toIpv6(), plen));
    else
        addr.set(a.toIpv4().doAnd(Ipv4Address::makeNetmask(plen)));
    len = plen;
}

} // namespace babel
} // namespace inet
