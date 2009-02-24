//
// (C) 2005 Vojtech Janota
//
// This library is free software, you can redistribute it
// and/or modify
// it under  the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation;
// either version 2 of the License, or any later version.
// The library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//

#include <omnetpp.h>
#include <algorithm>

#include "TED.h"
#include "IPControlInfo.h"
#include "IPv4InterfaceData.h"
#include "NotifierConsts.h"
#include "RoutingTableAccess.h"
#include "InterfaceTableAccess.h"
#include "NotificationBoard.h"

#define LS_INFINITY   1e16

Define_Module(TED);

TED::TED()
{
}

TED::~TED()
{
}

void TED::initialize(int stage)
{
    // we have to wait for stage 2 until interfaces get registered (stage 0)
    // and get their auto-assigned IP addresses (stage 2); routerId gets
    // assigned in stage 3
    if (stage!=4)
        return;

    rt = RoutingTableAccess().get();
    ift = InterfaceTableAccess().get();
    routerId = rt->getRouterId();
    nb = NotificationBoardAccess().get();

    maxMessageId = 0;

    ASSERT(!routerId.isUnspecified());

    //
    // Extract initial TED contents from the routing table.
    //
    // We need to create one TED entry (TELinkStateInfo) for each link,
    // i.e. for each physical interface.
    //
    for (int i = 0; i < ift->getNumInterfaces(); i++)
    {
        InterfaceEntry *ie = ift->getInterface(i);

        if (ie->getNodeOutputGateId() == -1)  // ignore if it's not a physical interface
            continue;

        //
        // We'll need to fill in "linkid" and "remote" (ie. peer addr).
        //
        // Real link state protocols find the peer address by exchanging HELLO messages;
        // in this model we haven't implemented HELLO but provide peer addresses via
        // preconfigured static host routes in routing table.
        //
        const IPRoute *rentry = NULL;
        for (int j = 0; j < rt->getNumRoutes(); j++)
        {
            rentry = rt->getRoute(j);
            if (rentry->getInterface()==ie && rentry->getType()==IPRoute::DIRECT)
                break;
        }
        ASSERT(rentry);
        IPAddress linkid = rentry->getHost();
        IPAddress remote = rentry->getGateway();
        ASSERT(!remote.isUnspecified());

        // find bandwidth of the link
        cGate *g = getParentModule()->gate(ie->getNodeOutputGateId());
        ASSERT(g);
        double linkBandwidth = g->getChannel()->par("datarate");

        //
        // fill in and insert TED entry
        //
        TELinkStateInfo entry;
        entry.advrouter = routerId;
        entry.local = ie->ipv4Data()->getIPAddress();
        entry.linkid = linkid;
        entry.remote = remote;
        entry.MaxBandwidth = linkBandwidth;
        for (int j = 0; j < 8; j++)
            entry.UnResvBandwidth[j] = entry.MaxBandwidth;
        entry.state = true;

        // use g->getChannel()->par("delay").doubleValue() for shortest delay calculation
        entry.metric = rentry->getInterface()->ipv4Data()->getMetric();

        EV << "metric set to=" << entry.metric << endl;

        entry.sourceId = routerId.getInt();
        entry.messageId = ++maxMessageId;
        entry.timestamp = simTime();

        ted.push_back(entry);
    }

    // extract list of local interface addresses into interfaceAddrs[]
    for (int i = 0; i < ift->getNumInterfaces(); i++)
    {
        InterfaceEntry *ie = ift->getInterface(i);
        if (rt->getInterfaceByAddress(ie->ipv4Data()->getIPAddress()) != ie)
            error("MPLS models assume interfaces to have unique addresses, "
                  "but address of '%s' (%s) is not unique",
                  ie->getName(), ie->ipv4Data()->getIPAddress().str().c_str());
        if (!ie->isLoopback())
            interfaceAddrs.push_back(ie->ipv4Data()->getIPAddress());
    }

    rebuildRoutingTable();

    WATCH_VECTOR(ted);
}

void TED::handleMessage(cMessage * msg)
{
    ASSERT(false);
}

std::ostream & operator<<(std::ostream & os, const TELinkStateInfo& info)
{
    os << "advrouter:" << info.advrouter;
    os << "  linkid:" << info.linkid;
    os << "  local:" << info.local;
    os << "  remote:" << info.remote;
    os << "  state:" << info.state;
    os << "  metric:" << info.metric;
    os << "  maxBW:" << info.MaxBandwidth;
    os << "  unResvBW[7]:" << info.UnResvBandwidth[7];  // FIXME comment: what is 7 ?
    os << "  unResvBW[4]:" << info.UnResvBandwidth[4];  // FIXME comment: what is 4 ?

    return os;
}

