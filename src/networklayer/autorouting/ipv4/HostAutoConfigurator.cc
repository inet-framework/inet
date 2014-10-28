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

#include "HostAutoConfigurator.h"

#include "IPvXAddressResolver.h"
#include "IPv4InterfaceData.h"
#include "IRoutingTable.h"
#include "IInterfaceTable.h"
#include "IPv4Address.h"

Define_Module(HostAutoConfigurator);


void HostAutoConfigurator::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == 2)
    {
        setupNetworkLayer();
    }
}

void HostAutoConfigurator::finish() {
}

void HostAutoConfigurator::handleMessage(cMessage* apMsg) {
}

void HostAutoConfigurator::setupNetworkLayer()
{
    EV << "host auto configuration started" << std::endl;

    std::string interfaces = par("interfaces").stringValue();
    IPv4Address addressBase = IPv4Address(par("addressBase").stringValue());
    IPv4Address netmask = IPv4Address(par("netmask").stringValue());
    std::string mcastGroups = par("mcastGroups").stringValue();
    IPv4Address myAddress = IPv4Address(addressBase.getInt() + uint32(getParentModule()->getId()));

    // address test
    if (!IPv4Address::maskedAddrAreEqual(myAddress, addressBase, netmask))
        throw cRuntimeError("Generated IP address is out of specified address range");

    // get our host module
    cModule* host = getParentModule();
    if (!host) throw cRuntimeError("No parent module found");

    // get our routing table
    IRoutingTable* routingTable = IPvXAddressResolver().routingTableOf(host);
    if (!routingTable) throw cRuntimeError("No routing table found");

    // get our interface table
    IInterfaceTable *ift = IPvXAddressResolver().interfaceTableOf(host);
    if (!ift) throw cRuntimeError("No interface table found");

    // look at all interface table entries
    cStringTokenizer interfaceTokenizer(interfaces.c_str());
    const char *ifname;
    while ((ifname = interfaceTokenizer.nextToken()) != NULL) {
        InterfaceEntry* ie = ift->getInterfaceByName(ifname);
        if (!ie) throw cRuntimeError("No such interface '%s'", ifname);

        // assign IP Address to all connected interfaces
        if (ie->isLoopback()) {
            EV << "interface " << ifname << " skipped (is loopback)" << std::endl;
            continue;
        }

        EV << "interface " << ifname << " gets " << myAddress.str() << "/" << netmask.str() << std::endl;

        ie->ipv4Data()->setIPAddress(myAddress);
        ie->ipv4Data()->setNetmask(netmask);
        ie->setBroadcast(true);

        // associate interface with default multicast groups
        ie->ipv4Data()->joinMulticastGroup(IPv4Address::ALL_HOSTS_MCAST);
        ie->ipv4Data()->joinMulticastGroup(IPv4Address::ALL_ROUTERS_MCAST);

        // associate interface with specified multicast groups
        cStringTokenizer interfaceTokenizer(mcastGroups.c_str());
        const char *mcastGroup_s;
        while ((mcastGroup_s = interfaceTokenizer.nextToken()) != NULL) {
            IPv4Address mcastGroup(mcastGroup_s);
            ie->ipv4Data()->joinMulticastGroup(mcastGroup);
        }
    }
}

