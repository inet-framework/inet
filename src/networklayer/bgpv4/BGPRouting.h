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

#include "INETDefs.h"

#include "TCPSocket.h"
#include "TCPSocketMap.h"
#include "RoutingTableAccess.h"
#include "InterfaceTableAccess.h"
#include "OSPFRoutingAccess.h"
#include "BGPRoutingTableEntry.h"
#include "BGPCommon.h"
#include "IPv4InterfaceData.h"
#include "IPv4Address.h"
#include "BGPOpen.h"
#include "BGPKeepAlive.h"
#include "BGPUpdate.h"

class BGPSession;


class INET_API BGPRouting : public cSimpleModule, public TCPSocket::CallbackInterface
{
public:
    BGPRouting()
        : _myAS(0), _inft(0), _rt(0) {}

    virtual ~BGPRouting();

protected:
    virtual int  numInitStages() const  { return 5; }
    virtual void initialize(int stage);
    virtual void handleMessage(cMessage *msg);
    virtual void finish();

    virtual void socketDataArrived(int connId, void *yourPtr, cPacket *msg, bool urgent);
    virtual void socketEstablished(int connId, void *yourPtr);
    virtual void socketFailure(int connId, void *yourPtr, int code);
    virtual void socketPeerClosed(int connId, void *yourPtr) {}
    virtual void socketClosed(int connId, void *yourPtr) {}

    friend class BGPSession;
    //functions used by the BGPSession class
    int             getScheduleAt(simtime_t t, cMessage* msg)   { return scheduleAt(t, msg);}
    simtime_t       getSimTime()                                { return simTime();}
    void            getCancelAndDelete(cMessage* msg)           { return cancelAndDelete(msg);}
    cMessage*       getCancelEvent(cMessage* msg)               { return cancelEvent(msg);}
    cGate*          getGate(const char* gateName)               { return gate(gateName);}
    IRoutingTable*  getIPRoutingTable()                         { return _rt;}
    std::vector<BGP::RoutingTableEntry*> getBGPRoutingTable()   { return _BGPRoutingTable;}
    /**
     * \brief active listenSocket for a given session (used by BGPFSM)
     */
    void listenConnectionFromPeer(BGP::SessionID sessionID);
    /**
     * \brief active TCPConnection for a given session (used by BGPFSM)
     */
    void openTCPConnectionToPeer(BGP::SessionID sessionID);
    /**
     * \brief RFC 4271, 9.2 : Update-Send Process / Sent or not new UPDATE messages to its peers
      */
    void updateSendProcess(const unsigned char decisionProcessResult, BGP::SessionID sessionIndex, BGP::RoutingTableEntry* entry);
    /**
     * \brief find the next SessionID compared to his type and start this session if boolean is true
     */
    BGP::SessionID findNextSession(BGP::type type, bool startSession = false);
    /**
     * \brief check if the route is in OSPF external RoutingTable
     *
     * \return true if it is, false else
     */
    bool checkExternalRoute(const IPv4Route* ospfRoute);

private:
    void handleTimer(cMessage *timer);

    void processMessageFromTCP(cMessage *msg);
    void processMessage(const BGPOpenMessage& msg);
    void processMessage(const BGPKeepAliveMessage& msg);
    void processMessage(const BGPUpdateMessage& msg);

    bool deleteBGPRoutingEntry(BGP::RoutingTableEntry* entry);
    /**
     * \brief RFC 4271: 9.1. : Decision Process used when an UPDATE message is received
     *  As matches, routes are sent or not to UpdateSentProcess
     *  The result can be ROUTE_DESTINATION_CHANGED, NEW_ROUTE_ADDED or 0 if no routingTable modification
     */
    unsigned char decisionProcess(const BGPUpdateMessage& msg, BGP::RoutingTableEntry* entry, BGP::SessionID sessionIndex);
    /**
     * \brief RFC 4271: 9.1.2.2 Breaking Ties used when BGP speaker may have several routes
     *  to the same destination that have the same degree of preference.
     *
     * \return bool, true if this process changed the route, false else
     */
    bool tieBreakingProcess(BGP::RoutingTableEntry* oldEntry, BGP::RoutingTableEntry* entry);

    BGP::SessionID createSession(BGP::type typeSession, const char* peerAddr);
    bool isInASList(std::vector<BGP::ASID> ASList, BGP::RoutingTableEntry* entry);
    unsigned long   isInTable(std::vector<BGP::RoutingTableEntry*> rtTable, BGP::RoutingTableEntry* entry);

    std::vector<const char *> loadASConfig(cXMLElementList& ASConfig);
    void loadSessionConfig(cXMLElementList& sessionList, simtime_t* delayTab);
    void loadConfigFromXML(cXMLElement *bgpConfig);
    BGP::ASID findMyAS(cXMLElementList& ASList, int& outRouterPosition);
    bool ospfExist(IRoutingTable* rtTable);
    void loadTimerConfig(cXMLElementList& timerConfig, simtime_t* delayTab);
    unsigned char asLoopDetection(BGP::RoutingTableEntry* entry, BGP::ASID myAS);
    BGP::SessionID findIdFromPeerAddr(std::map<BGP::SessionID, BGPSession*> sessions, IPv4Address peerAddr);
    int isInRoutingTable(IRoutingTable* rtTable, IPv4Address addr);
    int isInInterfaceTable(IInterfaceTable* rtTable, IPv4Address addr);
    BGP::SessionID findIdFromSocketConnId(std::map<BGP::SessionID, BGPSession*> sessions, int connId);
    unsigned int calculateStartDelay(int rtListSize, unsigned char rtPosition, unsigned char rtPeerPosition);

    TCPSocketMap                            _socketMap;
    BGP::ASID                               _myAS;
    BGP::SessionID                          _currSessionId;

    IInterfaceTable*                        _inft;
    IRoutingTable*                          _rt;                // The IP routing table
    std::vector<BGP::RoutingTableEntry*>    _BGPRoutingTable;   // The BGP routing table
    std::vector<BGP::RoutingTableEntry*>    _prefixListIN;
    std::vector<BGP::RoutingTableEntry*>    _prefixListOUT;
    std::vector<BGP::ASID>                  _ASListIN;
    std::vector<BGP::ASID>                  _ASListOUT;
    std::map<BGP::SessionID, BGPSession*>   _BGPSessions;

    static const int  BGP_TCP_CONNECT_VALID = 71;
    static const int  BGP_TCP_CONNECT_CONFIRM = 72;
    static const int  BGP_TCP_CONNECT_FAILED = 73;
    static const int  BGP_TCP_CONNECT_OPEN_RCV = 74;
    static const int  BGP_TCP_KEEP_ALIVE_RCV = 75;
};

#endif

