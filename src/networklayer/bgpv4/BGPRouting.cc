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

#include "BGPRouting.h"
#include "RoutingTableAccess.h"
#include "OSPFRouting.h"
#include "BGPSession.h"

Define_Module(BGPRouting);

BGPRouting::~BGPRouting(void)
{
    for (std::map<BGP::SessionID, BGPSession*>::iterator sessionIterator = _BGPSessions.begin();
        sessionIterator != _BGPSessions.end(); sessionIterator ++)
    {
        (*sessionIterator).second->~BGPSession();
    }
    _BGPRoutingTable.erase(_BGPRoutingTable.begin(), _BGPRoutingTable.end());
    _prefixListIN.erase(_prefixListIN.begin(), _prefixListIN.end());
    _prefixListOUT.erase(_prefixListOUT.begin(), _prefixListOUT.end());
}

void BGPRouting::initialize(int stage)
{
    if (stage==4) // we must wait until RoutingTable is completely initialized
    {
        _rt = RoutingTableAccess().get();
        _inft = InterfaceTableAccess().get();

        // read BGP configuration
        cXMLElement *bgpConfig = par("bgpConfig").xmlValue();
        loadConfigFromXML(bgpConfig);
        createWatch("myAutonomousSystem", _myAS);
        WATCH_PTRVECTOR(_BGPRoutingTable);
    }
}

void BGPRouting::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage()) //BGP level
    {
        handleTimer(msg);
    }
    else if (!strcmp(msg->getArrivalGate()->getName(), "tcpIn")) //TCP level
    {
        processMessageFromTCP(msg);
    }
    else
    {
        delete msg;
    }
}

void BGPRouting::handleTimer(cMessage *timer)
{
    BGPSession* pSession = (BGPSession*)timer->getContextPointer();
    if (pSession)
    {
        switch (timer->getKind())
        {
            case BGP::START_EVENT_KIND:
                EV << "Processing Start Event" << std::endl;
                pSession->getFSM()->ManualStart();
                break;
            case BGP::CONNECT_RETRY_KIND:
                EV << "Expiring Connect Retry Timer" << std::endl;
                pSession->getFSM()->ConnectRetryTimer_Expires();
                break;
            case BGP::HOLD_TIME_KIND:
                EV << "Expiring Hold Timer" << std::endl;
                pSession->getFSM()->HoldTimer_Expires();
                break;
            case BGP::KEEP_ALIVE_KIND:
                EV << "Expiring Keep Alive timer" << std::endl;
                pSession->getFSM()->KeepaliveTimer_Expires();
                break;
            default :
                throw cRuntimeError("Invalid timer kind %d", timer->getKind());
        }
    }
}

void BGPRouting::finish()
{
    unsigned int statTab[BGP::NB_STATS] = {0, 0, 0, 0, 0, 0};
    for (std::map<BGP::SessionID, BGPSession*>::iterator sessionIterator = _BGPSessions.begin(); sessionIterator != _BGPSessions.end(); sessionIterator ++)
    {
        (*sessionIterator).second->getStatistics(statTab);
    }
    recordScalar("OPENMsgSent", statTab[0]);
    recordScalar("OPENMsgRecv", statTab[1]);
    recordScalar("KeepAliveMsgSent", statTab[2]);
    recordScalar("KeepAliveMsgRcv", statTab[3]);
    recordScalar("UpdateMsgSent", statTab[4]);
    recordScalar("UpdateMsgRcv", statTab[5]);
}

void BGPRouting::listenConnectionFromPeer(BGP::SessionID sessionID)
{
    if (_BGPSessions[sessionID]->getSocketListen()->getState() == TCPSocket::CLOSED)
    {
        //session StartDelayTime error, it's anormal that listenSocket is closed.
        _socketMap.removeSocket(_BGPSessions[sessionID]->getSocketListen());
        _BGPSessions[sessionID]->getSocketListen()->abort();
        _BGPSessions[sessionID]->getSocketListen()->renewSocket();
    }
    if (_BGPSessions[sessionID]->getSocketListen()->getState() != TCPSocket::LISTENING)
    {
        _BGPSessions[sessionID]->getSocketListen()->setOutputGate(gate("tcpOut"));
        _BGPSessions[sessionID]->getSocketListen()->readDataTransferModePar(*this);
        _BGPSessions[sessionID]->getSocketListen()->bind(BGP::TCP_PORT);
        _BGPSessions[sessionID]->getSocketListen()->listen();
    }
}

