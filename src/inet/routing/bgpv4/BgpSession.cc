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
    const auto& openMsg = makeShared<BgpOpenMessage>();
    openMsg->setMyAS(_info.ASValue);
    openMsg->setHoldTime(_holdTime);
    openMsg->setBGPIdentifier(_info.socket->getLocalAddress().toIpv4());

    EV_INFO << "Sending BGP Open packet to " << _info.peerAddr.str(false) <<
            " on interface " << _info.linkIntf->getInterfaceName() <<
            "[" << _info.linkIntf->getInterfaceId() << "] with contents:\n";
    EV_INFO << "  My AS: " << openMsg->getMyAS() << "\n";
    EV_INFO << "  Hold time: " << openMsg->getHoldTime() << "s \n";
    EV_INFO << "  BGP Id: " << openMsg->getBGPIdentifier() << "\n";
    if(openMsg->getOptionalParametersArraySize() == 0)
        EV_INFO << "  Optional parameters: empty \n";
    for(uint32_t i = 0; i < openMsg->getOptionalParametersArraySize(); i++) {
        const BgpOptionalParameters& optParams = openMsg->getOptionalParameters(i);
        EV_INFO << "  Optional parameter " << i+1 << ": \n";
        EV_INFO << "    Parameter type: " << optParams.parameterType << "\n";
        EV_INFO << "    Parameter length: " << optParams.parameterLength << "\n";
    }

    Packet *pk = new Packet("BgpOpen");
    pk->insertAtFront(openMsg);

    _info.socket->send(pk);
    _openMsgSent++;
}

void BgpSession::sendUpdateMessage(BgpUpdatePathAttributeList &content, BgpUpdateNlri &NLRI)
{
    const auto& updateMsg = makeShared<BgpUpdateMessage>();
    updateMsg->setPathAttributeListArraySize(1);
    updateMsg->setPathAttributeList(content);
    updateMsg->setNLRI(NLRI);

    EV_INFO << "Sending BGP Update packet to " << _info.peerAddr.str(false) <<
            " on interface " << _info.linkIntf->getInterfaceName() <<
            "[" << _info.linkIntf->getInterfaceId() << "] with contents:\n";

    if(updateMsg->getWithdrawnRoutesArraySize() == 0)
        EV_INFO << "  Withdrawn routes: empty \n";
    for(uint32_t i = 0; i < updateMsg->getWithdrawnRoutesArraySize(); i++) {
        const BgpUpdateWithdrawnRoutes& withdrwan = updateMsg->getWithdrawnRoutes(i);
        EV_INFO << "  Withdrawn route " << i+1 << ": \n";
        EV_INFO << "    length: " << (int)withdrwan.length << "\n";
        EV_INFO << "    prefix: " << withdrwan.prefix << "\n";
    }
    if(updateMsg->getPathAttributeListArraySize() == 0)
        EV_INFO << "  Path attribute: empty \n";
    for(uint32_t i = 0; i < updateMsg->getPathAttributeListArraySize(); i++) {
        const BgpUpdatePathAttributeList& pathAttrib = updateMsg->getPathAttributeList(i);
        EV_INFO << "  Path attribute " << i+1 << ": \n";
        EV_INFO << "    ORIGIN: ";
        inet::bgp::BgpSessionType sessionType = pathAttrib.getOrigin().getValue();
        if(sessionType == IGP)
            EV_INFO << "IGP \n";
        else if(sessionType == EGP)
            EV_INFO << "EGP \n";
        else if(sessionType == INCOMPLETE)
            EV_INFO << "INCOMPLETE \n";
        else
            EV_INFO << "Unknown \n";
        EV_INFO << "    AS_PATH: ";
        if(pathAttrib.getAsPathArraySize() == 0)
            EV_INFO << "empty";
        for(uint32_t j = 0; j < pathAttrib.getAsPathArraySize(); j++) {
            const BgpUpdatePathAttributesAsPath& asPath = pathAttrib.getAsPath(j);
            for(uint32_t k = 0; k < asPath.getValueArraySize(); k++) {
                const BgpAsPathSegment& asPathVal = asPath.getValue(k);
                for(uint32_t n = 0; n < asPathVal.getAsValueArraySize(); n++) {
                    EV_INFO << asPathVal.getAsValue(n) << " ";
                }
            }
        }
        EV_INFO << "\n";
        EV_INFO << "    NEXT_HOP: " << pathAttrib.getNextHop().getValue().str(false) << "\n";
        EV_INFO << "    LOCAL_PREF: ";
        if(pathAttrib.getLocalPrefArraySize() == 0)
            EV_INFO << "empty";
        for(uint32_t j = 0; j < pathAttrib.getLocalPrefArraySize(); j++) {
            const BgpUpdatePathAttributesLocalPref& localPref = pathAttrib.getLocalPref(j);
            EV_INFO << localPref.getValue() << " ";
        }
        EV_INFO << "\n";
        EV_INFO << "    ATOMIC_AGGREGATE: ";
        if(pathAttrib.getAtomicAggregateArraySize() == 0)
            EV_INFO << "empty";
        for(uint32_t j = 0; j < pathAttrib.getAtomicAggregateArraySize(); j++) {
            const BgpUpdatePathAttributesAtomicAggregate& attomicAgg = pathAttrib.getAtomicAggregate(j);
            EV_INFO << attomicAgg.getValue() << " ";
        }
        EV_INFO << "\n";
    }
    auto& NLRI_Base = updateMsg.get()->getNLRI();
    EV_INFO << "  Network Layer Reachability Information (NLRI): \n";
    EV_INFO << "    NLRI length: " << (int)NLRI_Base.length << "\n";
    EV_INFO << "    NLRI prefix: " << NLRI_Base.prefix << "\n";

    Packet *pk = new Packet("BgpUpdate");
    pk->insertAtFront(updateMsg);

    _info.socket->send(pk);
    _updateMsgSent++;
}

void BgpSession::sendNotificationMessage()
{
    // TODO

    // const auto& updateMsg = makeShared<BgpNotificationMessage>();

//    EV_INFO << "Sending BGP Notification packet to " << _info.peerAddr.str(false) <<
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

    EV_INFO << "Sending BGP Keep-alive packet to " << _info.peerAddr.str(false) <<
            " on interface " << _info.linkIntf->getInterfaceName() <<
            "[" << _info.linkIntf->getInterfaceId() << "] \n";

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

} // namespace bgp

} // namespace inet

