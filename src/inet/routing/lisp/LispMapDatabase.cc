//
// Copyright (C) 2013 - today Brno University of Technology, Czech Republic
// Copyright (C) OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
// LISP (Locator/ID Separation Protocol, RFC 6830) ported from the ANSAINET project.
// Original author: Vladimir Vesely (Brno University of Technology).
//

#include "inet/routing/lisp/LispMapDatabase.h"

#include "inet/networklayer/common/NetworkInterface.h"
#ifdef INET_WITH_IPv4
#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"
#endif
#ifdef INET_WITH_IPv6
#include "inet/networklayer/ipv6/Ipv6InterfaceData.h"
#endif

namespace inet {
namespace lisp {

// reads an interface's IPv4 and IPv6 addresses (unspecified if absent)
static void getInterfaceAddresses(NetworkInterface *ie, Ipv4Address& a4, Ipv6Address& a6)
{
    a4 = Ipv4Address::UNSPECIFIED_ADDRESS;
    a6 = Ipv6Address::UNSPECIFIED_ADDRESS;
#ifdef INET_WITH_IPv4
    if (auto data = ie->findProtocolData<Ipv4InterfaceData>())
        a4 = data->getIPAddress();
#endif
#ifdef INET_WITH_IPv6
    if (auto data = ie->findProtocolData<Ipv6InterfaceData>())
        a6 = data->getPreferredAddress();
#endif
}

void LispMapDatabase::load(cXMLElement *etrMappingConfig, IInterfaceTable *ift, bool advertOnlyOwnEids)
{
    if (etrMappingConfig)
        parseMapEntry(etrMappingConfig);

    // keep only the EIDs that match one of this node's interface addresses
    if (advertOnlyOwnEids) {
        MapStorage filtered;
        for (auto& entry : MappingStorage) {
            int len = entry.getEidPrefix().getEidLength();
            for (int i = 0; i < ift->getNumInterfaces(); ++i) {
                Ipv4Address a4;
                Ipv6Address a6;
                getInterfaceAddresses(ift->getInterface(i), a4, a6);
                if (entry.getEidPrefix().getEidAddr() == LispCommon::getNetworkAddress(a4, len)
                        || entry.getEidPrefix().getEidAddr() == LispCommon::getNetworkAddress(a6, len)) {
                    filtered.push_back(entry);
                    break;
                }
            }
        }
        MappingStorage = filtered;
    }

    // mark locators that are local interface addresses
    for (int i = 0; i < ift->getNumInterfaces(); ++i) {
        Ipv4Address a4;
        Ipv6Address a6;
        getInterfaceAddresses(ift->getInterface(i), a4, a6);
        for (auto& entry : MappingStorage)
            for (auto& rloc : entry.getRlocs())
                if (rloc.getRlocAddr() == a4 || rloc.getRlocAddr() == a6) {
                    rloc.setLocal(true);
                    rloc.setState(LispRlocator::UP); // the ETR's own locators are reachable
                }
    }
}

} // namespace lisp
} // namespace inet
