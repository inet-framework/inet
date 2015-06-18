//
// Copyright (C) 2005 Eric Wu
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

#include "inet/networklayer/configurator/ipv6/FlatNetworkConfigurator6.h"

#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/networklayer/ipv6/IPv6InterfaceData.h"
#include "inet/networklayer/ipv6/IPv6RoutingTable.h"

namespace inet {

// FIXME UPDATE DOCU!!!!!!!

Define_Module(FlatNetworkConfigurator6);

void FlatNetworkConfigurator6::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_NETWORK_LAYER_2) {
        cTopology topo("topo");

        // extract topology
        topo.extractByProperty("networkNode");
        EV_DEBUG << "cTopology found " << topo.getNumNodes() << " nodes\n";

        configureAdvPrefixes(topo);

        addOwnAdvPrefixRoutes(topo);
        addStaticRoutes(topo);
    }
}

void FlatNetworkConfigurator6::handleMessage(cMessage *)
{
    throw cRuntimeError("this module doesn't handle messages, it runs only in initialize()");
}

void FlatNetworkConfigurator6::setDisplayString(int numIPNodes, int numNonIPNodes)
{
    // update display string
    char buf[80];
    sprintf(buf, "%d IPv6 nodes\n%d non-IP nodes", numIPNodes, numNonIPNodes);
    getDisplayString().setTagArg("t", 0, buf);
}

bool FlatNetworkConfigurator6::isIPNode(cTopology::Node *node)
{
    return L3AddressResolver().findIPv6RoutingTableOf(node->getModule()) != nullptr
           && L3AddressResolver().findInterfaceTableOf(node->getModule()) != nullptr;
}

void FlatNetworkConfigurator6::configureAdvPrefixes(cTopology& topo)
{
    // assign advertised prefixes to all router interfaces
    for (int i = 0; i < topo.getNumNodes(); i++) {
        // skip bus types
        if (!isIPNode(topo.getNode(i)))
            continue;

        int nodeIndex = i;

        // find interface table and assign address to all (non-loopback) interfaces
        cModule *mod = topo.getNode(i)->getModule();
        IInterfaceTable *ift = L3AddressResolver().interfaceTableOf(mod);
        IPv6RoutingTable *rt = L3AddressResolver().findIPv6RoutingTableOf(mod);

        // skip non-IPv6 nodes
        if (!rt)
            continue;

        // skip hosts
        if (!rt->par("isRouter").boolValue())
            continue;

        // assign prefix to interfaces
        for (int k = 0; k < ift->getNumInterfaces(); k++) {
            InterfaceEntry *ie = ift->getInterface(k);
            if (!ie->ipv6Data() || ie->isLoopback())
                continue;
            if (ie->ipv6Data()->getNumAdvPrefixes() > 0)
                continue; // already has one

            // add a prefix
            IPv6Address prefix(0xaaaa0000 + nodeIndex, ie->getNetworkLayerGateIndex() << 16, 0, 0);
            ASSERT(prefix.isGlobal());

            IPv6InterfaceData::AdvPrefix p;
            p.prefix = prefix;
            p.prefixLength = 64;
            // RFC 2461:6.2.1. Only default values are used in FlatNetworkConfigurator6
            // Default: 2592000 seconds (30 days), fixed (i.e., stays the same in
            // consecutive advertisements).
            p.advValidLifetime = 2592000;
            // Default: TRUE
            p.advOnLinkFlag = true;
            // Default: 604800 seconds (7 days), fixed (i.e., stays the same in consecutive
            // advertisements).
            p.advPreferredLifetime = 604800;
            // Default: TRUE
            p.advAutonomousFlag = true;
            ie->ipv6Data()->addAdvPrefix(p);

            // add a link-local address (tentative) if it doesn't have one
            if (ie->ipv6Data()->getLinkLocalAddress().isUnspecified())
                ie->ipv6Data()->assignAddress(IPv6Address::formLinkLocalAddress(ie->getInterfaceToken()), true, SIMTIME_ZERO, SIMTIME_ZERO);
        }
    }
}

void FlatNetworkConfigurator6::addOwnAdvPrefixRoutes(cTopology& topo)
{
    // add globally routable prefixes to routing table
    for (int i = 0; i < topo.getNumNodes(); i++) {
        cTopology::Node *node = topo.getNode(i);

        // skip bus types
        if (!isIPNode(node))
            continue;

        IPv6RoutingTable *rt = L3AddressResolver().findIPv6RoutingTableOf(node->getModule());
        IInterfaceTable *ift = L3AddressResolver().interfaceTableOf(node->getModule());

        // skip non-IPv6 nodes
        if (!rt)
            continue;

        // skip hosts
        if (!rt->par("isRouter").boolValue())
            continue;

        // add globally routable prefixes to routing table
        for (int x = 0; x < ift->getNumInterfaces(); x++) {
            InterfaceEntry *ie = ift->getInterface(x);

            if (ie->isLoopback())
                continue;

            for (int y = 0; y < ie->ipv6Data()->getNumAdvPrefixes(); y++)
                if (ie->ipv6Data()->getAdvPrefix(y).prefix.isGlobal())
                    rt->addOrUpdateOwnAdvPrefix(ie->ipv6Data()->getAdvPrefix(y).prefix,
                            ie->ipv6Data()->getAdvPrefix(y).prefixLength,
                            ie->getInterfaceId(), SIMTIME_ZERO);

        }
    }
}

