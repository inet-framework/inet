//
// Copyright (C) 2009 Christoph Sommer <christoph.sommer@informatik.uni-erlangen.de>
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/networklayer/configurator/ipv4/HostAutoConfigurator.h"

#include <algorithm>

#include "inet/common/ModuleAccess.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/contract/ipv4/Ipv4Address.h"
#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"

namespace inet {

Define_Module(HostAutoConfigurator);

void HostAutoConfigurator::initialize(int stage)
{
    OperationalBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        interfaceTable.reference(this, "interfaceTableModule", true);
    }
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

    Ipv4Address myAddress = Ipv4Address(addressBase.getInt() + uint32_t(host->getId()));

    // address test
    if (!Ipv4Address::maskedAddrAreEqual(myAddress, addressBase, netmask))
        throw cRuntimeError("Generated IP address is out of specified address range");

    // get our routing table
    IIpv4RoutingTable *routingTable = L3AddressResolver().getIpv4RoutingTableOf(host);
    if (!routingTable)
        throw cRuntimeError("No routing table found");

    // look at all interface table entries
    cStringTokenizer interfaceTokenizer(interfaces.c_str());
    const char *ifname;
    uint32_t loopbackAddr = Ipv4Address::LOOPBACK_ADDRESS.getInt();
    while ((ifname = interfaceTokenizer.nextToken()) != nullptr) {
        NetworkInterface *ie = interfaceTable->findInterfaceByName(ifname);
        if (!ie)
            throw cRuntimeError("No such interface '%s'", ifname);

        auto ipv4Data = ie->getProtocolDataForUpdate<Ipv4InterfaceData>();
        // assign IP Address to all connected interfaces
        if (ie->isLoopback()) {
            ipv4Data->setIPAddress(Ipv4Address(loopbackAddr++));
            ipv4Data->setNetmask(Ipv4Address::LOOPBACK_NETMASK);
            ipv4Data->setMetric(1);
            EV_INFO << "loopback interface " << ifname << " gets " << ipv4Data->getIPAddress() << "/" << ipv4Data->getNetmask() << std::endl;
            continue;
        }

        EV_INFO << "interface " << ifname << " gets " << myAddress.str() << "/" << netmask.str() << std::endl;

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

void HostAutoConfigurator::handleStartOperation(LifecycleOperation *operation)
{
    if (operation == nullptr) {
        // in initialize:
        for (int i = 0; i < interfaceTable->getNumInterfaces(); i++)
            interfaceTable->getInterface(i)->addProtocolData<Ipv4InterfaceData>();
    }
    setupNetworkLayer();
}

void HostAutoConfigurator::handleStopOperation(LifecycleOperation *operation)
{
}

void HostAutoConfigurator::handleCrashOperation(LifecycleOperation *operation)
{
}

} // namespace inet

