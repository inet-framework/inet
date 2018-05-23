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

#ifndef __INET_BGPROUTING_H
#define __INET_BGPROUTING_H

#include "inet/common/INETDefs.h"

#include "inet/transportlayer/contract/tcp/TcpSocket.h"
#include "inet/common/socket/SocketMap.h"
#include "inet/routing/bgpv4/BgpRoutingTableEntry.h"
#include "inet/routing/bgpv4/BgpCommon.h"
#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"
#include "inet/networklayer/contract/ipv4/Ipv4Address.h"
#include "inet/routing/bgpv4/bgpmessage/BgpHeader_m.h"
#include "inet/routing/bgpv4/bgpmessage/BgpUpdate.h"
#include "inet/common/lifecycle/ILifecycle.h"

namespace inet {

namespace bgp {

class BgpSession;

class INET_API Bgp : public cSimpleModule, public ILifecycle, public TcpSocket::ICallback
{
  public:
    Bgp() {}

    virtual ~Bgp();

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override;
    virtual bool handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback) override;
    virtual void finish() override;

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
    //functions used by the BgpSession class
    void getScheduleAt(simtime_t t, cMessage *msg) { scheduleAt(t, msg); }
    simtime_t getSimTime() { return simTime(); }
    void getCancelAndDelete(cMessage *msg) { return cancelAndDelete(msg); }
    cMessage *getCancelEvent(cMessage *msg) { return cancelEvent(msg); }
    cGate *getGate(const char *gateName) { return gate(gateName); }
    IIpv4RoutingTable *getIPRoutingTable() { return _rt; }
    std::vector<RoutingTableEntry *> getBGPRoutingTable() { return _BGPRoutingTable; }
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
    void updateSendProcess(const unsigned char decisionProcessResult, SessionId sessionIndex, RoutingTableEntry *entry);
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
    void handleTimer(cMessage *timer);

    void processMessageFromTCP(cMessage *msg);
    void processMessage(const BgpOpenMessage& msg);
    void processMessage(const BgpKeepAliveMessage& msg);
    void processMessage(const BgpUpdateMessage& msg);

    bool deleteBGPRoutingEntry(RoutingTableEntry *entry);
    /**
     * \brief RFC 4271: 9.1. : Decision Process used when an UPDATE message is received
     *  As matches, routes are sent or not to UpdateSentProcess
     *  The result can be ROUTE_DESTINATION_CHANGED, NEW_ROUTE_ADDED or 0 if no routingTable modification
     */
    unsigned char decisionProcess(const BgpUpdateMessage& msg, RoutingTableEntry *entry, SessionId sessionIndex);
    /**
     * \brief RFC 4271: 9.1.2.2 Breaking Ties used when BGP speaker may have several routes
     *  to the same destination that have the same degree of preference.
     *
     * \return bool, true if this process changed the route, false else
     */
    bool tieBreakingProcess(RoutingTableEntry *oldEntry, RoutingTableEntry *entry);

    SessionId createSession(BgpSessionType typeSession, const char *peerAddr);
    bool isInASList(std::vector<AsId> ASList, RoutingTableEntry *entry);
    unsigned long isInTable(std::vector<RoutingTableEntry *> rtTable, RoutingTableEntry *entry);

    std::vector<const char *> loadASConfig(cXMLElementList& ASConfig);
    void loadSessionConfig(cXMLElementList& sessionList, simtime_t *delayTab);
    void loadConfigFromXML(cXMLElement *bgpConfig);
    AsId findMyAS(cXMLElementList& ASList, int& outRouterPosition);
    bool ospfExist(IIpv4RoutingTable *rtTable);
    void loadTimerConfig(cXMLElementList& timerConfig, simtime_t *delayTab);
    unsigned char asLoopDetection(RoutingTableEntry *entry, AsId myAS);
    SessionId findIdFromPeerAddr(std::map<SessionId, BgpSession *> sessions, Ipv4Address peerAddr);
    int isInRoutingTable(IIpv4RoutingTable *rtTable, Ipv4Address addr);
    int isInInterfaceTable(IInterfaceTable *rtTable, Ipv4Address addr);
    SessionId findIdFromSocketConnId(std::map<SessionId, BgpSession *> sessions, int connId);
    unsigned int calculateStartDelay(int rtListSize, unsigned char rtPosition, unsigned char rtPeerPosition);

    SocketMap _socketMap;
    AsId _myAS = 0;
    SessionId _currSessionId = 0;

    IInterfaceTable *_inft = nullptr;
    IIpv4RoutingTable *_rt = nullptr;    // The IP routing table
    typedef std::vector<RoutingTableEntry *> RoutingTableEntryVector;
    RoutingTableEntryVector _BGPRoutingTable;    // The BGP routing table
    RoutingTableEntryVector _prefixListIN;
    RoutingTableEntryVector _prefixListOUT;
    RoutingTableEntryVector _prefixListINOUT;      // store union of pointers in _prefixListIN and _prefixListOUT
    std::vector<AsId> _ASListIN;
    std::vector<AsId> _ASListOUT;
    std::map<SessionId, BgpSession *> _BGPSessions;

    static const int BGP_TCP_CONNECT_VALID = 71;
    static const int BGP_TCP_CONNECT_CONFIRM = 72;
    static const int BGP_TCP_CONNECT_FAILED = 73;
    static const int BGP_TCP_CONNECT_OPEN_RCV = 74;
    static const int BGP_TCP_KEEP_ALIVE_RCV = 75;
};

} // namespace bgp

} // namespace inet

#endif // ifndef __INET_BGPROUTING_H

