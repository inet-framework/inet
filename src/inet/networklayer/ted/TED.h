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

#ifndef __INET_TED_H
#define __INET_TED_H

#include "inet/common/INETDefs.h"

#include "inet/networklayer/ted/TED_m.h"
#include "inet/networklayer/rsvp_te/IntServ.h"
#include "inet/common/lifecycle/ILifecycle.h"

namespace inet {

class IIPv4RoutingTable;
class IInterfaceTable;
class InterfaceEntry;

/**
 * Contains the Traffic Engineering Database and provides public methods
 * to access it from MPLS signalling protocols (LDP, RSVP-TE).
 *
 * See NED file for more info.
 */
class INET_API TED : public cSimpleModule, public ILifecycle
{
  public:
    /**
     * Only used internally, during shortest path calculation:
     * vertex in the graph we build from links in TELinkStateInfoVector.
     */
    struct vertex_t
    {
        IPv4Address node;    // FIXME *** is this the routerID? ***
        int parent;    // index into the same vertex_t vector
        double dist;    // distance to root (???)
    };

    /**
     * Only used internally, during shortest path calculation:
     * edge in the graph we build from links in TELinkStateInfoVector.
     */
    struct edge_t
    {
        int src;    // index into the vertex_t[] vector
        int dest;    // index into the vertex_t[] vector
        double metric;    // link cost
    };

    /**
     * The link state database. (TELinkStateInfoVector is defined in TED.msg)
     */
    TELinkStateInfoVector ted;

  public:
    TED();
    virtual ~TED();

  protected:
    virtual void initialize(int stage) override;
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void handleMessage(cMessage *msg) override;

    virtual void initializeTED();

    virtual IPAddressVector calculateShortestPath(IPAddressVector dest,
            const TELinkStateInfoVector& topology, double req_bandwidth, int priority);

  public:
    /** @name Public interface to the Traffic Engineering Database */
    //@{
    virtual IPv4Address getInterfaceAddrByPeerAddress(IPv4Address peerIP);
    virtual IPv4Address peerRemoteInterface(IPv4Address peerIP);
    virtual IPv4Address getPeerByLocalAddress(IPv4Address localInf);
    virtual IPv4Address primaryAddress(IPv4Address localInf);
    virtual bool isLocalPeer(IPv4Address inetAddr);
    virtual bool isLocalAddress(IPv4Address addr);
    virtual unsigned int linkIndex(IPv4Address localInf);
    virtual unsigned int linkIndex(IPv4Address advrouter, IPv4Address linkid);
    virtual IPAddressVector getLocalAddress();

    virtual void rebuildRoutingTable();
    //@}

    virtual bool handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback) override;

  protected:
    IIPv4RoutingTable *rt = nullptr;
    IInterfaceTable *ift = nullptr;
    IPv4Address routerId;

    IPAddressVector interfaceAddrs;    // list of local interface addresses

    int maxMessageId = 0;

  protected:
    virtual int assignIndex(std::vector<vertex_t>& vertices, IPv4Address nodeAddr);

    std::vector<vertex_t> calculateShortestPaths(const TELinkStateInfoVector& topology,
            double req_bandwidth, int priority);

  public:    //FIXME
    virtual bool checkLinkValidity(TELinkStateInfo link, TELinkStateInfo *& match);
    virtual void updateTimestamp(TELinkStateInfo *link);
};

std::ostream& operator<<(std::ostream& os, const TELinkStateInfo& info);

} // namespace inet

#endif // ifndef __INET_TED_H

