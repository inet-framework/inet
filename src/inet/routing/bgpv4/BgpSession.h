//
// Copyright (C) 2010 Helene Lageber
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_BGPSESSION_H
#define __INET_BGPSESSION_H

#include <vector>

#include "inet/routing/bgpv4/Bgp.h"
#include "inet/routing/bgpv4/BgpCommon.h"
#include "inet/routing/bgpv4/BgpFsm.h"
#include "inet/transportlayer/contract/tcp/TcpSocket.h"

namespace inet {
namespace bgp {

class INET_API BgpSession : public cObject
{
public:
    enum {
        BGP_RETRY_TIME = 120,
        BGP_HOLD_TIME = 180,
        BGP_KEEP_ALIVE = 60 // 1/3 of BGP_HOLD_TIME
    };
  private:
    SessionInfo _info;
    BgpRouter& bgpRouter;

    // Timers
    simtime_t _StartEventTime;
    cMessage *_ptrStartEvent = nullptr;
    unsigned int _connectRetryCounter = 0;
    simtime_t _connectRetryTime = BGP_RETRY_TIME;
    cMessage *_ptrConnectRetryTimer = nullptr;
    simtime_t _holdTime = BGP_HOLD_TIME;
    cMessage *_ptrHoldTimer = nullptr;
    simtime_t _keepAliveTime = BGP_KEEP_ALIVE;
    cMessage *_ptrKeepAliveTimer = nullptr;

    // Statistics
    unsigned int _openMsgSent = 0;
    unsigned int _openMsgRcv = 0;
    unsigned int _updateMsgSent = 0;
    unsigned int _updateMsgRcv = 0;
    unsigned int _notificationMsgSent = 0;
    unsigned int _notificationMsgRcv = 0;
    unsigned int _keepAliveMsgSent = 0;
    unsigned int _keepAliveMsgRcv = 0;

    // FINAL STATE MACHINE
    fsm::TopState::Box *_box = nullptr;
    Macho::Machine<fsm::TopState> *_fsm = nullptr;

    friend struct fsm::Idle;
    friend struct fsm::Connect;
    friend struct fsm::Active;
    friend struct fsm::OpenSent;
    friend struct fsm::OpenConfirm;
    friend struct fsm::Established;

  public:
    enum { NB_STATS = 6 };
    BgpSession(BgpRouter& bgpRouter);
    virtual ~BgpSession();

    void startConnection();
    void restartsHoldTimer();
    void restartsKeepAliveTimer();
    void restartsConnectRetryTimer(bool start = true);

    void sendOpenMessage();
    void sendUpdateMessage(std::vector<BgpUpdatePathAttributes *>& content, BgpUpdateNlri& NLRI);
    void sendNotificationMessage();
    void sendKeepAliveMessage();

    void listenConnectionFromPeer() { bgpRouter.listenConnectionFromPeer(_info.sessionID); }
    void openTCPConnectionToPeer() { bgpRouter.openTCPConnectionToPeer(_info.sessionID); }
    SessionId findAndStartNextSession(BgpSessionType type) { return bgpRouter.findNextSession(type, true); }

    // setters for creating and editing the information in the Bgp session:
    void setInfo(SessionInfo info);
    void setTimers(simtime_t *delayTab);
    void setlinkIntf(NetworkInterface *intf) { _info.linkIntf = intf; }
    void setNextHopSelf(bool nextHopSelf) { _info.nextHopSelf = nextHopSelf; }
    void setLocalPreference(int localPreference) { _info.localPreference = localPreference; }
    void setSocket(TcpSocket *socket) { delete _info.socket; _info.socket = socket; }
    void setSocketListen(TcpSocket *socket) { delete _info.socketListen; _info.socketListen = socket; }

    // getters for accessing session information:
    simtime_t getStartEventTime() const { return _StartEventTime; }
    simtime_t getConnectionRetryTime() const { return _connectRetryTime; }
    simtime_t getHoldTime() const { return _holdTime; }
    simtime_t getKeepAliveTime() const { return _keepAliveTime; }
    void getStatistics(unsigned int *statTab);
    bool isEstablished() const { return _info.sessionEstablished; }
    SessionId getSessionID() const { return _info.sessionID; }
    BgpSessionType getType() const { return _info.sessionType; }
    static const std::string getTypeString(BgpSessionType sessionType);
    NetworkInterface *getLinkIntf() const { return _info.linkIntf; }
    bool getCheckConnection() const { return _info.checkConnection; }
    Ipv4Address getPeerAddr() const { return _info.peerAddr; }
    bool getNextHopSelf() const { return _info.nextHopSelf; }
    int getLocalPreference() const { return _info.localPreference; }
    TcpSocket *getSocket() const { return _info.socket; }
    TcpSocket *getSocketListen() const { return _info.socketListen; }
    int getEbgpMultihop() const { return _info.ebgpMultihop; }
    IIpv4RoutingTable *getIPRoutingTable() const { return bgpRouter.getIPRoutingTable(); }
    std::vector<BgpRoutingTableEntry *> getBGPRoutingTable() const { return bgpRouter.getBGPRoutingTable(); }
    Macho::Machine<fsm::TopState>& getFSM() const { return *_fsm; }
    void updateSendProcess(BgpRoutingTableEntry *entry) const { return bgpRouter.updateSendProcess(NEW_SESSION_ESTABLISHED, _info.sessionID, entry); }
    bool isRouteExcluded(const Ipv4Route& rtEntry) const { return bgpRouter.isRouteExcluded(rtEntry); }
};

std::ostream& operator<<(std::ostream& out, const BgpSession& entry);

} // namespace bgp
} // namespace inet

#endif

