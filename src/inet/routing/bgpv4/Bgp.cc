//
// Copyright (C) 2010 Helene Lageber,
//    2019 Adrian Novak - multi address-family support
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
#include "inet/common/ModuleAccess.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/routing/bgpv4/BgpSession.h"

namespace inet {

namespace bgp {

Define_Module(Bgp);

Bgp::~Bgp(void)
{
    for (auto & elem : _BGPSessions)
    {
        delete (elem).second;
    }

    for (auto & elem : _prefixListIN) {
        delete (elem);
    }

    for (auto & elem : _prefixListOUT) {
        delete (elem);
    }

    for (auto & elem : _prefixList) {
        delete (elem);
    }

    for (auto & elem : _prefixListIN6) {
        delete (elem);
    }

    for (auto & elem : _prefixListOUT6) {
        delete (elem);
    }

    for (auto & elem : _prefixList6) {
        delete (elem);
    }

}

void Bgp::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_ROUTING_PROTOCOLS) {
        bool isOperational;
        NodeStatus *nodeStatus = dynamic_cast<NodeStatus *>(findContainingNode(this)->getSubmodule("status"));
        isOperational = (!nodeStatus) || nodeStatus->getState() == NodeStatus::UP;
        if (!isOperational)
            throw cRuntimeError("This module doesn't support starting in node DOWN state");

        // we must wait until Ipv4RoutingTable is completely initialized
        _rt = getModuleFromPar<IIpv4RoutingTable>(par("routingTableModule"), this);
        _inft = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);

        _rt6 = getModuleFromPar<Ipv6RoutingTable>(par("routingTableModule6"), this);

        // read BGP configuration
        cXMLElement *config = par("bgpConfig");
        loadConfigFromXML(config);
        createWatch("myAutonomousSystem", _myAS);
        WATCH_PTRVECTOR(_BGPRoutingTable);
        WATCH_PTRVECTOR(_BGPRoutingTable6);
    }
}

void Bgp::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage()) {    //BGP level
        handleTimer(msg);
    }
    else if (!strcmp(msg->getArrivalGate()->getName(), "socketIn")) {    //TCP level
        processMessageFromTCP(msg);
    }
    else {
        delete msg;
    }
}

void Bgp::handleTimer(cMessage *timer)
{
    BgpSession *pSession = (BgpSession *)timer->getContextPointer();
    if (pSession) {
        switch (timer->getKind()) {
            case START_EVENT_KIND:
                EV_INFO << "Processing Start Event" << std::endl;
                pSession->getFSM()->ManualStart();
                break;

            case CONNECT_RETRY_KIND:
                EV_INFO << "Expiring Connect Retry Timer" << std::endl;
                pSession->getFSM()->ConnectRetryTimer_Expires();
                break;

            case HOLD_TIME_KIND:
                EV_INFO << "Expiring Hold Timer" << std::endl;
                pSession->getFSM()->HoldTimer_Expires();
                break;

            case KEEP_ALIVE_KIND:
                EV_INFO << "Expiring Keep Alive timer" << std::endl;
                pSession->getFSM()->KeepaliveTimer_Expires();
                break;

            default:
                throw cRuntimeError("Invalid timer kind %d", timer->getKind());
        }
    }
}

bool Bgp::handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback)
{
    throw cRuntimeError("Lifecycle operation support not implemented");
}

void Bgp::finish()
{
    unsigned int statTab[NB_STATS] = {
        0, 0, 0, 0, 0, 0
    };
    for (auto & elem : _BGPSessions) {
        (elem).second->getStatistics(statTab);
    }
    recordScalar("OPENMsgSent", statTab[0]);
    recordScalar("OPENMsgRecv", statTab[1]);
    recordScalar("KeepAliveMsgSent", statTab[2]);
    recordScalar("KeepAliveMsgRcv", statTab[3]);
    recordScalar("UpdateMsgSent", statTab[4]);
    recordScalar("UpdateMsgRcv", statTab[5]);
}

void Bgp::listenConnectionFromPeer(SessionId sessionID)
{
    if (_BGPSessions[sessionID]->getSocketListen()->getState() == TcpSocket::CLOSED) {
        //session StartDelayTime error, it's anormal that listenSocket is closed.
        _socketMap.removeSocket(_BGPSessions[sessionID]->getSocketListen());
        _BGPSessions[sessionID]->getSocketListen()->abort();
        _BGPSessions[sessionID]->getSocketListen()->renewSocket();
    }
    //bind to specific ip address - support for multihoming (multiaddress family)
    if (_BGPSessions[sessionID]->getSocketListen()->getState() != TcpSocket::LISTENING) {
        _BGPSessions[sessionID]->getSocketListen()->setOutputGate(gate("socketOut"));
        if(_BGPSessions[sessionID]->isMultiAddress())
            _BGPSessions[sessionID]->getSocketListen()->bind(_BGPSessions[sessionID]->getLocalAddr6(),TCP_PORT);
        else
            _BGPSessions[sessionID]->getSocketListen()->bind(_BGPSessions[sessionID]->getLocalAddr(),TCP_PORT);
        _BGPSessions[sessionID]->getSocketListen()->listen();
        _socketMap.addSocket(_BGPSessions[sessionID]->getSocketListen());
    }
}

void Bgp::openTCPConnectionToPeer(SessionId sessionID)
{
    InterfaceEntry *intfEntry = _BGPSessions[sessionID]->getLinkIntf();
    TcpSocket *socket = _BGPSessions[sessionID]->getSocket();
    if (socket->getState() != TcpSocket::NOT_BOUND) {
        _socketMap.removeSocket(socket);
        socket->abort();
        socket->renewSocket();
    }
    socket->setCallback(this);
    socket->setUserData((void *)(uintptr_t)sessionID);
    socket->setOutputGate(gate("socketOut"));
    if(_BGPSessions[sessionID]->isMultiAddress()){
        socket->bind(intfEntry->ipv6Data()->getGlblAddress(), 0);
        socket->connect(_BGPSessions[sessionID]->getPeerAddr6(), TCP_PORT);
    } else {
        socket->bind(intfEntry->ipv4Data()->getIPAddress(), 0);
        socket->connect(_BGPSessions[sessionID]->getPeerAddr(), TCP_PORT);
    }
    _socketMap.addSocket(socket);
}

void Bgp::processMessageFromTCP(cMessage *msg)
{
    TcpSocket *socket = check_and_cast_nullable<TcpSocket*>(_socketMap.findSocketFor(msg));
    if (!socket) {
        socket = new TcpSocket(msg);
        socket->setOutputGate(gate("socketOut"));

        L3Address peerAddr = socket->getRemoteAddress();
        SessionId i = findIdFromPeerAddr(_BGPSessions, peerAddr);
        if (i == static_cast<SessionId>(-1)) {
            socket->close();
            delete socket;
            delete msg;
            return;
        }
        socket->setCallback(this);
        socket->setUserData((void *)(uintptr_t)i);

        _socketMap.addSocket(socket);
        _BGPSessions[i]->getSocket()->abort();
        _BGPSessions[i]->setSocket(socket);
    }

    socket->processMessage(msg);
}

void Bgp::socketEstablished(TcpSocket *socket)
{
    int connId = socket->getSocketId();
    _currSessionId = findIdFromSocketConnId(_BGPSessions, connId);
    if (_currSessionId == static_cast<SessionId>(-1)) {
        throw cRuntimeError("socket id=%d is not established", connId);
    }

    //if it's an IGP Session, TCPConnectionConfirmed only if all EGP Sessions established
    if (_BGPSessions[_currSessionId]->getType() == IGP &&
        this->findNextSession(EGP) != static_cast<SessionId>(-1)) {
        _BGPSessions[_currSessionId]->getFSM()->TcpConnectionFails();
    }
    else {
        _BGPSessions[_currSessionId]->getFSM()->TcpConnectionConfirmed();
        _BGPSessions[_currSessionId]->getSocketListen()->abort();
    }

    if (_BGPSessions[_currSessionId]->getSocketListen()->getSocketId() != connId &&
        _BGPSessions[_currSessionId]->getType() == EGP &&
        this->findNextSession(EGP) != static_cast<SessionId>(-1)) {
        _BGPSessions[_currSessionId]->getSocketListen()->abort();
    }
}

