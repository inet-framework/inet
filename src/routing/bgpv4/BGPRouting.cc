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

#include "inet/routing/bgpv4/BGPRouting.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/routing/ospfv2/OSPFRouting.h"
#include "inet/routing/bgpv4/BGPSession.h"

namespace inet {

namespace bgp {

Define_Module(BGPRouting);

BGPRouting::~BGPRouting(void)
{
    for (std::map<SessionID, BGPSession *>::iterator sessionIterator = _BGPSessions.begin();
         sessionIterator != _BGPSessions.end(); sessionIterator++)
    {
        (*sessionIterator).second->~BGPSession();
    }
    _BGPRoutingTable.erase(_BGPRoutingTable.begin(), _BGPRoutingTable.end());
    _prefixListIN.erase(_prefixListIN.begin(), _prefixListIN.end());
    _prefixListOUT.erase(_prefixListOUT.begin(), _prefixListOUT.end());
}

void BGPRouting::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_ROUTING_PROTOCOLS) {
        bool isOperational;
        NodeStatus *nodeStatus = dynamic_cast<NodeStatus *>(findContainingNode(this)->getSubmodule("status"));
        isOperational = (!nodeStatus) || nodeStatus->getState() == NodeStatus::UP;
        if (!isOperational)
            throw cRuntimeError("This module doesn't support starting in node DOWN state");

        // we must wait until IPv4RoutingTable is completely initialized
        _rt = getModuleFromPar<IIPv4RoutingTable>(par("routingTableModule"), this);
        _inft = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);

        // read BGP configuration
        cXMLElement *bgpConfig = par("bgpConfig").xmlValue();
        loadConfigFromXML(bgpConfig);
        createWatch("myAutonomousSystem", _myAS);
        WATCH_PTRVECTOR(_BGPRoutingTable);
    }
}

void BGPRouting::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage()) {    //BGP level
        handleTimer(msg);
    }
    else if (!strcmp(msg->getArrivalGate()->getName(), "tcpIn")) {    //TCP level
        processMessageFromTCP(msg);
    }
    else {
        delete msg;
    }
}

void BGPRouting::handleTimer(cMessage *timer)
{
    BGPSession *pSession = (BGPSession *)timer->getContextPointer();
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

bool BGPRouting::handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback)
{
    throw cRuntimeError("Lifecycle operation support not implemented");
}

void BGPRouting::finish()
{
    unsigned int statTab[NB_STATS] = {
        0, 0, 0, 0, 0, 0
    };
    for (std::map<SessionID, BGPSession *>::iterator sessionIterator = _BGPSessions.begin(); sessionIterator != _BGPSessions.end(); sessionIterator++) {
        (*sessionIterator).second->getStatistics(statTab);
    }
    recordScalar("OPENMsgSent", statTab[0]);
    recordScalar("OPENMsgRecv", statTab[1]);
    recordScalar("KeepAliveMsgSent", statTab[2]);
    recordScalar("KeepAliveMsgRcv", statTab[3]);
    recordScalar("UpdateMsgSent", statTab[4]);
    recordScalar("UpdateMsgRcv", statTab[5]);
}

void BGPRouting::listenConnectionFromPeer(SessionID sessionID)
{
    if (_BGPSessions[sessionID]->getSocketListen()->getState() == TCPSocket::CLOSED) {
        //session StartDelayTime error, it's anormal that listenSocket is closed.
        _socketMap.removeSocket(_BGPSessions[sessionID]->getSocketListen());
        _BGPSessions[sessionID]->getSocketListen()->abort();
        _BGPSessions[sessionID]->getSocketListen()->renewSocket();
    }
    if (_BGPSessions[sessionID]->getSocketListen()->getState() != TCPSocket::LISTENING) {
        _BGPSessions[sessionID]->getSocketListen()->setOutputGate(gate("tcpOut"));
        _BGPSessions[sessionID]->getSocketListen()->readDataTransferModePar(*this);
        _BGPSessions[sessionID]->getSocketListen()->bind(TCP_PORT);
        _BGPSessions[sessionID]->getSocketListen()->listen();
    }
}

