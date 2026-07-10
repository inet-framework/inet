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
class INET_API Ted : public RoutingProtocolBase
{
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

    virtual Ipv4AddressVector calculateShortestPath(Ipv4AddressVector dest,
            const TeLinkStateInfoVector& topology, double req_bandwidth, int priority);

  public:
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

    virtual void handleStartOperation(LifecycleOperation *operation) override;
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;

  protected:
    ModuleRefByPar<IIpv4RoutingTable> rt;
    ModuleRefByPar<IInterfaceTable> ift;
    Ipv4Address routerId;

    bool installRoutes = true; // whether rebuildRoutingTable() is allowed to touch the routing table

    Ipv4AddressVector interfaceAddrs; // list of local interface addresses

    int maxMessageId = 0;

    /**
     * The link state database. (TeLinkStateInfoVector is defined in Ted.msg)
     * Access from outside Ted goes through the accessor methods above.
     */
    TeLinkStateInfoVector ted;

  protected:
    virtual int assignIndex(std::vector<Vertex>& vertices, Ipv4Address nodeAddr);

    std::vector<Vertex> calculateShortestPaths(const TeLinkStateInfoVector& topology,
            double req_bandwidth, int priority);
};

std::ostream& operator<<(std::ostream& os, const TeLinkStateInfo& info);

} // namespace inet

#endif