void Bgp::socketDataArrived(TcpSocket *socket, Packet *msg, bool urgent)
{
    _currSessionId = findIdFromSocketConnId(_BGPSessions, socket->getSocketId());
    if (_currSessionId != static_cast<SessionId>(-1)) {
        //TODO: should queuing incoming payloads, and peek from the queue
        //xnovak1j created while if multiple bgp update packets are here
        while(msg->getByteLength() != 0) {
            auto& ptrHdr = msg->popAtFront<BgpHeader>();

            switch (ptrHdr->getType()) {
                case BGP_OPEN:
                    processMessage(*check_and_cast<const BgpOpenMessage *>(ptrHdr.get()));
                    break;

                case BGP_KEEPALIVE:
                    processMessage(*check_and_cast<const BgpKeepAliveMessage *>(ptrHdr.get()));
                    break;

                case BGP_UPDATE:
                {
                    if(_BGPSessions[_currSessionId]->isMultiAddress()) {
                        //const BgpUpdateMessage6 &a = *check_and_cast<const BgpUpdateMessage6 *>(ptrHdr.get());
                        processMessage(*check_and_cast<const BgpUpdateMessage6 *>(ptrHdr.get()));
                    } else {
                        //const BgpUpdateMessage &a = *check_and_cast<const BgpUpdateMessage *>(ptrHdr.get());
                        processMessage(*check_and_cast<const BgpUpdateMessage *>(ptrHdr.get()));
                    }
                    break;
                }
                default:
                    throw cRuntimeError("Invalid BGP message type %d", ptrHdr->getType());
            }
        }
    }
    delete msg;
}

void Bgp::socketFailure(TcpSocket *socket, int code)
{
    int connId = socket->getSocketId();
    _currSessionId = findIdFromSocketConnId(_BGPSessions, connId);
    if (_currSessionId != static_cast<SessionId>(-1)) {
        _BGPSessions[_currSessionId]->getFSM()->TcpConnectionFails();
    }
}

void Bgp::processMessage(const BgpOpenMessage& msg)
{
    EV_INFO << "Processing BGP OPEN message" << std::endl;
    _BGPSessions[_currSessionId]->getFSM()->OpenMsgEvent();
}

void Bgp::processMessage(const BgpKeepAliveMessage& msg)
{
    EV_INFO << "Processing BGP Keep Alive message" << std::endl;
    _BGPSessions[_currSessionId]->getFSM()->KeepAliveMsgEvent();
}

void Bgp::processMessage(const BgpUpdateMessage& msg)
{
    EV_INFO << "Processing BGP Update 4 message" << std::endl;
    _BGPSessions[_currSessionId]->getFSM()->UpdateMsgEvent();

    if(msg.getWithdrawnRoutesLength() == 1) {
        RoutingTableEntry *entry;
        int i = isInRoutingTable(_rt, msg.getWithdrawnRoutes().prefix);
        if (i != -1){
            entry = static_cast<RoutingTableEntry *>(_rt->getRoute(i));
            updateSendProcess(WITHDRAWN_ROUTE, _currSessionId, entry);
        }

    } else {
        unsigned char decisionProcessResult;
        Ipv4Address netMask(Ipv4Address::ALLONES_ADDRESS);
        RoutingTableEntry *entry = new RoutingTableEntry();
        const unsigned char length = msg.getNLRI().length;
        unsigned int ASValueCount = 0;
        if(msg.getPathAttributeList(0).getAsPathArraySize() != 0)
            ASValueCount = msg.getPathAttributeList(0).getAsPath(0).getValue(0).getAsValueArraySize();

        entry->setDestination(msg.getNLRI().prefix);
        netMask = Ipv4Address::makeNetmask(length);
        entry->setNetmask(netMask);
        for (unsigned int j = 0; j < ASValueCount; j++) {
            entry->addAS(msg.getPathAttributeList(0).getAsPath(0).getValue(0).getAsValue(j));
        }

        decisionProcessResult = asLoopDetection(entry, _myAS);

        if (decisionProcessResult == ASLOOP_NO_DETECTED) {
            // RFC 4271, 9.1.  Decision Process
            decisionProcessResult = decisionProcess(msg, entry, _currSessionId);
            //RFC 4271, 9.2.  Update-Send Process
            if (decisionProcessResult != 0)
                updateSendProcess(decisionProcessResult, _currSessionId, entry);
        }
        else
            delete entry;
    }
}

void Bgp::processMessage(const BgpUpdateMessage6& msg)
{
    EV_INFO << "Processing BGP Update 6 message" << std::endl;
    _BGPSessions[_currSessionId]->getFSM()->UpdateMsgEvent();

    if (msg.getPathAttributeList(0).getMpUnreachNlri().getLength() != 0) {
        RoutingTableEntry6 *entry;
        int i = isInRoutingTable6(_rt6, msg.getPathAttributeList(0).getMpUnreachNlri().getMpUnreachNlriValue().getNLRI().prefix);
        if (i != -1){
            entry = static_cast<RoutingTableEntry6 *>(_rt6->getRoute(i));
            updateSendProcess6(WITHDRAWN_ROUTE, _currSessionId, entry);
        }
    } else {
        unsigned char decisionProcessResult;
        RoutingTableEntry6 *entry = new RoutingTableEntry6();

        const unsigned char length = msg.getPathAttributeList(0).getMpReachNlri().getMpReachNlriValue().getNLRI().length;
        unsigned int ASValueCount = 0;
        if(msg.getPathAttributeList(0).getAsPathArraySize() != 0)
            ASValueCount = msg.getPathAttributeList(0).getAsPath(0).getValue(0).getAsValueArraySize();

        entry->setDestination(msg.getPathAttributeList(0).getMpReachNlri().getMpReachNlriValue().getNLRI().prefix);
        int prefLength = length;

        entry->setPrefixLength(prefLength);

        for (unsigned int j = 0; j < ASValueCount; j++) {
            entry->addAS(msg.getPathAttributeList(0).getAsPath(0).getValue(0).getAsValue(j));
        }

        decisionProcessResult = asLoopDetection6(entry, _myAS);

        if (decisionProcessResult == ASLOOP_NO_DETECTED) {
            // RFC 4271, 9.1.  Decision Process
            decisionProcessResult = decisionProcess6(msg, entry, _currSessionId);
            //RFC 4271, 9.2.  Update-Send Process
            if (decisionProcessResult != 0)
                updateSendProcess6(decisionProcessResult, _currSessionId, entry);
        }
        else
            delete entry;
    }
}

/* add entry to routing table, or delete entry */
unsigned char Bgp::decisionProcess(const BgpUpdateMessage& msg, RoutingTableEntry *entry, SessionId sessionIndex)
{
    //Don't add the route if it exists in PrefixListINTable or in ASListINTable
    if (isInTable(_prefixListIN, entry) != (unsigned long)-1  || isInTable(_prefixList, entry) != (unsigned long)-1 || isInASList(_ASListIN, entry)) {
        delete entry;
        return 0;
    }

    /*If the AS_PATH attribute of a BGP route contains an AS loop, the BGP
       route should be excluded from the decision process. */
    entry->setPathType(msg.getPathAttributeList(0).getOrigin().getValue());
    entry->setGateway(msg.getPathAttributeList(0).getNextHop().getValue());

    //if the route already exist in BGP routing table, tieBreakingProcess();
    //(RFC 4271: 9.1.2.2 Breaking Ties)
    unsigned long BGPindex = isInTable(_BGPRoutingTable, entry);
    if (BGPindex != (unsigned long)-1) {
        if (tieBreakingProcess(_BGPRoutingTable[BGPindex], entry)) {
            delete entry;
            return 0;
        }
        else {
            entry->setInterface(_BGPSessions[sessionIndex]->getLinkIntf());
            _BGPRoutingTable.push_back(entry);
            _rt->addRoute(entry);

            _BGPSessions[sessionIndex]->setNetworkFromPeer(entry->getDestination());

            return ROUTE_DESTINATION_CHANGED;
        }
    }

    //Don't add the route if it exists in Ipv4 routing table except if the msg come from IGP session
    int indexIP = isInRoutingTable(_rt, entry->getDestination());
    if (indexIP != -1 && _rt->getRoute(indexIP)->getSourceType() != IRoute::BGP) {
        if (_BGPSessions[sessionIndex]->getType() != IGP) {
            delete entry;
            return 0;
        }
        else {
            Ipv4Route *oldEntry = _rt->getRoute(indexIP);
            Ipv4Route *newEntry = new Ipv4Route;
            newEntry->setDestination(oldEntry->getDestination());
            newEntry->setNetmask(oldEntry->getNetmask());
            newEntry->setGateway(oldEntry->getGateway());
            newEntry->setInterface(oldEntry->getInterface());
            newEntry->setSourceType(IRoute::BGP);
            _rt->deleteRoute(oldEntry);
            _rt->addRoute(newEntry);

            _BGPSessions[sessionIndex]->setNetworkFromPeer(newEntry->getDestination());
           // _routesFromPeer[_BGPSessions[sessionIndex]->getPeerAddr()].push_back(newEntry->getDestination());
            //FIXME model error: the RoutingTableEntry *entry will be stored in _BGPRoutingTable, but not stored in _rt, memory leak
            //FIXME model error: The entry inserted to _BGPRoutingTable, but newEntry inserted to _rt; entry and newEntry are differ.
        }
    }

    entry->setInterface(_BGPSessions[sessionIndex]->getLinkIntf());
    _BGPRoutingTable.push_back(entry);
    _rt->addRoute(entry);

    _BGPSessions[sessionIndex]->setNetworkFromPeer(entry->getDestination());

    return NEW_ROUTE_ADDED;     //FIXME model error: When returns NEW_ROUTE_ADDED then entry stored in _BGPRoutingTable, but sometimes not stored in _rt
}