void BGPRouting::openTCPConnectionToPeer(SessionID sessionID)
{
    InterfaceEntry *intfEntry = _BGPSessions[sessionID]->getLinkIntf();
    TCPSocket *socket = _BGPSessions[sessionID]->getSocket();
    if (socket->getState() != TCPSocket::NOT_BOUND) {
        _socketMap.removeSocket(socket);
        socket->abort();
        socket->renewSocket();
    }
    socket->setCallbackObject(this, (void *)sessionID);
    socket->setOutputGate(gate("tcpOut"));
    socket->readDataTransferModePar(*this);
    socket->bind(intfEntry->ipv4Data()->getIPAddress(), 0);
    _socketMap.addSocket(socket);

    socket->connect(_BGPSessions[sessionID]->getPeerAddr(), TCP_PORT);
}

void BGPRouting::processMessageFromTCP(cMessage *msg)
{
    TCPSocket *socket = _socketMap.findSocketFor(msg);
    if (!socket) {
        socket = new TCPSocket(msg);
        socket->readDataTransferModePar(*this);
        socket->setOutputGate(gate("tcpOut"));
        IPv4Address peerAddr = socket->getRemoteAddress().toIPv4();
        SessionID i = findIdFromPeerAddr(_BGPSessions, peerAddr);
        if (i == (SessionID)-1) {
            socket->close();
            delete socket;
            delete msg;
            return;
        }
        socket->setCallbackObject(this, (void *)i);

        _socketMap.addSocket(socket);
        _BGPSessions[i]->getSocket()->abort();
        _BGPSessions[i]->setSocket(socket);
    }

    socket->processMessage(msg);
}

void BGPRouting::socketEstablished(int connId, void *yourPtr)
{
    _currSessionId = findIdFromSocketConnId(_BGPSessions, connId);
    if (_currSessionId == (SessionID)-1) {
        throw cRuntimeError("socket id=%d is not established", connId);
    }

    //if it's an IGP Session, TCPConnectionConfirmed only if all EGP Sessions established
    if (_BGPSessions[_currSessionId]->getType() == IGP &&
        this->findNextSession(EGP) != (SessionID)-1)
    {
        _BGPSessions[_currSessionId]->getFSM()->TcpConnectionFails();
    }
    else {
        _BGPSessions[_currSessionId]->getFSM()->TcpConnectionConfirmed();
        _BGPSessions[_currSessionId]->getSocketListen()->abort();
    }

    if (_BGPSessions[_currSessionId]->getSocketListen()->getConnectionId() != connId &&
        _BGPSessions[_currSessionId]->getType() == EGP &&
        this->findNextSession(EGP) != (SessionID)-1)
    {
        _BGPSessions[_currSessionId]->getSocketListen()->abort();
    }
}

void BGPRouting::socketDataArrived(int connId, void *yourPtr, cPacket *msg, bool urgent)
{
    _currSessionId = findIdFromSocketConnId(_BGPSessions, connId);
    if (_currSessionId != (SessionID)-1) {
        BGPHeader *ptrHdr = check_and_cast<BGPHeader *>(msg);
        switch (ptrHdr->getType()) {
            case BGP_OPEN:
                //BGPOpenMessage* ptrMsg = check_and_cast<BGPOpenMessage*>(msg);
                processMessage(*check_and_cast<BGPOpenMessage *>(msg));
                break;

            case BGP_KEEPALIVE:
                processMessage(*check_and_cast<BGPKeepAliveMessage *>(msg));
                break;

            case BGP_UPDATE:
                processMessage(*check_and_cast<BGPUpdateMessage *>(msg));
                break;

            default:
                throw cRuntimeError("Invalid BGP message type %d", ptrHdr->getType());
        }
    }
    delete msg;
}

void BGPRouting::socketFailure(int connId, void *yourPtr, int code)
{
    _currSessionId = findIdFromSocketConnId(_BGPSessions, connId);
    if (_currSessionId != (SessionID)-1) {
        _BGPSessions[_currSessionId]->getFSM()->TcpConnectionFails();
    }
}

void BGPRouting::processMessage(const BGPOpenMessage& msg)
{
    EV_INFO << "Processing BGP OPEN message" << std::endl;
    _BGPSessions[_currSessionId]->getFSM()->OpenMsgEvent();
}

void BGPRouting::processMessage(const BGPKeepAliveMessage& msg)
{
    EV_INFO << "Processing BGP Keep Alive message" << std::endl;
    _BGPSessions[_currSessionId]->getFSM()->KeepAliveMsgEvent();
}

