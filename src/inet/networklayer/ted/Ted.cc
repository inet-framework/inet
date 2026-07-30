//
// Copyright (C) 2005 Vojtech Janota
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/networklayer/ted/Ted.h"

#include <algorithm>
#include <cstdlib>
#include <map>

#include "inet/common/ModuleAccess.h"
#include "inet/common/Simsignals.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/common/stlutils.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/ipv4/IIpv4RoutingTable.h"
#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"

namespace inet {

#define LS_INFINITY    1e16

Define_Module(Ted);

Ted::Ted()
{
}

Ted::~Ted()
{
}

void Ted::initialize(int stage)
{
    RoutingProtocolBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        maxMessageId = 0;

        WATCH(interfaceAddrs);
        WATCH(maxMessageId);
        WATCH(routerId);
        WATCH(ted);

        rt.reference(this, "routingTableModule", true);
        ift.reference(this, "interfaceTableModule", true);

        installRoutes = par("installRoutes");
    }
}

void Ted::initializeTED()
{
    routerId = rt->getRouterId();
    ASSERT(!routerId.isUnspecified());

    //
    // Parse optional per-interface TE-attribute overrides (D3: teMetric,
    // adminGroup, srlgs -- see Ted.ned's "linkAttributes" param doc for the
    // XML schema). Interfaces not mentioned keep the defaults (teMetric=0,
    // i.e. fall back to `metric`; adminGroup=0; no SRLGs). Unknown interface
    // names are a configuration error and abort with a cRuntimeError that
    // includes the offending element's source location.
    //
    std::map<std::string, cXMLElement *> linkAttrsByInterface;
    cXMLElement *linkAttributesXml = par("linkAttributes").xmlValue();
    for (cXMLElement *linkElem : linkAttributesXml->getChildrenByTagName("link")) {
        const char *ifName = linkElem->getAttribute("interface");
        if (!ifName)
            throw cRuntimeError("Ted's linkAttributes has a <link> element missing the mandatory 'interface' attribute, at %s", linkElem->getSourceLocation());
        if (!ift->findInterfaceByName(ifName))
            throw cRuntimeError("Ted's linkAttributes refers to unknown interface '%s', at %s", ifName, linkElem->getSourceLocation());
        if (!linkAttrsByInterface.emplace(ifName, linkElem).second)
            throw cRuntimeError("Ted's linkAttributes has more than one <link> entry for interface '%s', at %s", ifName, linkElem->getSourceLocation());
    }

    //
    // Extract initial TED contents from the routing table.
    //
    // We need to create one TED entry (TeLinkStateInfo) for each link,
    // i.e. for each physical interface.
    //
    for (int i = 0; i < ift->getNumInterfaces(); i++) {
        NetworkInterface *ie = ift->getInterface(i);

        if (ie->getNodeOutputGateId() == -1) // ignore if it's not a physical interface
            continue;

        //
        // We'll need to fill in "linkid" and "remote" (ie. peer addr).
        //
        // Real link state protocols find the peer address by exchanging HELLO messages;
        // in this model we haven't implemented HELLO but provide peer addresses via
        // preconfigured static host routes in routing table.
        //
        // find bandwidth of the link
        cGate *g = CHK(getParentModule()->gate(ie->getNodeOutputGateId()));
        double linkBandwidth = g->getChannel()->getNominalDatarate();

        // find destination node for current interface
        cModule *destNode = nullptr;
        while (g) {
            g = g->getNextGate();
            cModule *mod = g->getOwnerModule();
            cProperties *props = mod->getProperties();
            if (props && props->getAsBool("networkNode")) {
                destNode = mod;
                break;
            }
        }
        if (!g) // not connected
            continue;
        IIpv4RoutingTable *destRt = L3AddressResolver().findIpv4RoutingTableOf(destNode);
        if (!destRt) // switch, hub, bus, accesspoint, etc
            continue;
        Ipv4Address destRouterId = destRt->getRouterId();
        IInterfaceTable *destIft = L3AddressResolver().interfaceTableOf(destNode);
        NetworkInterface *destIe = CHK(destIft->findInterfaceByNodeInputGateId(g->getId()));

        //
        // fill in and insert TED entry
        //
        TeLinkStateInfo entry;
        entry.advrouter = routerId;
        entry.local = ie->getProtocolData<Ipv4InterfaceData>()->getIPAddress();
        entry.linkid = destRouterId;
        entry.remote = destIe->getProtocolData<Ipv4InterfaceData>()->getIPAddress();
        entry.MaxBandwidth = linkBandwidth;
        for (int j = 0; j < 8; j++)
            entry.UnResvBandwidth[j] = entry.MaxBandwidth;
        entry.state = true;

        // use g->getChannel()->par("delay") for shortest delay calculation
        entry.metric = ie->getProtocolData<Ipv4InterfaceData>()->getMetric();

        EV_INFO << "metric set to=" << entry.metric << endl;

        entry.sourceId = routerId.getInt();
        entry.messageId = ++maxMessageId;
        entry.timestamp = simTime();

        // D3: apply this interface's TE-attribute overrides, if any (defaults
        // already set by the generated constructor: teMetric=0, adminGroup=0,
        // srlgsCount=0, srlgs all-zero).
        auto linkAttrIt = linkAttrsByInterface.find(ie->getInterfaceName());
        if (linkAttrIt != linkAttrsByInterface.end()) {
            cXMLElement *linkElem = linkAttrIt->second;
            if (const char *teMetricAttr = linkElem->getAttribute("teMetric"))
                entry.teMetric = atof(teMetricAttr);
            if (const char *adminGroupAttr = linkElem->getAttribute("adminGroup"))
                entry.adminGroup = strtoul(adminGroupAttr, nullptr, 0);
            if (const char *srlgsAttr = linkElem->getAttribute("srlgs")) {
                cStringTokenizer tokenizer(srlgsAttr);
                std::vector<uint32_t> srlgs;
                const char *token;
                while ((token = tokenizer.nextToken()) != nullptr)
                    srlgs.push_back(strtoul(token, nullptr, 0));
                if ((int)srlgs.size() > Ted::MAX_SRLGS)
                    throw cRuntimeError("Ted's linkAttributes for interface '%s' lists %zu SRLGs, more than the MAX_SRLGS=%d limit, at %s",
                            ie->getInterfaceName(), srlgs.size(), Ted::MAX_SRLGS, linkElem->getSourceLocation());
                entry.srlgsCount = srlgs.size();
                for (size_t k = 0; k < srlgs.size(); k++)
                    entry.srlgs[k] = srlgs[k];
            }
        }

        ted.push_back(entry);
    }

    // extract list of local interface addresses into interfaceAddrs[]
    for (int i = 0; i < ift->getNumInterfaces(); i++) {
        NetworkInterface *ie = ift->getInterface(i);
        NetworkInterface *ie2 = rt->getInterfaceByAddress(ie->getProtocolData<Ipv4InterfaceData>()->getIPAddress());
        if (ie2 != ie)
            throw cRuntimeError("MPLS models assume interfaces to have unique addresses, "
                                "but address of '%s' (%s) is not unique",
                    ie->getInterfaceName(), ie->getProtocolData<Ipv4InterfaceData>()->getIPAddress().str().c_str());
        if (!ie->isLoopback())
            interfaceAddrs.push_back(ie->getProtocolData<Ipv4InterfaceData>()->getIPAddress());
    }

    rebuildRoutingTable();
}