unsigned char Bgp::decisionProcess6(const BgpUpdateMessage6& msg, RoutingTableEntry6 *entry, SessionId sessionIndex)
{
    //Don't add the route if it exists in PrefixListINTable or in ASListINTable
    if (isInTable6(_prefixListIN6, entry) != (unsigned long)-1 || isInTable6(_prefixList6, entry) != (unsigned long)-1 || isInASList6(_ASListIN6, entry)) {
        delete entry;
        return 0;
    }

    /*If the AS_PATH attribute of a BGP route contains an AS loop, the BGP
       route should be excluded from the decision process. */
    entry->setPathType(msg.getPathAttributeList(0).getOrigin().getValue());
    entry->setNextHop(msg.getPathAttributeList(0).getMpReachNlri().getMpReachNlriValue().getNextHop().getValue());

    //if the route already exist in BGP routing table, tieBreakingProcess();
    //(RFC 4271: 9.1.2.2 Breaking Ties)
    unsigned long BGPindex = isInTable6(_BGPRoutingTable6, entry);
    if (BGPindex != (unsigned long)-1) {
        if (tieBreakingProcess6(_BGPRoutingTable6[BGPindex], entry)) {
            delete entry;
            return 0;
        }
        else {
            entry->setInterface(_BGPSessions[sessionIndex]->getLinkIntf());
            _BGPRoutingTable6.push_back(entry);
            _rt6->addRoutingProtocolRoute(entry);

            _BGPSessions[sessionIndex]->setNetworkFromPeer6(entry->getDestPrefix());

            return ROUTE_DESTINATION_CHANGED;
        }
    }

    //Don't add the route if it exists in Ipv6 routing table except if the msg come from IGP session
    int indexIP = isInRoutingTable6(_rt6, entry->getDestPrefix());
    if (indexIP != -1 && _rt6->getRoute(indexIP)->getSourceType() != IRoute::BGP) {
        if (_BGPSessions[sessionIndex]->getType() != IGP) {
            delete entry;
            return 0;
        }
        else {
            Ipv6Route *oldEntry = _rt6->getRoute(indexIP);
            Ipv6Route *newEntry = new Ipv6Route;
            newEntry->setDestination(oldEntry->getDestPrefix());
            newEntry->setPrefixLength(oldEntry->getPrefixLength());
            newEntry->setNextHop(oldEntry->getNextHop());
            newEntry->setInterface(oldEntry->getInterface());
            newEntry->setSourceType(IRoute::BGP);
            _rt6->deleteRoute(oldEntry);
            _rt6->addRoutingProtocolRoute(newEntry);

            _BGPSessions[sessionIndex]->setNetworkFromPeer6(newEntry->getDestPrefix());

            //FIXME model error: the RoutingTableEntry *entry will be stored in _BGPRoutingTable, but not stored in _rt, memory leak
            //FIXME model error: The entry inserted to _BGPRoutingTable, but newEntry inserted to _rt; entry and newEntry are differ.
        }
    }

    entry->setInterface(_BGPSessions[sessionIndex]->getLinkIntf());
    _BGPRoutingTable6.push_back(entry);
    _rt6->addRoutingProtocolRoute(entry);

    _BGPSessions[sessionIndex]->setNetworkFromPeer6(entry->getDestPrefix());

    return NEW_ROUTE_ADDED;     //FIXME model error: When returns NEW_ROUTE_ADDED then entry stored in _BGPRoutingTable, but sometimes not stored in _rt
}

bool Bgp::tieBreakingProcess(RoutingTableEntry *oldEntry, RoutingTableEntry *entry)
{
    /*a) Remove from consideration all routes that are not tied for
         having the smallest number of AS numbers present in their
         AS_PATH attributes.*/
    if (entry->getASCount() < oldEntry->getASCount()) {
        deleteBGPRoutingEntry(oldEntry);
        return false;
    }
    /* b) Remove from consideration all routes that are not tied for
            having the lowest Origin number in their Origin attribute.*/
   if (entry->getPathType() < oldEntry->getPathType()) {
       deleteBGPRoutingEntry(oldEntry);
       return false;
   }
    return true;
}

bool Bgp::tieBreakingProcess6(RoutingTableEntry6 *oldEntry, RoutingTableEntry6 *entry)
{
    /*a) Remove from consideration all routes that are not tied for
         having the smallest number of AS numbers present in their
         AS_PATH attributes.*/
    if (entry->getASCount() < oldEntry->getASCount()) {
        deleteBGPRoutingEntry6(oldEntry);
        return false;
    }

    /* b) Remove from consideration all routes that are not tied for
         having the lowest Origin number in their Origin attribute.*/
    if (entry->getPathType() < oldEntry->getPathType()) {
        deleteBGPRoutingEntry6(oldEntry);
        return false;
    }
    return true;
}

