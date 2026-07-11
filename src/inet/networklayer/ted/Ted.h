//
// Copyright (C) 2005 Vojtech Janota
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_TED_H
#define __INET_TED_H

#include "inet/common/ModuleRefByPar.h"
#include "inet/networklayer/rsvpte/IntServ_m.h"
#include "inet/networklayer/ted/Ted_m.h"
#include "inet/routing/base/RoutingProtocolBase.h"

namespace inet {

class IIpv4RoutingTable;
class IInterfaceTable;
class NetworkInterface;

/**
 * Contains the Traffic Engineering Database and provides public methods
 * to access it from MPLS signalling protocols (LDP, RSVP-TE).
 *
 * See NED file for more info.
 */
class INET_API Ted : public RoutingProtocolBase, public cListener
{
  public:
    // Cap on the number of SRLGs a single TeLinkStateInfo can carry (D3).
    // TeLinkStateInfo.srlgs[] is a fixed-size array (see Ted.msg -- `struct`
    // msg types don't support true dynamic arrays); this constant is the
    // single source of truth, kept in sync with the array's literal size in
    // Ted.msg and with LinkStateRoutingSerializer's wire layout.
    static constexpr int MAX_SRLGS = 8;

  public:
    /**
     * Only used internally, during shortest path calculation:
     * vertex in the graph we build from links in TeLinkStateInfoVector.
     */
    struct Vertex {
        Ipv4Address node; // the router id of this vertex
        int parent; // index into the same Vertex vector
        double dist; // distance from the root along the shortest path found so far
    };

    /**
     * Only used internally, during shortest path calculation:
     * edge in the graph we build from links in TeLinkStateInfoVector.
     */
    struct Edge {
        int src; // index into the Vertex[] vector
        int dest; // index into the Vertex[] vector
        double metric; // link cost
    };

  public:
    Ted();
    virtual ~Ted();

  protected:
    virtual void initialize(int stage) override;
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void handleMessageWhenUp(cMessage *msg) override;

    virtual void initializeTED();

  public:
    // CSPF (Constrained Shortest Path First), revived for Workstream C6 (RsvpTe's ingress
    // path computation) -- previously DEAD code (zero callers since 2005; see calculateShortestPaths()
    // below for the verification history). Runs Bellman-Ford (calculateShortestPaths()) on the
    // subset of `topology` that (a) is up, (b) has at least req_bandwidth unreserved at
    // `priority`, and (c) -- if includeAny/excludeAny are nonzero -- satisfies matchesAffinity();
    // then walks parent pointers back from the closest reachable member of `dest` to build a
    // full hop list. Returns an empty vector if no member of `dest` is reachable under the
    // constraints. `dest` is a SET of acceptable destination addresses (e.g. a router's several
    // advertised interface addresses), not a single required target -- the nearest reachable one
    // wins.
    virtual Ipv4AddressVector calculateShortestPath(Ipv4AddressVector dest,
            const TeLinkStateInfoVector& topology, double req_bandwidth, int priority,
            uint32_t includeAny = 0, uint32_t excludeAny = 0);

    // Same computation as above, but rooted at an explicit `root` router instead of this Ted
    // instance's own routerId (TI-LFA/Workstream F2: computing extended P-space needs SPTs
    // rooted at THIS router's neighbors, and Q-space needs an SPT rooted at the protected
    // destination over a reversed topology -- see SegmentRouting's TI-LFA code for how this is
    // used). The no-root overload above is now a thin wrapper: `calculateShortestPath(dest,
    // topology, ...)` == `calculateShortestPath(routerId, dest, topology, ...)`, so every
    // existing caller (RsvpTe's CSPF, SegmentRouting's own node-SID programming) is completely
    // unaffected.
    virtual Ipv4AddressVector calculateShortestPath(Ipv4Address root, Ipv4AddressVector dest,
            const TeLinkStateInfoVector& topology, double req_bandwidth, int priority,
            uint32_t includeAny = 0, uint32_t excludeAny = 0);

    // Reports the shortest-path COST (not just the hop list) from `root` to `dest`, via the
    // same constrained Bellman-Ford as calculateShortestPath() above. Returns false (leaves
    // outDistance untouched) if dest is unreachable from root under the given constraints.
    // TI-LFA (Workstream F2) needs true path COST, not hop count, to decide whether a node's
    // shortest path actually depends on a specific link: a node can be reachable via a strictly
    // LONGER alternate path once a link is excluded, which is a real, meaningful difference
    // that a hop-count-only comparison would miss (two paths can have the same hop count but
    // very different cost, or vice versa) -- see SegmentRouting's TI-LFA P-space/Q-space
    // membership test for how this is used.
    virtual bool getShortestPathCost(Ipv4Address root, Ipv4Address dest,
            const TeLinkStateInfoVector& topology, double req_bandwidth, int priority,
            double& outDistance, uint32_t includeAny = 0, uint32_t excludeAny = 0);

    /** @name Public interface to the Traffic Engineering Database */
    //@{
    virtual Ipv4Address getInterfaceAddrByPeerAddress(Ipv4Address peerIP);
    virtual Ipv4Address peerRemoteInterface(Ipv4Address peerIP);
    virtual Ipv4Address getPeerByLocalAddress(Ipv4Address localInf);
    virtual Ipv4Address primaryAddress(Ipv4Address localInf);
    virtual bool isLocalPeer(Ipv4Address inetAddr);
    virtual bool isLocalAddress(Ipv4Address addr);
    virtual unsigned int linkIndex(Ipv4Address localInf);
    virtual unsigned int linkIndex(Ipv4Address advrouter, Ipv4Address linkid);
    virtual Ipv4AddressVector getLocalAddress();