void Ted::handleMessageWhenUp(cMessage *msg)
{
    throw cRuntimeError("Ted does not process messages, but received '%s'", msg->getName());
}

std::ostream& operator<<(std::ostream& os, const TeLinkStateInfo& info)
{
    os << "advrouter:" << info.advrouter;
    os << "  linkid:" << info.linkid;
    os << "  local:" << info.local;
    os << "  remote:" << info.remote;
    os << "  state:" << info.state;
    os << "  metric:" << info.metric;
    os << "  maxBW:" << info.MaxBandwidth;
    os << "  unResvBW[7]:" << info.UnResvBandwidth[7]; // FIXME comment: what is 7 ?
    os << "  unResvBW[4]:" << info.UnResvBandwidth[4]; // FIXME comment: what is 4 ?

    return os;
}

// FIXME should this be called findOrCreateVertex() or something like that?
int Ted::assignIndex(std::vector<Vertex>& vertices, Ipv4Address nodeAddr)
{
    // find node in vertices[] whose Ipv4 address is nodeAddr
    for (unsigned int i = 0; i < vertices.size(); i++)
        if (vertices[i].node == nodeAddr)
            return i;

    // if not found, create
    Vertex newVertex;
    newVertex.node = nodeAddr;
    newVertex.dist = LS_INFINITY;
    newVertex.parent = -1;

    vertices.push_back(newVertex);
    return vertices.size() - 1;
}