void Bgp::updateSendProcess(const unsigned char type, SessionId sessionIndex, RoutingTableEntry *entry)
{
    //Don't send the update Message if the route exists in listOUTTable
    //SESSION = EGP : send an update message to all BGP Peer (EGP && IGP)
    //if it is not the currentSession and if the session is already established
    //SESSION = IGP : send an update message to External BGP Peer (EGP) only
    //if it is not the currentSession and if the session is already established
    for (auto & elem : _BGPSessions)
    {
        if((elem).second->isMultiAddress() == false) {
            /*withdrawn route
             * want to send withdrawn message to other peers
             * */
            if (type == WITHDRAWN_ROUTE && (elem).first != sessionIndex) {
                if ((_BGPSessions[sessionIndex]->getType() == IGP && (elem).second->getType() == EGP) ||
                    _BGPSessions[sessionIndex]->getType() == EGP) {
                    BgpUpdateWithdrawnRoutes withdrawnRoutes;
                    Ipv4Address netMask = entry->getNetmask();
                    withdrawnRoutes.prefix = entry->getDestination().doAnd(netMask);
                    withdrawnRoutes.length = (unsigned char)netMask.getNetmaskLength();

                    Packet *pk = new Packet("BgpUpdate");
                    const auto& updateMsg = makeShared<BgpUpdateMessage>();
                    updateMsg->setWithdrawnRoutesLength(1);
                    updateMsg->setWithdrawnRoutes(withdrawnRoutes);
                    pk->insertAtFront(updateMsg);
                    (elem).second->getSocket()->send(pk);
                    (elem).second->addUpdateMsgSent();
                }

            } else if ( type != WITHDRAWN_ROUTE ) {

                if (isInTable(_prefixListOUT, entry) != (unsigned long)-1 || isInTable(_prefixList, entry) != (unsigned long)-1 || isInASList(_ASListOUT, entry) ||
                    ((elem).first == sessionIndex && type != NEW_SESSION_ESTABLISHED) ||
                    (type == NEW_SESSION_ESTABLISHED && (elem).first != sessionIndex) ||
                    !(elem).second->isEstablished())
                {
                    continue;
                }
                if ((_BGPSessions[sessionIndex]->getType() == IGP && (elem).second->getType() == EGP) ||
                    _BGPSessions[sessionIndex]->getType() == EGP ||
                    (type == ROUTE_DESTINATION_CHANGED && ((_BGPSessions[sessionIndex]->getType() == IGP && (entry->getPathType() == EGP ||  entry->getGateway() == Ipv4Address::UNSPECIFIED_ADDRESS )) || _BGPSessions[sessionIndex]->getType() == EGP)) ||
                    (type == NEW_SESSION_ESTABLISHED && ((_BGPSessions[sessionIndex]->getType() == IGP && (entry->getPathType() == EGP ||  entry->getGateway() == Ipv4Address::UNSPECIFIED_ADDRESS )) || _BGPSessions[sessionIndex]->getType() == EGP)))
                {
                    BgpUpdateNlri NLRI;
                    BgpUpdatePathAttributeList content;

                    unsigned int nbAS = entry->getASCount();
                    if((elem).second->getType() == IGP && nbAS == 0) {
                       content.setAsPathArraySize(0);
                    } else {

                        content.setAsPathArraySize(1);

                        content.getAsPathForUpdate(0).setValueArraySize(1);
                        content.getAsPathForUpdate(0).getFlagsForUpdate().transitiveBit = true;
                        content.getAsPathForUpdate(0).getValueForUpdate(0).setType(AS_SEQUENCE);
                        //RFC 4271 : set My AS in first position if it is not already

                        if ((_BGPSessions[sessionIndex]->getType() == EGP && (elem).second->getType() == EGP) || (elem).second->getType() == EGP) {
                            if(nbAS != 0) {
                                if (entry->getAS(0) != _myAS) {
                                    content.getAsPathForUpdate(0).setLength(2+(4*(nbAS + 1)));
                                    content.getAsPathForUpdate(0).getValueForUpdate(0).setAsValueArraySize(nbAS + 1);
                                    content.getAsPathForUpdate(0).getValueForUpdate(0).setLength(nbAS + 1);
                                    content.getAsPathForUpdate(0).getValueForUpdate(0).setAsValue(0, _myAS);
                                    for (unsigned int j = 1; j < nbAS + 1; j++) {
                                        content.getAsPathForUpdate(0).getValueForUpdate(0).setAsValue(j, entry->getAS(j - 1));
                                    }
                                }
                                else
                                {
                                    content.getAsPathForUpdate(0).setLength(2+(4*nbAS));
                                    content.getAsPathForUpdate(0).getValueForUpdate(0).setAsValueArraySize(nbAS);
                                    content.getAsPathForUpdate(0).getValueForUpdate(0).setLength(nbAS);
                                    for (unsigned int j = 0; j < nbAS; j++) {
                                        content.getAsPathForUpdate(0).getValueForUpdate(0).setAsValue(j, entry->getAS(j));
                                    }
                                }
                            } else {
                                content.getAsPathForUpdate(0).setLength(6);
                                content.getAsPathForUpdate(0).getValueForUpdate(0).setAsValueArraySize(1);
                                content.getAsPathForUpdate(0).getValueForUpdate(0).setLength(1);
                                content.getAsPathForUpdate(0).getValueForUpdate(0).setAsValue(0, _myAS);
                            }
                        } else {
                            content.getAsPathForUpdate(0).setLength(2+(4*nbAS));
                            content.getAsPathForUpdate(0).getValueForUpdate(0).setAsValueArraySize(nbAS);
                            content.getAsPathForUpdate(0).getValueForUpdate(0).setLength(nbAS);
                            for (unsigned int j = 0; j < nbAS; j++) {
                                content.getAsPathForUpdate(0).getValueForUpdate(0).setAsValue(j, entry->getAS(j));
                            }
                        }
                    }

                    InterfaceEntry *iftEntry = (elem).second->getLinkIntf();
                    content.getOriginForUpdate().getFlagsForUpdate().transitiveBit = true;
                    content.getOriginForUpdate().setValue((elem).second->getType());
                    content.getNextHopForUpdate().getFlagsForUpdate().transitiveBit = true;
                    content.getNextHopForUpdate().setValue(iftEntry->ipv4Data()->getIPAddress());
                    Ipv4Address netMask = entry->getNetmask();
                    NLRI.prefix = entry->getDestination().doAnd(netMask);
                    NLRI.length = (unsigned char)netMask.getNetmaskLength();
                    {
                        Packet *pk = new Packet("BgpUpdate");
                        const auto& updateMsg = makeShared<BgpUpdateMessage>();
                        updateMsg->setPathAttributeLength(1);
                        updateMsg->setPathAttributeListArraySize(1);
                        updateMsg->setPathAttributeList(content);
                        updateMsg->setNLRI(NLRI);
                        pk->insertAtFront(updateMsg);
                        (elem).second->getSocket()->send(pk);
                        (elem).second->addUpdateMsgSent();
                    }
                }
            }
        }
    }
    if (type == WITHDRAWN_ROUTE)
        deleteBGPRoutingEntry(entry);
}

