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

#include "inet/common/INETDefs.h"

#include "inet/routing/bgpv4/BgpCommon.h"
#include "inet/transportlayer/contract/tcp/TcpSocket.h"
#include "inet/routing/bgpv4/Bgp.h"
#include "inet/routing/bgpv4/BgpFsm.h"

namespace inet {

namespace bgp {

class INET_API BgpSession : public cObject
{
private:
  SessionInfo _info;
  BgpRouter& bgpRouter;

  //Timers
  simtime_t _StartEventTime;
  cMessage *_ptrStartEvent = nullptr;
  unsigned int _connectRetryCounter = 0;
  simtime_t _connectRetryTime = BGP_RETRY_TIME;
  cMessage *_ptrConnectRetryTimer = nullptr;
  simtime_t _holdTime = BGP_HOLD_TIME;
  cMessage *_ptrHoldTimer = nullptr;
  simtime_t _keepAliveTime = BGP_KEEP_ALIVE;
  cMessage *_ptrKeepAliveTimer = nullptr;

  //Statistics
  unsigned int _openMsgSent = 0;
  unsigned int _openMsgRcv = 0;
  unsigned int _updateMsgSent = 0;
  unsigned int _updateMsgRcv = 0;
  unsigned int _notificationMsgSent = 0;
  unsigned int _notificationMsgRcv = 0;
  unsigned int _keepAliveMsgSent = 0;
  unsigned int _keepAliveMsgRcv = 0;

  //FINAL STATE MACHINE
  fsm::TopState::Box *_box;
  Macho::Machine<fsm::TopState> *_fsm;

  friend struct fsm::Idle;
  friend struct fsm::Connect;
  friend struct fsm::Active;
  friend struct fsm::OpenSent;
  friend struct fsm::OpenConfirm;
  friend struct fsm::Established;

  public:
    BgpSession(BgpRouter& bgpRouter);
    virtual ~BgpSession();

    void startConnection();
    void restartsHoldTimer();
    void restartsKeepAliveTimer();
    void restartsConnectRetryTimer(bool start = true);

    void sendOpenMessage();
    void sendUpdateMessage(BgpUpdatePathAttributeList &content, BgpUpdateNlri &NLRI);
    void sendNotificationMessage();
    void sendKeepAliveMessage();

    void listenConnectionFromPeer() { bgpRouter.listenConnectionFromPeer(_info.sessionID); }
    void openTCPConnectionToPeer() { bgpRouter.openTCPConnectionToPeer(_info.sessionID); }
    SessionId findAndStartNextSession(BgpSessionType type) { return bgpRouter.findNextSession(type, true); }

    //setters for creating and editing the information in the Bgp session:
    void setInfo(SessionInfo info);
    void setTimers(simtime_t *delayTab);
    void setlinkIntf(InterfaceEntry *intf) { _info.linkIntf = intf; }
    void setSocket(TcpSocket *socket) { delete _info.socket; _info.socket = socket; }
    void setSocketListen(TcpSocket *socket) { delete _info.socketListen; _info.socketListen = socket; }

    //getters for accessing session information:
    void getStatistics(unsigned int *statTab);
    bool isEstablished() { return _info.sessionEstablished; }
    SessionId getSessionID() { return _info.sessionID; }
    BgpSessionType getType() { return _info.sessionType; }
    InterfaceEntry *getLinkIntf() { return _info.linkIntf; }
    Ipv4Address getPeerAddr() { return _info.peerAddr; }
    TcpSocket *getSocket() { return _info.socket; }
    TcpSocket *getSocketListen() { return _info.socketListen; }
    IIpv4RoutingTable *getIPRoutingTable() { return bgpRouter.getIPRoutingTable(); }
    std::vector<BgpRoutingTableEntry *> getBGPRoutingTable() { return bgpRouter.getBGPRoutingTable(); }
    Macho::Machine<fsm::TopState>& getFSM() { return *_fsm; }
    bool checkExternalRoute(const Ipv4Route *ospfRoute) { return bgpRouter.checkExternalRoute(ospfRoute); }
    void updateSendProcess(BgpRoutingTableEntry *entry) { return bgpRouter.updateSendProcess(NEW_SESSION_ESTABLISHED, _info.sessionID, entry); }
};

} // namespace bgp

} // namespace inet

#endif // ifndef __INET_BGPSESSION_H