void BGPRouting::processMessage(const BGPUpdateMessage& msg)
{
    EV_INFO << "Processing BGP Update message" << std::endl;
    _BGPSessions[_currSessionId]->getFSM()->UpdateMsgEvent();

    unsigned char decisionProcessResult;
    IPv4Address netMask(IPv4Address::ALLONES_ADDRESS);
    RoutingTableEntry *entry = new RoutingTableEntry();
    const unsigned char length = msg.getNLRI().length;
    unsigned int ASValueCount = msg.getPathAttributeList(0).getAsPath(0).getValue(0).getAsValueArraySize();

    entry->setDestination(msg.getNLRI().prefix);
    netMask = IPv4Address::makeNetmask(length);
    entry->setNetmask(netMask);
    for (unsigned int j = 0; j < ASValueCount; j++) {
        entry->addAS(msg.getPathAttributeList(0).getAsPath(0).getValue(0).getAsValue(j));
    }

    decisionProcessResult = asLoopDetection(entry, _myAS);

    if (decisionProcessResult == ASLOOP_NO_DETECTED) {
        // RFC 4271, 9.1.  Decision Process
        decisionProcessResult = decisionProcess(msg, entry, _currSessionId);
        //RFC 4271, 9.2.  Update-Send Process
        if (decisionProcessResult != 0) {
            updateSendProcess(decisionProcessResult, _currSessionId, entry);
        }
    }
}

unsigned char BGPRouting::decisionProcess(const BGPUpdateMessage& msg, RoutingTableEntry *entry, SessionID sessionIndex)
{
    //Don't add the route if it exists in PrefixListINTable or in ASListINTable
    if (isInTable(_prefixListIN, entry) != (unsigned long)-1 || isInASList(_ASListIN, entry)) {
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
            return 0;
        }
        else {
            entry->setInterface(_BGPSessions[sessionIndex]->getLinkIntf());
            _BGPRoutingTable.push_back(entry);
            _rt->addRoute(entry);
            return ROUTE_DESTINATION_CHANGED;
        }
    }

    //Don't add the route if it exists in IPv4 routing table except if the msg come from IGP session
    int indexIP = isInRoutingTable(_rt, entry->getDestination());
    if (indexIP != -1 && _rt->getRoute(indexIP)->getSourceType() != IRoute::BGP) {
        if (_BGPSessions[sessionIndex]->getType() != IGP) {
            return 0;
        }
        else {
            IPv4Route *newEntry = new IPv4Route;
            newEntry->setDestination(_rt->getRoute(indexIP)->getDestination());
            newEntry->setNetmask(_rt->getRoute(indexIP)->getNetmask());
            newEntry->setGateway(_rt->getRoute(indexIP)->getGateway());
            newEntry->setInterface(_rt->getRoute(indexIP)->getInterface());
            newEntry->setSourceType(IRoute::BGP);
            _rt->deleteRoute(_rt->getRoute(indexIP));
            _rt->addRoute(newEntry);
        }
    }

    entry->setInterface(_BGPSessions[sessionIndex]->getLinkIntf());
    _BGPRoutingTable.push_back(entry);

    if (_BGPSessions[sessionIndex]->getType() == EGP) {
        std::string entryh = entry->getDestination().str();
        std::string entryn = entry->getNetmask().str();
        _rt->addRoute(entry);
        //insertExternalRoute on OSPF ExternalRoutingTable if OSPF exist on this BGP router
        if (ospfExist(_rt)) {
            ospf::IPv4AddressRange OSPFnetAddr;
            OSPFnetAddr.address = entry->getDestination();
            OSPFnetAddr.mask = entry->getNetmask();
            ospf::OSPFRouting *ospf = findModuleFromPar<ospf::OSPFRouting>(par("ospfRoutingModule"), this);
            InterfaceEntry *ie = entry->getInterface();
            if (!ie)
                throw cRuntimeError("Model error: interface entry is NULL");
            ospf->insertExternalRoute(ie->getInterfaceId(), OSPFnetAddr);
            simulation.setContext(this);
        }
    }
    return NEW_ROUTE_ADDED;
}

bool BGPRouting::tieBreakingProcess(RoutingTableEntry *oldEntry, RoutingTableEntry *entry)
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

