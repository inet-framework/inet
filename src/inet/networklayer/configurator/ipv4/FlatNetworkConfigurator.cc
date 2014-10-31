//
// Copyright (C) 2004 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include <algorithm>
#include "inet/networklayer/ipv4/IIPv4RoutingTable.h"
#include "inet/networklayer/common/IInterfaceTable.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/networklayer/configurator/ipv4/FlatNetworkConfigurator.h"
#include "inet/networklayer/common/InterfaceEntry.h"
#include "inet/networklayer/ipv4/IPv4InterfaceData.h"

namespace inet {

typedef cTopology WeightedTopology;

Define_Module(FlatNetworkConfigurator);

void FlatNetworkConfigurator::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_NETWORK_LAYER_2) {
        cTopology topo("topo");
        NodeInfoVector nodeInfo;    // will be of size topo.nodes[]

        // extract topology into the cTopology object, then fill in
        // isIPNode, rt and ift members of nodeInfo[]
        extractTopology(topo, nodeInfo);

        // assign addresses to IPv4 nodes, and also store result in nodeInfo[].address
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

void FlatNetworkConfigurator::extractTopology(cTopology& topo, NodeInfoVector& nodeInfo)
{
    // extract topology
    topo.extractByProperty("node");
    EV_DEBUG << "cTopology found " << topo.getNumNodes() << " nodes\n";

    // fill in isIPNode, ift and rt members in nodeInfo[]
    nodeInfo.resize(topo.getNumNodes());
    for (int i = 0; i < topo.getNumNodes(); i++) {
        cModule *mod = topo.getNode(i)->getModule();
        nodeInfo[i].isIPNode = L3AddressResolver().findIPv4RoutingTableOf(mod) != NULL && L3AddressResolver().findInterfaceTableOf(mod) != NULL;
        if (nodeInfo[i].isIPNode) {
            nodeInfo[i].ift = L3AddressResolver().interfaceTableOf(mod);
            nodeInfo[i].rt = L3AddressResolver().routingTableOf(mod);
            nodeInfo[i].ipForwardEnabled = mod->hasPar("forwarding") ? mod->par("forwarding").boolValue() : false;
            topo.getNode(i)->setWeight(nodeInfo[i].ipForwardEnabled ? 0.0 : INFINITY);
        }
    }
}

void FlatNetworkConfigurator::assignAddresses(cTopology& topo, NodeInfoVector& nodeInfo)
{
    // assign IPv4 addresses
    uint32 networkAddress = IPv4Address(par("networkAddress").stringValue()).getInt();
    uint32 netmask = IPv4Address(par("netmask").stringValue()).getInt();
    int maxNodes = (~netmask) - 1;    // 0 and ffff have special meaning and cannot be used
    if (topo.getNumNodes() > maxNodes)
        throw cRuntimeError("netmask too large, not enough addresses for all %d nodes", topo.getNumNodes());

    int numIPNodes = 0;
    for (int i = 0; i < topo.getNumNodes(); i++) {
        // skip bus types
        if (!nodeInfo[i].isIPNode)
            continue;

        uint32 addr = networkAddress | uint32(++numIPNodes);
        nodeInfo[i].address.set(addr);

        // find interface table and assign address to all (non-loopback) interfaces
        IInterfaceTable *ift = nodeInfo[i].ift;
        for (int k = 0; k < ift->getNumInterfaces(); k++) {
            InterfaceEntry *ie = ift->getInterface(k);
            if (!ie->isLoopback()) {
                ie->ipv4Data()->setIPAddress(IPv4Address(addr));
                ie->ipv4Data()->setNetmask(IPv4Address::ALLONES_ADDRESS);    // full address must match for local delivery
            }
        }
    }
}

void FlatNetworkConfigurator::addDefaultRoutes(cTopology& topo, NodeInfoVector& nodeInfo)
{
    // add default route to nodes with exactly one (non-loopback) interface
    for (int i = 0; i < topo.getNumNodes(); i++) {
        cTopology::Node *node = topo.getNode(i);

        // skip bus types
        if (!nodeInfo[i].isIPNode)
            continue;

        IInterfaceTable *ift = nodeInfo[i].ift;
        IIPv4RoutingTable *rt = nodeInfo[i].rt;

        // count non-loopback interfaces
        int numIntf = 0;
        InterfaceEntry *ie = NULL;
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
        IPv4Route *e = new IPv4Route();
        e->setDestination(IPv4Address());
        e->setNetmask(IPv4Address());
        e->setInterface(ie);
        e->setSourceType(IRoute::MANUAL);
        //e->getMetric() = 1;
        rt->addRoute(e);
    }
}

void FlatNetworkConfigurator::fillRoutingTables(cTopology& topo, NodeInfoVector& nodeInfo)
{
    // fill in routing tables with static routes
    for (int i = 0; i < topo.getNumNodes(); i++) {
        cTopology::Node *destNode = topo.getNode(i);

        // skip bus types
        if (!nodeInfo[i].isIPNode)
            continue;

        IPv4Address destAddr = nodeInfo[i].address;
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

            cTopology::Node *atNode = topo.getNode(j);
            if (atNode->getNumPaths() == 0)
                continue; // not connected
            if (nodeInfo[j].usesDefaultRoute)
                continue; // already added default route here

            IPv4Address atAddr = nodeInfo[j].address;

            IInterfaceTable *ift = nodeInfo[j].ift;

            int outputGateId = atNode->getPath(0)->getLocalGate()->getId();
            InterfaceEntry *ie = ift->getInterfaceByNodeOutputGateId(outputGateId);
            if (!ie)
                throw cRuntimeError("%s has no interface for output gate id %d", ift->getFullPath().c_str(), outputGateId);

            EV_INFO << "  from " << atNode->getModule()->getFullName() << "=" << IPv4Address(atAddr);
            EV_INFO << " towards " << destModName << "=" << IPv4Address(destAddr) << " interface " << ie->getName() << endl;

            // add route
            IIPv4RoutingTable *rt = nodeInfo[j].rt;
            IPv4Route *e = new IPv4Route();
            e->setDestination(destAddr);
            e->setNetmask(IPv4Address(255, 255, 255, 255));    // full match needed
            e->setInterface(ie);
            e->setSourceType(IRoute::MANUAL);
            //e->getMetric() = 1;
            rt->addRoute(e);
        }
    }
}

void FlatNetworkConfigurator::handleMessage(cMessage *msg)
{
    throw cRuntimeError("this module doesn't handle messages, it runs only in initialize()");
}

void FlatNetworkConfigurator::setDisplayString(cTopology& topo, NodeInfoVector& nodeInfo)
{
    int numIPNodes = 0;
    for (int i = 0; i < topo.getNumNodes(); i++)
        if (nodeInfo[i].isIPNode)
            numIPNodes++;


    // update display string
    char buf[80];
    sprintf(buf, "%d IPv4 nodes\n%d non-IPv4 nodes", numIPNodes, topo.getNumNodes() - numIPNodes);
    getDisplayString().setTagArg("t", 0, buf);
}

} // namespace inet

