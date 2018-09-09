//
// Copyright (C) 2010 Helene Lageber
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

#ifndef __INET_BGPROUTER_H
#define __INET_BGPROUTER_H

#include "inet/common/INETDefs.h"

#include "inet/transportlayer/contract/tcp/TcpSocket.h"
#include "inet/common/socket/SocketMap.h"
#include "inet/routing/bgpv4/BgpRoutingTableEntry.h"
#include "inet/routing/bgpv4/BgpCommon.h"
#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"
#include "inet/networklayer/contract/ipv4/Ipv4Address.h"
#include "inet/routing/bgpv4/bgpmessage/BgpHeader_m.h"
#include "inet/routing/bgpv4/bgpmessage/BgpUpdate.h"

namespace inet {

namespace bgp {

class BgpSession;

class INET_API BgpRouter : public TcpSocket::ICallback
{
private:
    IIpv4RoutingTable *rt = nullptr;    // The IP routing table
    RouterId routerID;    // The router ID assigned by the IP layer
    AsId myAsId = 0;
    cSimpleModule *bgpModule = nullptr;
    SocketMap _socketMap;
    SessionId _currSessionId = 0;
    std::map<SessionId, BgpSession *> _BGPSessions;

    typedef std::vector<BgpRoutingTableEntry *> RoutingTableEntryVector;
    RoutingTableEntryVector bgpRoutingTable;    // The BGP routing table
    RoutingTableEntryVector _prefixListIN;
    RoutingTableEntryVector _prefixListOUT;
    RoutingTableEntryVector _prefixListINOUT;   // store union of pointers in _prefixListIN and _prefixListOUT
    std::vector<AsId> _ASListIN;
    std::vector<AsId> _ASListOUT;

  public:
    BgpRouter(RouterId id, cSimpleModule *bgpModule, IIpv4RoutingTable *rt);
    virtual ~BgpRouter();

    void setRouterId(RouterId routerID) { this->routerID = routerID; }
    RouterId getRouterId() { return routerID; }
    void setAsId(AsId myAsId) { this->myAsId = myAsId; }
    AsId getAsId() { return myAsId; }
    int getNumBgpSessions() { return _BGPSessions.size(); }
    void addWatches();
    void recordStatistics();

    SessionId createSession(BgpSessionType typeSession, const char *peerAddr);
    void setTimer(SessionId id, simtime_t *delayTab);
    void setSocketListen(SessionId id);
    void addToPrefixList(std::string nodeName, BgpRoutingTableEntry *entry);
    void addToAsList(std::string nodeName, AsId id);
    void processMessageFromTCP(cMessage *msg);

  protected:
    /** @name TcpSocket::ICallback callback methods */
    //@{
    virtual void socketDataArrived(TcpSocket *socket, Packet *msg, bool urgent) override;
    virtual void socketAvailable(TcpSocket *socket, TcpAvailableInfo *availableInfo) override { socket->accept(availableInfo->getNewSocketId()); }      //TODO
    virtual void socketEstablished(TcpSocket *socket) override;
    virtual void socketPeerClosed(TcpSocket *socket) override {}
    virtual void socketClosed(TcpSocket *socket) override {}
    virtual void socketFailure(TcpSocket *socket, int code) override;
    virtual void socketStatusArrived(TcpSocket *socket, TcpStatusInfo *status) override { }
    virtual void socketDeleted(TcpSocket *socket) override {}
    //@}

    friend class BgpSession;
    // functions used by the BgpSession class
    void getScheduleAt(simtime_t t, cMessage *msg) { bgpModule->scheduleAt(t, msg); }
    void getCancelAndDelete(cMessage *msg) { return bgpModule->cancelAndDelete(msg); }
    cMessage *getCancelEvent(cMessage *msg) { return bgpModule->cancelEvent(msg); }
    IIpv4RoutingTable *getIPRoutingTable() { return rt; }
    std::vector<BgpRoutingTableEntry *> getBGPRoutingTable() { return bgpRoutingTable; }

    /**
     * \brief active listenSocket for a given session (used by fsm)
     */
    void listenConnectionFromPeer(SessionId sessionID);
    /**
     * \brief active TcpConnection for a given session (used by fsm)
     */
    void openTCPConnectionToPeer(SessionId sessionID);
    /**
     * \brief RFC 4271, 9.2 : Update-Send Process / Sent or not new UPDATE messages to its peers
     */
    void updateSendProcess(const unsigned char decisionProcessResult, SessionId sessionIndex, BgpRoutingTableEntry *entry);
    /**
     * \brief find the next SessionId compared to his type and start this session if boolean is true
     */
    SessionId findNextSession(BgpSessionType type, bool startSession = false);
    /**
     * \brief check if the route is in OSPF external Ipv4RoutingTable
     *
     * \return true if it is, false else
     */
    bool checkExternalRoute(const Ipv4Route *ospfRoute);

  private:

    void processMessage(const BgpOpenMessage& msg);
    void processMessage(const BgpKeepAliveMessage& msg);
    void processMessage(const BgpUpdateMessage& msg);

    bool deleteBGPRoutingEntry(BgpRoutingTableEntry *entry);
    /**
     * \brief RFC 4271: 9.1. : Decision Process used when an UPDATE message is received
     *  As matches, routes are sent or not to UpdateSentProcess
     *  The result can be ROUTE_DESTINATION_CHANGED, NEW_ROUTE_ADDED or 0 if no routingTable modification
     */
    unsigned char decisionProcess(const BgpUpdateMessage& msg, BgpRoutingTableEntry *entry, SessionId sessionIndex);
    /**
     * \brief RFC 4271: 9.1.2.2 Breaking Ties used when BGP speaker may have several routes
     *  to the same destination that have the same degree of preference.
     *
     * \return bool, true if this process changed the route, false else
     */
    bool tieBreakingProcess(BgpRoutingTableEntry *oldEntry, BgpRoutingTableEntry *entry);

    bool isInASList(std::vector<AsId> ASList, BgpRoutingTableEntry *entry);
    unsigned long isInTable(std::vector<BgpRoutingTableEntry *> rtTable, BgpRoutingTableEntry *entry);

    bool ospfExist(IIpv4RoutingTable *rtTable);
    unsigned char asLoopDetection(BgpRoutingTableEntry *entry, AsId myAS);
    int isInRoutingTable(IIpv4RoutingTable *rtTable, Ipv4Address addr);
    SessionId findIdFromPeerAddr(std::map<SessionId, BgpSession *> sessions, Ipv4Address peerAddr);
    SessionId findIdFromSocketConnId(std::map<SessionId, BgpSession *> sessions, int connId);
};

} // namespace bgp

} // namespace inet

#endif // ifndef __INET_BGPROUTER_H

