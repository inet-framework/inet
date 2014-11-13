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

#include "inet/routing/bgpv4/BGPSession.h"
#include "inet/routing/bgpv4/BGPRouting.h"
#include "inet/routing/bgpv4/BGPFSM.h"

namespace inet {

namespace bgp {

BGPSession::BGPSession(BGPRouting& _bgpRouting)
    : _bgpRouting(_bgpRouting), _ptrStartEvent(0), _connectRetryCounter(0)
    , _connectRetryTime(BGP_RETRY_TIME), _ptrConnectRetryTimer(0)
    , _holdTime(BGP_HOLD_TIME), _ptrHoldTimer(0)
    , _keepAliveTime(BGP_KEEP_ALIVE), _ptrKeepAliveTimer(0)
    , _openMsgSent(0), _openMsgRcv(0), _keepAliveMsgSent(0)
    , _keepAliveMsgRcv(0), _updateMsgSent(0), _updateMsgRcv(0)
{
    _box = new fsm::TopState::Box(*this);
    _fsm = new Macho::Machine<fsm::TopState>(_box);
    _info.sessionEstablished = false;
};

BGPSession::~BGPSession()
{
    _bgpRouting.getCancelAndDelete(_ptrConnectRetryTimer);
    _bgpRouting.getCancelAndDelete(_ptrStartEvent);
    _bgpRouting.getCancelAndDelete(_ptrHoldTimer);
    _bgpRouting.getCancelAndDelete(_ptrKeepAliveTimer);
    delete _info.socket;
    delete _info.socketListen;
    delete _fsm;
}

void BGPSession::setInfo(SessionInfo info)
{
    _info.sessionType = info.sessionType;
    _info.ASValue = info.ASValue;
    _info.routerID = info.routerID;
    _info.peerAddr = info.peerAddr;
    _info.sessionID = info.sessionID;
    _info.linkIntf = info.linkIntf;
    _info.socket = new TCPSocket();
}

void BGPSession::setTimers(simtime_t *delayTab)
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
        _bgpRouting.getScheduleAt(_bgpRouting.getSimTime() + _StartEventTime, _ptrStartEvent);
        _ptrStartEvent->setContextPointer(this);
    }
    _ptrConnectRetryTimer = new cMessage("BGP Connect Retry", CONNECT_RETRY_KIND);
    _ptrHoldTimer = new cMessage("BGP Hold Timer", HOLD_TIME_KIND);
    _ptrKeepAliveTimer = new cMessage("BGP Keep Alive Timer", KEEP_ALIVE_KIND);

    _ptrConnectRetryTimer->setContextPointer(this);
    _ptrHoldTimer->setContextPointer(this);
    _ptrKeepAliveTimer->setContextPointer(this);
}

void BGPSession::startConnection()
{
    if (_ptrStartEvent == 0) {
        _ptrStartEvent = new cMessage("BGP Start", START_EVENT_KIND);
    }
    if (_info.sessionType == IGP) {
        if (_bgpRouting.getSimTime() > _StartEventTime) {
            _StartEventTime = _bgpRouting.getSimTime();
        }
        _bgpRouting.getScheduleAt(_StartEventTime, _ptrStartEvent);
        _ptrStartEvent->setContextPointer(this);
    }
}

void BGPSession::restartsHoldTimer()
{
    if (_holdTime != 0) {
        _bgpRouting.getCancelEvent(_ptrHoldTimer);
        _bgpRouting.getScheduleAt(_bgpRouting.getSimTime() + _holdTime, _ptrHoldTimer);
    }
}

void BGPSession::restartsKeepAliveTimer()
{
    _bgpRouting.getCancelEvent(_ptrKeepAliveTimer);
    _bgpRouting.getScheduleAt(_bgpRouting.getSimTime() + _keepAliveTime, _ptrKeepAliveTimer);
}

void BGPSession::restartsConnectRetryTimer(bool start)
{
    _bgpRouting.getCancelEvent(_ptrConnectRetryTimer);
    if (!start) {
        _bgpRouting.getScheduleAt(_bgpRouting.getSimTime() + _connectRetryTime, _ptrConnectRetryTimer);
    }
}

void BGPSession::sendOpenMessage()
{
    BGPOpenMessage *openMsg = new BGPOpenMessage("BGPOpen");
    openMsg->setMyAS(_info.ASValue);
    openMsg->setHoldTime(_holdTime);
    openMsg->setBGPIdentifier(_info.socket->getLocalAddress().toIPv4());
    _info.socket->send(openMsg);
    _openMsgSent++;
}

void BGPSession::sendKeepAliveMessage()
{
    BGPKeepAliveMessage *keepAliveMsg = new BGPKeepAliveMessage("BGPKeepAlive");
    _info.socket->send(keepAliveMsg);
    _keepAliveMsgSent++;
}

void BGPSession::getStatistics(unsigned int *statTab)
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