void Bgp::updateSendProcess6(const unsigned char type, SessionId sessionIndex, RoutingTableEntry6 *entry)
{
    //Don't send the update Message if the route exists in listOUTTable
    //SESSION = EGP : send an update message to all BGP Peer (EGP && IGP)
    //if it is not the currentSession and if the session is already established
    //SESSION = IGP : send an update message to External BGP Peer (EGP) only
    //if it is not the currentSession and if the session is already established
    for (auto & elem : _BGPSessions)
    {
        if((elem).second->isMultiAddress()) {
            if (type == WITHDRAWN_ROUTE && (elem).first != sessionIndex) {
                if ((_BGPSessions[sessionIndex]->getType() == IGP && (elem).second->getType() == EGP) ||
                    _BGPSessions[sessionIndex]->getType() == EGP) {

                    BgpUpdateWithdrawnRoutes6 NLRI;
                    BgpUpdatePathAttributeList6 content;

                    unsigned int nbAS = entry->getASCount();
                    //AS PATH
                    content.setAsPathArraySize(1);
                    content.getAsPathForUpdate(0).setValueArraySize(1);
                    content.getAsPathForUpdate(0).getFlagsForUpdate().transitiveBit = true;
                    content.getAsPathForUpdate(0).getValueForUpdate(0).setType(AS_SEQUENCE);

                    //RFC 4271 : set My AS in first position if it is not already
                    if ((_BGPSessions[sessionIndex]->getType() == EGP && (elem).second->getType() != IGP) || (elem).second->getType() == EGP) {
                       if (entry->getAS(0) != _myAS) {
                           content.getAsPathForUpdate(0).setLength(2+(4*(nbAS + 1)));
                           content.getAsPathForUpdate(0).getValueForUpdate(0).setAsValueArraySize(nbAS + 1);
                           content.getAsPathForUpdate(0).getValueForUpdate(0).setLength(nbAS + 1);
                           content.getAsPathForUpdate(0).getValueForUpdate(0).setAsValue(0, _myAS);
                           for (unsigned int j = 1; j < nbAS + 1; j++) {
                               content.getAsPathForUpdate(0).getValueForUpdate(0).setAsValue(j, entry->getAS(j - 1));
                           }
                       }
                       else {
                           content.getAsPathForUpdate(0).setLength(2+(4*nbAS));
                           content.getAsPathForUpdate(0).getValueForUpdate(0).setAsValueArraySize(nbAS);
                           content.getAsPathForUpdate(0).getValueForUpdate(0).setLength(nbAS);
                           for (unsigned int j = 0; j < nbAS; j++) {
                               content.getAsPathForUpdate(0).getValueForUpdate(0).setAsValue(j, entry->getAS(j));
                           }
                       }
                    } else {
                       content.getAsPathForUpdate(0).setLength(2+(4*nbAS));
                       content.getAsPathForUpdate(0).getValueForUpdate(0).setAsValueArraySize(nbAS);
                       content.getAsPathForUpdate(0).getValueForUpdate(0).setLength(nbAS);
                       for (unsigned int j = 0; j < nbAS; j++) {
                           content.getAsPathForUpdate(0).getValueForUpdate(0).setAsValue(j, entry->getAS(j));
                       }
                    }
                    unsigned char prefLength = (unsigned char)entry->getPrefixLength();
                    NLRI.prefix = entry->getDestPrefix();
                    NLRI.length = prefLength;

                    //origin
                    InterfaceEntry *iftEntry = (elem).second->getLinkIntf();
                    content.getOriginForUpdate().getFlagsForUpdate().transitiveBit = true;
                    content.getOriginForUpdate().setValue((elem).second->getType());
                    //MP reach NLRI
                    content.getMpReachNlriForUpdate().getFlagsForUpdate().optionalBit = true;
                    content.getMpReachNlriForUpdate().setLength(0);
                    //next hop
                    content.getMpReachNlriForUpdate().getMpReachNlriValueForUpdate().getNextHopForUpdate().getFlagsForUpdate().transitiveBit = true;
                    //nlri
                    //MP unreach NLRI
                    content.getMpUnreachNlriForUpdate().getFlagsForUpdate().optionalBit = true;
                    content.getMpUnreachNlriForUpdate().setLength(20);
                    content.getMpUnreachNlriForUpdate().getMpUnreachNlriValueForUpdate().setNLRI(NLRI);
                    //create BGP update message for ipv6 multiaddress family routing
                    {
                        Packet *pk = new Packet("BgpUpdate");
                        const auto& updateMsg = makeShared<BgpUpdateMessage6>();
                        updateMsg->setPathAttributeListArraySize(1);
                        updateMsg->setPathAttributeList(content);

                        pk->insertAtFront(updateMsg);
                        (elem).second->getSocket()->send(pk);
                        (elem).second->addUpdateMsgSent();
                    }
                }

            } else if ( type != WITHDRAWN_ROUTE ) {

                if (isInTable6(_prefixListOUT6, entry) != (unsigned long)-1 || isInTable6(_prefixList6, entry) != (unsigned long)-1 || isInASList6(_ASListOUT6, entry) ||
                    ((elem).first == sessionIndex && type != NEW_SESSION_ESTABLISHED) ||
                    (type == NEW_SESSION_ESTABLISHED && (elem).first != sessionIndex) ||
                    !(elem).second->isEstablished())
                {
                    continue;
                }

                Ipv6Address tmp;
                if ((_BGPSessions[sessionIndex]->getType() == IGP && (elem).second->getType() == EGP) ||
                    _BGPSessions[sessionIndex]->getType() == EGP ||
                    (type == ROUTE_DESTINATION_CHANGED && ((_BGPSessions[sessionIndex]->getType() == IGP && (entry->getPathType() == EGP ||  entry->getNextHop() == tmp )) || _BGPSessions[sessionIndex]->getType() == EGP)) ||
                    (type == NEW_SESSION_ESTABLISHED && ((_BGPSessions[sessionIndex]->getType() == IGP && (entry->getPathType() == EGP ||  entry->getNextHop() == tmp )) || _BGPSessions[sessionIndex]->getType() == EGP)))
                {
                    BgpUpdateNlri6 NLRI;
                    BgpUpdatePathAttributeList6 content;

                    unsigned int nbAS = entry->getASCount();
                    if((elem).second->getType() == IGP && nbAS == 0) {
                       content.setAsPathArraySize(0);
                    } else {

                        content.setAsPathArraySize(1);

                        content.getAsPathForUpdate(0).setValueArraySize(1);
                        content.getAsPathForUpdate(0).getFlagsForUpdate().transitiveBit = true;
                        content.getAsPathForUpdate(0).getValueForUpdate(0).setType(AS_SEQUENCE);
                        //RFC 4271 : set My AS in first position if it is not already

                        if ((_BGPSessions[sessionIndex]->getType() == EGP && (elem).second->getType() == EGP) || (elem).second->getType() == EGP) {
                            if(nbAS != 0) {
                                if (entry->getAS(0) != _myAS) {
                                    content.getAsPathForUpdate(0).setLength(2+(4*(nbAS + 1)));
                                    content.getAsPathForUpdate(0).getValueForUpdate(0).setAsValueArraySize(nbAS + 1);
                                    content.getAsPathForUpdate(0).getValueForUpdate(0).setLength(nbAS + 1);
                                    content.getAsPathForUpdate(0).getValueForUpdate(0).setAsValue(0, _myAS);
                                    for (unsigned int j = 1; j < nbAS + 1; j++) {
                                        content.getAsPathForUpdate(0).getValueForUpdate(0).setAsValue(j, entry->getAS(j - 1));
                                    }
                                }
                                else
                                {
                                    content.getAsPathForUpdate(0).setLength(2+(4*nbAS));
                                    content.getAsPathForUpdate(0).getValueForUpdate(0).setAsValueArraySize(nbAS);
                                    content.getAsPathForUpdate(0).getValueForUpdate(0).setLength(nbAS);
                                    for (unsigned int j = 0; j < nbAS; j++) {
                                        content.getAsPathForUpdate(0).getValueForUpdate(0).setAsValue(j, entry->getAS(j));
                                    }
                                }
                            } else {
                                content.getAsPathForUpdate(0).setLength(6);
                                content.getAsPathForUpdate(0).getValueForUpdate(0).setAsValueArraySize(1);
                                content.getAsPathForUpdate(0).getValueForUpdate(0).setLength(1);
                                content.getAsPathForUpdate(0).getValueForUpdate(0).setAsValue(0, _myAS);
                            }
                        } else {
                            content.getAsPathForUpdate(0).setLength(2+(4*nbAS));
                            content.getAsPathForUpdate(0).getValueForUpdate(0).setAsValueArraySize(nbAS);
                            content.getAsPathForUpdate(0).getValueForUpdate(0).setLength(nbAS);
                            for (unsigned int j = 0; j < nbAS; j++) {
                                content.getAsPathForUpdate(0).getValueForUpdate(0).setAsValue(j, entry->getAS(j));
                            }
                        }
                    }

                    unsigned char prefLength = (unsigned char)entry->getPrefixLength();
                    NLRI.prefix = entry->getDestPrefix();
                    NLRI.length = prefLength;

                    //origin
                    InterfaceEntry *iftEntry = (elem).second->getLinkIntf();
                    content.getOriginForUpdate().getFlagsForUpdate().transitiveBit = true;
                    content.getOriginForUpdate().setValue((elem).second->getType());
                    //MP reach NLRI
                    content.getMpReachNlriForUpdate().getFlagsForUpdate().optionalBit = true;
                    content.getMpReachNlriForUpdate().setLength(39);
                    //next hop
                    content.getMpReachNlriForUpdate().getMpReachNlriValueForUpdate().getNextHopForUpdate().getFlagsForUpdate().transitiveBit = true;
                    content.getMpReachNlriForUpdate().getMpReachNlriValueForUpdate().getNextHopForUpdate().setValue(iftEntry->ipv6Data()->getGlblAddress());
                    //nlri
                    content.getMpReachNlriForUpdate().getMpReachNlriValueForUpdate().setNLRI(NLRI);
                    //MP unreach NLRI
                    content.getMpUnreachNlriForUpdate().getFlagsForUpdate().optionalBit = true;
                    //create BGP update message for ipv6 multiaddress family routing
                    {
                        Packet *pk = new Packet("BgpUpdate");
                        const auto& updateMsg = makeShared<BgpUpdateMessage6>();
                        updateMsg->setPathAttributeListArraySize(1);
                        updateMsg->setPathAttributeList(content);

                        pk->insertAtFront(updateMsg);
                        (elem).second->getSocket()->send(pk);
                        (elem).second->addUpdateMsgSent();
                    }
                }
            }
        }
    }
    if (type == WITHDRAWN_ROUTE)
        deleteBGPRoutingEntry6(entry);
}

//bool Bgp::checkExternalRoute(const Ipv4Route *route)
//{
//    Ipv4Address OSPFRoute;
//    OSPFRoute = route->getDestination();
//    ospf::Ospf *ospf = getModuleFromPar<ospf::Ospf>(par("ospfRoutingModule"), this);
//    bool returnValue = ospf->checkExternalRoute(OSPFRoute);
//    return returnValue;
//}

void Bgp::addToDenyList(const char * addr6c, int flag)
{
    std::string add6 = addr6c;
    std::string prefix6 = add6.substr(0, add6.find("/"));
    int prefLength;
    Ipv6Address address6;
    if (!(address6.tryParseAddrWithPrefix(addr6c, prefLength)))
         throw cRuntimeError("Cannot parse Ipv6 address: '%s", addr6c);

    address6 = Ipv6Address(prefix6.c_str());

    RoutingTableEntry6 * entry = new RoutingTableEntry6();
    entry->setDestination(address6);
    entry->setPrefixLength(prefLength);

    if (flag == 0)
        _prefixListIN6.push_back(entry);
    else if (flag == 1)
        _prefixListOUT6.push_back(entry);
    else if (flag == 2)
        _prefixList6.push_back(entry);

}
//parse input XML
void Bgp::loadTimerConfig(cXMLElementList& timerConfig, simtime_t *delayTab)
{
    for (auto & elem : timerConfig) {
        std::string nodeName = (elem)->getTagName();
        if (nodeName == "connectRetryTime") {
            delayTab[0] = (double)atoi((elem)->getNodeValue());
        }
        else if (nodeName == "holdTime") {
            delayTab[1] = (double)atoi((elem)->getNodeValue());
        }
        else if (nodeName == "keepAliveTime") {
            delayTab[2] = (double)atoi((elem)->getNodeValue());
        }
        else if (nodeName == "startDelay") {
            delayTab[3] = (double)atoi((elem)->getNodeValue());
        }
    }
}