// FIXME should this be called findOrCreateVertex() or something like that?
int TED::assignIndex(std::vector<vertex_t>& vertices, IPAddress nodeAddr)
{
    // find node in vertices[] whose IP address is nodeAddr
    for (unsigned int i = 0 ; i < vertices.size(); i++)
        if (vertices[i].node == nodeAddr)
            return i;

    // if not found, create
    vertex_t newVertex;
    newVertex.node = nodeAddr;
    newVertex.dist = LS_INFINITY;
    newVertex.parent = -1;

    vertices.push_back(newVertex);
    return vertices.size() - 1;
}

IPAddressVector TED::calculateShortestPath(IPAddressVector dest,
            const TELinkStateInfoVector& topology, double req_bandwidth, int priority)
{
    // FIXME comment: what do we do here?
    std::vector<vertex_t> V = calculateShortestPaths(topology, req_bandwidth, priority);

    double minDist = LS_INFINITY;
    int minIndex = -1;

    // FIXME comment: what do we do in this block?
    for (unsigned int i = 0; i < V.size(); i++)
    {
        if (V[i].dist >= minDist)
            continue;

        if (find(dest.begin(), dest.end(), V[i].node) == dest.end())
            continue;

        minDist = V[i].dist;
        minIndex = i;
    }

    IPAddressVector result;

    if (minIndex < 0)
        return result;

    result.push_back(V[minIndex].node);
    while (V[minIndex].parent != -1)
    {
        minIndex = V[minIndex].parent;
        result.insert(result.begin(), V[minIndex].node);
    }

    return result;
}

void TED::rebuildRoutingTable()
{
    EV << "rebuilding routing table at " << routerId << endl;

    std::vector<vertex_t> V = calculateShortestPaths(ted, 0.0, 7);

    // remove all routing entries, except multicast ones (we don't care about them)
    int n = rt->getNumRoutes();
    int j = 0;
    for (int i = 0; i < n; i++)
    {
        const IPRoute *entry = rt->getRoute(j);
        if (entry->getHost().isMulticast())
        {
            ++j;
        }
        else
        {
            rt->deleteRoute(entry);
        }
    }

//  for (unsigned int i = 0; i < V.size(); i++)
//  {
//      EV << "V[" << i << "].node=" << V[i].node << endl;
//      EV << "V[" << i << "].parent=" << V[i].parent << endl;
//      EV << "V[" << i << "].dist=" << V[i].dist << endl;
//  }

    // insert remote destinations

    for (unsigned int i = 0; i < V.size(); i++)
    {
        if (V[i].node == routerId) // us
            continue;

        if (V[i].parent == -1) // unreachable
            continue;

        if (isLocalPeer(V[i].node)) // local peer
            continue;

        int nHop = i;

        while (!isLocalPeer(V[nHop].node))
        {
            nHop = V[nHop].parent;
        }

        ASSERT(isLocalPeer(V[nHop].node));

        IPRoute *entry = new IPRoute;
        entry->setHost(V[i].node);

        if (V[i].node == V[nHop].node)
        {
            entry->setGateway(IPAddress());
            entry->setType(entry->DIRECT);
        }
        else
        {
            entry->setGateway(V[nHop].node);
            entry->setType(entry->REMOTE);
        }
        entry->setInterface(rt->getInterfaceByAddress(getInterfaceAddrByPeerAddress(V[nHop].node)));
        entry->setSource(IPRoute::OSPF);

        entry->setNetmask(0xffffffff);
        entry->setMetric(0);

        EV << "  inserting route: host=" << entry->getHost() << " interface=" << entry->getInterfaceName() << " nexthop=" << entry->getGateway() << "\n";

        rt->addRoute(entry);
    }

    // insert local peers

    for (unsigned int i = 0; i < interfaceAddrs.size(); i++)
    {
        IPRoute *entry = new IPRoute;

        entry->setHost(getPeerByLocalAddress(interfaceAddrs[i]));
        entry->setGateway(IPAddress());
        entry->setType(entry->DIRECT);
        entry->setInterface(rt->getInterfaceByAddress(interfaceAddrs[i]));
        entry->setSource(IPRoute::OSPF);

        entry->setNetmask(0xffffffff);
        entry->setMetric(0); // XXX FIXME what's that?

        EV << "  inserting route: local=" << interfaceAddrs[i] << " peer=" << entry->getHost() << " interface=" << entry->getInterfaceName() << "\n";

        rt->addRoute(entry);
    }
}

IPAddress TED::getInterfaceAddrByPeerAddress(IPAddress peerIP)
{
    std::vector<TELinkStateInfo>::iterator it;
    for (it = ted.begin(); it != ted.end(); it++)
        if (it->linkid == peerIP && it->advrouter == routerId)
            return it->local;
    error("not a local peer: %s", peerIP.str().c_str());
    return IPAddress(); // prevent warning
}

