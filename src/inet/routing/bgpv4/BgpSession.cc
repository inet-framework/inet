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

#include "inet/routing/bgpv4/BgpSession.h"
#include "inet/routing/bgpv4/Bgp.h"
#include "inet/routing/bgpv4/BgpFsm.h"

namespace inet {

namespace bgp {

BgpSession::BgpSession(BgpRouter& bgpRouter)
    : bgpRouter(bgpRouter), _ptrStartEvent(nullptr), _connectRetryCounter(0)
    , _connectRetryTime(BGP_RETRY_TIME), _ptrConnectRetryTimer(nullptr)
    , _holdTime(BGP_HOLD_TIME), _ptrHoldTimer(nullptr)
    , _keepAliveTime(BGP_KEEP_ALIVE), _ptrKeepAliveTimer(nullptr)
    , _openMsgSent(0), _openMsgRcv(0), _keepAliveMsgSent(0)
    , _keepAliveMsgRcv(0), _updateMsgSent(0), _updateMsgRcv(0)
{
    _box = new fsm::TopState::Box(*this);
    _fsm = new Macho::Machine<fsm::TopState>(_box);
    _info.sessionEstablished = false;
};

BgpSession::~BgpSession()
{
    bgpRouter.getCancelAndDelete(_ptrConnectRetryTimer);
    bgpRouter.getCancelAndDelete(_ptrStartEvent);
    bgpRouter.getCancelAndDelete(_ptrHoldTimer);
    bgpRouter.getCancelAndDelete(_ptrKeepAliveTimer);
    delete _info.socket;
    delete _info.socketListen;
    delete _fsm;
}

void BgpSession::setInfo(SessionInfo info)
{
    _info.sessionType = info.sessionType;
    _info.ASValue = info.ASValue;
    _info.routerID = info.routerID;
    _info.peerAddr = info.peerAddr;
    _info.sessionID = info.sessionID;
    _info.linkIntf = info.linkIntf;
    _info.socket = new TcpSocket();
}

void BgpSession::setTimers(simtime_t *delayTab)
{
    _connectRetryTime = delayTab[0];
    _holdTime = delayTab[1];
    _keepAliveTime = delayTab[2];
    if (_info.sessionType == IGP) {
        _StartEventTime = delayTab[3];
    }
    else if (delayTab[3] != SIMTIME_ZERO) {
        _StartEventTime = delayTab[3];
        _ptrStartEvent = new cMessage("BGP Start", START_EVENT_KIND);
        bgpRouter.getScheduleAt(simTime() + _StartEventTime, _ptrStartEvent);
        _ptrStartEvent->setContextPointer(this);
    }
    _ptrConnectRetryTimer = new cMessage("BGP Connect Retry", CONNECT_RETRY_KIND);
    _ptrHoldTimer = new cMessage("BGP Hold Timer", HOLD_TIME_KIND);
    _ptrKeepAliveTimer = new cMessage("BGP Keep Alive Timer", KEEP_ALIVE_KIND);

    _ptrConnectRetryTimer->setContextPointer(this);
    _ptrHoldTimer->setContextPointer(this);
    _ptrKeepAliveTimer->setContextPointer(this);
}

void BgpSession::startConnection()
{
    if (_ptrStartEvent == nullptr) {
        _ptrStartEvent = new cMessage("BGP Start", START_EVENT_KIND);
    }
    if (_info.sessionType == IGP) {
        if (simTime() > _StartEventTime) {
            _StartEventTime = simTime();
        }
        bgpRouter.getScheduleAt(_StartEventTime, _ptrStartEvent);
        _ptrStartEvent->setContextPointer(this);
    }
}

void BgpSession::restartsHoldTimer()
{
    if (_holdTime != 0) {
        bgpRouter.getCancelEvent(_ptrHoldTimer);
        bgpRouter.getScheduleAt(simTime() + _holdTime, _ptrHoldTimer);
    }
}

void BgpSession::restartsKeepAliveTimer()
{
    bgpRouter.getCancelEvent(_ptrKeepAliveTimer);
    bgpRouter.getScheduleAt(simTime() + _keepAliveTime, _ptrKeepAliveTimer);
}

void BgpSession::restartsConnectRetryTimer(bool start)
{
    bgpRouter.getCancelEvent(_ptrConnectRetryTimer);
    if (!start) {
        bgpRouter.getScheduleAt(simTime() + _connectRetryTime, _ptrConnectRetryTimer);
    }
}

void BgpSession::sendOpenMessage()
{
    Packet *pk = new Packet("BgpOpen");
    const auto& openMsg = makeShared<BgpOpenMessage>();
    openMsg->setMyAS(_info.ASValue);
    openMsg->setHoldTime(_holdTime);
    openMsg->setBGPIdentifier(_info.socket->getLocalAddress().toIpv4());
    pk->insertAtFront(openMsg);
    _info.socket->send(pk);
    _openMsgSent++;
}

void BgpSession::sendKeepAliveMessage()
{
    Packet *pk = new Packet("BgpKeepAlive");
    const auto &keepAliveMsg = makeShared<BgpKeepAliveMessage>();
    pk->insertAtFront(keepAliveMsg);
    _info.socket->send(pk);
    _keepAliveMsgSent++;
}

void BgpSession::getStatistics(unsigned int *statTab)
{
    statTab[0] += _openMsgSent;
    statTab[1] += _openMsgRcv;
    statTab[2] += _keepAliveMsgSent;
    statTab[3] += _keepAliveMsgRcv;
    statTab[4] += _updateMsgSent;
    statTab[5] += _updateMsgRcv;
}

} // namespace bgp

} // namespace inet

