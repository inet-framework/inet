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

#ifndef __INET_BGPSESSION_H
#define __INET_BGPSESSION_H

#include <vector>

#include "INETDefs.h"

#include "BGPCommon.h"
#include "TCPSocket.h"
#include "BGPRouting.h"
#include "BGPFSM.h"

class  INET_API BGPSession : public cObject
{
public:
    BGPSession(BGPRouting& _bgpRouting);

    virtual ~BGPSession();

    void            startConnection();
    void            restartsHoldTimer();
    void            restartsKeepAliveTimer();
    void            restartsConnectRetryTimer(bool start = true);
    void            sendOpenMessage();
    void            sendKeepAliveMessage();
    void            addUpdateMsgSent()                          { _updateMsgSent ++;}
    void            listenConnectionFromPeer()                  { _bgpRouting.listenConnectionFromPeer(_info.sessionID);}
    void            openTCPConnectionToPeer()                   { _bgpRouting.openTCPConnectionToPeer(_info.sessionID);}
    BGP::SessionID  findAndStartNextSession(BGP::type type)     { return _bgpRouting.findNextSession(type, true);}

    //setters for creating and editing the information in the BGPRouting session:
    void            setInfo(BGP::SessionInfo info);
    void            setTimers(simtime_t* delayTab);
    void            setlinkIntf(InterfaceEntry* intf)           { _info.linkIntf = intf;}
    void            setSocket(TCPSocket* socket)                { _info.socket = socket;}
    void            setSocketListen(TCPSocket* socket)          { _info.socketListen = socket;}

    //getters for accessing session information:
    void            getStatistics(unsigned int* statTab);
    bool            isEstablished()                             { return _info.sessionEstablished;}
    BGP::SessionID  getSessionID()                              { return _info.sessionID;}
    BGP::type       getType()                                   { return _info.sessionType;}
    InterfaceEntry* getLinkIntf()                               { return _info.linkIntf;}
    IPv4Address       getPeerAddr()                               { return _info.peerAddr;}
    TCPSocket*      getSocket()                                 { return _info.socket;}
    TCPSocket*      getSocketListen()                           { return _info.socketListen;}
    IRoutingTable*  getIPRoutingTable()                         { return _bgpRouting.getIPRoutingTable();}
    std::vector<BGP::RoutingTableEntry*> getBGPRoutingTable()   { return _bgpRouting.getBGPRoutingTable();}
    Macho::Machine<BGPFSM::TopState>&    getFSM()               { return *_fsm;}
    bool checkExternalRoute(const IPv4Route* ospfRoute)           { return _bgpRouting.checkExternalRoute(ospfRoute);}
    void updateSendProcess(BGP::RoutingTableEntry* entry)       { return _bgpRouting.updateSendProcess(BGP::NEW_SESSION_ESTABLISHED, _info.sessionID, entry);}

private:
    BGP::SessionInfo    _info;
    BGPRouting&         _bgpRouting;

    static const int    BGP_RETRY_TIME = 120;
    static const int    BGP_HOLD_TIME = 180;
    static const int    BGP_KEEP_ALIVE = 60; // 1/3 of BGP_HOLD_TIME
    static const int    NB_SEC_START_EGP_SESSION = 1;

    //Timers
    simtime_t       _StartEventTime;
    cMessage *      _ptrStartEvent;
    unsigned int    _connectRetryCounter;
    simtime_t       _connectRetryTime;
    cMessage *      _ptrConnectRetryTimer;
    simtime_t       _holdTime;
    cMessage *      _ptrHoldTimer;
    simtime_t       _keepAliveTime;
    cMessage *      _ptrKeepAliveTimer;

    //Statistics
    unsigned int    _openMsgSent;
    unsigned int    _openMsgRcv;
    unsigned int    _keepAliveMsgSent;
    unsigned int    _keepAliveMsgRcv;
    unsigned int    _updateMsgSent;
    unsigned int    _updateMsgRcv;


    //FINAL STATE MACHINE
    BGPFSM::TopState::Box*              _box;
    Macho::Machine<BGPFSM::TopState>*   _fsm;

    friend struct BGPFSM::Idle;
    friend struct BGPFSM::Connect;
    friend struct BGPFSM::Active;
    friend struct BGPFSM::OpenSent;
    friend struct BGPFSM::OpenConfirm;
    friend struct BGPFSM::Established;
};

#endif

