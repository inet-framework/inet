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

#include "inet/transportlayer/contract/tcp/TCPSocket.h"
#include "inet/transportlayer/contract/tcp/TCPSocketMap.h"
#include "inet/routing/bgpv4/BGPRoutingTableEntry.h"
#include "inet/routing/bgpv4/BGPCommon.h"
#include "inet/networklayer/ipv4/IPv4InterfaceData.h"
#include "inet/networklayer/contract/ipv4/IPv4Address.h"
#include "inet/routing/bgpv4/BGPMessage/BGPOpen.h"
#include "inet/routing/bgpv4/BGPMessage/BGPKeepAlive.h"
#include "inet/routing/bgpv4/BGPMessage/BGPUpdate.h"
#include "inet/common/lifecycle/ILifecycle.h"

namespace inet {

namespace bgp {

class BGPSession;

class INET_API BGPRouting : public cSimpleModule, public ILifecycle, public TCPSocket::CallbackInterface
{
  public:
    BGPRouting()
        : _myAS(0), _inft(0), _rt(0) {}

    virtual ~BGPRouting();

  protected:
    virtual int numInitStages() const { return NUM_INIT_STAGES; }
    virtual void initialize(int stage);
    virtual void handleMessage(cMessage *msg);
    virtual bool handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback);
    virtual void finish();

    virtual void socketDataArrived(int connId, void *yourPtr, cPacket *msg, bool urgent);
    virtual void socketEstablished(int connId, void *yourPtr);
    virtual void socketFailure(int connId, void *yourPtr, int code);
    virtual void socketPeerClosed(int connId, void *yourPtr) {}
    virtual void socketClosed(int connId, void *yourPtr) {}

    friend class BGPSession;
    //functions used by the BGPSession class
    int getScheduleAt(simtime_t t, cMessage *msg) { return scheduleAt(t, msg); }
    simtime_t getSimTime() { return simTime(); }
    void getCancelAndDelete(cMessage *msg) { return cancelAndDelete(msg); }
    cMessage *getCancelEvent(cMessage *msg) { return cancelEvent(msg); }
    cGate *getGate(const char *gateName) { return gate(gateName); }
    IIPv4RoutingTable *getIPRoutingTable() { return _rt; }
    std::vector<RoutingTableEntry *> getBGPRoutingTable() { return _BGPRoutingTable; }
    /**
     * \brief active listenSocket for a given session (used by fsm)
     */
    void listenConnectionFromPeer(SessionID sessionID);
    /**
     * \brief active TCPConnection for a given session (used by fsm)
     */
    void openTCPConnectionToPeer(SessionID sessionID);
    /**
     * \brief RFC 4271, 9.2 : Update-Send Process / Sent or not new UPDATE messages to its peers
     */
    void updateSendProcess(const unsigned char decisionProcessResult, SessionID sessionIndex, RoutingTableEntry *entry);
    /**
     * \brief find the next SessionID compared to his type and start this session if boolean is true
     */
    SessionID findNextSession(BGPSessionType type, bool startSession = false);
    /**
     * \brief check if the route is in OSPF external IPv4RoutingTable
     *
     * \return true if it is, false else
     */
    bool checkExternalRoute(const IPv4Route *ospfRoute);

  private:
    void handleTimer(cMessage *timer);

    void processMessageFromTCP(cMessage *msg);
    void processMessage(const BGPOpenMessage& msg);
    void processMessage(const BGPKeepAliveMessage& msg);
    void processMessage(const BGPUpdateMessage& msg);

    bool deleteBGPRoutingEntry(RoutingTableEntry *entry);
    /**
     * \brief RFC 4271: 9.1. : Decision Process used when an UPDATE message is received
     *  As matches, routes are sent or not to UpdateSentProcess
     *  The result can be ROUTE_DESTINATION_CHANGED, NEW_ROUTE_ADDED or 0 if no routingTable modification
     */
    unsigned char decisionProcess(const BGPUpdateMessage& msg, RoutingTableEntry *entry, SessionID sessionIndex);
    /**
     * \brief RFC 4271: 9.1.2.2 Breaking Ties used when BGP speaker may have several routes
     *  to the same destination that have the same degree of preference.
     *
     * \return bool, true if this process changed the route, false else
     */
    bool tieBreakingProcess(RoutingTableEntry *oldEntry, RoutingTableEntry *entry);

    SessionID createSession(BGPSessionType typeSession, const char *peerAddr);
    bool isInASList(std::vector<ASID> ASList, RoutingTableEntry *entry);
    unsigned long isInTable(std::vector<RoutingTableEntry *> rtTable, RoutingTableEntry *entry);

    std::vector<const char *> loadASConfig(cXMLElementList& ASConfig);
    void loadSessionConfig(cXMLElementList& sessionList, simtime_t *delayTab);
    void loadConfigFromXML(cXMLElement *bgpConfig);
    ASID findMyAS(cXMLElementList& ASList, int& outRouterPosition);
    bool ospfExist(IIPv4RoutingTable *rtTable);
    void loadTimerConfig(cXMLElementList& timerConfig, simtime_t *delayTab);
    unsigned char asLoopDetection(RoutingTableEntry *entry, ASID myAS);
    SessionID findIdFromPeerAddr(std::map<SessionID, BGPSession *> sessions, IPv4Address peerAddr);
    int isInRoutingTable(IIPv4RoutingTable *rtTable, IPv4Address addr);
    int isInInterfaceTable(IInterfaceTable *rtTable, IPv4Address addr);
    SessionID findIdFromSocketConnId(std::map<SessionID, BGPSession *> sessions, int connId);
    unsigned int calculateStartDelay(int rtListSize, unsigned char rtPosition, unsigned char rtPeerPosition);

    TCPSocketMap _socketMap;
    ASID _myAS;
    SessionID _currSessionId;

    IInterfaceTable *_inft;
    IIPv4RoutingTable *_rt;    // The IP routing table
    std::vector<RoutingTableEntry *> _BGPRoutingTable;    // The BGP routing table
    std::vector<RoutingTableEntry *> _prefixListIN;
    std::vector<RoutingTableEntry *> _prefixListOUT;
    std::vector<ASID> _ASListIN;
    std::vector<ASID> _ASListOUT;
    std::map<SessionID, BGPSession *> _BGPSessions;

    static const int BGP_TCP_CONNECT_VALID = 71;
    static const int BGP_TCP_CONNECT_CONFIRM = 72;
    static const int BGP_TCP_CONNECT_FAILED = 73;
    static const int BGP_TCP_CONNECT_OPEN_RCV = 74;
    static const int BGP_TCP_KEEP_ALIVE_RCV = 75;
};

} // namespace bgp

} // namespace inet

#endif // ifndef __INET_BGPROUTING_H

