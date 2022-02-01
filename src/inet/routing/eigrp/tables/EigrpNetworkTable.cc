//
// Copyright (C) 2009 - today Brno University of Technology, Czech Republic
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
/**
 * @author Jan Zavrel (honza.zavrel96@gmail.com)
 * @author Jan Bloudicek (jbloudicek@gmail.com)
 * @author Vit Rek (rek@kn.vutbr.cz)
 * @author Vladimir Vesely (ivesely@fit.vutbr.cz)
 * @copyright Brno University of Technology (www.fit.vutbr.cz) under GPLv3
 */

#include "inet/routing/eigrp/tables/EigrpNetworkTable.h"
namespace inet {
namespace eigrp {
template<typename IPAddress>
EigrpNetworkTable<IPAddress>::~EigrpNetworkTable()
{
    int cnt = networkVec.size();

    for (int i = 0; i < cnt; i++) {
        delete networkVec[i];
    }
    networkVec.clear();
}

template<typename IPAddress>
EigrpNetwork<IPAddress> *EigrpNetworkTable<IPAddress>::addNetwork(IPAddress& address, IPAddress& mask)
{
    typename std::vector<EigrpNetwork<IPAddress> *>::iterator it;
    for (it = networkVec.begin(); it != networkVec.end(); ++it) { // through all networks search same
        if ((*it)->getAddress() == address && (*it)->getMask() == mask) { // found same -> do not add new
            return *it;
        }
    }

    // Not found -> add new
    EigrpNetwork<IPAddress> *net = new EigrpNetwork<IPAddress>(address, mask, networkCnt++);
    networkVec.push_back(net);
    return net;
}

template<typename IPAddress>
EigrpNetwork<IPAddress> *EigrpNetworkTable<IPAddress>::findNetworkById(int netId)
{
    typename std::vector<EigrpNetwork<IPAddress> *>::iterator it;

    for (it = networkVec.begin(); it != networkVec.end(); it++) {
        if ((*it)->getNetworkId() == netId) {
            return *it;
        }
    }

    return nullptr;
}

template<>
bool EigrpNetworkTable<Ipv4Address>::isInterfaceIncluded(const Ipv4Address& ifAddress, const Ipv4Address& ifMask, int *resultNetId)
{
    typename std::vector<EigrpNetwork<Ipv4Address> *>::iterator it;
    int netMaskLen, ifMaskLen;

    if (ifAddress.isUnspecified())
        return false;

    for (it = networkVec.begin(); it != networkVec.end(); it++) {
        Ipv4Address netPrefix = (*it)->getAddress();
        Ipv4Address netMask = (*it)->getMask();

        netMaskLen = (netMask.isUnspecified()) ? getNetmaskLength(netPrefix.getNetworkMask()) : getNetmaskLength(netMask);
        ifMaskLen = getNetmaskLength(ifMask);

        // prefix isUnspecified -> network = 0.0.0.0 -> all interfaces, or
        // mask is unspecified -> classful match or
        // mask is specified -> classless match
        if (netPrefix.isUnspecified() ||
            (netMask.isUnspecified() && netPrefix.isNetwork(ifAddress) && netMaskLen <= ifMaskLen) ||
            (maskedAddrAreEqual(netPrefix, ifAddress, netMask) && netMaskLen <= ifMaskLen))
        { // IP address of the interface match the prefix
            (*resultNetId) = (*it)->getNetworkId();
            return true;
        }
    }

    return false;
}

/**
 * Determines if specified netaddress/mask is included in eigrp process
 * @param   ifAddress   address of interface
 * @param   ifMask      mask of interface
 * @param   resultNetId ID of network
 * @return  True (and set resultNetId) if address is included in network, which is enabled in Eigrp process, otherwise false (resultNetId is undefined).
 *
 * In IPv6 is netmask always specified, because it is strictly classless.
 * Configuration is defined per interfaces, and there is no wildcard.
 * In other words matching is much simpler that in the IPv4.
 */
template<>
bool EigrpNetworkTable<Ipv6Address>::isInterfaceIncluded(const Ipv6Address& ifAddress, const Ipv6Address& ifMask, int *resultNetId)
{
    typename std::vector<EigrpNetwork<Ipv6Address> *>::iterator it;
    int netMaskLen, ifMaskLen;

    if (ifAddress.isUnspecified())
        return false; // interface without address can not be included in Eigrp process

    for (it = networkVec.begin(); it != networkVec.end(); it++) { // go through all networks and determine if ifAddress is from that network
        Ipv6Address netPrefix = (*it)->getAddress(); // network address
        Ipv6Address netMask = (*it)->getMask(); // network mask

        netMaskLen = getNetmaskLength(netMask);
        ifMaskLen = getNetmaskLength(ifMask);

        // prefix isUnspecified -> network = 0.0.0.0 -> all interfaces, or
        // prefix is specified -> classless match
        if (netPrefix.isUnspecified() ||
            (maskedAddrAreEqual(netPrefix, ifAddress, netMask) && netMaskLen <= ifMaskLen)) // TODO - PROB-02 - reused from IPv4 version, is it ok?
        { // IP address of the interface match the prefix
            (*resultNetId) = (*it)->getNetworkId();
            return true;
        }
    }

    return false;
}

template class EigrpNetworkTable<Ipv4Address>;

#ifndef DISABLE_EIGRP_IPV6
template class EigrpNetworkTable<Ipv6Address>;
#endif /* DISABLE_EIGRP_IPV6 */
} // eigrp
} // inet