void BGPRouting::updateSendProcess(const unsigned char type, SessionID sessionIndex, RoutingTableEntry *entry)
{
    //Don't send the update Message if the route exists in listOUTTable
    //SESSION = EGP : send an update message to all BGP Peer (EGP && IGP)
    //if it is not the currentSession and if the session is already established
    //SESSION = IGP : send an update message to External BGP Peer (EGP) only
    //if it is not the currentSession and if the session is already established
    for (std::map<SessionID, BGPSession *>::iterator sessionIt = _BGPSessions.begin();
         sessionIt != _BGPSessions.end(); sessionIt++)
    {
        if (isInTable(_prefixListOUT, entry) != (unsigned long)-1 || isInASList(_ASListOUT, entry) ||
            ((*sessionIt).first == sessionIndex && type != NEW_SESSION_ESTABLISHED) ||
            (type == NEW_SESSION_ESTABLISHED && (*sessionIt).first != sessionIndex) ||
            !(*sessionIt).second->isEstablished())
        {
            continue;
        }
        if ((_BGPSessions[sessionIndex]->getType() == IGP && (*sessionIt).second->getType() == EGP) ||
            _BGPSessions[sessionIndex]->getType() == EGP ||
            type == ROUTE_DESTINATION_CHANGED ||
            type == NEW_SESSION_ESTABLISHED)
        {
            BGPUpdateNLRI NLRI;
            BGPUpdatePathAttributeList content;

            unsigned int nbAS = entry->getASCount();
            content.setAsPathArraySize(1);
            content.getAsPath(0).setValueArraySize(1);
            content.getAsPath(0).getValue(0).setType(AS_SEQUENCE);
            //RFC 4271 : set My AS in first position if it is not already
            if (entry->getAS(0) != _myAS) {
                content.getAsPath(0).getValue(0).setAsValueArraySize(nbAS + 1);
                content.getAsPath(0).getValue(0).setLength(1);
                content.getAsPath(0).getValue(0).setAsValue(0, _myAS);
                for (unsigned int j = 1; j < nbAS + 1; j++) {
                    content.getAsPath(0).getValue(0).setAsValue(j, entry->getAS(j - 1));
                }
            }
            else {
                content.getAsPath(0).getValue(0).setAsValueArraySize(nbAS);
                content.getAsPath(0).getValue(0).setLength(1);
                for (unsigned int j = 0; j < nbAS; j++) {
                    content.getAsPath(0).getValue(0).setAsValue(j, entry->getAS(j));
                }
            }

            InterfaceEntry *iftEntry = (*sessionIt).second->getLinkIntf();
            content.getOrigin().setValue((*sessionIt).second->getType());
            content.getNextHop().setValue(iftEntry->ipv4Data()->getIPAddress());
            IPv4Address netMask = entry->getNetmask();
            NLRI.prefix = entry->getDestination().doAnd(netMask);
            NLRI.length = (unsigned char)netMask.getNetmaskLength();
            {
                BGPUpdateMessage *updateMsg = new BGPUpdateMessage("BGPUpdate");
                updateMsg->setPathAttributeListArraySize(1);
                updateMsg->setPathAttributeList(content);
                updateMsg->setNLRI(NLRI);
                (*sessionIt).second->getSocket()->send(updateMsg);
                (*sessionIt).second->addUpdateMsgSent();
            }
        }
    }
}

bool BGPRouting::checkExternalRoute(const IPv4Route *route)
{
    IPv4Address OSPFRoute;
    OSPFRoute = route->getDestination();
    ospf::OSPFRouting *ospf = findModuleFromPar<ospf::OSPFRouting>(par("ospfRoutingModule"), this);
    bool returnValue = ospf->checkExternalRoute(OSPFRoute);
    simulation.setContext(this);
    return returnValue;
}

void BGPRouting::loadTimerConfig(cXMLElementList& timerConfig, simtime_t *delayTab)
{
    for (cXMLElementList::iterator timerElemIt = timerConfig.begin(); timerElemIt != timerConfig.end(); timerElemIt++) {
        std::string nodeName = (*timerElemIt)->getTagName();
        if (nodeName == "connectRetryTime") {
            delayTab[0] = (double)atoi((*timerElemIt)->getNodeValue());
        }
        else if (nodeName == "holdTime") {
            delayTab[1] = (double)atoi((*timerElemIt)->getNodeValue());
        }
        else if (nodeName == "keepAliveTime") {
            delayTab[2] = (double)atoi((*timerElemIt)->getNodeValue());
        }
        else if (nodeName == "startDelay") {
            delayTab[3] = (double)atoi((*timerElemIt)->getNodeValue());
        }
    }
}

