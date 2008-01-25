//
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
#include "RoutingTable.h"
#include "InterfaceTable.h"
#include "IPAddressResolver.h"
#include "FlatNetworkConfigurator.h"
#include "IPv4InterfaceData.h"


Define_Module(FlatNetworkConfigurator);


void FlatNetworkConfigurator::initialize(int stage)
{
    if (stage==2)
    {
        cTopology topo("topo");
        NodeInfoVector nodeInfo; // will be of size topo.nodes[]

        // extract topology into the cTopology object, then fill in
        // isIPNode, rt and ift members of nodeInfo[]
        extractTopology(topo, nodeInfo);

        // assign addresses to IP nodes, and also store result in nodeInfo[].address
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
    // FIXME eliminate nonIPModuleTypes, like in NetworkConfigurator
    StringVector types = cStringTokenizer(par("moduleTypes"), " ").asVector();
    StringVector nonIPTypes = cStringTokenizer(par("nonIPModuleTypes"), " ").asVector();
    for (unsigned int i=0; i<nonIPTypes.size(); i++)
        types.push_back(nonIPTypes[i]);

    // extract topology
    topo.extractByModuleType(types);
    EV << "cTopology found " << topo.nodes() << " nodes\n";

    // fill in isIPNode, ift and rt members in nodeInfo[]
    nodeInfo.resize(topo.nodes());
    for (int i=0; i<topo.nodes(); i++)
    {
        cModule *mod = topo.node(i)->module();
        nodeInfo[i].isIPNode = std::find(nonIPTypes.begin(),nonIPTypes.end(), mod->className())==nonIPTypes.end();
        if (nodeInfo[i].isIPNode)
        {
            nodeInfo[i].ift = IPAddressResolver().interfaceTableOf(mod);
            nodeInfo[i].rt = IPAddressResolver().routingTableOf(mod);
        }
    }
}

void FlatNetworkConfigurator::assignAddresses(cTopology& topo, NodeInfoVector& nodeInfo)
{
    // assign IP addresses
    uint32 networkAddress = IPAddress(par("networkAddress").stringValue()).getInt();
    uint32 netmask = IPAddress(par("netmask").stringValue()).getInt();
    int maxNodes = (~netmask)-1;  // 0 and ffff have special meaning and cannot be used
    if (topo.nodes()>maxNodes)
        error("netmask too large, not enough addresses for all %d nodes", topo.nodes());

    int numIPNodes = 0;
    for (int i=0; i<topo.nodes(); i++)
    {
        // skip bus types
        if (!nodeInfo[i].isIPNode)
            continue;

        uint32 addr = networkAddress | uint32(++numIPNodes);
        nodeInfo[i].address.set(addr);

        // find interface table and assign address to all (non-loopback) interfaces
        InterfaceTable *ift = nodeInfo[i].ift;
        for (int k=0; k<ift->numInterfaces(); k++)
        {
            InterfaceEntry *ie = ift->interfaceAt(k);
            if (!ie->isLoopback())
            {
                ie->ipv4()->setInetAddress(IPAddress(addr));
                ie->ipv4()->setNetmask(IPAddress::ALLONES_ADDRESS); // full address must match for local delivery
            }
        }
    }
}

void FlatNetworkConfigurator::addDefaultRoutes(cTopology& topo, NodeInfoVector& nodeInfo)
{
    // add default route to nodes with exactly one (non-loopback) interface
    for (int i=0; i<topo.nodes(); i++)
    {
        cTopology::Node *node = topo.node(i);

        // skip bus types
        if (!nodeInfo[i].isIPNode)
            continue;

        InterfaceTable *ift = nodeInfo[i].ift;
        RoutingTable *rt = nodeInfo[i].rt;

        // count non-loopback interfaces
        int numIntf = 0;
        InterfaceEntry *ie = NULL;
        for (int k=0; k<ift->numInterfaces(); k++)
            if (!ift->interfaceAt(k)->isLoopback())
                {ie = ift->interfaceAt(k); numIntf++;}

        nodeInfo[i].usesDefaultRoute = (numIntf==1);
        if (numIntf!=1)
            continue; // only deal with nodes with one interface plus loopback

        EV << "  " << node->module()->fullName() << "=" << nodeInfo[i].address
           << " has only one (non-loopback) interface, adding default route\n";

        // add route
        RoutingEntry *e = new RoutingEntry();
        e->host = IPAddress();
        e->netmask = IPAddress();
        e->interfaceName = ie->name();
        e->interfacePtr = ie;
        e->type = RoutingEntry::REMOTE;
        e->source = RoutingEntry::MANUAL;
        //e->metric() = 1;
        rt->addRoutingEntry(e);
    }
}

void FlatNetworkConfigurator::fillRoutingTables(cTopology& topo, NodeInfoVector& nodeInfo)
{
    // fill in routing tables with static routes
    for (int i=0; i<topo.nodes(); i++)
    {
        cTopology::Node *destNode = topo.node(i);

        // skip bus types
        if (!nodeInfo[i].isIPNode)
            continue;

        IPAddress destAddr = nodeInfo[i].address;
        std::string destModName = destNode->module()->fullName();

        // calculate shortest paths from everywhere towards destNode
        topo.unweightedSingleShortestPathsTo(destNode);

        // add route (with host=destNode) to every routing table in the network
        // (excepting nodes with only one interface -- there we'll set up a default route)
        for (int j=0; j<topo.nodes(); j++)
        {
            if (i==j) continue;
            if (!nodeInfo[j].isIPNode)
                continue;

            cTopology::Node *atNode = topo.node(j);
            if (atNode->paths()==0)
                continue; // not connected
            if (nodeInfo[j].usesDefaultRoute)
                continue; // already added default route here

            IPAddress atAddr = nodeInfo[j].address;

            InterfaceTable *ift = nodeInfo[j].ift;

            int outputGateId = atNode->path(0)->localGate()->id();
            InterfaceEntry *ie = ift->interfaceByNodeOutputGateId(outputGateId);
            if (!ie)
                error("%s has no interface for output gate id %d", ift->fullPath().c_str(), outputGateId);

            EV << "  from " << atNode->module()->fullName() << "=" << IPAddress(atAddr);
            EV << " towards " << destModName << "=" << IPAddress(destAddr) << " interface " << ie->name() << endl;

            // add route
            RoutingTable *rt = nodeInfo[j].rt;
            RoutingEntry *e = new RoutingEntry();
            e->host = destAddr;
            e->netmask = IPAddress(255,255,255,255); // full match needed
            e->interfaceName = ie->name();
            e->interfacePtr = ie;
            e->type = RoutingEntry::DIRECT;
            e->source = RoutingEntry::MANUAL;
            //e->metric() = 1;
            rt->addRoutingEntry(e);
        }
    }
}

void FlatNetworkConfigurator::handleMessage(cMessage *msg)
{
    error("this module doesn't handle messages, it runs only in initialize()");
}

void FlatNetworkConfigurator::setDisplayString(cTopology& topo, NodeInfoVector& nodeInfo)
{
    int numIPNodes = 0;
    for (int i=0; i<topo.nodes(); i++)
        if (nodeInfo[i].isIPNode)
            numIPNodes++;

    // update display string
    char buf[80];
    sprintf(buf, "%d IP nodes\n%d non-IP nodes", numIPNodes, topo.nodes()-numIPNodes);
    displayString().setTagArg("t",0,buf);
}


