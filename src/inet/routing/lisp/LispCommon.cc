//
// Copyright (C) 2013 - today Brno University of Technology, Czech Republic
// Copyright (C) OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
// LISP (Locator/ID Separation Protocol, RFC 6830) ported from the ANSAINET project.
// Original author: Vladimir Vesely (Brno University of Technology).
//

#include "inet/routing/lisp/LispCommon.h"

namespace inet {
namespace lisp {

void LispCommon::parseIpAddress(const char *str, std::string& address, std::string& length)
{
    std::string addr = str;
    size_t pos = addr.find("/");
    length = addr.substr(pos + 1);
    address = addr.substr(0, pos);
}

int LispCommon::getNumMatchingPrefixBits4(Ipv4Address addr1, Ipv4Address addr2)
{
    uint32_t res = addr1.getInt() ^ addr2.getInt();
    for (int i = 31; i >= 0; i--)
        if (res & (1u << i))
            return 31 - i; // first differing bit from the left
    return -1; // exact match
}

int LispCommon::getNumMatchingPrefixBits6(Ipv6Address addr1, Ipv6Address addr2)
{
    const uint32_t *w1 = addr1.words();
    const uint32_t *w2 = addr2.words();
    for (int j = 0; j <= 3; ++j) {
        uint32_t res = w1[j] ^ w2[j];
        for (int i = 31; i >= 0; i--)
            if (res & (1u << i))
                return 32 * j + 31 - i;
    }
    return -1; // exact match
}

int LispCommon::doPrefixMatch(L3Address addr1, L3Address addr2)
{
    if ((addr1.getType() == L3Address::IPv6) != (addr2.getType() == L3Address::IPv6))
        return -2; // incomparable address families
    if (addr1.getType() != L3Address::IPv6)
        return getNumMatchingPrefixBits4(addr1.toIpv4(), addr2.toIpv4());
    else
        return getNumMatchingPrefixBits6(addr1.toIpv6(), addr2.toIpv6());
}

L3Address LispCommon::getNetworkAddress(L3Address address, int length)
{
    if (address.getType() == L3Address::IPv6 && length >= 0 && length <= 128) {
        Ipv6Address mask = Ipv6Address::constructMask(length);
        const uint32_t *w1 = address.toIpv6().words();
        const uint32_t *w2 = mask.words();
        return Ipv6Address(w1[0] & w2[0], w1[1] & w2[1], w1[2] & w2[2], w1[3] & w2[3]);
    }
    else if (address.getType() != L3Address::IPv6 && length >= 0 && length <= 32) {
        Ipv4Address mask = Ipv4Address::makeNetmask(length);
        return address.toIpv4().doAnd(mask);
    }
    return L3Address();
}

} // namespace lisp
} // namespace inet
