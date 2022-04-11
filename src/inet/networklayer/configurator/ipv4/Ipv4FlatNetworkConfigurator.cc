//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/networklayer/configurator/ipv4/Ipv4FlatNetworkConfigurator.h"

#include <algorithm>

#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/networklayer/common/NetworkInterface.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/ipv4/IIpv4RoutingTable.h"
#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"

namespace inet {

Define_Module(Ipv4FlatNetworkConfigurator);

void Ipv4FlatNetworkConfigurator::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_NETWORK_CONFIGURATION) {
        Topology topo("topo");
        NodeInfoVector nodeInfo; // will be of size topo.nodes[]

        // extract topology into the Topology object, then fill in
        // isIPNode, rt and ift members of nodeInfo[]
        extractTopology(topo, nodeInfo);

        // assign addresses to Ipv4 nodes, and also store result in nodeInfo[].address
        assignAddresses(topo, nodeInfo);

        // add default routes to hosts (nodes with a single attachment);
        // also remember result in nodeInfo[].usesDefaultRoute
        addDefaultRoutes(topo, nodeInfo);

        // calculate shortest paths, and add corresponding static routes
        fillRoutingTables(topo, nodeInfo);

        // update display string
        setDisplayString(topo, nodeInfo);
    }
}

void Ipv4FlatNetworkConfigurator::extractTopology(Topology& topo, NodeInfoVector& nodeInfo)
{
    // extract topology
    topo.extractByProperty("networkNode");
    EV_DEBUG << "Topology found " << topo.getNumNodes() << " nodes\n";

    // fill in isIPNode, ift and rt members in nodeInfo[]
    nodeInfo.resize(topo.getNumNodes());
    for (int i = 0; i < topo.getNumNodes(); i++) {
        cModule *mod = topo.getNode(i)->getModule();
        nodeInfo[i].isIPNode = L3AddressResolver().findIpv4RoutingTableOf(mod) != nullptr && L3AddressResolver().findInterfaceTableOf(mod) != nullptr;
        if (nodeInfo[i].isIPNode) {
            nodeInfo[i].ift = L3AddressResolver().interfaceTableOf(mod);
            nodeInfo[i].rt = L3AddressResolver().getIpv4RoutingTableOf(mod);
            nodeInfo[i].ipForwardEnabled = mod->hasPar("forwarding") ? mod->par("forwarding") : false;
            topo.getNode(i)->setWeight(nodeInfo[i].ipForwardEnabled ? 0.0 : INFINITY);
        }
    }
}

void Ipv4FlatNetworkConfigurator::assignAddresses(Topology& topo, NodeInfoVector& nodeInfo)
{
    // assign Ipv4 addresses
    uint32_t networkAddress = Ipv4Address(par("networkAddress").stringValue()).getInt();
    uint32_t netmask = Ipv4Address(par("netmask").stringValue()).getInt();
    int maxNodes = (~netmask) - 1; // 0 and ffff have special meaning and cannot be used
    if (topo.getNumNodes() > maxNodes)
        throw cRuntimeError("netmask too large, not enough addresses for all %d nodes", topo.getNumNodes());

    int numIPNodes = 0;
    for (int i = 0; i < topo.getNumNodes(); i++) {
        // skip bus types
        if (!nodeInfo[i].isIPNode)
            continue;

        uint32_t addr = networkAddress | uint32_t(++numIPNodes);
        nodeInfo[i].address.set(addr);

        // find interface table and assign address to all (non-loopback) interfaces
        IInterfaceTable *ift = nodeInfo[i].ift;
        for (int k = 0; k < ift->getNumInterfaces(); k++) {
            NetworkInterface *ie = ift->getInterface(k);
            if (!ie->isLoopback()) {
                auto ipv4Data = ie->getProtocolDataForUpdate<Ipv4InterfaceData>();
                ipv4Data->setIPAddress(Ipv4Address(addr));
                ipv4Data->setNetmask(Ipv4Address::ALLONES_ADDRESS); // full address must match for local delivery
            }
        }
    }
}