    // Read-only access to the link state database, for callers outside ted/
    // (LinkStateRouting's flooding code, RsvpTe's CAC/bandwidth checks) that
    // used to reach into the `ted` vector directly.
    virtual int getNumLinks() const { return ted.size(); }
    virtual const TeLinkStateInfo& getLink(int index) const;
    // Whole-database read access, kept alongside the per-index accessors
    // above for callers that iterate the full topology (e.g. deriving the
    // peer list from the TED, or building a flood snapshot); returning a
    // const reference avoids copying on every use.
    virtual const TeLinkStateInfoVector& getLinks() const { return ted; }

    // Appends a link discovered via flooding (LinkStateRouting's topology
    // merge); distinct from setLinkState() below, which only ever toggles
    // the up/down state of an EXISTING link.
    virtual void addLink(const TeLinkStateInfo& link) { ted.push_back(link); }

    // Narrow write accessor for RSVP-TE's CAC bandwidth bookkeeping
    // (allocateResource()/preempt()), so callers don't need a non-const
    // reference into the link database.
    virtual void adjustUnresvBandwidth(int index, int priority, double delta);

    // Single owner of link-liveness changes: no-ops if the link is already
    // in the requested state (so redundant reports from Ldp/RsvpTe -- e.g. a
    // Hello session re-establishing without the link ever having gone down
    // in the TED's eyes -- don't cause spurious rebuilds/floods); otherwise
    // flips the state and, in this canonical order, (1) rebuilds the routing
    // table (respecting installRoutes) and (2) announces the change via
    // tedChangedSignal so LinkStateRouting can flood it. Ldp/RsvpTe used to
    // each perform this flip+rebuild+announce choreography inline (with
    // per-call-site ordering that differed from each other); now they just
    // report the observed liveness change and Ted owns what happens next.
    virtual void setLinkState(int index, bool up);

    virtual void rebuildRoutingTable();

    // Emits tedChangedSignal for link `index` without touching its state --
    // used directly by RsvpTe for bandwidth-only changes (CAC accounting),
    // which aren't a liveness flip and so don't go through setLinkState().
    virtual void announceLinkChange(int index);

    // Escape hatch for LinkStateRouting's flood-merge/topology-discovery
    // logic: checkLinkValidity() hands back a pointer into the internal
    // link database (non-const) for the caller to update in place when a
    // newer link-state record supersedes a stale one. This predates (and
    // is out of scope for) the link-liveness encapsulation above -- it's
    // about topology discovery, not liveness ownership.
    virtual bool checkLinkValidity(TeLinkStateInfo link, TeLinkStateInfo *& match);
    virtual void updateTimestamp(int index);
    //@}

    /** @name TE attribute query helpers (D3)
     * Consumed by calculateShortestPath()/calculateShortestPaths() below (Workstream C6,
     * CSPF-with-affinities); TI-LFA (Workstream F2) is expected to be a future caller too.
     * Static because they only look at the fields already carried by a
     * TeLinkStateInfo record (populated by initializeTED() from the
     * "linkAttributes" NED param, or defaulted to 0/empty).
     */
    //@{
    // teMetric==0 means "not configured"; fall back to the IGP metric.
    static double getTeMetric(const TeLinkStateInfo& link) { return link.teMetric != 0 ? link.teMetric : link.metric; }

    // RFC 3209 Section 4.7.4-style affinity match: the link's adminGroup bits
    // must intersect includeAny (or includeAny==0, meaning "no include
    // constraint"), and must not intersect excludeAny.
    static bool matchesAffinity(const TeLinkStateInfo& link, uint32_t includeAny, uint32_t excludeAny)
    {
        return (includeAny == 0 || (link.adminGroup & includeAny) != 0) && (link.adminGroup & excludeAny) == 0;
    }
    //@}

    virtual void handleStartOperation(LifecycleOperation *operation) override;
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;

    // cListener method: reacts to interfaceStateChangedSignal when trackInterfaceState is
    // enabled (see Ted.ned), translating a local interface's up/down (carrier) transition into
    // a setLinkState() call for the matching TED link.
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details) override;

  protected:
    ModuleRefByPar<IIpv4RoutingTable> rt;
    ModuleRefByPar<IInterfaceTable> ift;
    Ipv4Address routerId;

    bool installRoutes = true; // whether rebuildRoutingTable() is allowed to touch the routing table
    bool trackInterfaceState = false; // whether this module subscribes to interfaceStateChangedSignal (see Ted.ned)

    Ipv4AddressVector interfaceAddrs; // list of local interface addresses

    int maxMessageId = 0;

    /**
     * The link state database. (TeLinkStateInfoVector is defined in Ted.msg)
     * Access from outside Ted goes through the accessor methods above.
     */
    TeLinkStateInfoVector ted;

  protected:
    virtual int assignIndex(std::vector<Vertex>& vertices, Ipv4Address nodeAddr);

    // `root` used to be implicitly this Ted's own routerId (see Workstream F2's explicit-root
    // extension); every call site now passes it explicitly.
    std::vector<Vertex> calculateShortestPaths(Ipv4Address root, const TeLinkStateInfoVector& topology,
            double req_bandwidth, int priority, uint32_t includeAny = 0, uint32_t excludeAny = 0);
};

std::ostream& operator<<(std::ostream& os, const TeLinkStateInfo& info);

} // namespace inet

#endif