void Bgp::routerIntfAndRouteConfig(cXMLElement *rtrConfig)
{
    cXMLElementList interfaceList = rtrConfig->getElementsByTagName("Interface");
    InterfaceEntry * myInterface;

    for (auto & interface : interfaceList) {
        myInterface = (_inft->getInterfaceByName((interface)->getAttribute("id")));

        if (myInterface->isLoopback()) {
            const char * ipv41 = "127.0.0.0";
            Ipv4Address tmpipv4;
            tmpipv4.set(ipv41);
            int i = isInRoutingTable(_rt, tmpipv4);
            if (i != -1){
                _rt->deleteRoute(_rt->getRoute(i));
            }
        }

        //interface ipv4 configuration
        Ipv4Address addr = (Ipv4Address(((interface)->getElementByPath("Ipv4"))->getAttribute("address")));
        Ipv4Address mask = (Ipv4Address(((interface)->getElementByPath("Ipv4"))->getAttribute("netmask")));
        Ipv4InterfaceData * intfData = myInterface->ipv4Data(); //new Ipv4InterfaceData();
        intfData->setIPAddress(addr);
        intfData->setNetmask(mask);

        //add directly connected ip address to routing table
        Ipv4Address networkAdd = (Ipv4Address((intfData->getIPAddress()).getInt() & (intfData->getNetmask()).getInt()));
        Ipv4Route *entry = new Ipv4Route;

        entry->setDestination(networkAdd);
        entry->setNetmask(intfData->getNetmask());
        entry->setInterface(myInterface);
        entry->setMetric(0);
        entry->setSourceType(IRoute::IFACENETMASK);

        _rt->addRoute(entry);

        //interface ipv6 configuration

        cXMLElementList ipv6List = interface->getElementsByTagName("Ipv6");
        for (auto & ipv6Rec : ipv6List) {
            const char * addr6c = ipv6Rec->getAttribute("address");

            std::string add6 = addr6c;
            std::string prefix6 = add6.substr(0, add6.find("/"));

            Ipv6InterfaceData * intfData6 = myInterface->ipv6Data();

            int prefLength;
            Ipv6Address address6;
            if (!(address6.tryParseAddrWithPrefix(addr6c, prefLength)))
                 throw cRuntimeError("Cannot parse Ipv6 address: '%s", addr6c);

            address6 = Ipv6Address(prefix6.c_str());

            Ipv6InterfaceData::AdvPrefix p;
            p.prefix = address6;
            p.prefixLength = prefLength;

            intfData6->assignAddress(address6, false, SIMTIME_ZERO, SIMTIME_ZERO);

            // add this routes to routing table
            Ipv6Route *route = new Ipv6Route(p.prefix.getPrefix(prefLength), p.prefixLength, IRoute::IFACENETMASK);
            route->setInterface(myInterface);
            route->setExpiryTime(SIMTIME_ZERO);
            route->setMetric(0);
            route->setAdminDist(Ipv6Route::dDirectlyConnected);

            _rt6->addRoutingProtocolRoute(route);
        }
    }
    //add static routes to ipv4 routing table
    cXMLElementList routeNodes = rtrConfig->getElementsByTagName("Route");
    for (auto & elem : routeNodes) {
        Ipv4Route *entry = new Ipv4Route;
        entry->setDestination(Ipv4Address((elem)->getAttribute("destination")));
        entry->setNetmask(Ipv4Address((elem)->getAttribute("netmask")));
        entry->setInterface(_inft->getInterfaceByName((elem)->getAttribute("interface")));
        entry->setMetric(1);
        Ipv4Address nexthop = (Ipv4Address(elem->getAttribute("nexthop")));
        entry->setGateway(nexthop);
        entry->setSourceType(IRoute::MANUAL);

        _rt->addRoute(entry);
    }

    //add static routes to ipv6 routing table
    cXMLElementList routeNodes6 = rtrConfig->getElementsByTagName("Route6");
    for (auto & elem : routeNodes6) {

        const char * addr6c = elem->getAttribute("destination");
        std::string add6 = addr6c;
        std::string prefix6 = add6.substr(0, add6.find("/"));

        int prefLength;
        Ipv6Address address6;
        if (!(address6.tryParseAddrWithPrefix(addr6c, prefLength)))
            throw cRuntimeError("Cannot parse Ipv6 address: '%s", addr6c);

        address6 = Ipv6Address(prefix6.c_str());
        Ipv6Address nextHop6 = (Ipv6Address(elem->getAttribute("nexthop")));
        _rt6->addStaticRoute(address6, prefLength,_inft->getInterfaceByName((elem)->getAttribute("interface"))->getInterfaceId(), nextHop6);
    }
}

void Bgp::loadBgpNodeConfig(cXMLElement *bgpNode, simtime_t *delayTab, int pos)
{
    simtime_t saveStartDelay = delayTab[3];
    _myAS = atoi((bgpNode)->getAttribute("as"));

    cXMLElementList afNodes = bgpNode->getElementsByTagName("Address-family");
    for (auto & elem : afNodes) {
        //IPv4 address family
        if(strcmp((elem)->getAttribute("id"), "Ipv4") == 0) {
            cXMLElementList networkNodes = elem->getElementsByTagName("Network");
            cXMLElementList neighborNodes = elem->getElementsByTagName("Neighbor");

            cXMLElementList denyRouteIn = elem->getElementsByTagName("DenyRouteIN");
            cXMLElementList denyRouteOut = elem->getElementsByTagName("DenyRouteOUT");
            cXMLElementList denyRoute = elem->getElementsByTagName("DenyRoute");
            cXMLElementList denyAsIn = elem->getElementsByTagName("DenyASIN");
            cXMLElementList denyAsOut = elem->getElementsByTagName("DenyASOUT");
            cXMLElementList denyAs = elem->getElementsByTagName("DenyAS");

            for (auto & route : denyRouteIn) {
                RoutingTableEntry *entry = new RoutingTableEntry();
                entry->setDestination(Ipv4Address((route)->getAttribute("address")));
                entry->setNetmask(Ipv4Address((route)->getAttribute("netmask")));

                _prefixListIN.push_back(entry);
            }

            for (auto & route : denyRouteOut) {
                RoutingTableEntry *entry = new RoutingTableEntry();
                entry->setDestination(Ipv4Address((route)->getAttribute("address")));
                entry->setNetmask(Ipv4Address((route)->getAttribute("netmask")));

                _prefixListOUT.push_back(entry);
            }

            for (auto & route : denyRoute) {
                RoutingTableEntry *entry = new RoutingTableEntry();
                entry->setDestination(Ipv4Address((route)->getAttribute("address")));
                entry->setNetmask(Ipv4Address((route)->getAttribute("netmask")));

                _prefixList.push_back(entry);
            }

            for (auto & as : denyAsIn) {
                AsId ASCur = atoi((as)->getAttribute("as"));
                _ASListIN.push_back(ASCur);
            }

            for (auto & as : denyAsOut) {
                AsId ASCur = atoi((as)->getAttribute("as"));
                _ASListOUT.push_back(ASCur);
            }

            for (auto & as : denyAs) {
                AsId ASCur = atoi((as)->getAttribute("as"));
                _ASListIN.push_back(ASCur);
                _ASListOUT.push_back(ASCur);
            }

            for (auto & network : networkNodes) {
                _networksToAdvertise.push_back(Ipv4Address((network)->getAttribute("address")));

                int i = isInRoutingTable(_rt, Ipv4Address((network)->getAttribute("address")));
                if (i != -1) {
                    const Ipv4Route *rtEntry = _rt->getRoute(i);
                    RoutingTableEntry *BGPEntry = new RoutingTableEntry(rtEntry);
                    if(rtEntry->getSourceType() != IRoute::IFACENETMASK) {

                    }
                    BGPEntry->setPathType(IGP);
                    _BGPRoutingTable.push_back(BGPEntry);
                }
            }

            for (auto & neighbor : neighborNodes) {
                if (atoi((neighbor)->getAttribute("remote-as")) == _myAS) {
                    _routerInSameASList.push_back((neighbor)->getAttribute("address"));
                } else {
                    //start EGP sessions

                    Ipv4Address peerAddr = Ipv4Address((neighbor)->getAttribute("address"));

                    if((_rt->getInterfaceForDestAddr(peerAddr)->getIpv4Address()).getInt() < peerAddr.getInt()){
                        delayTab[3] += pos;
                    } else {
                        delayTab[3] += pos;
                    }
                    SessionId newSessionID = createSession(EGP, peerAddr.str().c_str());
                    _BGPSessions[newSessionID]->setTimers(delayTab);
                    TcpSocket *socketListenEGP = new TcpSocket();
                    _BGPSessions[newSessionID]->setSocketListen(socketListenEGP);

                    delayTab[3] = saveStartDelay;
                }
            }
        } else if(strcmp((elem)->getAttribute("id"), "Ipv6") == 0) { //Ipv6 address family
            cXMLElementList networkNodes = elem->getElementsByTagName("Network");
            cXMLElementList neighborNodes = elem->getElementsByTagName("Neighbor");

            cXMLElementList denyRouteIn = elem->getElementsByTagName("DenyRouteIN");
            cXMLElementList denyRouteOut = elem->getElementsByTagName("DenyRouteOUT");
            cXMLElementList denyRoute = elem->getElementsByTagName("DenyRoute");
            cXMLElementList denyAsIn = elem->getElementsByTagName("DenyASIN");
            cXMLElementList denyAsOut = elem->getElementsByTagName("DenyASOUT");
            cXMLElementList denyAs = elem->getElementsByTagName("DenyAS");

            for (auto & route : denyRouteIn) {
                addToDenyList(route->getAttribute("address"), 0);
            }

            for (auto & route : denyRouteOut) {
                addToDenyList(route->getAttribute("address"), 1);
            }

            for (auto & route : denyRoute) {
                addToDenyList(route->getAttribute("address"), 2);
            }

            for (auto & as : denyAsIn) {
                AsId ASCur = atoi((as)->getAttribute("as"));
                _ASListIN6.push_back(ASCur);
            }

            for (auto & as : denyAsOut) {
                AsId ASCur = atoi((as)->getAttribute("as"));
                _ASListOUT6.push_back(ASCur);
            }

            for (auto & as : denyAs) {
                AsId ASCur = atoi((as)->getAttribute("as"));
                _ASListIN6.push_back(ASCur);
                _ASListOUT6.push_back(ASCur);
            }

            for (auto & network : networkNodes) {
                _networksToAdvertise6.push_back(Ipv6Address((network)->getAttribute("address")));

                int i = isInRoutingTable6(_rt6, Ipv6Address((network)->getAttribute("address")));
               if (i != -1) {
                   const Ipv6Route *rtEntry = _rt6->getRoute(i);
                   RoutingTableEntry6 *BGPEntry = new RoutingTableEntry6(rtEntry);
                   BGPEntry->setPathType(IGP);
                   _BGPRoutingTable6.push_back(BGPEntry);
               }
            }

            for (auto & neighbor : neighborNodes) {
                if (atoi((neighbor)->getAttribute("remote-as")) == _myAS) {
                    _routerInSameASList6.push_back((neighbor)->getAttribute("address"));
                } else {
                    //start EGP sessions

                    Ipv6Address peerAddr = Ipv6Address((neighbor)->getAttribute("address"));

                   if(_rt6->getOutputInterfaceForDestination(peerAddr)->ipv6Data()->getGlblAddress().compare(peerAddr) < 0){
                       delayTab[3] += pos + 0.5;
                   } else {
                       delayTab[3] += pos + 0.5;
                   }

                   SessionId newSessionID = createSession6(EGP, peerAddr.str().c_str());
                   _BGPSessions[newSessionID]->setTimers(delayTab);
                   TcpSocket *socketListenEGP = new TcpSocket();
                   _BGPSessions[newSessionID]->setSocketListen(socketListenEGP);

                   delayTab[3] = saveStartDelay;
                }
            }
        }
    }
}

