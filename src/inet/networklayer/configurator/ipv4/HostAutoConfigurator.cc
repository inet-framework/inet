/*
 * Copyright (C) 2009 Christoph Sommer <christoph.sommer@informatik.uni-erlangen.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <stdexcept>
#include <algorithm>

#include "inet/networklayer/configurator/ipv4/HostAutoConfigurator.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"
#include "inet/networklayer/ipv4/IIpv4RoutingTable.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/contract/ipv4/Ipv4Address.h"

namespace inet {

Define_Module(HostAutoConfigurator);

void HostAutoConfigurator::initialize(int stage)
{
    OperationalBase::initialize(stage);
}

void HostAutoConfigurator::finish()
{
}

void HostAutoConfigurator::handleMessageWhenUp(cMessage *apMsg)
{
}

void HostAutoConfigurator::setupNetworkLayer()
{
    EV_INFO << "host auto configuration started" << std::endl;

    std::string interfaces = par("interfaces");
    Ipv4Address addressBase = Ipv4Address(par("addressBase").stringValue());
    Ipv4Address netmask = Ipv4Address(par("netmask").stringValue());
    std::string mcastGroups = par("mcastGroups").stdstringValue();

    // get our host module
    cModule *host = getContainingNode(this);

    Ipv4Address myAddress = Ipv4Address(addressBase.getInt() + uint32(host->getId()));

    // address test
    if (!Ipv4Address::maskedAddrAreEqual(myAddress, addressBase, netmask))
        throw cRuntimeError("Generated IP address is out of specified address range");

    // get our routing table
    IIpv4RoutingTable *routingTable = L3AddressResolver().getIpv4RoutingTableOf(host);
    if (!routingTable)
        throw cRuntimeError("No routing table found");

    // get our interface table
    IInterfaceTable *ift = L3AddressResolver().interfaceTableOf(host);
    if (!ift)
        throw cRuntimeError("No interface table found");

    // look at all interface table entries
    cStringTokenizer interfaceTokenizer(interfaces.c_str());
    const char *ifname;
    while ((ifname = interfaceTokenizer.nextToken()) != nullptr) {
        InterfaceEntry *ie = ift->getInterfaceByName(ifname);
        if (!ie)
            throw cRuntimeError("No such interface '%s'", ifname);

        // assign IP Address to all connected interfaces
        if (ie->isLoopback()) {
            EV_INFO << "interface " << ifname << " skipped (is loopback)" << std::endl;
            continue;
        }

        EV_INFO << "interface " << ifname << " gets " << myAddress.str() << "/" << netmask.str() << std::endl;

        auto ipv4Data = ie->getProtocolData<Ipv4InterfaceData>();
        ipv4Data->setIPAddress(myAddress);
        ipv4Data->setNetmask(netmask);
        ie->setBroadcast(true);

        // associate interface with default multicast groups
        ipv4Data->joinMulticastGroup(Ipv4Address::ALL_HOSTS_MCAST);
        ipv4Data->joinMulticastGroup(Ipv4Address::ALL_ROUTERS_MCAST);

        // associate interface with specified multicast groups
        cStringTokenizer interfaceTokenizer(mcastGroups.c_str());
        const char *mcastGroup_s;
        while ((mcastGroup_s = interfaceTokenizer.nextToken()) != nullptr) {
            Ipv4Address mcastGroup(mcastGroup_s);
            ipv4Data->joinMulticastGroup(mcastGroup);
        }
    }
}

} // namespace inet