Ipv4AddressVector Ted::calculateShortestPath(Ipv4AddressVector dest,
        const TeLinkStateInfoVector& topology, double req_bandwidth, int priority,
        uint32_t includeAny, uint32_t excludeAny)
{
    // Run the constrained Bellman-Ford below, then pick the CLOSEST reachable vertex that is
    // a member of `dest` (a destination address SET -- e.g. several advertised interface
    // addresses of the same egress router all count), and walk parent pointers back to the
    // root to build the full hop list root..dest.
    std::vector<Vertex> V = calculateShortestPaths(topology, req_bandwidth, priority, includeAny, excludeAny);

    double minDist = LS_INFINITY;
    int minIndex = -1;

    // Among all vertices, keep only the ones in `dest`, and track the nearest one. Vertices
    // that Bellman-Ford never reached keep dist==LS_INFINITY, so they're never selected here
    // unless literally nothing is reachable (minIndex then stays -1, see below).
    for (unsigned int i = 0; i < V.size(); i++) {
        if (V[i].dist >= minDist)
            continue;

        if (!contains(dest, V[i].node))
            continue;

        minDist = V[i].dist;
        minIndex = i;
    }

    Ipv4AddressVector result;

    if (minIndex < 0)
        return result;

    result.push_back(V[minIndex].node);
    while (V[minIndex].parent != -1) {
        minIndex = V[minIndex].parent;
        result.insert(result.begin(), V[minIndex].node);
    }

    return result;
}

void Ted::rebuildRoutingTable()
{
    if (!installRoutes) {
        // Some other means (IGP, static routes, Ipv4NetworkConfigurator) owns the routing
        // table; Ted only tracks TE state, so there's nothing to compute or install here.
        // (Nothing else consumes calculateShortestPaths()'s result, so it's safe to skip
        // the whole method, not just the route-install tail.)
        EV_INFO << "installRoutes=false, not touching the routing table at " << routerId << endl;
        return;
    }

    EV_INFO << "rebuilding routing table at " << routerId << endl;

    std::vector<Vertex> V = calculateShortestPaths(ted, 0.0, 7);

    // remove all routing entries, except multicast ones (we don't care about them)
    int n = rt->getNumRoutes();
    int j = 0;
    for (int i = 0; i < n; i++) {
        Ipv4Route *entry = rt->getRoute(j);
        if (entry->getDestination().isMulticast()) {
            ++j;
        }
        else {
            rt->deleteRoute(entry);
        }
    }

    // insert remote destinations

    for (unsigned int i = 0; i < V.size(); i++) {
        if (V[i].node == routerId) // us
            continue;

        if (V[i].parent == -1) // unreachable
            continue;

        if (isLocalPeer(V[i].node)) // local peer
            continue;

        int nHop = i;

        while (!isLocalPeer(V[nHop].node)) {
            nHop = V[nHop].parent;
        }

        ASSERT(isLocalPeer(V[nHop].node));

        Ipv4Route *entry = new Ipv4Route;
        entry->setDestination(V[i].node);

        if (V[i].node == V[nHop].node) {
            entry->setGateway(Ipv4Address());
        }
        else {
            entry->setGateway(V[nHop].node);
        }
        entry->setInterface(rt->getInterfaceByAddress(getInterfaceAddrByPeerAddress(V[nHop].node)));
        entry->setSourceType(IRoute::OSPF);

        entry->setNetmask(Ipv4Address::ALLONES_ADDRESS);
        entry->setMetric(0);

        EV_DETAIL << "  inserting route: dest=" << entry->getDestination() << " interface=" << entry->getInterfaceName() << " nexthop=" << entry->getGateway() << "\n";

        rt->addRoute(entry);
    }

    // insert local peers

    for (auto& elem : interfaceAddrs) {
        Ipv4Route *entry = new Ipv4Route;

        entry->setDestination(getPeerByLocalAddress(elem));
        entry->setGateway(Ipv4Address());
        entry->setInterface(rt->getInterfaceByAddress(elem));
        entry->setSourceType(IRoute::OSPF);

        entry->setNetmask(Ipv4Address::ALLONES_ADDRESS);
        entry->setMetric(0); // FIXME what's that?

        EV_DETAIL << "  inserting route: local=" << elem << " peer=" << entry->getDestination() << " interface=" << entry->getInterfaceName() << "\n";

        rt->addRoute(entry);
    }
}

