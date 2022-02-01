//
// Copyright (C) 2009 - today Brno University of Technology, Czech Republic
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
/**
 * @file EigrpDualStack.h
 * @author Jan Zavrel (honza.zavrel96@gmail.com)
 * @author Vit Rek (rek@kn.vutbr.cz)
 * @author Vladimir Vesely (ivesely@fit.vutbr.cz)
 * @copyright Brno University of Technology (www.fit.vutbr.cz) under GPLv3
 * @date 27. 10. 2014
 * @brief EIGRP Dual-stack (IPv4/IPv6) support header file
 * @detail Functions prototypes for dual-stack (IPv4/IPv6) support
 */

#ifndef __INET_EIGRPDUALSTACK_H
#define __INET_EIGRPDUALSTACK_H

//#include "IPv4Address.h"
//#include "IPv6Address.h"
#include "inet/networklayer/common/L3Address.h"
//#include "ANSAIPv6Address.h"

//#define DISABLE_EIGRP_IPV6

namespace inet {

/**
 * Computes length of IPv4 netmask represented as address
 *
 * @param   netmask IPv4 netmask
 * @return  Length of netmask
 */
int getNetmaskLength(const Ipv4Address& netmask);

/**
 * Computes length of IPv6 netmask represented as address
 *
 * @param   netmask IPv6 netmask
 * @return  Length of netmask
 */
int getNetmaskLength(const Ipv6Address& netmask);

/**
 * Compare two IPv4 addresses masked by netmask
 *
 * @param   addr1   first address to compare
 * @param   addr2   second address to compare
 * @param   netmask network mask used for masking
 * @return  True if masked addresses are equal, otherwise false
 */
bool maskedAddrAreEqual(const Ipv4Address& addr1, const Ipv4Address& addr2, const Ipv4Address& netmask);

/**
 * Compare two IPv6 addresses masked by netmask
 *
 * @param   addr1   first address to compare
 * @param   addr2   second address to compare
 * @param   netmask network mask used for masking
 * @return  True if masked addresses are equal, otherwise false
 */
bool maskedAddrAreEqual(const Ipv6Address& addr1, const Ipv6Address& addr2, const Ipv6Address& netmask);

/**
 * Get prefix from IPv6 address and network mask (represented as address)
 *
 * @param   addr    address
 * @param   netmask network mask
 * @return  IPv6 network prefix
 */
Ipv6Address getPrefix(const Ipv6Address& addr, const Ipv6Address& netmask);

/**
 * Make network mask represented as IPv6 address from netmask length
 *
 * @param   mask length
 * @return  network mask represented as IPv6 address
 *
 * @note    Instead of this function, you can simply use: IPv6Address(0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff).getPrefix(prefixLength)
 */
Ipv6Address makeNetmask(int length);

} // namespace inet
#endif