ASID BGPRouting::findMyAS(cXMLElementList& asList, int& outRouterPosition)
{
    // find my own IPv4 address in the configuration file and return the AS id under which it is configured
    // and also the 1 based position of the entry inside the AS config element
    for (cXMLElementList::iterator asListIt = asList.begin(); asListIt != asList.end(); asListIt++) {
        cXMLElementList routerList = (*asListIt)->getChildrenByTagName("Router");
        outRouterPosition = 1;
        for (cXMLElementList::iterator routerListIt = routerList.begin(); routerListIt != routerList.end(); routerListIt++) {
            IPv4Address routerAddr = IPv4Address((*routerListIt)->getAttribute("interAddr"));
            for (int i = 0; i < _inft->getNumInterfaces(); i++) {
                if (_inft->getInterface(i)->ipv4Data()->getIPAddress() == routerAddr)
                    return atoi((*routerListIt)->getParentNode()->getAttribute("id"));
            }
            outRouterPosition++;
        }
    }

    return 0;
}

void BGPRouting::loadSessionConfig(cXMLElementList& sessionList, simtime_t *delayTab)
{
    simtime_t saveStartDelay = delayTab[3];
    for (cXMLElementList::iterator sessionListIt = sessionList.begin(); sessionListIt != sessionList.end(); sessionListIt++, delayTab[3] = saveStartDelay) {
        const char *exterAddr = (*sessionListIt)->getFirstChild()->getAttribute("exterAddr");
        IPv4Address routerAddr1 = IPv4Address(exterAddr);
        exterAddr = (*sessionListIt)->getLastChild()->getAttribute("exterAddr");
        IPv4Address routerAddr2 = IPv4Address(exterAddr);
        if (isInInterfaceTable(_inft, routerAddr1) == -1 && isInInterfaceTable(_inft, routerAddr2) == -1) {
            continue;
        }
        IPv4Address peerAddr;
        if (isInInterfaceTable(_inft, routerAddr1) != -1) {
            peerAddr = routerAddr2;
            delayTab[3] += atoi((*sessionListIt)->getAttribute("id"));
        }
        else {
            peerAddr = routerAddr1;
            delayTab[3] += atoi((*sessionListIt)->getAttribute("id")) + 0.5;
        }
        if (peerAddr.isUnspecified()) {
            throw cRuntimeError("BGP Error: No valid external address for session ID : %s", (*sessionListIt)->getAttribute("id"));
        }

        SessionID newSessionID = createSession(EGP, peerAddr.str().c_str());
        _BGPSessions[newSessionID]->setTimers(delayTab);
        TCPSocket *socketListenEGP = new TCPSocket();
        _BGPSessions[newSessionID]->setSocketListen(socketListenEGP);
    }
}

std::vector<const char *> BGPRouting::loadASConfig(cXMLElementList& ASConfig)
{
    //create deny Lists
    std::vector<const char *> routerInSameASList;

    for (cXMLElementList::iterator ASConfigIt = ASConfig.begin(); ASConfigIt != ASConfig.end(); ASConfigIt++) {
        std::string nodeName = (*ASConfigIt)->getTagName();
        if (nodeName == "Router") {
            if (isInInterfaceTable(_inft, IPv4Address((*ASConfigIt)->getAttribute("interAddr"))) == -1) {
                routerInSameASList.push_back((*ASConfigIt)->getAttribute("interAddr"));
            }
            continue;
        }
        if (nodeName == "DenyRoute" || nodeName == "DenyRouteIN" || nodeName == "DenyRouteOUT") {
            RoutingTableEntry *entry = new RoutingTableEntry();
            entry->setDestination(IPv4Address((*ASConfigIt)->getAttribute("Address")));
            entry->setNetmask(IPv4Address((*ASConfigIt)->getAttribute("Netmask")));
            if (nodeName == "DenyRouteIN") {
                _prefixListIN.push_back(entry);
            }
            else if (nodeName == "DenyRouteOUT") {
                _prefixListOUT.push_back(entry);
            }
            else {
                _prefixListIN.push_back(entry);
                _prefixListOUT.push_back(entry);
            }
        }
        else if (nodeName == "DenyAS" || nodeName == "DenyASIN" || nodeName == "DenyASOUT") {
            ASID ASCur = atoi((*ASConfigIt)->getNodeValue());
            if (nodeName == "DenyASIN") {
                _ASListIN.push_back(ASCur);
            }
            else if (nodeName == "DenyASOUT") {
                _ASListOUT.push_back(ASCur);
            }
            else {
                _ASListIN.push_back(ASCur);
                _ASListOUT.push_back(ASCur);
            }
        }
        else {
            throw cRuntimeError("BGP Error: unknown element named '%s' for AS %u", nodeName.c_str(), _myAS);
        }
    }
    return routerInSameASList;
}

