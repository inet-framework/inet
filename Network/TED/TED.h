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

#ifndef __TED_H__
#define __TED_H__

#include <omnetpp.h>

#include "TED_m.h"
#include "IntServ.h"

#define LS_INFINITY   1e16

#define TED_TRAFFIC         1

class RoutingTable;
class InterfaceTable;
class InterfaceEntry;
class NotificationBoard;


/**
 * Contains the Traffic Engineering Database and provides public methods
 * to access it from MPLS signalling protocols (LDP, RSVP-TE).
 *
 * Also contains implementation of a minimalistic link state routing protocol.
 *
 * See NED file for more info.
 */
class TED : public cSimpleModule
{
  public:
    /**
     * Only used internally, during shortest path calculation:
     * vertex in the graph we build from links in TELinkStateInfoVector.
     */
    struct vertex_t
    {
        IPAddress node; // FIXME *** is this the routerID? ***
        int parent;     // index into the same vertex_t vector
        double dist;    // distance to root (???)
    };

    /**
     * Only used internally, during shortest path calculation:
     * edge in the graph we build from links in TELinkStateInfoVector.
     */
    struct edge_t
    {
        int src;       // index into the vertex_t[] vector
        int dest;      // index into the vertex_t[] vector
        double metric; // link cost
    };

    /**
     * The link state database. (TELinkStateInfoVector is defined in TED.msg)
     */
    TELinkStateInfoVector ted;

  public:
    TED();
    virtual ~TED();

  protected:
    virtual void initialize(int stage);
    virtual int numInitStages() const  {return 4;}
    virtual void handleMessage(cMessage *msg);

    IPAddressVector calculateShortestPath(IPAddressVector dest,
        const TELinkStateInfoVector& topology, double req_bandwidth, int priority);

  public:
    /** @name Public interface to the Traffic Engineering Database */
    //@{
    IPAddress interfaceAddrByPeerAddress(IPAddress peerIP);
    IPAddress peerRemoteInterface(IPAddress peerIP);
    IPAddress peerByLocalAddress(IPAddress localInf);
    IPAddress primaryAddress(IPAddress localInf);
    bool isLocalPeer(IPAddress inetAddr);
    bool isLocalAddress(IPAddress addr);
    unsigned int linkIndex(IPAddress localInf);
    unsigned int linkIndex(IPAddress advrouter, IPAddress linkid);
    IPAddressVector getLocalAddress();

    void rebuildRoutingTable();
    //@}

  private:
    RoutingTable *rt;
    InterfaceTable *ift;
    IPAddress routerId;
    NotificationBoard *nb;

    IPAddressVector LocalAddress; // FIXME *** indexed by what? ted link index? how to keep consistent? ***
    IPAddressVector TEDPeer;      // FIXME *** indexed by what? ted link index? how to keep consistent? ***

    cMessage *announceMsg;
    int maxMessageId;

    int assignIndex(std::vector<vertex_t>& vertices, IPAddress nodeAddr);

    std::vector<vertex_t> calculateShortestPaths(const TELinkStateInfoVector& topology,
        double req_bandwidth, int priority);

    void sendToPeers(const std::vector<TELinkStateInfo>& list, bool req, IPAddress exPeer);
    void sendToPeer(IPAddress peer, const std::vector<TELinkStateInfo> & list);
    void sendToIP(LinkStateMsg *msg, IPAddress destAddr);

    void processLINK_STATE_MESSAGE(LinkStateMsg* msg, IPAddress sender);
    void processLINK_NOTIFY(LinkNotifyMsg* msg);

    bool checkLinkValidity(TELinkStateInfo link, TELinkStateInfo **match);
    void updateTimestamp(TELinkStateInfo *link);
};

std::ostream & operator<<(std::ostream & os, const TELinkStateInfo& info);

#endif


