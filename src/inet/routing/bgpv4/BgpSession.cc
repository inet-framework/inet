//
// Copyright (C) 2010 Helene Lageber
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/routing/bgpv4/BgpSession.h"

#include "inet/routing/bgpv4/Bgp.h"
#include "inet/routing/bgpv4/BgpFsm.h"
#include "inet/routing/bgpv4/bgpmessage/BgpUpdate.h"

namespace inet {
namespace bgp {

BgpSession::BgpSession(BgpRouter& bgpRouter) : bgpRouter(bgpRouter)
{
    _box = new fsm::TopState::Box(*this);
    _fsm = new Macho::Machine<fsm::TopState>(_box);
    _info.sessionEstablished = false;
};

BgpSession::~BgpSession()
{
    bgpRouter.cancelAndDelete(_ptrConnectRetryTimer);
    bgpRouter.cancelAndDelete(_ptrStartEvent);
    bgpRouter.cancelAndDelete(_ptrHoldTimer);
    bgpRouter.cancelAndDelete(_ptrKeepAliveTimer);
    delete _info.socket;
    delete _fsm;
}

void BgpSession::setInfo(SessionInfo info)
{
    _info.sessionType = info.sessionType;
    _info.ASValue = info.ASValue;
    _info.routerId = info.routerId;
    _info.peerAddr = info.peerAddr;
    _info.sessionId = info.sessionId;
    _info.linkIntf = info.linkIntf;
    _info.ebgpMultihop = info.ebgpMultihop;
    _info.socket = new TcpSocket();
}

void BgpSession::setTimers(simtime_t *delayTab)
{
    _connectRetryTime = delayTab[0];
    _holdTime = delayTab[1];
    _keepAliveTime = delayTab[2];
    if (_info.sessionType == IGP) {
        _StartEventTime = delayTab[3];
        // if this BGP router does not establish any EGP connection, then start this IGP session
        if (bgpRouter.getNumEgpSessions() == 0) {
            _ptrStartEvent = new cMessage("BGP Start", START_EVENT_KIND);
            bgpRouter.scheduleAt(simTime() + _StartEventTime, _ptrStartEvent);
            _ptrStartEvent->setContextPointer(this);
        }
    }
    else if (delayTab[3] != SIMTIME_ZERO) {
        _StartEventTime = delayTab[3];
        _ptrStartEvent = new cMessage("BGP Start", START_EVENT_KIND);
        bgpRouter.scheduleAt(simTime() + _StartEventTime, _ptrStartEvent);
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
    if (_ptrStartEvent == nullptr)
        _ptrStartEvent = new cMessage("BGP Start", START_EVENT_KIND);

    if (simTime() > _StartEventTime)
        _StartEventTime = simTime();
    if (!_ptrStartEvent->isScheduled())
        bgpRouter.scheduleAt(_StartEventTime, _ptrStartEvent);
    _ptrStartEvent->setContextPointer(this);
}

void BgpSession::scheduleReconnect()
{
    // Re-establish a dropped session after the ConnectRetryTimer interval (RFC 4271), NOT at
    // the same instant as the loss. An instant reconnect makes two peers that both lost the
    // session ping-pong: each re-opens, collides with the other, drops, and re-opens again at
    // (nearly) the same simtime — a livelock that never reconverges. The delay spaces the
    // retries so the session can actually come back up.
    if (_ptrStartEvent == nullptr)
        _ptrStartEvent = new cMessage("BGP Start", START_EVENT_KIND);
    if (!_ptrStartEvent->isScheduled())
        bgpRouter.scheduleAt(simTime() + _connectRetryTime, _ptrStartEvent);
    _ptrStartEvent->setContextPointer(this);
}

void BgpSession::cancelReconnect()
{
    // Once (re-)established, drop any pending reconnect so a stale Start event cannot later
    // disrupt the live session.
    if (_ptrStartEvent != nullptr)
        bgpRouter.cancelEvent(_ptrStartEvent);
}

void BgpSession::restartHoldTimer()
{
    if (_holdTime != 0) {
        bgpRouter.cancelEvent(_ptrHoldTimer);
        bgpRouter.scheduleAt(simTime() + _holdTime, _ptrHoldTimer);
    }
}

void BgpSession::restartKeepAliveTimer()
{
    bgpRouter.cancelEvent(_ptrKeepAliveTimer);
    bgpRouter.scheduleAt(simTime() + _keepAliveTime, _ptrKeepAliveTimer);
}

void BgpSession::restartConnectRetryTimer()
{
    bgpRouter.cancelEvent(_ptrConnectRetryTimer);
    bgpRouter.scheduleAt(simTime() + _connectRetryTime, _ptrConnectRetryTimer);
}

void BgpSession::stopConnectRetryTimer()
{
    bgpRouter.cancelEvent(_ptrConnectRetryTimer);
}

void BgpSession::sendOpenMessage()
{
    const auto& openMsg = makeShared<BgpOpenMessage>();
    openMsg->setMyAS(_info.ASValue);
    openMsg->setHoldTime(_holdTime);
    // BGP Identifier is a 4-octet router id; for IPv6 it cannot be the (IPv6) local address
    openMsg->setBgpIdentifier(bgpRouter.isIpv6() ? bgpRouter.getRouterId() : _info.socket->getLocalAddress().toIpv4());

    EV_INFO << "Sending BGP Open message to " << _info.peerAddr.str()
            << " on interface " << _info.linkIntf->getInterfaceName()
            << "[" << _info.linkIntf->getInterfaceId() << "] with contents:\n";
    bgpRouter.printOpenMessage(*openMsg);

    Packet *pk = new Packet("BgpOpen");
    pk->insertAtFront(openMsg);

    _info.socket->send(pk);
    _openMsgSent++;
}

void BgpSession::sendUpdateMessage(std::vector<BgpUpdatePathAttributes *>& content, BgpUpdateNlri& nlri)
{
    const auto& updateMsg = makeShared<BgpUpdateMessage>();

    updateMsg->setWithDrawnRoutesLength(0);

    size_t attrLength = 0;
    updateMsg->setPathAttributesArraySize(content.size());
    for (size_t i = 0; i < content.size(); i++) {
        attrLength += computePathAttributeBytes(*content[i]);
        updateMsg->setPathAttributes(i, content[i]);
    }
    updateMsg->setTotalPathAttributeLength(attrLength);
    updateMsg->addChunkLength(B(attrLength));

    updateMsg->setNlriArraySize(1);
    updateMsg->setNlri(0, nlri);
    updateMsg->addChunkLength(B(1 + (nlri.length + 7) / 8));
    updateMsg->setTotalLength(updateMsg->getChunkLength().get<B>());

    EV_INFO << "Sending BGP Update message to " << _info.peerAddr.str()
            << " on interface " << _info.linkIntf->getInterfaceName()
            << "[" << _info.linkIntf->getInterfaceId() << "] with contents:\n";
    bgpRouter.printUpdateMessage(*updateMsg);

    Packet *pk = new Packet("BgpUpdate");
    pk->insertAtFront(updateMsg);

    _info.socket->send(pk);
    _updateMsgSent++;
}

void BgpSession::sendUpdateMessage(std::vector<BgpUpdatePathAttributes *>& content)
{
    // MP-BGP UPDATE (RFC 4760): reachability rides in an MP_REACH_NLRI path attribute, so the
    // legacy NLRI field is left empty.
    const auto& updateMsg = makeShared<BgpUpdateMessage>();

    updateMsg->setWithDrawnRoutesLength(0);

    size_t attrLength = 0;
    updateMsg->setPathAttributesArraySize(content.size());
    for (size_t i = 0; i < content.size(); i++) {
        attrLength += computePathAttributeBytes(*content[i]);
        updateMsg->setPathAttributes(i, content[i]);
    }
    updateMsg->setTotalPathAttributeLength(attrLength);
    updateMsg->addChunkLength(B(attrLength));
    updateMsg->setTotalLength(updateMsg->getChunkLength().get<B>());

    EV_INFO << "Sending BGP Update message to " << _info.peerAddr.str()
            << " on interface " << _info.linkIntf->getInterfaceName()
            << "[" << _info.linkIntf->getInterfaceId() << "] with contents:\n";
    bgpRouter.printUpdateMessage(*updateMsg);

    Packet *pk = new Packet("BgpUpdate");
    pk->insertAtFront(updateMsg);

    _info.socket->send(pk);
    _updateMsgSent++;
}

void BgpSession::sendNotificationMessage()
{
    // TODO

//    const auto& updateMsg = makeShared<BgpNotificationMessage>();

//    EV_INFO << "Sending BGP Notification message to " << _info.peerAddr.str()
//            << " on interface " << _info.linkIntf->getInterfaceName()
//            << "[" << _info.linkIntf->getInterfaceId() << "] with contents:\n";

//    Packet *pk = new Packet("BgpNotification");
//    pk->insertAtFront(updateMsg);

//    _info.socket->send(pk);
//    _notificationMsgSent++;
}

void BgpSession::sendKeepAliveMessage()
{
    const auto& keepAliveMsg = makeShared<BgpKeepAliveMessage>();

    EV_INFO << "Sending BGP Keep-alive message to " << _info.peerAddr.str()
            << " on interface " << _info.linkIntf->getInterfaceName()
            << "[" << _info.linkIntf->getInterfaceId() << "] \n";
    bgpRouter.printKeepAliveMessage(*keepAliveMsg);

    Packet *pk = new Packet("BgpKeepAlive");
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

const std::string BgpSession::getTypeString(BgpSessionType sessionType)
{
    if (sessionType == IGP)
        return "IGP";
    else if (sessionType == EGP)
        return "EGP";
    else if (sessionType == INCOMPLETE)
        return "INCOMPLETE";

    return "Unknown";
}

std::ostream& operator<<(std::ostream& out, const BgpSession& entry)
{
    out << "sessionId: " << entry.getSessionId() << " "
        << "sessionType: " << entry.getTypeString(entry.getType()) << " "
        << "established: " << (entry.isEstablished() == true ? "true" : "false") << " "
        << "state: " << entry.getFsm().currentState().name() << " "
        << "peer: " << entry.getPeerAddr().str() << " "
        << "nextHopSelf: " << (entry.getNextHopSelf() == true ? "true" : "false") << " "
        << "startEventTime: " << entry.getStartEventTime() << " "
        << "connectionRetryTime: " << entry.getConnectionRetryTime() << " "
        << "holdTime: " << entry.getHoldTime() << " "
        << "keepAliveTime: " << entry.getKeepAliveTime();

    return out;
}

} // namespace bgp
} // namespace inet
