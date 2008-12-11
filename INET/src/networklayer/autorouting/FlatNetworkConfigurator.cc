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
#include "IRoutingTable.h"
#include "IInterfaceTable.h"
#include "IPAddressResolver.h"
#include "FlatNetworkConfigurator.h"
#include "InterfaceEntry.h"
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
    // extract topology
    topo.extractByProperty("node");
    EV << "cTopology found " << topo.getNumNodes() << " nodes\n";

    // fill in isIPNode, ift and rt members in nodeInfo[]
    nodeInfo.resize(topo.getNumNodes());
    for (int i=0; i<topo.getNumNodes(); i++)
    {
        cModule *mod = topo.getNode(i)->getModule();
        nodeInfo[i].isIPNode = IPAddressResolver().findInterfaceTableOf(mod)!=NULL;
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
    if (topo.getNumNodes()>maxNodes)
        error("netmask too large, not enough addresses for all %d nodes", topo.getNumNodes());

    int numIPNodes = 0;
    for (int i=0; i<topo.getNumNodes(); i++)
    {
        // skip bus types
        if (!nodeInfo[i].isIPNode)
            continue;

        uint32 addr = networkAddress | uint32(++numIPNodes);
        nodeInfo[i].address.set(addr);

        // find interface table and assign address to all (non-loopback) interfaces
        IInterfaceTable *ift = nodeInfo[i].ift;
        for (int k=0; k<ift->getNumInterfaces(); k++)
        {
            InterfaceEntry *ie = ift->getInterface(k);
            if (!ie->isLoopback())
            {
                ie->ipv4Data()->setIPAddress(IPAddress(addr));
                ie->ipv4Data()->setNetmask(IPAddress::ALLONES_ADDRESS); // full address must match for local delivery
            }
        }
    }
}

void FlatNetworkConfigurator::addDefaultRoutes(cTopology& topo, NodeInfoVector& nodeInfo)
{
    // add default route to nodes with exactly one (non-loopback) interface
    for (int i=0; i<topo.getNumNodes(); i++)
    {
        cTopology::Node *node = topo.getNode(i);

        // skip bus types
        if (!nodeInfo[i].isIPNode)
            continue;

        IInterfaceTable *ift = nodeInfo[i].ift;
        IRoutingTable *rt = nodeInfo[i].rt;

        // count non-loopback interfaces
        int numIntf = 0;
        InterfaceEntry *ie = NULL;
        for (int k=0; k<ift->getNumInterfaces(); k++)
            if (!ift->getInterface(k)->isLoopback())
                {ie = ift->getInterface(k); numIntf++;}

        nodeInfo[i].usesDefaultRoute = (numIntf==1);
        if (numIntf!=1)
            continue; // only deal with nodes with one interface plus loopback

        EV << "  " << node->getModule()->getFullName() << "=" << nodeInfo[i].address
           << " has only one (non-loopback) interface, adding default route\n";

        // add route
        IPRoute *e = new IPRoute();
        e->setHost(IPAddress());
        e->setNetmask(IPAddress());
        e->setInterface(ie);
        e->setType(IPRoute::REMOTE);
        e->setSource(IPRoute::MANUAL);
        //e->getMetric() = 1;
        rt->addRoute(e);
    }
}

void FlatNetworkConfigurator::fillRoutingTables(cTopology& topo, NodeInfoVector& nodeInfo)
{
    // fill in routing tables with static routes
    for (int i=0; i<topo.getNumNodes(); i++)
    {
        cTopology::Node *destNode = topo.getNode(i);

        // skip bus types
        if (!nodeInfo[i].isIPNode)
            continue;

        IPAddress destAddr = nodeInfo[i].address;
        std::string destModName = destNode->getModule()->getFullName();

        // calculate shortest paths from everywhere towards destNode
        topo.calculateUnweightedSingleShortestPathsTo(destNode);

        // add route (with host=destNode) to every routing table in the network
        // (excepting nodes with only one interface -- there we'll set up a default route)
        for (int j=0; j<topo.getNumNodes(); j++)
        {
            if (i==j) continue;
            if (!nodeInfo[j].isIPNode)
                continue;

            cTopology::Node *atNode = topo.getNode(j);
            if (atNode->getNumPaths()==0)
                continue; // not connected
            if (nodeInfo[j].usesDefaultRoute)
                continue; // already added default route here

            IPAddress atAddr = nodeInfo[j].address;

            IInterfaceTable *ift = nodeInfo[j].ift;

            int outputGateId = atNode->getPath(0)->getLocalGate()->getId();
            InterfaceEntry *ie = ift->getInterfaceByNodeOutputGateId(outputGateId);
            if (!ie)
                error("%s has no interface for output gate id %d", ift->getFullPath().c_str(), outputGateId);

            EV << "  from " << atNode->getModule()->getFullName() << "=" << IPAddress(atAddr);
            EV << " towards " << destModName << "=" << IPAddress(destAddr) << " interface " << ie->getName() << endl;

            // add route
            IRoutingTable *rt = nodeInfo[j].rt;
            IPRoute *e = new IPRoute();
            e->setHost(destAddr);
            e->setNetmask(IPAddress(255,255,255,255)); // full match needed
            e->setInterface(ie);
            e->setType(IPRoute::DIRECT);
            e->setSource(IPRoute::MANUAL);
            //e->getMetric() = 1;
            rt->addRoute(e);
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
    for (int i=0; i<topo.getNumNodes(); i++)
        if (nodeInfo[i].isIPNode)
            numIPNodes++;

    // update display string
    char buf[80];
    sprintf(buf, "%d IP nodes\n%d non-IP nodes", numIPNodes, topo.getNumNodes()-numIPNodes);
    getDisplayString().setTagArg("t",0,buf);
}


