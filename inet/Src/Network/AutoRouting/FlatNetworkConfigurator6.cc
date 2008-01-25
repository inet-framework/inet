//
// Copyright (C) 2005 Eric Wu
// Copyright (C) 2004 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

#ifdef _MSC_VER
#pragma warning(disable:4786)
#endif

#include <algorithm>
#include "FlatNetworkConfigurator6.h"
#include "InterfaceTable.h"
#include "IPAddressResolver.h"
#ifdef WITH_IPv6
#include "IPv6InterfaceData.h"
#include "RoutingTable6.h"
#endif

// FIXME UPDATE DOCU!!!!!!!

Define_Module(FlatNetworkConfigurator6);

void FlatNetworkConfigurator6::initialize(int stage)
{
#ifdef WITH_IPv6
    // FIXME refactor: make routers[] array? (std::vector<cTopology::Node*>)
    // FIXME: spare common beginning for all stages?

    cTopology topo("topo");
    StringVector types = cStringTokenizer(par("moduleTypes"), " ").asVector();
    StringVector nonIPTypes = cStringTokenizer(par("nonIPModuleTypes"), " ").asVector();
    for (unsigned int i=0; i<nonIPTypes.size(); i++)
        types.push_back(nonIPTypes[i]);

    // extract topology
    topo.extractByModuleType(types);
    EV << "cTopology found " << topo.nodes() << " nodes\n";

    if (stage==2)
    {
        configureAdvPrefixes(topo, nonIPTypes);
    }
    else if (stage==3)
    {
        addOwnAdvPrefixRoutes(topo, nonIPTypes);
        addStaticRoutes(topo, nonIPTypes);
    }
#else
    error("FlatNetworkConfigurator6 not supported: WITH_IPv6 option was off during compilation");
#endif
}

void FlatNetworkConfigurator6::handleMessage(cMessage * msg)
{
    error("this module doesn't handle messages, it runs only in initialize()");
}

void FlatNetworkConfigurator6::setDisplayString(int numIPNodes, int numNonIPNodes)
{
    // update display string
    char buf[80];
    sprintf(buf, "%d IPv6 nodes\n%d non-IP nodes", numIPNodes, numNonIPNodes);
    displayString().setTagArg("t", 0, buf);
}

bool FlatNetworkConfigurator6::isNonIPType(cTopology::Node *node, StringVector& nonIPTypes)
{
    return std::find(nonIPTypes.begin(), nonIPTypes.end(), node->module()->className())!=nonIPTypes.end();
}

#ifdef WITH_IPv6
void FlatNetworkConfigurator6::configureAdvPrefixes(cTopology& topo, StringVector& nonIPTypes)
{
    // assign advertised prefixes to all router interfaces
    for (int i = 0; i < topo.nodes(); i++)
    {
        // skip bus types
        if (isNonIPType(topo.node(i), nonIPTypes))
            continue;

        // find interface table and assign address to all (non-loopback) interfaces
        cModule *mod = topo.node(i)->module();
        InterfaceTable *ift = IPAddressResolver().interfaceTableOf(mod);
        RoutingTable6 *rt = IPAddressResolver().routingTable6Of(mod);

        // skip hosts
        if (!rt->par("isRouter").boolValue())
            continue;

        // assign prefix to interfaces
        for (int k = 0; k < ift->numInterfaces(); k++)
        {
            InterfaceEntry *ie = ift->interfaceAt(k);
            if (!ie->ipv6() || ie->isLoopback())
                continue;
            if (ie->ipv6()->numAdvPrefixes()>0)
                continue;  // already has one

            // add a prefix
            IPv6Address prefix(0xaaaa0000+ift->id(), ie->networkLayerGateIndex()<<16, 0, 0);
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
            ie->ipv6()->addAdvPrefix(p);

            // add a link-local address (tentative) if it doesn't have one
            if (ie->ipv6()->linkLocalAddress().isUnspecified())
                ie->ipv6()->assignAddress(IPv6Address::formLinkLocalAddress(ie->interfaceToken()), true, 0, 0);
        }
    }
}

