//
// Copyright (C) 2009 - today Brno University of Technology, Czech Republic
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
/**
 * @file EigrpDualStack.cc
 * @author Jan Zavrel (honza.zavrel96@gmail.com)
 * @author Vit Rek (rek@kn.vutbr.cz)
 * @author Vladimir Vesely (ivesely@fit.vutbr.cz)
 * @copyright Brno University of Technology (www.fit.vutbr.cz) under GPLv3
 * @date 27. 10. 2014
 * @brief EIGRP Dual-stack (IPv4/IPv6) support
 * @detail Contains functions for dual-stack (IPv4/IPv6) support
 */

#include "inet/routing/eigrp/EigrpDualStack.h"
namespace inet {
/**
 * Uses IPv4Address.getNetmaskLength() method
 */
int getNetmaskLength(const Ipv4Address& netmask)
{
    return netmask.getNetmaskLength();
}

/**
 * Uses four times IPv4Address.getNetmaskLength() method on four parts of IPv6 address
 */
int getNetmaskLength(const Ipv6Address& netmask)
{
    int length = 0;

    for (int i = 0; i <= 3; ++i) {
//        length += IPv4Address(netmask.words()[i]).getNetmaskLength();

        length += (static_cast<Ipv4Address>(netmask.words()[i])).getNetmaskLength(); // TODO - verify!
    }

    return length;
}

bool maskedAddrAreEqual(const Ipv4Address& addr1, const Ipv4Address& addr2, const Ipv4Address& netmask)
{
//    return !(bool)((addr1.addr ^ addr2.addr) & netmask.addr);
    return Ipv4Address::maskedAddrAreEqual(addr1, addr2, netmask);
}

bool maskedAddrAreEqual(const Ipv6Address& addr1, const Ipv6Address& addr2, const Ipv6Address& netmask)
{
    const uint32_t *a1 = addr1.words();
    const uint32_t *a2 = addr2.words();
    const uint32_t *mask = netmask.words();

    return !(static_cast<bool> (
            ((a1[0] ^ a2[0]) & mask[0]) |
            ((a1[1] ^ a2[1]) & mask[1]) |
            ((a1[2] ^ a2[2]) & mask[2]) |
            ((a1[3] ^ a2[3]) & mask[3]))); //TODO - verify!


}

Ipv6Address getPrefix(const Ipv6Address& addr, const Ipv6Address& netmask)
{
    const uint32_t *addrp = addr.words();
    const uint32_t *netmaskp = netmask.words();

    return Ipv6Address(addrp[0] & netmaskp[0], addrp[1] & netmaskp[1], addrp[2] & netmaskp[2], addrp[3] & netmaskp[3]); // TODO - verify
}

Ipv6Address makeNetmask(int length) // TODO - verify
{
    uint32_t netmask[4] = { 0, 0, 0, 0 };

    for (int i = 0; i < 4; ++i) { // through 4 parts of address
        int wlen = length - (i * 32); // computes number of ones bits in part

        if (wlen > 0) { // some bits to set
            netmask[i] = (wlen >= 32) ? 0xffffffffu : ~(0xffffffffu >> wlen); // (Implementation note: MSVC refuses to shift by 32 bits!)
        }
        else { // nothing to set
            break;
        }
    }

    return Ipv6Address(netmask[0], netmask[1], netmask[2], netmask[3]);
}

} // namespace inet

