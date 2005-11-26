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
#include "IPControlInfo_m.h"
#include "IPv4InterfaceData.h"
#include "NotifierConsts.h"
#include "RoutingTableAccess.h"
#include "InterfaceTableAccess.h"
#include "NotificationBoard.h"

Define_Module(TED);

TED::TED()
{
    announceMsg = NULL;
}

TED::~TED()
{
    cancelAndDelete(announceMsg);
}

void TED::initialize(int stage)
{
    // we have to wait for stage 2 until interfaces get registered (stage 0)
    // and get their auto-assigned IP addresses (stage 2)
    if (stage!=3)
        return;

    rt = RoutingTableAccess().get();
    ift = InterfaceTableAccess().get();
    routerId = rt->getRouterId();
    nb = NotificationBoardAccess().get();

    maxMessageId = 0;

    ASSERT(!routerId.isUnspecified());

    for (int i = 0; i < ift->numInterfaces(); i++)
    {
        InterfaceEntry *ie = ift->interfaceAt(i);

        if (ie->nodeOutputGateId() == -1)
            continue;

        for (int j = 0; j < rt->numRoutingEntries(); j++)
        {
            RoutingEntry *rentry = rt->routingEntry(j);

            if(rentry->interfacePtr != ift->interfaceAt(i))
                continue;

            if (rentry->type != rentry->DIRECT)
                continue;

            IPAddress linkid = rt->routingEntry(j)->host;
            IPAddress remote = rt->routingEntry(j)->gateway;

            ASSERT(!remote.isUnspecified());

            cGate *g = parentModule()->gate(ie->nodeOutputGateId());
            ASSERT(g);

            TELinkStateInfo entry;
            entry.advrouter = routerId;
            entry.local = ie->ipv4()->inetAddress();
            entry.linkid = linkid;
            entry.remote = remote;
            entry.MaxBandwidth = g->datarate()->doubleValue();
            for(int j = 0; j < 8; j++)
                entry.UnResvBandwidth[j] = entry.MaxBandwidth;
            entry.state = true;

            // use g->delay()->doubleValue() for shortest delay calculation
            entry.metric = rentry->interfacePtr->ipv4()->metric();

            EV << "metric set to=" << entry.metric << endl;

            entry.sourceId = routerId.getInt();
            entry.messageId = ++maxMessageId;
            entry.timestamp = simTime();

            ted.push_back(entry);

            break;
        }
    }

    for (int i = 0; i < ift->numInterfaces(); i++)
    {
        cGate *g = parentModule()->gate("out", i);
        if(g) LocalAddress.push_back(ift->interfaceByPortNo(g->index())->ipv4()->inetAddress());
    }


    cStringTokenizer tokenizer(par("peers"));
    const char *token;
    while ((token = tokenizer.nextToken())!=NULL)
    {
        ASSERT(ift->interfaceByName(token));
        TEDPeer.push_back(ift->interfaceByName(token)->ipv4()->inetAddress());
    }

    rebuildRoutingTable();

    announceMsg = new cMessage("announce");

    scheduleAt(simTime() + exponential(0.01), announceMsg);

    WATCH_VECTOR(ted);
}

void TED::handleMessage(cMessage * msg)
{
    if (msg == announceMsg)
    {
        delete msg;
        sendToPeers(ted, true, IPAddress());
    }
    else if (!strcmp(msg->arrivalGate()->name(), "inotify"))
    {
        processLINK_NOTIFY(check_and_cast<LinkNotifyMsg*>(msg));
    }
    else if (!strcmp(msg->arrivalGate()->name(), "from_ip"))
    {
        EV << "Processing message from IP: " << msg << endl;
        IPControlInfo *controlInfo = check_and_cast<IPControlInfo *>(msg->controlInfo());
        IPAddress sender = controlInfo->srcAddr();

        int command = check_and_cast<TEDMsg*>(msg)->getCommand();
        switch (command)
        {
            case LINK_STATE_MESSAGE:
                processLINK_STATE_MESSAGE(check_and_cast<LinkStateMsg*>(msg), sender);
                break;

            default:
                ASSERT(false);
        }
    }
    else
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
    os << "  unResvBW[7]:" << info.UnResvBandwidth[7];
    os << "  unResvBW[4]:" << info.UnResvBandwidth[4];

    return os;
}