void Bgp::loadConfigFromXML(cXMLElement *config)
{
    if (strcmp(config->getTagName(), "BGPConfig"))
            throw cRuntimeError("Cannot read BGP configuration, unaccepted '%s' node at %s", config->getTagName(), config->getSourceLocation());

    // load bgp timer parameters informations
    simtime_t delayTab[NB_TIMERS];
    cXMLElement *paramNode = config->getElementByPath("TimerParams");
    if (paramNode == nullptr)
        throw cRuntimeError("BGP Error: No configuration for BGP timer parameters");
    cXMLElementList timerConfig = paramNode->getChildren();
    loadTimerConfig(timerConfig, delayTab);

    //get router configuration - devices section in configuration file
    cXMLElement *devicesNode = config->getElementByPath("Devices");
    cXMLElementList routerList = devicesNode->getChildren();
    //router position in AS from configuration file
    int routerPosition;

    std::map<int, int> routerPositionMap;   //count of routers in specific AS
    int positionRouterInConfig = 1;
    int myPos;

    for (auto & elem : routerList) {
        cXMLElement *tmp = (elem)->getElementByPath("Bgp");
        int tmpAS = atoi((tmp)->getAttribute("as"));

        routerPositionMap[tmpAS]++;

        // read specific router configuration part
        if(strcmp((elem)->getAttribute("name"), getParentModule()->getName()) == 0){
            // set router ID from configuration file
            _rt->setRouterId(Ipv4Address((elem)->getAttribute("id")));
            //parse Interface and static Routes elements for specific router
            routerIntfAndRouteConfig(elem);
            //parse Bgp parameter - configure parameters for BGP protocol, create external sessions;
            cXMLElement *bgpNode = elem->getElementByPath("Bgp");
            simtime_t saveStartDelay = delayTab[3];
            loadBgpNodeConfig(bgpNode, delayTab, positionRouterInConfig);
            delayTab[3] = saveStartDelay;
            myPos = positionRouterInConfig;
            routerPosition = routerPositionMap[_myAS];
        }
        positionRouterInConfig++;
    }

    if (_myAS == 0)
        throw cRuntimeError("BGP Error:  No AS configuration for Router ID: %s", _rt->getRouterId().str().c_str());

    //create IGP Session(s)

    if (_routerInSameASList.size()) {
        unsigned int routerPeerPosition = 1;
        delayTab[3] += myPos;
        for (auto it = _routerInSameASList.begin(); it != _routerInSameASList.end(); it++, routerPeerPosition++) {
            SessionId newSessionID;
            TcpSocket *socketListenIGP = new TcpSocket();
            newSessionID = createSession(IGP, (*it));
            Ipv4Address tmp;
            tmp.set((*it));
            if(_rt->getRouterId().getInt() > tmp.getInt()){
                delayTab[3] += 1;
            }
            delayTab[3] += calculateStartDelay(_routerInSameASList.size(), routerPosition, routerPeerPosition);

            _BGPSessions[newSessionID]->setTimers(delayTab);
            _BGPSessions[newSessionID]->setSocketListen(socketListenIGP);
        }
    }

    //create IGP sessions ipv6
    if (_routerInSameASList6.size()) {
        unsigned int routerPeerPosition = 1;
        delayTab[3] += myPos;
        for (auto it = _routerInSameASList6.begin(); it != _routerInSameASList6.end(); it++, routerPeerPosition++) {
            SessionId newSessionID;
            TcpSocket *socketListenIGP = new TcpSocket();
            newSessionID = createSession6(IGP, (*it));
            delayTab[3] += calculateStartDelay(_routerInSameASList6.size(), routerPosition, routerPeerPosition);
            _BGPSessions[newSessionID]->setTimers(delayTab);
            _BGPSessions[newSessionID]->setSocketListen(socketListenIGP);
        }
    }
}

unsigned int Bgp::calculateStartDelay(int rtListSize, unsigned char rtPosition, unsigned char rtPeerPosition)
{
    unsigned int startDelay = 0;
    if (rtPeerPosition == 1) {
        if (rtPosition == 1) {
            startDelay = 1;
        }
        else {
            startDelay = (rtPosition - 1) * 2;
        }
        return startDelay;
    }

    if (rtPosition < rtPeerPosition) {
        startDelay = 2;
    }
    else if (rtPosition > rtPeerPosition) {
        startDelay = (rtListSize - 1) * 2 - 2 * (rtPeerPosition - 2);
    }
    else {
        startDelay = (rtListSize - 1) * 2 + 1;
    }
    return startDelay;
}

SessionId Bgp::createSession(BgpSessionType typeSession, const char *peerAddr)
{
    BgpSession *newSession = new BgpSession(*this);
    SessionId newSessionId;
    SessionInfo info;

    info.sessionType = typeSession;
    info.ASValue = _myAS;
    info.routerID = _rt->getRouterId();
    info.peerAddr.set(peerAddr);
    info.localAddr = _rt->getInterfaceForDestAddr(info.peerAddr)->getIpv4Address();

    info.linkIntf = _rt->getInterfaceForDestAddr(info.peerAddr);
    if (info.linkIntf == nullptr) {
        throw cRuntimeError("BGP Error: No configuration interface for peer address: %s", peerAddr);
    }
    if (typeSession == EGP) {
//        info.linkIntf = _rt->getInterfaceForDestAddr(info.peerAddr);
//        if (info.linkIntf == nullptr) {
//            throw cRuntimeError("BGP Error: No configuration interface for peer address: %s", peerAddr);
//        }
        info.sessionID = info.peerAddr.getInt() + info.linkIntf->ipv4Data()->getIPAddress().getInt();
    }
    else {
        info.sessionID = info.peerAddr.getInt() + info.routerID.getInt();
    }
    newSessionId = info.sessionID;
    newSession->setInfo(info);
    _BGPSessions[newSessionId] = newSession;

    return newSessionId;
}