void BGPRouting::loadConfigFromXML(cXMLElement *bgpConfig)
{
    if (strcmp(bgpConfig->getTagName(), "BGPConfig"))
        throw cRuntimeError("Cannot read BGP configuration, unaccepted '%s' node at %s", bgpConfig->getTagName(), bgpConfig->getSourceLocation());

    // load bgp timer parameters informations
    simtime_t delayTab[NB_TIMERS];
    cXMLElement *paramNode = bgpConfig->getElementByPath("TimerParams");
    if (paramNode == NULL)
        throw cRuntimeError("BGP Error: No configuration for BGP timer parameters");

    cXMLElementList timerConfig = paramNode->getChildren();
    loadTimerConfig(timerConfig, delayTab);

    //find my AS
    cXMLElementList ASList = bgpConfig->getElementsByTagName("AS");
    int routerPosition;
    _myAS = findMyAS(ASList, routerPosition);
    if (_myAS == 0)
        throw cRuntimeError("BGP Error:  No AS configuration for Router ID: %s", _rt->getRouterId().str().c_str());

    // load EGP Session informations
    cXMLElementList sessionList = bgpConfig->getElementsByTagName("Session");
    simtime_t saveStartDelay = delayTab[3];
    loadSessionConfig(sessionList, delayTab);
    delayTab[3] = saveStartDelay;

    // load AS information
    char ASXPath[32];
    sprintf(ASXPath, "AS[@id='%d']", _myAS);

    cXMLElement *ASNode = bgpConfig->getElementByPath(ASXPath);
    std::vector<const char *> routerInSameASList;
    if (ASNode == NULL)
        throw cRuntimeError("BGP Error:  No configuration for AS ID: %d", _myAS);

    cXMLElementList ASConfig = ASNode->getChildren();
    routerInSameASList = loadASConfig(ASConfig);

    //create IGP Session(s)
    if (routerInSameASList.size()) {
        unsigned int routerPeerPosition = 1;
        delayTab[3] += sessionList.size() * 2;
        for (std::vector<const char *>::iterator it = routerInSameASList.begin(); it != routerInSameASList.end(); it++, routerPeerPosition++) {
            SessionID newSessionID;
            TCPSocket *socketListenIGP = new TCPSocket();
            newSessionID = createSession(IGP, (*it));
            delayTab[3] += calculateStartDelay(routerInSameASList.size(), routerPosition, routerPeerPosition);
            _BGPSessions[newSessionID]->setTimers(delayTab);
            _BGPSessions[newSessionID]->setSocketListen(socketListenIGP);
        }
    }
}

unsigned int BGPRouting::calculateStartDelay(int rtListSize, unsigned char rtPosition, unsigned char rtPeerPosition)
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