void BGPRouting::openTCPConnectionToPeer(BGP::SessionID sessionID)
{
    InterfaceEntry* intfEntry = _BGPSessions[sessionID]->getLinkIntf();
    TCPSocket *socket = _BGPSessions[sessionID]->getSocket();
    if (socket->getState() != TCPSocket::NOT_BOUND)
    {
        _socketMap.removeSocket(socket);
        socket->abort();
        socket->renewSocket();
    }
    socket->setCallbackObject(this, (void*)sessionID);
    socket->setOutputGate(gate("tcpOut"));
    socket->readDataTransferModePar(*this);
    socket->bind(intfEntry->ipv4Data()->getIPAddress(), 0);
    _socketMap.addSocket(socket);

    socket->connect(_BGPSessions[sessionID]->getPeerAddr(), BGP::TCP_PORT);
}

void BGPRouting::processMessageFromTCP(cMessage *msg)
{
    TCPSocket *socket = _socketMap.findSocketFor(msg);
    if (!socket)
    {
        socket = new TCPSocket(msg);
        socket->readDataTransferModePar(*this);
        socket->setOutputGate(gate("tcpOut"));
        IPv4Address peerAddr = socket->getRemoteAddress().get4();
        BGP::SessionID i = findIdFromPeerAddr(_BGPSessions, peerAddr);
        if (i == (BGP::SessionID)-1)
        {
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
    if (_currSessionId == (BGP::SessionID)-1)
    {
        error("socket id=%d is not established", connId);
    }

    //if it's an IGP Session, TCPConnectionConfirmed only if all EGP Sessions established
    if (_BGPSessions[_currSessionId]->getType() == BGP::IGP &&
        this->findNextSession(BGP::EGP) != (BGP::SessionID)-1)
    {
        _BGPSessions[_currSessionId]->getFSM()->TcpConnectionFails();
    }
    else
    {
        _BGPSessions[_currSessionId]->getFSM()->TcpConnectionConfirmed();
        _BGPSessions[_currSessionId]->getSocketListen()->abort();
    }

    if (_BGPSessions[_currSessionId]->getSocketListen()->getConnectionId() != connId &&
        _BGPSessions[_currSessionId]->getType() == BGP::EGP &&
        this->findNextSession(BGP::EGP) != (BGP::SessionID)-1 )
    {
        _BGPSessions[_currSessionId]->getSocketListen()->abort();
    }
}

void BGPRouting::socketDataArrived(int connId, void *yourPtr, cPacket *msg, bool urgent)
{
    _currSessionId = findIdFromSocketConnId(_BGPSessions, connId);
    if (_currSessionId != (BGP::SessionID)-1)
    {
        BGPHeader* ptrHdr = check_and_cast<BGPHeader*>(msg);
        switch (ptrHdr->getType())
        {
            case BGP_OPEN:
                //BGPOpenMessage* ptrMsg = check_and_cast<BGPOpenMessage*>(msg);
                processMessage(*check_and_cast<BGPOpenMessage*>(msg));
                break;
            case BGP_KEEPALIVE:
                processMessage(*check_and_cast<BGPKeepAliveMessage*>(msg));
                break;
            case BGP_UPDATE:
                processMessage(*check_and_cast<BGPUpdateMessage*>(msg));
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
    if (_currSessionId != (BGP::SessionID)-1)
    {
        _BGPSessions[_currSessionId]->getFSM()->TcpConnectionFails();
    }
}

void BGPRouting::processMessage(const BGPOpenMessage& msg)
{
    EV << "Processing BGP OPEN message" << std::endl;
    _BGPSessions[_currSessionId]->getFSM()->OpenMsgEvent();
}

void BGPRouting::processMessage(const BGPKeepAliveMessage& msg)
{
    EV << "Processing BGP Keep Alive message" << std::endl;
    _BGPSessions[_currSessionId]->getFSM()->KeepAliveMsgEvent();
}

void BGPRouting::processMessage(const BGPUpdateMessage& msg)
{
    EV << "Processing BGP Update message" << std::endl;
    _BGPSessions[_currSessionId]->getFSM()->UpdateMsgEvent();

    unsigned char               decisionProcessResult;
    IPv4Address                 netMask(IPv4Address::ALLONES_ADDRESS);
    BGP::RoutingTableEntry*     entry = new BGP::RoutingTableEntry();
    const unsigned char         length = msg.getNLRI().length;
    unsigned int                ASValueCount = msg.getPathAttributeList(0).getAsPath(0).getValue(0).getAsValueArraySize();

    entry->setDestination(msg.getNLRI().prefix);
    netMask = IPv4Address::makeNetmask(length);
    entry->setNetmask(netMask);
    for (unsigned int j=0; j < ASValueCount; j++)
    {
        entry->addAS(msg.getPathAttributeList(0).getAsPath(0).getValue(0).getAsValue(j));
    }

    decisionProcessResult = asLoopDetection(entry, _myAS);

    if (decisionProcessResult == BGP::ASLOOP_NO_DETECTED)
    {
        // RFC 4271, 9.1.  Decision Process
        decisionProcessResult = decisionProcess(msg, entry, _currSessionId);
        //RFC 4271, 9.2.  Update-Send Process
        if (decisionProcessResult != 0)
        {
            updateSendProcess(decisionProcessResult, _currSessionId, entry);
        }
    }
}

unsigned char BGPRouting::decisionProcess(const BGPUpdateMessage& msg, BGP::RoutingTableEntry* entry, BGP::SessionID sessionIndex)
{
    //Don't add the route if it exists in PrefixListINTable or in ASListINTable
    if (isInTable(_prefixListIN, entry) != (unsigned long)-1 || isInASList(_ASListIN, entry))
    {
        return 0;
    }

    /*If the AS_PATH attribute of a BGP route contains an AS loop, the BGP
    route should be excluded from the decision process. */
    entry->setPathType(msg.getPathAttributeList(0).getOrigin().getValue());
    entry->setGateway(msg.getPathAttributeList(0).getNextHop().getValue());

    //if the route already exist in BGP routing table, tieBreakingProcess();
    //(RFC 4271: 9.1.2.2 Breaking Ties)
    unsigned long BGPindex = isInTable(_BGPRoutingTable, entry);
    if (BGPindex != (unsigned long)-1)
    {
        if (tieBreakingProcess(_BGPRoutingTable[BGPindex], entry))
        {
            return 0;
        }
        else
        {
            entry->setInterface(_BGPSessions[sessionIndex]->getLinkIntf());
            _BGPRoutingTable.push_back(entry);
            _rt->addRoute(entry);
            return BGP::ROUTE_DESTINATION_CHANGED;
        }
    }

    //Don't add the route if it exists in IPv4 routing table except if the msg come from IGP session
    int indexIP = isInRoutingTable(_rt, entry->getDestination());
    if (indexIP != -1 && _rt->getRoute(indexIP)->getSource() != IPv4Route::BGP )
    {
        if (_BGPSessions[sessionIndex]->getType() != BGP::IGP )
        {
            return 0;
        }
        else
        {
            IPv4Route* newEntry = new IPv4Route;
            newEntry->setDestination(_rt->getRoute(indexIP)->getDestination());
            newEntry->setNetmask(_rt->getRoute(indexIP)->getNetmask());
            newEntry->setGateway(_rt->getRoute(indexIP)->getGateway());
            newEntry->setInterface(_rt->getRoute(indexIP)->getInterface());
            newEntry->setSource(IPv4Route::BGP);
            _rt->deleteRoute(_rt->getRoute(indexIP));
            _rt->addRoute(newEntry);
        }
    }

    entry->setInterface(_BGPSessions[sessionIndex]->getLinkIntf());
    _BGPRoutingTable.push_back(entry);

    if (_BGPSessions[sessionIndex]->getType() == BGP::EGP)
    {
        std::string entryh = entry->getDestination().str();
        std::string entryn = entry->getNetmask().str();
        _rt->addRoute(entry);
        //insertExternalRoute on OSPF ExternalRoutingTable if OSPF exist on this BGP router
        if (ospfExist(_rt))
        {
            OSPF::IPv4AddressRange  OSPFnetAddr;
            OSPFnetAddr.address = entry->getDestination();
            OSPFnetAddr.mask = entry->getNetmask();
            OSPFRouting* ospf = OSPFRoutingAccess().getIfExists();
            InterfaceEntry *ie = entry->getInterface();
            if (!ie)
                throw cRuntimeError("Model error: interface entry is NULL");
            ospf->insertExternalRoute(ie->getInterfaceId(), OSPFnetAddr);
            simulation.setContext(this);
        }
    }
    return BGP::NEW_ROUTE_ADDED;
}

bool BGPRouting::tieBreakingProcess(BGP::RoutingTableEntry* oldEntry, BGP::RoutingTableEntry* entry)
{
    /*a) Remove from consideration all routes that are not tied for
         having the smallest number of AS numbers present in their
         AS_PATH attributes.*/
    if (entry->getASCount() < oldEntry->getASCount())
    {
        deleteBGPRoutingEntry(oldEntry);
        return false;
    }

    /* b) Remove from consideration all routes that are not tied for
         having the lowest Origin number in their Origin attribute.*/
    if (entry->getPathType() < oldEntry->getPathType())
    {
        deleteBGPRoutingEntry(oldEntry);
        return false;
    }
    return true;
}

void BGPRouting::updateSendProcess(const unsigned char type, BGP::SessionID sessionIndex, BGP::RoutingTableEntry* entry)
{
    //Don't send the update Message if the route exists in listOUTTable
    //SESSION = EGP : send an update message to all BGP Peer (EGP && IGP)
    //if it is not the currentSession and if the session is already established
    //SESSION = IGP : send an update message to External BGP Peer (EGP) only
    //if it is not the currentSession and if the session is already established
    for (std::map<BGP::SessionID, BGPSession*>::iterator sessionIt = _BGPSessions.begin();
        sessionIt != _BGPSessions.end(); sessionIt ++)
    {
        if (isInTable(_prefixListOUT, entry) != (unsigned long)-1 || isInASList(_ASListOUT, entry) ||
            ((*sessionIt).first == sessionIndex && type != BGP::NEW_SESSION_ESTABLISHED ) ||
            (type == BGP::NEW_SESSION_ESTABLISHED && (*sessionIt).first != sessionIndex ) ||
            !(*sessionIt).second->isEstablished() )
        {
            continue;
        }
        if ((_BGPSessions[sessionIndex]->getType()==BGP::IGP && (*sessionIt).second->getType()==BGP::EGP ) ||
            _BGPSessions[sessionIndex]->getType() == BGP::EGP ||
            type == BGP::ROUTE_DESTINATION_CHANGED ||
            type == BGP::NEW_SESSION_ESTABLISHED )
        {
            BGPUpdateNLRI                   NLRI;
            BGPUpdatePathAttributeList  content;

            unsigned int nbAS = entry->getASCount();
            content.setAsPathArraySize(1);
            content.getAsPath(0).setValueArraySize(1);
            content.getAsPath(0).getValue(0).setType(BGP::AS_SEQUENCE);
            //RFC 4271 : set My AS in first position if it is not already
            if (entry->getAS(0) != _myAS)
            {
                content.getAsPath(0).getValue(0).setAsValueArraySize(nbAS+1);
                content.getAsPath(0).getValue(0).setLength(1);
                content.getAsPath(0).getValue(0).setAsValue(0, _myAS);
                for (unsigned int j = 1; j < nbAS+1; j++)
                {
                    content.getAsPath(0).getValue(0).setAsValue(j, entry->getAS(j-1));
                }
            }
            else
            {
                content.getAsPath(0).getValue(0).setAsValueArraySize(nbAS);
                content.getAsPath(0).getValue(0).setLength(1);
                for (unsigned int j = 0; j < nbAS; j++)
                {
                    content.getAsPath(0).getValue(0).setAsValue(j, entry->getAS(j));
                }
            }

            InterfaceEntry*  iftEntry = (*sessionIt).second->getLinkIntf();
            content.getOrigin().setValue((*sessionIt).second->getType());
            content.getNextHop().setValue(iftEntry->ipv4Data()->getIPAddress());
            IPv4Address netMask = entry->getNetmask();
            NLRI.prefix = entry->getDestination().doAnd(netMask);
            NLRI.length = (unsigned char) netMask.getNetmaskLength();
            {
                BGPUpdateMessage* updateMsg = new BGPUpdateMessage("BGPUpdate");
                updateMsg->setPathAttributeListArraySize(1);
                updateMsg->setPathAttributeList(content);
                updateMsg->setNLRI(NLRI);
                (*sessionIt).second->getSocket()->send(updateMsg);
                (*sessionIt).second->addUpdateMsgSent();
            }
        }
    }
}

bool BGPRouting::checkExternalRoute(const IPv4Route* route)
{
    IPv4Address OSPFRoute;
    OSPFRoute = route->getDestination();
    OSPFRouting* ospf = OSPFRoutingAccess().getIfExists();
    bool returnValue = ospf->checkExternalRoute(OSPFRoute);
    simulation.setContext(this);
    return returnValue;
}

void BGPRouting::loadTimerConfig(cXMLElementList& timerConfig, simtime_t* delayTab)
{
    for (cXMLElementList::iterator timerElemIt = timerConfig.begin(); timerElemIt != timerConfig.end(); timerElemIt++)
    {
        std::string nodeName = (*timerElemIt)->getTagName();
        if (nodeName == "connectRetryTime")
        {
            delayTab[0] = (double)atoi((*timerElemIt)->getNodeValue());
        }
        else if (nodeName == "holdTime")
        {
            delayTab[1] = (double)atoi((*timerElemIt)->getNodeValue());
        }
        else if (nodeName == "keepAliveTime")
        {
            delayTab[2] = (double)atoi((*timerElemIt)->getNodeValue());
        }
        else if (nodeName == "startDelay")
        {
            delayTab[3] = (double)atoi((*timerElemIt)->getNodeValue());
        }
    }
}

BGP::ASID BGPRouting::findMyAS(cXMLElementList& asList, int& outRouterPosition)
{
    // find my own IPv4 address in the configuration file and return the AS id under which it is configured
    // and also the 1 based position of the entry inside the AS config element
    for (cXMLElementList::iterator asListIt = asList.begin(); asListIt != asList.end(); asListIt++)
    {
        cXMLElementList routerList = (*asListIt)->getChildrenByTagName("Router");
        outRouterPosition = 1;
        for (cXMLElementList::iterator routerListIt = routerList.begin(); routerListIt != routerList.end(); routerListIt++)
        {
            IPv4Address routerAddr = IPv4Address((*routerListIt)->getAttribute("interAddr"));
            for (int i=0; i<_inft->getNumInterfaces(); i++) {
                if (_inft->getInterface(i)->ipv4Data()->getIPAddress() == routerAddr)
                    return atoi((*routerListIt)->getParentNode()->getAttribute("id"));
            }
            outRouterPosition++;
        }
    }

    return 0;
}

void BGPRouting::loadSessionConfig(cXMLElementList& sessionList, simtime_t* delayTab)
{
    simtime_t saveStartDelay = delayTab[3];
    for (cXMLElementList::iterator sessionListIt = sessionList.begin(); sessionListIt != sessionList.end(); sessionListIt++, delayTab[3] = saveStartDelay)
    {
        const char* exterAddr = (*sessionListIt)->getFirstChild()->getAttribute("exterAddr");
        IPv4Address routerAddr1 = IPv4Address(exterAddr);
        exterAddr = (*sessionListIt)->getLastChild()->getAttribute("exterAddr");
        IPv4Address routerAddr2 = IPv4Address(exterAddr);
        if (isInInterfaceTable(_inft, routerAddr1) == -1 && isInInterfaceTable(_inft, routerAddr2) == -1)
        {
            continue;
        }
        IPv4Address peerAddr;
        if (isInInterfaceTable(_inft, routerAddr1) != -1)
        {
            peerAddr = routerAddr2;
            delayTab[3] += atoi((*sessionListIt)->getAttribute("id"));
        }
        else
        {
            peerAddr = routerAddr1;
            delayTab[3] += atoi((*sessionListIt)->getAttribute("id")) + 0.5;
        }
        if (peerAddr.isUnspecified())
        {
            error("BGP Error: No valid external address for session ID : %s", (*sessionListIt)->getAttribute("id"));
        }

        BGP::SessionID newSessionID = createSession(BGP::EGP, peerAddr.str().c_str());
        _BGPSessions[newSessionID]->setTimers(delayTab);
        TCPSocket* socketListenEGP = new TCPSocket();
        _BGPSessions[newSessionID]->setSocketListen(socketListenEGP);
    }
}

std::vector<const char *> BGPRouting::loadASConfig(cXMLElementList& ASConfig)
{
    //create deny Lists
    std::vector<const char *> routerInSameASList;

    for (cXMLElementList::iterator ASConfigIt = ASConfig.begin(); ASConfigIt != ASConfig.end(); ASConfigIt++)
    {
        std::string nodeName = (*ASConfigIt)->getTagName();
        if (nodeName == "Router")
        {
            if (isInInterfaceTable(_inft, IPv4Address((*ASConfigIt)->getAttribute("interAddr"))) == -1)
            {
                routerInSameASList.push_back((*ASConfigIt)->getAttribute("interAddr"));
            }
            continue;
        }
        if (nodeName == "DenyRoute" || nodeName == "DenyRouteIN" || nodeName == "DenyRouteOUT")
        {
            BGP::RoutingTableEntry* entry = new BGP::RoutingTableEntry();
            entry->setDestination(IPv4Address((*ASConfigIt)->getAttribute("Address")));
            entry->setNetmask(IPv4Address((*ASConfigIt)->getAttribute("Netmask")));
            if (nodeName == "DenyRouteIN")
            {
                _prefixListIN.push_back(entry);
            }
            else if (nodeName == "DenyRouteOUT")
            {
                _prefixListOUT.push_back(entry);
            }
            else
            {
                _prefixListIN.push_back(entry);
                _prefixListOUT.push_back(entry);
            }
        }
        else if (nodeName == "DenyAS" || nodeName == "DenyASIN" || nodeName == "DenyASOUT")
        {
            BGP::ASID ASCur = atoi((*ASConfigIt)->getNodeValue());
            if (nodeName == "DenyASIN")
            {
                _ASListIN.push_back(ASCur);
            }
            else if (nodeName == "DenyASOUT")
            {
                _ASListOUT.push_back(ASCur);
            }
            else
            {
                _ASListIN.push_back(ASCur);
                _ASListOUT.push_back(ASCur);
            }
        }
        else
        {
            error("BGP Error: unknown element named '%s' for AS %u", nodeName.c_str(), _myAS);
        }
    }
    return routerInSameASList;
}

void BGPRouting::loadConfigFromXML(cXMLElement *bgpConfig)
{
    if (strcmp(bgpConfig->getTagName(), "BGPConfig"))
        error("Cannot read BGP configuration, unaccepted '%s' node at %s", bgpConfig->getTagName(), bgpConfig->getSourceLocation());

    // load bgp timer parameters informations
    simtime_t delayTab[BGP::NB_TIMERS];
    cXMLElement* paramNode = bgpConfig->getElementByPath("TimerParams");
    if (paramNode == NULL)
        error("BGP Error: No configuration for BGP timer parameters");

    cXMLElementList timerConfig = paramNode->getChildren();
    loadTimerConfig(timerConfig, delayTab);

    //find my AS
    cXMLElementList ASList = bgpConfig->getElementsByTagName("AS");
    int routerPosition;
    _myAS = findMyAS(ASList, routerPosition);
    if (_myAS == 0)
        error("BGP Error:  No AS configuration for Router ID: %s", _rt->getRouterId().str().c_str());

    // load EGP Session informations
    cXMLElementList sessionList = bgpConfig->getElementsByTagName("Session");
    simtime_t saveStartDelay = delayTab[3];
    loadSessionConfig(sessionList, delayTab);
    delayTab[3] = saveStartDelay;

    // load AS information
    char ASXPath[32];
    sprintf(ASXPath, "AS[@id='%d']", _myAS);

    cXMLElement* ASNode = bgpConfig->getElementByPath(ASXPath);
    std::vector<const char *> routerInSameASList;
    if (ASNode == NULL)
        error("BGP Error:  No configuration for AS ID: %d", _myAS);

    cXMLElementList ASConfig = ASNode->getChildren();
    routerInSameASList = loadASConfig(ASConfig);

    //create IGP Session(s)
    if (routerInSameASList.size())
    {
        unsigned int routerPeerPosition = 1;
        delayTab[3] += sessionList.size()*2;
        for (std::vector<const char *>::iterator it = routerInSameASList.begin(); it != routerInSameASList.end(); it++, routerPeerPosition++)
        {
            BGP::SessionID newSessionID;
            TCPSocket* socketListenIGP = new TCPSocket();
            newSessionID = createSession(BGP::IGP, (*it));
            delayTab[3] += calculateStartDelay(routerInSameASList.size(), routerPosition, routerPeerPosition);
            _BGPSessions[newSessionID]->setTimers(delayTab);
            _BGPSessions[newSessionID]->setSocketListen(socketListenIGP);
        }
    }
}

unsigned int BGPRouting::calculateStartDelay(int rtListSize, unsigned char rtPosition, unsigned char rtPeerPosition)
{
    unsigned int startDelay = 0;
    if (rtPeerPosition == 1)
    {
        if (rtPosition == 1)
        {
            startDelay = 1;
        }
        else
        {
            startDelay = (rtPosition-1)*2;
        }
        return startDelay;
    }

    if (rtPosition < rtPeerPosition)
    {
        startDelay = 2;
    }
    else if (rtPosition > rtPeerPosition)
    {
        startDelay = (rtListSize-1)*2 - 2*(rtPeerPosition-2);
    }
    else
    {
        startDelay = (rtListSize-1)*2 +1;
    }
    return startDelay;
}

BGP::SessionID BGPRouting::createSession(BGP::type typeSession, const char* peerAddr)
{
    BGPSession*         newSession = new BGPSession(*this);
    BGP::SessionID      newSessionId;
    BGP::SessionInfo    info;

    info.sessionType = typeSession;
    info.ASValue = _myAS;
    info.routerID = _rt->getRouterId();
    info.peerAddr.set(peerAddr);
    if (typeSession == BGP::EGP)
    {
        info.linkIntf = _rt->getInterfaceForDestAddr(info.peerAddr);
        if (info.linkIntf == 0)
        {
            error("BGP Error: No configuration interface for peer address: %s", peerAddr);
        }
        info.sessionID = info.peerAddr.getInt() + info.linkIntf->ipv4Data()->getIPAddress().getInt();
    }
    else
    {
        info.sessionID = info.peerAddr.getInt() + info.routerID.getInt();
    }
    newSessionId = info.sessionID;
    newSession->setInfo(info);
    _BGPSessions[newSessionId] = newSession;

    return newSessionId;
}


BGP::SessionID BGPRouting::findIdFromPeerAddr(std::map<BGP::SessionID, BGPSession*> sessions, IPv4Address peerAddr)
{
    for (std::map<BGP::SessionID, BGPSession*>::iterator sessionIterator = sessions.begin();
        sessionIterator != sessions.end(); sessionIterator ++)
    {
        if ((*sessionIterator).second->getPeerAddr().equals(peerAddr))
        {
            return (*sessionIterator).first;
        }
    }
    return -1;
}

/*delete BGP Routing entry, if the route deleted correctly return true, false else*/
bool BGPRouting::deleteBGPRoutingEntry(BGP::RoutingTableEntry* entry){
    for (std::vector<BGP::RoutingTableEntry*>::iterator it = _BGPRoutingTable.begin();
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
int BGPRouting::isInRoutingTable(IRoutingTable* rtTable, IPv4Address addr)
{
    for (int i = 0; i < rtTable->getNumRoutes(); i++)
    {
        const IPv4Route* entry = rtTable->getRoute(i);
        if (IPv4Address::maskedAddrAreEqual(addr, entry->getDestination(), entry->getNetmask()))
        {
            return i;
        }
    }
    return -1;
}

int BGPRouting::isInInterfaceTable(IInterfaceTable* ifTable, IPv4Address addr)
{
    for (int i = 0; i < ifTable->getNumInterfaces(); i++)
    {
        if (ifTable->getInterface(i)->ipv4Data()->getIPAddress() == addr)
        {
            return i;
        }
    }
    return -1;
}

BGP::SessionID BGPRouting::findIdFromSocketConnId(std::map<BGP::SessionID, BGPSession*> sessions, int connId)
{
    for (std::map<BGP::SessionID, BGPSession*>::iterator sessionIterator = sessions.begin();
        sessionIterator != sessions.end(); sessionIterator ++)
    {
        TCPSocket* socket = (*sessionIterator).second->getSocket();
        if (socket->getConnectionId() == connId)
        {
            return (*sessionIterator).first;
        }
    }
    return -1;
}

/*return index of the table if the route is found, -1 else*/
unsigned long BGPRouting::isInTable(std::vector<BGP::RoutingTableEntry*> rtTable, BGP::RoutingTableEntry* entry)
{
    for (unsigned long i = 0; i < rtTable.size(); i++)
    {
        BGP::RoutingTableEntry* entryCur = rtTable[i];
        if ((entry->getDestination().getInt() & entry->getNetmask().getInt()) ==
            (entryCur->getDestination().getInt() & entryCur->getNetmask().getInt()))
        {
            return i;
        }
    }
    return -1;
}

/*return true if the AS is found, false else*/
bool BGPRouting::isInASList(std::vector<BGP::ASID> ASList, BGP::RoutingTableEntry* entry)
{
    for (std::vector<BGP::ASID>::iterator it = ASList.begin(); it != ASList.end(); it++)
    {
        for (unsigned int i = 0; i < entry->getASCount(); i++)
        {
            if ((*it) == entry->getAS(i))
            {
                return true;
            }
        }
    }
    return false;
}

/*return true if OSPF exists, false else*/
bool BGPRouting::ospfExist(IRoutingTable* rtTable)
{
    for (int i=0; i<rtTable->getNumRoutes(); i++)
    {
        if (rtTable->getRoute(i)->getSource() == IPv4Route::OSPF)
        {
            return true;
        }
    }
    return false;
}

unsigned char BGPRouting::asLoopDetection(BGP::RoutingTableEntry* entry, BGP::ASID myAS)
{
    for (unsigned int i=1; i < entry->getASCount(); i++)
    {
        if (myAS == entry->getAS(i))
        {
            return BGP::ASLOOP_DETECTED;
        }
    }
    return BGP::ASLOOP_NO_DETECTED;
}

/*return sessionID if the session is found, -1 else*/
BGP::SessionID BGPRouting::findNextSession(BGP::type type, bool startSession)
{
    BGP::SessionID sessionID = -1;
    for (std::map<BGP::SessionID, BGPSession*>::iterator sessionIterator = _BGPSessions.begin();
        sessionIterator != _BGPSessions.end(); sessionIterator ++)
    {
        if ((*sessionIterator).second->getType() == type && !(*sessionIterator).second->isEstablished())
        {
            sessionID = (*sessionIterator).first;
            break;
        }
    }
    if (startSession == true && type == BGP::IGP && sessionID != (BGP::SessionID)-1)
    {
        InterfaceEntry* linkIntf = _rt->getInterfaceForDestAddr(_BGPSessions[sessionID]->getPeerAddr());
        if (linkIntf == 0)
        {
            error("No configuration interface for peer address: %s", _BGPSessions[sessionID]->getPeerAddr().str().c_str());
        }
        _BGPSessions[sessionID]->setlinkIntf(linkIntf);
        _BGPSessions[sessionID]->startConnection();
    }
    return sessionID;
}

