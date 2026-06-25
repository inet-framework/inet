//
// Copyright (C) 2010 Helene Lageber
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_BGPROUTER_H
#define __INET_BGPROUTER_H

#include "inet/common/Protocol.h"
#include "inet/common/socket/SocketMap.h"
#include "inet/networklayer/contract/IRoutingTable.h"
#include "inet/networklayer/contract/ipv4/Ipv4Address.h"
#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"
#include "inet/routing/bgpv4/BgpCommon.h"
#include "inet/routing/bgpv4/BgpRoutingTableEntry.h"
#include "inet/routing/bgpv4/bgpmessage/BgpHeader_m.h"
#include "inet/routing/bgpv4/bgpmessage/BgpUpdate.h"
#include "inet/routing/ospfv2/Ospfv2.h"
#include "inet/transportlayer/contract/tcp/TcpSocket.h"

namespace inet {

namespace bgp {

class BgpSession;

class INET_API BgpRouter : public TcpSocket::BufferingCallback
{
  private:
    IInterfaceTable *ift = nullptr;
    IRoutingTable *rt = nullptr;
    cSimpleModule *bgpModule = nullptr;
    const Protocol *networkProtocol = &Protocol::ipv4; // address family this BGP router serves
    Ipv4Address routerIdParam; // explicit 4-octet BGP Identifier from the 'routerId' parameter (unspecified if not set)

    ospfv2::Ospfv2 *ospfModule = nullptr;
    AsId myAsId = 0;
    bool redistributeInternal = false;
    bool redistributeRip = false;
    bool redistributeOspf = false;
    struct redistributeOspfType_t {
        bool interArea;
        bool intraArea;
        bool externalType1;
        bool externalType2;
    };
    redistributeOspfType_t redistributeOspfType = {};
    SocketMap _socketMap;
    // A single shared listening socket per router accepts all incoming BGP connections
    // on TCP_PORT (wildcard bind) and demuxes them to sessions by peer address in
    // processMessageFromTcp(). RFC 4271: a BGP speaker listens for connections on port 179.
    TcpSocket *listeningSocket = nullptr;
    SessionId _currSessionId = 0;
    std::map<SessionId, BgpSession *> _bgpSessions;
    uint32_t numEgpSessions = 0;
    uint32_t numIgpSessions = 0;
    L3Address internalAddress;

    typedef std::vector<BgpRouteInfo *> RoutingTableEntryVector;
    RoutingTableEntryVector bgpRoutingTable; // The BGP routing table
    std::vector<L3Address> advertiseList;
    RoutingTableEntryVector _prefixListIN;
    RoutingTableEntryVector _prefixListOUT;
    RoutingTableEntryVector _prefixListINOUT; // store union of pointers in _prefixListIN and _prefixListOUT
    std::vector<AsId> _ASListIN;
    std::vector<AsId> _ASListOUT;

  public:
    enum { TCP_PORT = 179 };
    BgpRouter(cSimpleModule *bgpModule, IInterfaceTable *ift, IRoutingTable *rt, const Protocol *networkProtocol);
    virtual ~BgpRouter();

    bool isIpv6() const { return networkProtocol == &Protocol::ipv6; }
    RouterId getRouterId()
    {
        if (!routerIdParam.isUnspecified())
            return routerIdParam;
        L3Address id = rt->getRouterIdAsGeneric();
        if (id.getType() == L3Address::IPv4)
            return id.toIpv4();
        throw cRuntimeError("BGP: set the 'routerId' parameter (a 4-octet BGP Identifier) for IPv6 BGP");
    }
    void setAsId(AsId myAsId) { this->myAsId = myAsId; }
    AsId getAsId() { return myAsId; }
    int getNumBgpSessions() { return _bgpSessions.size(); }
    int getNumEgpSessions() { return numEgpSessions; }
    int getNumIgpSessions() { return numIgpSessions; }
    void setDefaultConfig();
    L3Address getInternalAddress() { return internalAddress; }
    void setInternalAddress(L3Address x) { internalAddress = x; }
    // the local BGP source address on the given link, per address family (routable: IPv4
    // address, or the preferred/global IPv6 address)
    L3Address getInterfaceAddress(NetworkInterface *ie);
    bool getRedistributeInternal() { return redistributeInternal; }
    void setRedistributeInternal(bool x) { this->redistributeInternal = x; }
    bool getRedistributeRip() { return redistributeRip; }
    void setRedistributeRip(bool x) { this->redistributeRip = x; }
    bool getRedistributeOspf() { return redistributeOspf; }
    void setRedistributeOspf(std::string x);
    void printSessionSummary();
    void addWatches();
    void recordStatistics();
    void closeSessions(bool abort);

    SessionId createEbgpSession(const char *peerAddr, SessionInfo& externalInfo);
    SessionId createIbgpSession(const char *peerAddr);
    void setTimer(SessionId id, simtime_t *delayTab);
    void addToAdvertiseList(const L3Address& address);
    void addToPrefixList(std::string nodeName, BgpRouteInfo *entry);
    // create an address-family-appropriate BGP RIB entry (Ipv4Route- or Ipv6Route-backed)
    BgpRouteInfo *createBgpRoutingTableEntry();
    BgpRouteInfo *createBgpRoutingTableEntry(const IRoute *from);
    void addToAsList(std::string nodeName, AsId id);
    void setNextHopSelf(Ipv4Address peer, bool nextHopSelf);
    void setLocalPreference(Ipv4Address peer, int localPref);
    bool isExternalAddress(const IRoute& rtEntry);
    void processMessageFromTcp(cMessage *msg);