Ipv4Address Ted::getInterfaceAddrByPeerAddress(Ipv4Address peerIP)
{
    for (auto& elem : ted)
        if (elem.linkid == peerIP && elem.advrouter == routerId)
            return elem.local;

    throw cRuntimeError("getInterfaceAddrByPeerAddress(): %s is not a directly-connected peer", peerIP.str().c_str());
}

Ipv4Address Ted::peerRemoteInterface(Ipv4Address peerIP)
{
    ASSERT(isLocalPeer(peerIP));
    for (auto& elem : ted)
        if (elem.linkid == peerIP && elem.advrouter == routerId)
            return elem.remote;

    throw cRuntimeError("peerRemoteInterface(): %s is not a directly-connected peer", peerIP.str().c_str());
}

bool Ted::isLocalPeer(Ipv4Address inetAddr)
{
    for (auto& elem : ted)
        if (elem.linkid == inetAddr && elem.advrouter == routerId)
            return true;

    return false;
}

std::vector<Ted::Vertex> Ted::calculateShortestPaths(const TeLinkStateInfoVector& topology,
        double req_bandwidth, int priority, uint32_t includeAny, uint32_t excludeAny)
{
    std::vector<Vertex> vertices;
    std::vector<Edge> edges;

    // Select edges that (a) are up, (b) have enough bandwidth left at the requested priority,
    // and (c) satisfy the affinity constraint (Workstream C6/D3; includeAny==0 && excludeAny==0
    // is a no-op match, so existing callers -- rebuildRoutingTable()'s plain routing-table
    // computation -- are unaffected). Meanwhile, collect vertices in vertices[].
    // Edge weight uses getTeMetric() (TE metric with IGP-metric fallback, Workstream D3): this
    // is a no-op change for every shipped example/showcase today since none of them configure a
    // nonzero teMetric via Ted's "linkAttributes" param, so getTeMetric() always returns
    // elem.metric there -- see this commit's fingerprint verification.
    for (auto& elem : topology) {
        if (!elem.state)
            continue;

        if (elem.UnResvBandwidth[priority] < req_bandwidth)
            continue;

        if (!matchesAffinity(elem, includeAny, excludeAny))
            continue;

        Edge edge;
        edge.src = assignIndex(vertices, elem.advrouter);
        edge.dest = assignIndex(vertices, elem.linkid);
        edge.metric = getTeMetric(elem);
        edges.push_back(edge);
    }

    Ipv4Address srcAddr = routerId;

    int srcIndex = assignIndex(vertices, srcAddr);
    vertices[srcIndex].dist = 0.0;

    // This is Bellman-Ford (not Dijkstra -- the "Dijkstra? just guessing..." comment that used
    // to sit here was the original 2005 author's own doubt about which algorithm he'd written,
    // on code that had ZERO callers until Workstream C6 revived it for RsvpTe's ingress CSPF).
    // Verified correct by direct review plus tests/unit/Ted_calculateShortestPath.test (a
    // hand-built topology with a known-correct answer, exercised BEFORE this function got its
    // first real caller): the outer loop's bound (vertices.size()-1 passes, the textbook
    // Bellman-Ford bound for a graph with no negative-weight cycles -- all link metrics/costs
    // here are non-negative) is correct and sufficient regardless of edge insertion order; the
    // early "no modification this pass -> stop" exit is a valid, safe optimization on top of
    // that bound, not a substitute for it.
    for (unsigned int i = 1; i < vertices.size(); i++) {
        bool mod = false;

        for (auto& edge : edges) {
            int src = edge.src;
            int dest = edge.dest;

            ASSERT(src >= 0);
            ASSERT(dest >= 0);
            ASSERT(src < (int)vertices.size());
            ASSERT(dest < (int)vertices.size());
            ASSERT(src != dest);

            if (vertices[src].dist + edge.metric >= vertices[dest].dist)
                continue;

            vertices[dest].dist = vertices[src].dist + edge.metric;
            vertices[dest].parent = src;

            mod = true;
        }

        if (!mod)
            break;
    }

    return vertices;
}