SessionID BGPRouting::createSession(BGPSessionType typeSession, const char *peerAddr)
{
    BGPSession *newSession = new BGPSession(*this);
    SessionID newSessionId;
    SessionInfo info;

    info.sessionType = typeSession;
    info.ASValue = _myAS;
    info.routerID = _rt->getRouterId();
    info.peerAddr.set(peerAddr);
    if (typeSession == EGP) {
        info.linkIntf = _rt->getInterfaceForDestAddr(info.peerAddr);
        if (info.linkIntf == 0) {
            throw cRuntimeError("BGP Error: No configuration interface for peer address: %s", peerAddr);
        }
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

SessionID BGPRouting::findIdFromPeerAddr(std::map<SessionID, BGPSession *> sessions, IPv4Address peerAddr)
{
    for (std::map<SessionID, BGPSession *>::iterator sessionIterator = sessions.begin();
         sessionIterator != sessions.end(); sessionIterator++)
    {
        if ((*sessionIterator).second->getPeerAddr().equals(peerAddr)) {
            return (*sessionIterator).first;
        }
    }
    return -1;
}

/*delete BGP Routing entry, if the route deleted correctly return true, false else*/
bool BGPRouting::deleteBGPRoutingEntry(RoutingTableEntry *entry)
{
    for (std::vector<RoutingTableEntry *>::iterator it = _BGPRoutingTable.begin();
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

/*return index of the IPv4 table if the route is found, -1 else*/
int BGPRouting::isInRoutingTable(IIPv4RoutingTable *rtTable, IPv4Address addr)
{
    for (int i = 0; i < rtTable->getNumRoutes(); i++) {
        const IPv4Route *entry = rtTable->getRoute(i);
        if (IPv4Address::maskedAddrAreEqual(addr, entry->getDestination(), entry->getNetmask())) {
            return i;
        }
    }
    return -1;
}

int BGPRouting::isInInterfaceTable(IInterfaceTable *ifTable, IPv4Address addr)
{
    for (int i = 0; i < ifTable->getNumInterfaces(); i++) {
        if (ifTable->getInterface(i)->ipv4Data()->getIPAddress() == addr) {
            return i;
        }
    }
    return -1;
}

SessionID BGPRouting::findIdFromSocketConnId(std::map<SessionID, BGPSession *> sessions, int connId)
{
    for (std::map<SessionID, BGPSession *>::iterator sessionIterator = sessions.begin();
         sessionIterator != sessions.end(); sessionIterator++)
    {
        TCPSocket *socket = (*sessionIterator).second->getSocket();
        if (socket->getConnectionId() == connId) {
            return (*sessionIterator).first;
        }
    }
    return -1;
}

/*return index of the table if the route is found, -1 else*/
unsigned long BGPRouting::isInTable(std::vector<RoutingTableEntry *> rtTable, RoutingTableEntry *entry)
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

/*return true if the AS is found, false else*/
bool BGPRouting::isInASList(std::vector<ASID> ASList, RoutingTableEntry *entry)
{
    for (std::vector<ASID>::iterator it = ASList.begin(); it != ASList.end(); it++) {
        for (unsigned int i = 0; i < entry->getASCount(); i++) {
            if ((*it) == entry->getAS(i)) {
                return true;
            }
        }
    }
    return false;
}

/*return true if OSPF exists, false else*/
bool BGPRouting::ospfExist(IIPv4RoutingTable *rtTable)
{
    for (int i = 0; i < rtTable->getNumRoutes(); i++) {
        if (rtTable->getRoute(i)->getSourceType() == IRoute::OSPF) {
            return true;
        }
    }
    return false;
}

unsigned char BGPRouting::asLoopDetection(RoutingTableEntry *entry, ASID myAS)
{
    for (unsigned int i = 1; i < entry->getASCount(); i++) {
        if (myAS == entry->getAS(i)) {
            return ASLOOP_DETECTED;
        }
    }
    return ASLOOP_NO_DETECTED;
}

/*return sessionID if the session is found, -1 else*/
SessionID BGPRouting::findNextSession(BGPSessionType type, bool startSession)
{
    SessionID sessionID = -1;
    for (std::map<SessionID, BGPSession *>::iterator sessionIterator = _BGPSessions.begin();
         sessionIterator != _BGPSessions.end(); sessionIterator++)
    {
        if ((*sessionIterator).second->getType() == type && !(*sessionIterator).second->isEstablished()) {
            sessionID = (*sessionIterator).first;
            break;
        }
    }
    if (startSession == true && type == IGP && sessionID != (SessionID)-1) {
        InterfaceEntry *linkIntf = _rt->getInterfaceForDestAddr(_BGPSessions[sessionID]->getPeerAddr());
        if (linkIntf == 0) {
            throw cRuntimeError("No configuration interface for peer address: %s", _BGPSessions[sessionID]->getPeerAddr().str().c_str());
        }
        _BGPSessions[sessionID]->setlinkIntf(linkIntf);
        _BGPSessions[sessionID]->startConnection();
    }
    return sessionID;
}

} // namespace bgp

} // namespace inet