    void printOpenMessage(const BgpOpenMessage& msg);
    void printUpdateMessage(const BgpUpdateMessage& msg);
//    void printNotificationMessage(const BgpNotificationMessage& msg);
    void printKeepAliveMessage(const BgpKeepAliveMessage& msg);

  protected:
    /** @name TcpSocket::ICallback callback methods */
    //@{
    virtual void socketDataArrived(TcpSocket *socket) override;
    virtual void socketDataArrived(TcpSocket *socket, Packet *packet, bool urgent) override;
    virtual void socketAvailable(TcpSocket *socket, TcpAvailableInfo *availableInfo) override { socket->accept(availableInfo->getNewSocketId()); } // TODO
    virtual void socketEstablished(TcpSocket *socket) override;
    virtual void socketPeerClosed(TcpSocket *socket) override;
    virtual void socketClosed(TcpSocket *socket) override {}
    virtual void socketFailure(TcpSocket *socket, int code) override;
    virtual void socketStatusArrived(TcpSocket *socket, TcpStatusInfo *status) override {}
    virtual void socketDeleted(TcpSocket *socket) override {}
    //@}

    friend class BgpSession;
    // functions used by the BgpSession class
    void scheduleAt(simtime_t t, cMessage *msg) { bgpModule->scheduleAt(t, msg); }
    void cancelAndDelete(cMessage *msg) { bgpModule->cancelAndDelete(msg); }
    cMessage *cancelEvent(cMessage *msg) { return bgpModule->cancelEvent(msg); }
    IRoutingTable *getIpRoutingTable() { return rt; }
    std::vector<BgpRouteInfo *> getBgpRoutingTable() { return bgpRoutingTable; }

    /**
     * \brief active listenSocket for a given session (used by fsm)
     */
    void listenConnectionFromPeer(SessionId sessionId);
    /**
     * \brief active TcpConnection for a given session (used by fsm)
     */
    void openTcpConnectionToPeer(SessionId sessionId);
    /**
     * \brief RFC 4271, 9.2 : Update-Send Process / Sent or not new UPDATE messages to its peers
     */
    void updateSendProcess(BgpProcessResult decisionProcessResult, SessionId sessionIndex, BgpRouteInfo *entry);
    /**
     * \brief find the next SessionId compared to his type and start this session if boolean is true
     */
    SessionId findNextSession(BgpSessionType type, bool startSession = false);

  private:

    void processChunks(const BgpHeader& ptrHdr);
    void processMessage(const BgpOpenMessage& msg);
    void processMessage(const BgpKeepAliveMessage& msg);
    void processMessage(const BgpUpdateMessage& msg);

    bool deleteBgpRoutingEntry(BgpRouteInfo *entry);

    /**
     * \brief RFC 4271: 9.1. : Decision Process used when an UPDATE message is received
     *  As matches, routes are sent or not to UpdateSentProcess
     *  The result can be ROUTE_DESTINATION_CHANGED, NEW_ROUTE_ADDED or 0 if no routingTable modification
     */
    BgpProcessResult decisionProcess(const BgpUpdateMessage& msg, BgpRouteInfo *entry, SessionId sessionIndex);

    /**
     * \brief RFC 4271: 9.1.2.2 Breaking Ties used when BGP speaker may have several routes
     *  to the same destination that have the same degree of preference.
     *
     * \return bool, true if this process changed the route, false else
     */
    bool tieBreakingProcess(BgpRouteInfo *oldEntry, BgpRouteInfo *entry);

    bool isInASList(std::vector<AsId> ASList, BgpRouteInfo *entry);
    unsigned long isInTable(std::vector<BgpRouteInfo *> rtTable, BgpRouteInfo *entry);

    bool ospfExist(IRoutingTable *rtTable);
    // check if the route is in OSPF external Ipv4RoutingTable
    int checkExternalRoute(const Ipv4Route *ospfRoute) { return ospfModule->checkExternalRoute(ospfRoute->getDestination()); }
    BgpProcessResult asLoopDetection(BgpRouteInfo *entry, AsId myAS);
    int isInRoutingTable(IRoutingTable *rtTable, const L3Address& addr);
    SessionId findIdFromPeerAddr(std::map<SessionId, BgpSession *> sessions, const L3Address& peerAddr);
    static uint32_t addressKey(const L3Address& addr); // AF-safe key for the session id (IPv4 value preserved)
    SessionId findIdFromSocketConnId(std::map<SessionId, BgpSession *> sessions, int connId);
    bool isRouteExcluded(const IRoute& rtEntry);
    bool isDefaultRoute(const IRoute *entry) const;
    bool isReachable(const L3Address& addr) const;
};

} // namespace bgp

} // namespace inet

#endif
