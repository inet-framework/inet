//
// Copyright (C) 2006 Andras Varga
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
#include "NetworkConfigurator.h"
#include "IPv4InterfaceData.h"


Define_Module(NetworkConfigurator);


void NetworkConfigurator::initialize(int stage)
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

        // add routes for point-to-point peers
        addPointToPointPeerRoutes(topo, nodeInfo);

        // add default routes to hosts (nodes with a single attachment);
        // also remember result in nodeInfo[].usesDefaultRoute
        addDefaultRoutes(topo, nodeInfo);

        // help configure RSVP and LinkStateRouting modules by setting their "peers" parameters
        setPeersParameter("rsvp", topo, nodeInfo);
        setPeersParameter("linkStateRouting", topo, nodeInfo);

        // calculate shortest paths, and add corresponding static routes
        fillRoutingTables(topo, nodeInfo);

        // update display string
        setDisplayString(topo, nodeInfo);
    }
}

void NetworkConfigurator::extractTopology(cTopology& topo, NodeInfoVector& nodeInfo)
{
    // extract topology
    topo.extractByProperty("node");
    EV << "cTopology found " << topo.getNumNodes() << " nodes\n";

    // fill in isIPNode, ift and rt members in nodeInfo[]
    nodeInfo.resize(topo.getNumNodes());
    for (int i=0; i<topo.getNumNodes(); i++)
    {
        cModule *mod = topo.getNode(i)->getModule();
        nodeInfo[i].ift = IPAddressResolver().findInterfaceTableOf(mod);
        nodeInfo[i].rt = IPAddressResolver().findRoutingTableOf(mod);
        nodeInfo[i].isIPNode = nodeInfo[i].rt!=NULL;
    }
}

void NetworkConfigurator::assignAddresses(cTopology& topo, NodeInfoVector& nodeInfo)
{
    uint32 base = 10 << 24;  // 10.x.x.x addresses
    int nodeCtr = 1;         // middle 16 bits

    for (int i=0; i<topo.getNumNodes(); i++)
    {
        // skip bus types
        if (!nodeInfo[i].isIPNode)
            continue;

        uint32 addr = base + (nodeCtr++ << 8);   // --> 10.nn.nn.0

        // assign address to all (non-loopback) interfaces
        IInterfaceTable *ift = nodeInfo[i].ift;
        for (int k=0; k<ift->getNumInterfaces(); k++)
        {
            InterfaceEntry *ie = ift->getInterface(k);
            if (!ie->isLoopback())
            {
                ie->ipv4Data()->setIPAddress(IPAddress(addr | (uint32)k));
                ie->ipv4Data()->setNetmask(IPAddress::ALLONES_ADDRESS); // full address must match for local delivery
            }
        }

        // set routerId as well (if not yet configured)
        IRoutingTable *rt = nodeInfo[i].rt;
        if (rt->getRouterId().isUnspecified())
        {
            rt->setRouterId(IPAddress(addr | 1U)); // 10.nn.nn.1
        }
    }
}

void NetworkConfigurator::addPointToPointPeerRoutes(cTopology& topo, NodeInfoVector& nodeInfo)
{
    bool useRouterIdForRoutes = true; // TODO make it parameter

    // add routes towards point-to-point routers (in real life these routes are
    // created automatically after PPP handshake when neighbour's address is learned)
    for (int i=0; i<topo.getNumNodes(); i++)
    {
        // skip bus types
        if (!nodeInfo[i].isIPNode)
            continue;

        cTopology::Node *node = topo.getNode(i);
        //InterfaceTable *ift = nodeInfo[i].ift;
        IRoutingTable *rt = nodeInfo[i].rt;

        // loop through neighbors
        for (int j=0; j<node->getNumOutLinks(); j++)
        {
            cTopology::Node *neighbor = node->getLinkOut(j)->getRemoteNode();

            // find neighbour's index in cTopology ==> k
            int k;
            for (k=0; k<topo.getNumNodes(); k++)
                if (topo.getNode(k)==neighbor)
                    break;
            ASSERT(k<=topo.getNumNodes());

            // if it's not an IP getNode(e.g. an Ethernet switch), then we're not interested
            if (!nodeInfo[k].isIPNode)
                continue;

            // find out neighbor's routerId
            IPAddress neighborRouterId = nodeInfo[k].rt->getRouterId();

            // find out neighbor's interface IP address
            int neighborGateId = node->getLinkOut(j)->getRemoteGate()->getId();
            InterfaceEntry *neighborIe = nodeInfo[k].ift->getInterfaceByNodeInputGateId(neighborGateId);
            ASSERT(neighborIe);
            IPAddress neighborAddr = neighborIe->ipv4Data()->getIPAddress();

            // find our own interface towards neighbor
            int gateId = node->getLinkOut(j)->getLocalGate()->getId();
            InterfaceEntry *ie = nodeInfo[i].ift->getInterfaceByNodeOutputGateId(gateId);
            ASSERT(ie);

            // add route
            IPRoute *e = new IPRoute();
            if (useRouterIdForRoutes)
            {
                e->setHost(neighborRouterId);
                e->setGateway(neighborAddr);
            }
            else
            {
                e->setHost(neighborAddr); // and no gateway
            }
            e->setNetmask(IPAddress(255,255,255,255)); // full match needed
            e->setInterface(ie);
            e->setType(IPRoute::DIRECT);
            e->setSource(IPRoute::MANUAL);
            //e->getMetric() = 1;
            rt->addRoute(e);
        }
    }
}

