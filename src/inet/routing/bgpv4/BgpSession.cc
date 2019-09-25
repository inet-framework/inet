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

#include "inet/routing/bgpv4/Bgp.h"
#include "inet/routing/bgpv4/BgpFsm.h"
#include "inet/routing/bgpv4/BgpSession.h"
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
        if(bgpRouter.getNumEgpSessions() == 0) {
            _ptrStartEvent = new cMessage("BGP Start", START_EVENT_KIND);
            bgpRouter.getScheduleAt(simTime() + _StartEventTime, _ptrStartEvent);
            _ptrStartEvent->setContextPointer(this);
        }
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
    if (_ptrStartEvent == nullptr)
        _ptrStartEvent = new cMessage("BGP Start", START_EVENT_KIND);

    if (_info.sessionType == IGP) {
        if (simTime() > _StartEventTime)
            _StartEventTime = simTime();
        if(!_ptrStartEvent->isScheduled())
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
    const auto& openMsg = makeShared<BgpOpenMessage>();
    openMsg->setMyAS(_info.ASValue);
    openMsg->setHoldTime(_holdTime);
    openMsg->setBGPIdentifier(_info.socket->getLocalAddress().toIpv4());

    EV_INFO << "Sending BGP Open message to " << _info.peerAddr.str(false) <<
            " on interface " << _info.linkIntf->getInterfaceName() <<
            "[" << _info.linkIntf->getInterfaceId() << "] with contents:\n";
    bgpRouter.printOpenMessage(*openMsg);

    Packet *pk = new Packet("BgpOpen");
    pk->insertAtFront(openMsg);

    _info.socket->send(pk);
    _openMsgSent++;
}

void BgpSession::sendUpdateMessage(std::vector<BgpUpdatePathAttributes*> &content, BgpUpdateNlri &NLRI)
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

    updateMsg->setNLRIArraySize(1);
    updateMsg->setNLRI(0, NLRI);
    updateMsg->addChunkLength(B(1 + (NLRI.length +7)/8));
    updateMsg->setTotalLength(B(updateMsg->getChunkLength()).get());

    EV_INFO << "Sending BGP Update message to " << _info.peerAddr.str(false) <<
            " on interface " << _info.linkIntf->getInterfaceName() <<
            "[" << _info.linkIntf->getInterfaceId() << "] with contents:\n";
    bgpRouter.printUpdateMessage(*updateMsg);

    Packet *pk = new Packet("BgpUpdate");
    pk->insertAtFront(updateMsg);

    _info.socket->send(pk);
    _updateMsgSent++;
}

void BgpSession::sendNotificationMessage()
{
    // TODO

    // const auto& updateMsg = makeShared<BgpNotificationMessage>();

//    EV_INFO << "Sending BGP Notification message to " << _info.peerAddr.str(false) <<
//            " on interface " << _info.linkIntf->getInterfaceName() <<
//            "[" << _info.linkIntf->getInterfaceId() << "] with contents:\n";

    //Packet *pk = new Packet("BgpNotification");
    //pk->insertAtFront(updateMsg);

    //_info.socket->send(pk);
    //_notificationMsgSent++;
}

void BgpSession::sendKeepAliveMessage()
{
    const auto &keepAliveMsg = makeShared<BgpKeepAliveMessage>();

    EV_INFO << "Sending BGP Keep-alive message to " << _info.peerAddr.str(false) <<
            " on interface " << _info.linkIntf->getInterfaceName() <<
            "[" << _info.linkIntf->getInterfaceId() << "] \n";
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
    if(sessionType == IGP)
        return "IGP";
    else if(sessionType == EGP)
        return "EGP";
    else if(sessionType == INCOMPLETE)
        return "INCOMPLETE";

    return "Unknown";
}

std::ostream& operator<<(std::ostream& out, const BgpSession& entry)
{
    out << "sessionId: " << entry.getSessionID() << " "
            << "sessionType: " << entry.getTypeString(entry.getType()) << " "
            << "established: " << (entry.isEstablished() == true ? "true" : "false") << " "
            << "state: " << entry.getFSM().currentState().name() << " "
            << "peer: " << entry.getPeerAddr().str(false) << " "
            << "nextHopSelf: " << (entry.getNextHopSelf() == true ? "true" : "false") << " "
            << "startEventTime: " << entry.getStartEventTime() << " "
            << "connectionRetryTime: " << entry.getConnectionRetryTime() << " "
            << "holdTime: " << entry.getHoldTime() << " "
            << "keepAliveTime: " << entry.getKeepAliveTime();

    return out;
}

} // namespace bgp
} // namespace inet