IPAddress TED::peerRemoteInterface(IPAddress peerIP)
{
    ASSERT(isLocalPeer(peerIP));
    std::vector<TELinkStateInfo>::iterator it;
    for (it = ted.begin(); it != ted.end(); it++)
        if (it->linkid == peerIP && it->advrouter == routerId)
            return it->remote;
    error("not a local peer: %s", peerIP.str().c_str());
    return IPAddress(); // prevent warning
}

bool TED::isLocalPeer(IPAddress inetAddr)
{
    std::vector<TELinkStateInfo>::iterator it;
    for (it = ted.begin(); it != ted.end(); it++)
        if (it->linkid == inetAddr && it->advrouter == routerId)
            break;
    return it != ted.end();
}

std::vector<TED::vertex_t> TED::calculateShortestPaths(const TELinkStateInfoVector& topology,
            double req_bandwidth, int priority)
{
    std::vector<vertex_t> vertices;
    std::vector<edge_t> edges;

    // select edges that have enough bandwidth left, and store them into edges[].
    // meanwhile, collect vertices in vectices[].
    for (unsigned int i = 0; i < topology.size(); i++)
    {
        if (!topology[i].state)
            continue;

        if (topology[i].UnResvBandwidth[priority] < req_bandwidth)
            continue;

        edge_t edge;
        edge.src = assignIndex(vertices, topology[i].advrouter);
        edge.dest = assignIndex(vertices, topology[i].linkid);
        edge.metric = topology[i].metric;
        edges.push_back(edge);
    }

    IPAddress srcAddr = routerId;

    int srcIndex = assignIndex(vertices, srcAddr);
    vertices[srcIndex].dist = 0.0;

    // FIXME comment: Dijkstra? just guessing...
    for (unsigned int i = 1; i < vertices.size(); i++)
    {
        bool mod = false;

        for (unsigned int j = 0; j < edges.size(); j++)
        {
            int src = edges[j].src;
            int dest = edges[j].dest;

            ASSERT(src >= 0);
            ASSERT(dest >= 0);
            ASSERT(src < (int)vertices.size());
            ASSERT(dest < (int)vertices.size());
            ASSERT(src != dest);

            if (vertices[src].dist + edges[j].metric >= vertices[dest].dist)
                continue;

            vertices[dest].dist = vertices[src].dist + edges[j].metric;
            vertices[dest].parent = src;

            mod = true;
        }

        if (!mod)
            break;
    }

    return vertices;
}

bool TED::checkLinkValidity(TELinkStateInfo link, TELinkStateInfo *&match)
{
    std::vector<TELinkStateInfo>::iterator it;

    match = NULL;

    for (it = ted.begin(); it != ted.end(); it++)
    {
        if (it->sourceId == link.sourceId && it->messageId == link.messageId && it->timestamp == link.timestamp)
        {
            // we've already seen this message, ignore it
            return false;
        }

        if (it->advrouter == link.advrouter && it->linkid == link.linkid)
        {
            // we've have info about this link

            if (it->timestamp < link.timestamp || (it->timestamp == link.timestamp && it->messageId < link.messageId))
            {
                // but it's older, use this new
                match = &(*it);
                break;
            }
            else
            {
                // and it's newer, forget this message
                return false;
            }
        }
    }

    // no or not up2date info, link is interesting
    return true;
}

unsigned int TED::linkIndex(IPAddress localInf)
{
    for (unsigned int i = 0; i < ted.size(); i++)
        if (ted[i].advrouter == routerId && ted[i].local == localInf)
            return i;
    ASSERT(false);
    return -1; // to eliminate warning
}

unsigned int TED::linkIndex(IPAddress advrouter, IPAddress linkid)
{
    for (unsigned int i = 0; i < ted.size(); i++)
        if (ted[i].advrouter == advrouter && ted[i].linkid == linkid)
            return i;
    ASSERT(false);
    return -1; // to eliminate warning
}

bool TED::isLocalAddress(IPAddress addr)
{
    for (unsigned int i = 0; i < interfaceAddrs.size(); i++)
        if (interfaceAddrs[i] == addr)
            return true;
    return false;
}

void TED::updateTimestamp(TELinkStateInfo *link)
{
    ASSERT(link->advrouter == routerId);

    link->timestamp = simTime();
    link->messageId = ++maxMessageId;
}

IPAddressVector TED::getLocalAddress()
{
    return interfaceAddrs;
}

IPAddress TED::primaryAddress(IPAddress localInf) // only used in RSVP::processHelloMsg
{
    for (unsigned int i = 0; i < ted.size(); i++)
    {
        if (ted[i].local == localInf)
            return ted[i].advrouter;

        if (ted[i].remote == localInf)
            return ted[i].linkid;
    }
    ASSERT(false);
    return IPAddress(); // to eliminate warning
}

IPAddress TED::getPeerByLocalAddress(IPAddress localInf)
{
    unsigned int index = linkIndex(localInf);
    return ted[index].linkid;
}