void NetworkConfigurator::addDefaultRoutes(cTopology& topo, NodeInfoVector& nodeInfo)
{
    // add default route to nodes with exactly one (non-loopback) interface
    for (int i=0; i<topo.getNumNodes(); i++)
    {
        // skip bus types
        if (!nodeInfo[i].isIPNode)
            continue;

        cTopology::Node *node = topo.getNode(i);
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

        EV << "  " << node->getModule()->getFullName() << " has only one (non-loopback) "
           "interface, adding default route\n";

        // add route
        IPRoute *e = new IPRoute();
        e->setHost(IPAddress());
        e->setNetmask(IPAddress());
        e->setInterface(ie);
        e->setType(IPRoute::REMOTE);
        e->setSource(IPRoute::MANUAL);
        //e->setMetric(1);
        rt->addRoute(e);
    }
}

void NetworkConfigurator::setPeersParameter(const char *submodName, cTopology& topo, NodeInfoVector& nodeInfo)
{
    // the RSVP module expects a "peers" module parameter to contain the interfaces
    // towards directly connected other RSVP routers. Since it's cumbersome to configure
    // manually in a large network, do it here (submodName = "rsvp").
    // The LinkStateRouting module is similar, so this function is also called with submodName = "LinkStateRouting".

    // for each RSVP router, collect neighbors which are also RSVP routers
    for (int i=0; i<topo.getNumNodes(); i++)
    {
        // if it doesn't have an RSVP submodule, we're not interested
        cModule *submod = topo.getNode(i)->getModule()->getSubmodule(submodName);
        if (submod==NULL)
            continue;

        std::string peers;
        cTopology::Node *node = topo.getNode(i);
        for (int j=0; j<node->getNumOutLinks(); j++)
        {
            // if neighbor is not an RSVP router, then we're not interested
            cModule *neighborSubmod = node->getLinkOut(j)->getRemoteNode()->getModule()->getSubmodule(submodName);
            if (neighborSubmod==NULL)
                continue;

            // find our own interface towards neighbor
            int gateId = node->getLinkOut(j)->getLocalGate()->getId();
            InterfaceEntry *ie = nodeInfo[i].ift->getInterfaceByNodeOutputGateId(gateId);
            ASSERT(ie);

            // interface name to peers list
            peers += std::string(" ") + ie->getName();
        }

        // set "peers" parameter
        submod->par("peers") = peers.c_str();
    }
}

void NetworkConfigurator::fillRoutingTables(cTopology& topo, NodeInfoVector& nodeInfo)
{
/* FIXME TBD
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
            e->gateway = ???
            e->setNetmask(IPAddress(255,255,255,255)); // full match needed
            e->setInterface(ie);
            e->setType(IPRoute::REMOTE);
            e->setSource(IPRoute::MANUAL);
            //e->getMetric() = 1;
            rt->addRoute(e);
        }
    }
*/
}

void NetworkConfigurator::handleMessage(cMessage *msg)
{
    error("this module doesn't handle messages, it runs only in initialize()");
}

void NetworkConfigurator::setDisplayString(cTopology& topo, NodeInfoVector& nodeInfo)
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