SessionId Bgp::createSession6(BgpSessionType typeSession, const char *peerAddr)
{
    BgpSession *newSession = new BgpSession(*this);
    SessionId newSessionId;
    SessionInfo info;
    uint32 *d;
    const uint32 *tmp;

    info.multiAddress = true;
    info.sessionType = typeSession;
    info.ASValue = _myAS;
    info.routerID = _rt->getRouterId();
    info.peerAddr6.set(peerAddr);
    info.localAddr6 = _rt6->getOutputInterfaceForDestination(info.peerAddr6)->ipv6Data()->getGlblAddress();

    info.linkIntf = _rt6->getOutputInterfaceForDestination(info.peerAddr6);
    if (info.linkIntf == nullptr) {
        throw cRuntimeError("BGP Error: No configuration interface for peer address: %s", peerAddr);
    }
    d = info.peerAddr6.words();
    tmp = info.linkIntf->ipv6Data()->getGlblAddress().words();
    //create sessionID for Ipv6, must by higher than Ipv4 ID.
    info.sessionID =  d[0] + d [3] + tmp[0] + tmp[3];
    info.sessionID<<=1;

    newSessionId = info.sessionID;
    newSession->setInfo(info);
    _BGPSessions[newSessionId] = newSession;

    return newSessionId;
}

SessionId Bgp::findIdFromPeerAddr(std::map<SessionId, BgpSession *> sessions, L3Address peerAddr)
{
    for (auto & session : sessions)
    {
        if (peerAddr.getType() == L3Address::IPv4) {
            if ((session).second->getPeerAddr().equals(peerAddr.toIpv4())) {
                    return (session).first;
                }
        } else if (peerAddr.getType() == L3Address::IPv6) {
            if ((session).second->getPeerAddr6() == (peerAddr.toIpv6())) {
                        return (session).first;
            }
        }
    }
    return -1;
}

/*
 *  Delete BGP Routing entry, if the route deleted correctly return true, false else.
 *  Side effects when returns true:
 *      _BGPRoutingTable changed, iterators on _BGPRoutingTable will be invalid.
 */
bool Bgp::deleteBGPRoutingEntry(RoutingTableEntry *entry)
{
    for (auto it = _BGPRoutingTable.begin();
         it != _BGPRoutingTable.end(); it++)
    {
        if (((*it)->getDestination().getInt() & (*it)->getNetmask().getInt()) ==
            (entry->getDestination().getInt() & entry->getNetmask().getInt()))
        {
            _BGPRoutingTable.erase(it);
            _rt->deleteRoute(entry);
            return true;
        }
    }
    return false;
}

bool Bgp::deleteBGPRoutingEntry6(RoutingTableEntry6 *entry)
{
    for (auto it = _BGPRoutingTable6.begin();
         it != _BGPRoutingTable6.end(); it++)
    {
        if (((*it)->getDestPrefix().getPrefix((*it)->getPrefixLength())) ==
            (entry->getDestPrefix().getPrefix(entry->getPrefixLength())))
        {
            _BGPRoutingTable6.erase(it);
            _rt6->deleteRoute(entry);
            return true;
        }
    }
    return false;
}

/*return index of the Ipv4 table if the route is found, -1 else*/
int Bgp::isInRoutingTable(IIpv4RoutingTable *rtTable, Ipv4Address addr)
{
    for (int i = 0; i < rtTable->getNumRoutes(); i++) {
        const Ipv4Route *entry = rtTable->getRoute(i);
        if (Ipv4Address::maskedAddrAreEqual(addr, entry->getDestination(), entry->getNetmask())) {
            return i;
        }
    }
    return -1;
}

int Bgp::isInRoutingTable6(Ipv6RoutingTable *rtTable, Ipv6Address addr)
{
    for (int i = 0; i < rtTable->getNumRoutes(); i++) {
        const Ipv6Route *entry = rtTable->getRoute(i);
        if (addr ==  entry->getDestPrefix()) {
            return i;
        }
    }
    return -1;
}

int Bgp::isInInterfaceTable(IInterfaceTable *ifTable, Ipv4Address addr)
{
    for (int i = 0; i < ifTable->getNumInterfaces(); i++) {
        if (ifTable->getInterface(i)->ipv4Data()->getIPAddress() == addr) {
            return i;
        }
    }
    return -1;
}

int Bgp::isInInterfaceTable6(IInterfaceTable *ifTable, Ipv6Address addr)
{
    for (int i = 0; i < ifTable->getNumInterfaces(); i++) {
        for (int j = 0; j < ifTable->getInterface(i)->ipv6Data()->getNumAddresses(); j++) {
            if (ifTable->getInterface(i)->ipv6Data()->getAddress(j) == addr) {
                return i;
            }
        }
    }
    return -1;
}

SessionId Bgp::findIdFromSocketConnId(std::map<SessionId, BgpSession *> sessions, int connId)
{
    for (auto & session : sessions)
    {
        TcpSocket *socket = (session).second->getSocket();
        if (socket->getSocketId() == connId) {
            return (session).first;
        }
    }
    return -1;
}

/*return index of the table if the route is found, -1 else*/
unsigned long Bgp::isInTable(std::vector<RoutingTableEntry *> rtTable, RoutingTableEntry *entry)
{
    for (unsigned long i = 0; i < rtTable.size(); i++) {
        RoutingTableEntry *entryCur = rtTable[i];
        if ((entry->getDestination().getInt() & entry->getNetmask().getInt()) ==
            (entryCur->getDestination().getInt() & entryCur->getNetmask().getInt()))
        {
            return i;
        }
    }
    return -1;
}

unsigned long Bgp::isInTable6(std::vector<RoutingTableEntry6 *> rtTable, RoutingTableEntry6 *entry)
{
    for (unsigned long i = 0; i < rtTable.size(); i++) {
        RoutingTableEntry6 *entryCur = rtTable[i];
        if (entry->getDestPrefix().getPrefix(entry->getPrefixLength()).compare(entryCur->getDestPrefix().getPrefix(entryCur->getPrefixLength())) == 0)
        {
            return i;
        }
    }
    return -1;
}

/*return true if the AS is found, false else*/
bool Bgp::isInASList(std::vector<AsId> ASList, RoutingTableEntry *entry)
{
    for (auto & elem : ASList) {
        for (unsigned int i = 0; i < entry->getASCount(); i++) {
            if ((elem) == entry->getAS(i)) {
                return true;
            }
        }
    }
    return false;
}

bool Bgp::isInASList6(std::vector<AsId> ASList, RoutingTableEntry6 *entry)
{
    for (auto & elem : ASList) {
        for (unsigned int i = 0; i < entry->getASCount(); i++) {
            if ((elem) == entry->getAS(i)) {
                return true;
            }
        }
    }
    return false;
}

/*return true if OSPF exists, false else*/
bool Bgp::ospfExist(IIpv4RoutingTable *rtTable)
{
    for (int i = 0; i < rtTable->getNumRoutes(); i++) {
        if (rtTable->getRoute(i)->getSourceType() == IRoute::OSPF) {
            return true;
        }
    }
    return false;
}

unsigned char Bgp::asLoopDetection(RoutingTableEntry *entry, AsId myAS)
{
    for (unsigned int i = 1; i < entry->getASCount(); i++) {
        if (myAS == entry->getAS(i)) {
            return ASLOOP_DETECTED;
        }
    }
    return ASLOOP_NO_DETECTED;
}

unsigned char Bgp::asLoopDetection6(RoutingTableEntry6 *entry, AsId myAS)
{
    for (unsigned int i = 1; i < entry->getASCount(); i++) {
        if (myAS == entry->getAS(i)) {
            return ASLOOP_DETECTED;
        }
    }
    return ASLOOP_NO_DETECTED;
}

/*return sessionID if the session is found, -1 else*/
SessionId Bgp::findNextSession(BgpSessionType type, bool startSession)
{
    SessionId sessionID = -1;
    for (auto & elem : _BGPSessions)
    {
        if ((elem).second->getType() == type && !(elem).second->isEstablished()) {
            sessionID = (elem).first;
            break;
        }
    }
//    if (startSession == true && type == IGP && sessionID != static_cast<SessionId>(-1)) {
//        InterfaceEntry *linkIntf;
//        if (_BGPSessions[sessionID]->isMultiAddress())
//            linkIntf = _rt6->getOutputInterfaceForDestination(_BGPSessions[sessionID]->getPeerAddr6());
//        else
//            linkIntf = _rt->getInterfaceForDestAddr(_BGPSessions[sessionID]->getPeerAddr());
//
//        if (linkIntf == nullptr) {
//            if (_BGPSessions[sessionID]->isMultiAddress())
//                throw cRuntimeError("No configuration interface for peer address: %s", _BGPSessions[sessionID]->getPeerAddr6().str().c_str());
//            else
//                throw cRuntimeError("No configuration interface for peer address: %s", _BGPSessions[sessionID]->getPeerAddr().str().c_str());
//        }
//        _BGPSessions[sessionID]->setlinkIntf(linkIntf);
//        _BGPSessions[sessionID]->startConnection();
//    }
    return sessionID;
}

} // namespace bgp

} // namespace inet