// XXX !isRouter nodes are not used as source/destination of routes
//     but can be internal nodes of the generated shortest paths.
//     the same problem with non-ipv6 nodes
void FlatNetworkConfigurator6::addStaticRoutes(cTopology& topo)
{
    int numIPNodes = 0;

    // fill in routing tables
    for (int i = 0; i < topo.getNumNodes(); i++) {
        cTopology::Node *destNode = topo.getNode(i);

        // skip bus types
        if (!isIPNode(destNode))
            continue;
/*
    void addOrUpdateOwnAdvPrefix(const IPv6Address& destPrefix, int prefixLength,
                                 int interfaceId, simtime_t expiryTime);
 */

        numIPNodes++;    // FIXME split into num hosts, num routers
        IPv6RoutingTable *destRt = L3AddressResolver().findIPv6RoutingTableOf(destNode->getModule());
        IInterfaceTable *destIft = L3AddressResolver().interfaceTableOf(destNode->getModule());

        // skip non-IPv6 nodes
        if (!destRt)
            continue;

        // don't add routes towards hosts
        if (!destRt->par("isRouter").boolValue())
            continue;

        // get a list of globally routable prefixes from the dest node
        std::vector<const IPv6InterfaceData::AdvPrefix *> destPrefixes;
        for (int x = 0; x < destIft->getNumInterfaces(); x++) {
            InterfaceEntry *destIf = destIft->getInterface(x);

            if (destIf->isLoopback())
                continue;

            for (int y = 0; y < destIf->ipv6Data()->getNumAdvPrefixes(); y++)
                if (destIf->ipv6Data()->getAdvPrefix(y).prefix.isGlobal())
                    destPrefixes.push_back(&destIf->ipv6Data()->getAdvPrefix(y));

        }

        std::string destModName = destNode->getModule()->getFullName();

        // calculate shortest paths from everywhere towards destNode
        topo.calculateUnweightedSingleShortestPathsTo(destNode);

        // add route (with dest=destPrefixes) to every router routing table in the network
        for (int j = 0; j < topo.getNumNodes(); j++) {
            if (i == j)
                continue;
            if (!isIPNode(topo.getNode(j)))
                continue;

            cTopology::Node *atNode = topo.getNode(j);
            if (atNode->getNumPaths() == 0)
                continue; // not connected

            IPv6RoutingTable *rt = L3AddressResolver().findIPv6RoutingTableOf(atNode->getModule());
            IInterfaceTable *ift = L3AddressResolver().interfaceTableOf(atNode->getModule());

            // skip non-IPv6 nodes
            if (!rt)
                continue;

            // skip hosts' routing tables
            if (!rt->par("isRouter").boolValue())
                continue;

            // determine the local interface id
            cGate *localGate = atNode->getPath(0)->getLocalGate();
            InterfaceEntry *localIf = ift->getInterfaceByNodeOutputGateId(localGate->getId());

            // determine next hop link address. That's a bit tricky because
            // the directly adjacent cTopo node might be a non-IP getNode(ethernet switch etc)
            // so we have to "seek through" them.
            cTopology::Node *prevNode = atNode;
            // if there's no ethernet switch between atNode and it's next hop
            // neighbour, we don't go into the following while() loop
            while (!isIPNode(prevNode->getPath(0)->getRemoteNode()))
                prevNode = prevNode->getPath(0)->getRemoteNode();

            // ok, the next hop is now just one step away from prevNode
            cGate *remoteGate = prevNode->getPath(0)->getRemoteGate();
            cModule *nextHop = remoteGate->getOwnerModule();
            IInterfaceTable *nextHopIft = L3AddressResolver().interfaceTableOf(nextHop);
            InterfaceEntry *nextHopOnlinkIf = nextHopIft->getInterfaceByNodeInputGateId(remoteGate->getId());

            // find link-local address for next hop
            IPv6Address nextHopLinkLocalAddr = nextHopOnlinkIf->ipv6Data()->getLinkLocalAddress();

            // traverse through address of each node
            // add to route table
            for (auto & destPrefixe : destPrefixes) {
                rt->addStaticRoute(destPrefixe->prefix, destPrefixe->prefixLength,
                        localIf->getInterfaceId(), nextHopLinkLocalAddr);
            }
        }
    }

    // update display string
    setDisplayString(numIPNodes, topo.getNumNodes() - numIPNodes);
}

} // namespace inet