void Ipv4FlatNetworkConfigurator::addDefaultRoutes(Topology& topo, NodeInfoVector& nodeInfo)
{
    // add default route to nodes with exactly one (non-loopback) interface
    for (int i = 0; i < topo.getNumNodes(); i++) {
        Topology::Node *node = topo.getNode(i);

        // skip bus types
        if (!nodeInfo[i].isIPNode)
            continue;

        IInterfaceTable *ift = nodeInfo[i].ift;
        IIpv4RoutingTable *rt = nodeInfo[i].rt;

        // count non-loopback interfaces
        int numIntf = 0;
        NetworkInterface *ie = nullptr;
        for (int k = 0; k < ift->getNumInterfaces(); k++)
            if (!ift->getInterface(k)->isLoopback()) {
                ie = ift->getInterface(k);
                numIntf++;
            }

        nodeInfo[i].usesDefaultRoute = (numIntf == 1);
        if (numIntf != 1)
            continue; // only deal with nodes with one interface plus loopback

        EV_INFO << "  " << node->getModule()->getFullName() << "=" << nodeInfo[i].address
                << " has only one (non-loopback) interface, adding default route\n";

        // add route
        Ipv4Route *e = new Ipv4Route();
        e->setDestination(Ipv4Address());
        e->setNetmask(Ipv4Address());
        e->setInterface(ie);
        e->setSourceType(IRoute::MANUAL);
//        e->getMetric() = 1;
        rt->addRoute(e);
    }
}

void Ipv4FlatNetworkConfigurator::fillRoutingTables(Topology& topo, NodeInfoVector& nodeInfo)
{
    // fill in routing tables with static routes
    for (int i = 0; i < topo.getNumNodes(); i++) {
        Topology::Node *destNode = topo.getNode(i);

        // skip bus types
        if (!nodeInfo[i].isIPNode)
            continue;

        Ipv4Address destAddr = nodeInfo[i].address;
        std::string destModName = destNode->getModule()->getFullName();

        // calculate shortest paths from everywhere towards destNode
        topo.calculateWeightedSingleShortestPathsTo(destNode);

        // add route (with host=destNode) to every routing table in the network
        // (excepting nodes with only one interface -- there we'll set up a default route)
        for (int j = 0; j < topo.getNumNodes(); j++) {
            if (i == j)
                continue;
            if (!nodeInfo[j].isIPNode)
                continue;

            Topology::Node *atNode = topo.getNode(j);
            if (atNode->getNumPaths() == 0)
                continue; // not connected
            if (nodeInfo[j].usesDefaultRoute)
                continue; // already added default route here

            Ipv4Address atAddr = nodeInfo[j].address;

            IInterfaceTable *ift = nodeInfo[j].ift;

            int outputGateId = atNode->getPath(0)->getLinkOutLocalGate()->getId();
            NetworkInterface *ie = ift->findInterfaceByNodeOutputGateId(outputGateId);
            if (!ie)
                throw cRuntimeError("%s has no interface for output gate id %d", ift->getFullPath().c_str(), outputGateId);

            EV_INFO << "  from " << atNode->getModule()->getFullName() << "=" << Ipv4Address(atAddr);
            EV_INFO << " towards " << destModName << "=" << Ipv4Address(destAddr) << " interface " << ie->getInterfaceName() << endl;

            // add route
            IIpv4RoutingTable *rt = nodeInfo[j].rt;
            Ipv4Route *e = new Ipv4Route();
            e->setDestination(destAddr);
            e->setNetmask(Ipv4Address(255, 255, 255, 255)); // full match needed
            e->setInterface(ie);
            e->setSourceType(IRoute::MANUAL);
//            e->getMetric() = 1;
            rt->addRoute(e);
        }
    }
}

void Ipv4FlatNetworkConfigurator::handleMessage(cMessage *msg)
{
    throw cRuntimeError("this module doesn't handle messages, it runs only in initialize()");
}

void Ipv4FlatNetworkConfigurator::setDisplayString(Topology& topo, NodeInfoVector& nodeInfo)
{
    int numIPNodes = 0;
    for (int i = 0; i < topo.getNumNodes(); i++)
        if (nodeInfo[i].isIPNode)
            numIPNodes++;

    // update display string
    char buf[80];
    sprintf(buf, "%d Ipv4 nodes\n%d non-Ipv4 nodes", numIPNodes, topo.getNumNodes() - numIPNodes);
    getDisplayString().setTagArg("t", 0, buf);
}

} // namespace inet