void FlatNetworkConfigurator6::addOwnAdvPrefixRoutes(cTopology& topo, StringVector& nonIPTypes)
{
    // add globally routable prefixes to routing table
    for (int i = 0; i < topo.nodes(); i++)
    {
        cTopology::Node *node = topo.node(i);

        // skip bus types
        if (isNonIPType(node, nonIPTypes))
            continue;

        RoutingTable6 *rt = IPAddressResolver().routingTable6Of(node->module());
        InterfaceTable *ift = IPAddressResolver().interfaceTableOf(node->module());

        // skip hosts
        if (!rt->par("isRouter").boolValue())
            continue;

        // add globally routable prefixes to routing table
        for (int x = 0; x < ift->numInterfaces(); x++)
        {
            InterfaceEntry *ie = ift->interfaceAt(x);

            if (ie->isLoopback())
                continue;

            for (int y = 0; y < ie->ipv6()->numAdvPrefixes(); y++)
                if (ie->ipv6()->advPrefix(y).prefix.isGlobal())
                    rt->addOrUpdateOwnAdvPrefix(ie->ipv6()->advPrefix(y).prefix,
                                                ie->ipv6()->advPrefix(y).prefixLength,
                                                x, 0);
        }
    }
}

void FlatNetworkConfigurator6::addStaticRoutes(cTopology& topo, StringVector& nonIPTypes)
{
    int numIPNodes = 0;

    // fill in routing tables
    for (int i = 0; i < topo.nodes(); i++)
    {
        cTopology::Node *destNode = topo.node(i);

        // skip bus types
        if (isNonIPType(destNode, nonIPTypes))
            continue;
/*
    void addOrUpdateOwnAdvPrefix(const IPv6Address& destPrefix, int prefixLength,
                                 int interfaceId, simtime_t expiryTime);
*/

        numIPNodes++; // FIXME split into num hosts, num routers
        RoutingTable6 *destRt = IPAddressResolver().routingTable6Of(destNode->module());
        InterfaceTable *destIft = IPAddressResolver().interfaceTableOf(destNode->module());

        // don't add routes towards hosts
        if (!destRt->par("isRouter").boolValue())
            continue;

        // get a list of globally routable prefixes from the dest node
        std::vector<const IPv6InterfaceData::AdvPrefix*> destPrefixes;
        for (int x = 0; x < destIft->numInterfaces(); x++)
        {
            InterfaceEntry *destIf = destIft->interfaceAt(x);

            if (destIf->isLoopback())
                continue;

            for (int y = 0; y < destIf->ipv6()->numAdvPrefixes(); y++)
                if (destIf->ipv6()->advPrefix(y).prefix.isGlobal())
                    destPrefixes.push_back(&destIf->ipv6()->advPrefix(y));
        }

        std::string destModName = destNode->module()->fullName();

        // calculate shortest paths from everywhere towards destNode
        topo.unweightedSingleShortestPathsTo(destNode);

        // add route (with dest=destPrefixes) to every router routing table in the network
        for (int j = 0; j < topo.nodes(); j++)
        {
            if (i == j)
                continue;
            if (isNonIPType(topo.node(j), nonIPTypes))
                continue;

            cTopology::Node *atNode = topo.node(j);
            if (atNode->paths() == 0)
                continue;       // not connected

            RoutingTable6 *rt = IPAddressResolver().routingTable6Of(atNode->module());
            InterfaceTable *ift = IPAddressResolver().interfaceTableOf(atNode->module());

            // skip hosts' routing tables
            if (!rt->par("isRouter").boolValue())
                continue;

            // determine the local interface id
            cGate *localGate = atNode->path(0)->localGate();
            InterfaceEntry *localIf = ift->interfaceByNodeOutputGateId(localGate->id());

            // determine next hop link address. That's a bit tricky because
            // the directly adjacent cTopo node might be a non-IP node (ethernet switch etc)
            // so we have to "seek through" them.
            cTopology::Node *prevNode = atNode;
            // if there's no ethernet switch between atNode and it's next hop
            // neighbour, we don't go into the following while() loop
            while (isNonIPType(prevNode->path(0)->remoteNode(), nonIPTypes))
                prevNode = prevNode->path(0)->remoteNode();

            // ok, the next hop is now just one step away from prevNode
            cGate *remoteGate = prevNode->path(0)->remoteGate();
            cModule *nextHop = remoteGate->ownerModule();
            InterfaceTable *nextHopIft = IPAddressResolver().interfaceTableOf(nextHop);
            InterfaceEntry *nextHopOnlinkIf = nextHopIft->interfaceByNodeInputGateId(remoteGate->id());

            // find link-local address for next hop
            IPv6Address nextHopLinkLocalAddr = nextHopOnlinkIf->ipv6()->linkLocalAddress();

            // traverse through address of each node
            // add to route table
            for (unsigned int k = 0; k < destPrefixes.size(); k++)
            {
                rt->addStaticRoute(destPrefixes[k]->prefix, destPrefixes[k]->prefixLength,
                                   localIf->interfaceId(), nextHopLinkLocalAddr);
            }
        }
    }

    // update display string
    setDisplayString(numIPNodes, topo.nodes()-numIPNodes);
}
#endif