int TED::assignIndex(std::vector<vertex_t>& vertices, IPAddress node)
{
    for (unsigned int i = 0 ; i < vertices.size(); i++)
    {
        if(vertices[i].node == node)
            return i;
    }

    vertex_t newVertex;
    newVertex.node = node;
    newVertex.dist = LS_INFINITY;
    newVertex.parent = -1;

    vertices.push_back(newVertex);
    return vertices.size() - 1;
}

IPAddressVector TED::calculateShortestPath(IPAddressVector dest,
            const TELinkStateInfoVector& topology, double req_bandwidth, int priority)
{
    std::vector<vertex_t> V = calculateShortestPaths(topology, req_bandwidth, priority);

    double minDist = LS_INFINITY;
    int minIndex = -1;

    for (unsigned int i = 0; i < V.size(); i++)
    {
        if(V[i].dist >= minDist)
            continue;

        if(find(dest.begin(), dest.end(), V[i].node) == dest.end())
            continue;

        minDist = V[i].dist;
        minIndex = i;
    }

    IPAddressVector result;

    if(minIndex < 0)
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

    int n = rt->numRoutingEntries();
    int j = 0;
    for (int i = 0; i < n; i++)
    {
        RoutingEntry *entry = rt->routingEntry(j);
        if (entry->host.isMulticast())
        {
            ++j;
        }
        else
        {
            rt->deleteRoutingEntry(entry);
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
        if(V[i].node == routerId) // us
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

        RoutingEntry *entry = new RoutingEntry;
        entry->host = V[i].node;

        if (V[i].node == V[nHop].node)
        {
            entry->gateway = IPAddress();
            entry->type = entry->DIRECT;
        }
        else
        {
            entry->gateway = V[nHop].node;
            entry->type = entry->REMOTE;
        }
        entry->interfacePtr = rt->interfaceByAddress(interfaceAddrByPeerAddress(V[nHop].node));
        entry->interfaceName = opp_string(entry->interfacePtr->name());
        entry->source = RoutingEntry::OSPF;

        entry->netmask = 0xffffffff;
        entry->metric = 0; // routing table routing entry? what's that?

        rt->addRoutingEntry(entry);
    }

    // insert local peers

    for (unsigned int i = 0; i < LocalAddress.size(); i++)
    {
        RoutingEntry *entry = new RoutingEntry;

        entry->host = peerByLocalAddress(LocalAddress[i]);
        entry->gateway = IPAddress();
        entry->type = entry->DIRECT;
        entry->interfacePtr = rt->interfaceByAddress(LocalAddress[i]);
        entry->interfaceName = opp_string(entry->interfacePtr->name());
        entry->source = RoutingEntry::OSPF;

        entry->netmask = 0xffffffff;
        entry->metric = 0; // XXX FIXME what's that?

        rt->addRoutingEntry(entry);
    }

    nb->fireChangeNotification(NF_IPv4_ROUTINGTABLE_CHANGED);

}

IPAddress TED::interfaceAddrByPeerAddress(IPAddress peerIP)
{
    std::vector<TELinkStateInfo>::iterator it;
    for (it = ted.begin(); it != ted.end(); it++)
    {
        if (it->linkid == peerIP && it->advrouter == routerId)
            return it->local;
    }
    EV << "not a local peer " << peerIP << endl;
    ASSERT(false);
}

IPAddress TED::peerRemoteInterface(IPAddress peerIP)
{
    ASSERT(isLocalPeer(peerIP));
    std::vector<TELinkStateInfo>::iterator it;
    for (it = ted.begin(); it != ted.end(); it++)
    {
        if (it->linkid == peerIP && it->advrouter == routerId)
            return it->remote;
    }
    ASSERT(false);
}

bool TED::isLocalPeer(IPAddress inetAddr)
{
    std::vector<TELinkStateInfo>::iterator it;
    for (it = ted.begin(); it != ted.end(); it++)
    {
        if (it->linkid == inetAddr && it->advrouter == routerId)
            break;
    }
    return (it != ted.end());
}

std::vector<TED::vertex_t> TED::calculateShortestPaths(const TELinkStateInfoVector& topology,
            double req_bandwidth, int priority)
{
    std::vector<vertex_t> vertices;
    std::vector<edge_t> edges;

    for (unsigned int i = 0; i < topology.size(); i++)
    {
        if(!topology[i].state)
            continue;

        if(topology[i].UnResvBandwidth[priority] < req_bandwidth)
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

    for (unsigned int i = 1; i < vertices.size(); i++)
    {
        bool mod = false;

        for (unsigned int j = 0; j < edges.size(); j++)
        {
            int src = edges[j].src;
            int dest = edges[j].dest;

            ASSERT(src >= 0);
            ASSERT(dest >= 0);
            ASSERT(src < vertices.size());
            ASSERT(dest < vertices.size());
            ASSERT(src != dest);

            if(vertices[src].dist + edges[j].metric >= vertices[dest].dist)
                continue;

            vertices[dest].dist = vertices[src].dist + edges[j].metric;
            vertices[dest].parent = src;

            mod = true;
        }

        if(!mod)
            break;
    }

    return vertices;
}

void TED::sendToPeers(const std::vector<TELinkStateInfo>& list, bool req, IPAddress exPeer)
{
    EV << "sending LINK_STATE message to peers" << endl;

    for (unsigned int i = 0; i < ted.size(); i++)
    {
        if(ted[i].advrouter != routerId)
            continue;

        if(ted[i].linkid == exPeer)
            continue;

        if(!ted[i].state)
            continue;

        if(find(TEDPeer.begin(), TEDPeer.end(), ted[i].local) == TEDPeer.end())
            continue;

        LinkStateMsg *out = new LinkStateMsg("link state message");
        out->setLinkInfoArraySize(list.size());

        for (unsigned int j = 0; j < list.size(); j++)
            out->setLinkInfo(j, list[j]);

        out->setRequest(req);
        out->setAck(false);

        sendToIP(out, ted[i].linkid);
    }
}

void TED::sendToIP(LinkStateMsg *msg, IPAddress destAddr)
{
    // attach control info to packet
    IPControlInfo *controlInfo = new IPControlInfo();
    controlInfo->setDestAddr(destAddr);
    controlInfo->setSrcAddr(routerId);
    controlInfo->setProtocol(IP_PROT_OSPF);
    msg->setControlInfo(controlInfo);
    msg->setKind(TED_TRAFFIC);

    int length = msg->getLinkInfoArraySize() * 72;

    msg->setByteLength(length);

    msg->addPar("color") = TED_TRAFFIC;

    send(msg, "to_ip");
}

void TED::processLINK_NOTIFY(LinkNotifyMsg* msg)
{
    EV << "received LINK_NOTIFY message" << endl;

    unsigned int k = msg->getLinkArraySize();

    ASSERT(k > 0);

    // build linkinfo list
    std::vector<TELinkStateInfo> links;
    for (unsigned int i = 0; i < k; i++)
    {
        TELink link = msg->getLink(i);

        unsigned int index = linkIndex(link.advrouter, link.linkid);

        updateTimestamp(&ted[index]);
        links.push_back(ted[index]);
    }

    sendToPeers(links, false, IPAddress());

    delete msg;
}

void TED::processLINK_STATE_MESSAGE(LinkStateMsg* msg, IPAddress sender)
{
    EV << "received LINK_STATE message from " << sender << endl;

    TELinkStateInfoVector forward;

    unsigned int n = msg->getLinkInfoArraySize();

    bool change = false; // in topology

    for (unsigned int i = 0; i < n; i++)
    {
        const TELinkStateInfo& link = msg->getLinkInfo(i);

        TELinkStateInfo *match;

        if(checkLinkValidity(link, &match))
        {
            ASSERT(link.sourceId == link.advrouter.getInt());

            EV << "new information found" << endl;

            if(!match)
            {
                // and we have no info on this link so far
                ted.push_back(link);
                change = true;
            }
            else
            {
                if(match->state != link.state)
                {
                    match->state = link.state;
                    change = true;
                }
                match->messageId = link.messageId;
                match->sourceId = link.sourceId;
                match->timestamp = link.timestamp;
                for(int i = 0; i < 8; i++)
                    match->UnResvBandwidth[i] = link.UnResvBandwidth[i];
                match->MaxBandwidth = link.MaxBandwidth;
                match->metric = link.metric;
            }

            forward.push_back(link);
        }
    }

    if(change)
        rebuildRoutingTable();

    if(msg->getRequest())
    {
        sendToPeer(sender, ted);
    }

    if(forward.size() > 0)
    {
        sendToPeers(forward, false, sender);
    }

    delete msg;
}

void TED::sendToPeer(IPAddress peer, const std::vector<TELinkStateInfo> & list)
{
    EV << "sending LINK_STATE message (ACK) to " << peer << endl;

    LinkStateMsg *out = new LinkStateMsg("link state message");

    out->setLinkInfoArraySize(list.size());
    for (unsigned int j = 0; j < list.size(); j++)
        out->setLinkInfo(j, list[j]);

    out->setRequest(false);
    out->setAck(true);

    sendToIP(out, peer);
}

bool TED::checkLinkValidity(TELinkStateInfo link, TELinkStateInfo **match)
{
    std::vector<TELinkStateInfo>::iterator it;

    *match = NULL;

    for(it = ted.begin(); it != ted.end(); it++)
    {
        if(it->sourceId == link.sourceId && it->messageId == link.messageId && it->timestamp == link.timestamp)
        {
            // we've already seen this message, ignore it
            return false;
        }

        if(it->advrouter == link.advrouter && it->linkid == link.linkid)
        {
            // we've have info about this link

            if(it->timestamp < link.timestamp || (it->timestamp == link.timestamp && it->messageId < link.messageId))
            {
                // but it's older, use this new
                *match = &(*it);
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
    {
        if(ted[i].advrouter != routerId)
            continue;

        if(ted[i].local != localInf)
            continue;

        return i;
    }
    ASSERT(false);
}

unsigned int TED::linkIndex(IPAddress advrouter, IPAddress linkid)
{
    for (unsigned int i = 0; i < ted.size(); i++)
    {
        if(ted[i].advrouter != advrouter)
            continue;

        if(ted[i].linkid != linkid)
            continue;

        return i;
    }
    ASSERT(false);
}

bool TED::isLocalAddress(IPAddress addr)
{
    for (unsigned int i = 0; i < LocalAddress.size(); i++)
    {
        if(LocalAddress[i] != addr)
            continue;

        return true;
    }
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
    return LocalAddress;
}

IPAddress TED::primaryAddress(IPAddress localInf)
{
    for (unsigned int i = 0; i < ted.size(); i++)
    {
        if(ted[i].local == localInf)
            return ted[i].advrouter;

        if(ted[i].remote == localInf)
            return ted[i].linkid;
    }
    ASSERT(false);
}

IPAddress TED::peerByLocalAddress(IPAddress localInf)
{
    unsigned int index = linkIndex(localInf);
    return ted[index].linkid;
}



