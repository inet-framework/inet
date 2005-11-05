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
 * TODO documentation
 */
class TED : public cSimpleModule
{
  public:
    struct vertex_t
    {
        IPAddress node;
        int parent;
        double dist;
    };

    struct edge_t
    {
        int src;
        int dest;
        double metric;
    };

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
    IPAddress interfaceByPeerAddress(IPAddress peerIP);
    IPAddress peerRemoteInterface(IPAddress peerIP);
    IPAddress peerByLocalAddress(IPAddress localInf);
    IPAddress primaryAddress(IPAddress localInf);
    bool isLocalPeer(IPAddress inetAddr);
    bool isLocalAddress(IPAddress addr);
    unsigned int linkIndex(IPAddress localInf);
    unsigned int linkIndex(IPAddress advrouter, IPAddress linkid);
    IPAddressVector getLocalAddress();
    InterfaceEntry *interfaceByAddress(IPAddress localInf);

    void rebuildRoutingTable();

  private:
    RoutingTable *rt;
    InterfaceTable *ift;
    IPAddress routerId;
    NotificationBoard *nb;

    IPAddressVector LocalAddress;
    IPAddressVector TEDPeer;

    cMessage *announceMsg;
    int maxMessageId;

    int assignIndex(std::vector<vertex_t>& vertices, IPAddress node);

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