bool Ted::checkLinkValidity(TeLinkStateInfo link, TeLinkStateInfo *& match)
{
    match = nullptr;

    for (auto& elem : ted) {
        if (elem.sourceId == link.sourceId && elem.messageId == link.messageId && elem.timestamp == link.timestamp) {
            // we've already seen this message, ignore it
            return false;
        }

        if (elem.advrouter == link.advrouter && elem.linkid == link.linkid) {
            // we've have info about this link

            if (elem.timestamp < link.timestamp || (elem.timestamp == link.timestamp && elem.messageId < link.messageId)) {
                // but it's older, use this new
                match = &(elem);
                break;
            }
            else {
                // and it's newer, forget this message
                return false;
            }
        }
    }

    // no or not up2date info, link is interesting
    return true;
}

unsigned int Ted::linkIndex(Ipv4Address localInf)
{
    for (unsigned int i = 0; i < ted.size(); i++)
        if (ted[i].advrouter == routerId && ted[i].local == localInf)
            return i;

    ASSERT(false);
    return -1; // to eliminate warning
}

unsigned int Ted::linkIndex(Ipv4Address advrouter, Ipv4Address linkid)
{
    for (unsigned int i = 0; i < ted.size(); i++)
        if (ted[i].advrouter == advrouter && ted[i].linkid == linkid)
            return i;

    ASSERT(false);
    return -1; // to eliminate warning
}

bool Ted::isLocalAddress(Ipv4Address addr)
{
    for (auto& elem : interfaceAddrs)
        if (elem == addr)
            return true;

    return false;
}

void Ted::updateTimestamp(int index)
{
    TeLinkStateInfo *link = &ted[index];
    ASSERT(link->advrouter == routerId);

    link->timestamp = simTime();
    link->messageId = ++maxMessageId;
}

const TeLinkStateInfo& Ted::getLink(int index) const
{
    ASSERT(index >= 0 && index < (int)ted.size());
    return ted[index];
}

void Ted::adjustUnresvBandwidth(int index, int priority, double delta)
{
    ted[index].UnResvBandwidth[priority] += delta;
}

void Ted::setLinkState(int index, bool up)
{
    if (ted[index].state == up)
        return; // nothing actually changed; don't rebuild/announce redundantly

    ted[index].state = up;
    rebuildRoutingTable();
    announceLinkChange(index);
}

void Ted::announceLinkChange(int index)
{
    TedChangeInfo d;
    d.setTedLinkIndicesArraySize(1);
    d.setTedLinkIndices(0, index);
    emit(tedChangedSignal, &d);
}

Ipv4AddressVector Ted::getLocalAddress()
{
    return interfaceAddrs;
}

Ipv4Address Ted::primaryAddress(Ipv4Address localInf) // only used in RsvpTe::processHelloMsg
{
    for (auto& elem : ted) {
        if (elem.local == localInf)
            return elem.advrouter;

        if (elem.remote == localInf)
            return elem.linkid;
    }
    ASSERT(false);
    return Ipv4Address(); // to eliminate warning
}

Ipv4Address Ted::getPeerByLocalAddress(Ipv4Address localInf)
{
    unsigned int index = linkIndex(localInf);
    return ted[index].linkid;
}

void Ted::handleStartOperation(LifecycleOperation *operation)
{
    initializeTED();
}

void Ted::handleStopOperation(LifecycleOperation *operation)
{
    ted.clear();
    interfaceAddrs.clear();
}

void Ted::handleCrashOperation(LifecycleOperation *operation)
{
    ted.clear();
    interfaceAddrs.clear();
}

} // namespace inet

