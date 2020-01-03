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

#include <algorithm>

#include "inet/common/ModuleAccess.h"
#include "inet/routing/bgpv4/BgpRouter.h"
#include "inet/routing/bgpv4/BgpSession.h"

namespace inet {
namespace bgp {

BgpRouter::BgpRouter(cSimpleModule *bgpModule, IInterfaceTable *ift, IIpv4RoutingTable *rt)
{
    this->bgpModule = bgpModule;
    this->ift = ift;
    this->rt = rt;

    ospfModule = getModuleFromPar<ospfv2::Ospfv2>(bgpModule->par("ospfRoutingModule"), bgpModule, false);
}

BgpRouter::~BgpRouter(void)
{
    for (auto & elem : _BGPSessions)
        delete (elem).second;

    for (auto & elem : _prefixListINOUT)
        delete (elem);
}

void BgpRouter::printSessionSummary()
{
    EV_DEBUG << "summary of BGP sessions: \n";
    for(auto &entry : _BGPSessions) {
        BgpSession *session = entry.second;
        BgpSessionType type = session->getType();
        if(type == IGP) {
            EV_DEBUG << "  IGP session to internal peer '" << session->getPeerAddr().str(false) <<
                    "' starts at " << session->getStartEventTime() << "s \n";
        }
        else if(type == EGP) {
            EV_DEBUG << "  EGP session to external peer '" << session->getPeerAddr().str(false) <<
                    "' starts at " << session->getStartEventTime() << "s \n";
        }
        else {
            EV_DEBUG << "  Unknown session to peer '" << session->getPeerAddr().str(false) <<
                    "' starts at " << session->getStartEventTime() << "s \n";
        }
    }
}

void BgpRouter::addWatches()
{
    WATCH(myAsId);
    WATCH_PTRMAP(_BGPSessions);
    WATCH_PTRVECTOR(bgpRoutingTable);
    _socketMap.addWatch();
}

void BgpRouter::recordStatistics()
{
    unsigned int statTab[NB_STATS] = {
        0, 0, 0, 0, 0, 0
    };

    for (auto & elem : _BGPSessions)
        (elem).second->getStatistics(statTab);

    bgpModule->recordScalar("OPENMsgSent", statTab[0]);
    bgpModule->recordScalar("OPENMsgRecv", statTab[1]);
    bgpModule->recordScalar("KeepAliveMsgSent", statTab[2]);
    bgpModule->recordScalar("KeepAliveMsgRcv", statTab[3]);
    bgpModule->recordScalar("UpdateMsgSent", statTab[4]);
    bgpModule->recordScalar("UpdateMsgRcv", statTab[5]);
}

SessionId BgpRouter::createIbgpSession(const char *peerAddr)
{
    SessionInfo info;
    info.sessionType = IGP;
    info.ASValue = myAsId;
    info.routerID = rt->getRouterId();
    info.peerAddr.set(peerAddr);
    info.sessionID = info.peerAddr.getInt() + info.routerID.getInt();

    numIgpSessions++;

    SessionId newSessionId;
    newSessionId = info.sessionID;

    BgpSession *newSession = new BgpSession(*this);
    newSession->setInfo(info);

    _BGPSessions[newSessionId] = newSession;

    return newSessionId;
}

SessionId BgpRouter::createEbgpSession(const char *peerAddr, SessionInfo& externalInfo)
{
    SessionInfo info;

    // set external info
    info.myAddr = externalInfo.myAddr;
    info.checkConnection = externalInfo.checkConnection;
    info.ebgpMultihop = externalInfo.ebgpMultihop;

    info.sessionType = EGP;
    info.ASValue = myAsId;
    info.routerID = rt->getRouterId();
    info.peerAddr.set(peerAddr);
    info.linkIntf = rt->getInterfaceForDestAddr(info.peerAddr);
    if(!info.linkIntf) {
        if(info.checkConnection)
            throw cRuntimeError("BGP Error: External BGP neighbor at address %s is not directly connected to BGP router %s", peerAddr, bgpModule->getOwner()->getFullName());
        else
            info.linkIntf = rt->getInterfaceForDestAddr(info.myAddr);
    }
    ASSERT(info.linkIntf);
    info.sessionID = info.peerAddr.getInt() + info.linkIntf->getProtocolData<Ipv4InterfaceData>()->getIPAddress().getInt();
    numEgpSessions++;

    SessionId newSessionId;
    newSessionId = info.sessionID;

    BgpSession *newSession = new BgpSession(*this);
    newSession->setInfo(info);

    _BGPSessions[newSessionId] = newSession;

    return newSessionId;
}

void BgpRouter::setTimer(SessionId id, simtime_t *delayTab)
{
    _BGPSessions[id]->setTimers(delayTab);
}

void BgpRouter::setSocketListen(SessionId id)
{
    TcpSocket *socketListen = new TcpSocket();
    _BGPSessions[id]->setSocketListen(socketListen);
}

void BgpRouter::setDefaultConfig()
{
    // per router params
    bool redistributeInternal = bgpModule->par("redistributeInternal").boolValue();
    std::string redistributeOspf = bgpModule->par("redistributeOspf").stdstringValue();
    bool redistributeRip = bgpModule->par("redistributeRip").boolValue();
    setRedistributeInternal(redistributeInternal);
    setRedistributeOspf(redistributeOspf);
    setRedistributeRip(redistributeRip);

    // per session params
    bool nextHopSelf = bgpModule->par("nextHopSelf").boolValue();
    int localPreference = bgpModule->par("localPreference").intValue();
    for(auto &session : _BGPSessions) {
        session.second->setNextHopSelf(nextHopSelf);
        session.second->setLocalPreference(localPreference);
    }
}

void BgpRouter::addToAdvertiseList(Ipv4Address address)
{
    bool routeFound = false;
    const Ipv4Route *rtEntry = nullptr;
    for (int i = 0; i < rt->getNumRoutes(); i++) {
        rtEntry = rt->getRoute(i);
        if(rtEntry->getDestination() == address) {
            routeFound = true;
            break;
        }
    }
    if(!routeFound)
        throw cRuntimeError("Network address '%s' is not found in the routing table of %s", address.str(false).c_str(), bgpModule->getOwner()->getFullName());

    auto position = std::find_if(advertiseList.begin(), advertiseList.end(),
            [&](const Ipv4Address m) -> bool { return (m == address); });
    if(position != advertiseList.end())
        throw cRuntimeError("Network address '%s' is already added to the advertised list of %s", address.str(false).c_str(), bgpModule->getOwner()->getFullName());
    advertiseList.push_back(address);

    BgpRoutingTableEntry *BGPEntry = new BgpRoutingTableEntry(rtEntry);
    BGPEntry->addAS(myAsId);
    BGPEntry->setPathType(IGP);
    BGPEntry->setLocalPreference(bgpModule->par("localPreference").intValue());
    bgpRoutingTable.push_back(BGPEntry);
}

void BgpRouter::addToPrefixList(std::string nodeName, BgpRoutingTableEntry *entry)
{
    if (nodeName == "DenyRouteIN") {
        _prefixListIN.push_back(entry);
        _prefixListINOUT.push_back(entry);
    }
    else if (nodeName == "DenyRouteOUT") {
        _prefixListOUT.push_back(entry);
        _prefixListINOUT.push_back(entry);
    }
    else {
        _prefixListIN.push_back(entry);
        _prefixListOUT.push_back(entry);
        _prefixListINOUT.push_back(entry);
    }
}

void BgpRouter::addToAsList(std::string nodeName, AsId id)
{
    if (nodeName == "DenyASIN") {
        _ASListIN.push_back(id);
    }
    else if (nodeName == "DenyASOUT") {
        _ASListOUT.push_back(id);
    }
    else {
        _ASListIN.push_back(id);
        _ASListOUT.push_back(id);
    }
}

void BgpRouter::setNextHopSelf(Ipv4Address peer, bool nextHopSelf)
{
    bool found = false;
    for(auto &session : _BGPSessions) {
        if(session.second->getPeerAddr() == peer) {
            found = true;
            session.second->setNextHopSelf(nextHopSelf);
            break;
        }
    }
    if(!found)
        throw cRuntimeError("Neighbor address '%s' cannot be found in BGP router %s", peer.str(false).c_str(), bgpModule->getOwner()->getFullName());
}

void BgpRouter::setLocalPreference(Ipv4Address peer, int localPref)
{
    bool found = false;
    for(auto &session : _BGPSessions) {
        if(session.second->getPeerAddr() == peer) {
            found = true;
            session.second->setLocalPreference(localPref);
            break;
        }
    }
    if(!found)
        throw cRuntimeError("Neighbor address '%s' cannot be found in BGP router %s", peer.str(false).c_str(), bgpModule->getOwner()->getFullName());
}

void BgpRouter::setRedistributeOspf(std::string str)
{
    if(str == "") {
        redistributeOspf = false;
        return;
    }

    redistributeOspf = true;
    std::vector<std::string> tokens = cStringTokenizer(str.c_str()).asVector();

    for(auto& Ospfv2RouteType : tokens) {
        std::transform(Ospfv2RouteType.begin(), Ospfv2RouteType.end(), Ospfv2RouteType.begin(), ::tolower);
        if(Ospfv2RouteType == "o")
            redistributeOspfType.intraArea = true;
        else if(Ospfv2RouteType == "ia")
            redistributeOspfType.interArea = true;
        else if(Ospfv2RouteType == "e1")
            redistributeOspfType.externalType1 = true;
        else if(Ospfv2RouteType == "e2")
            redistributeOspfType.externalType2 = true;
        else
            throw cRuntimeError("Unknown OSPF redistribute type '%s' in BGP router %s", Ospfv2RouteType.c_str(), bgpModule->getOwner()->getFullName());
    }
}

void BgpRouter::processMessageFromTCP(cMessage *msg)
{
    TcpSocket *socket = check_and_cast_nullable<TcpSocket*>(_socketMap.findSocketFor(msg));
    if (!socket) {
        socket = new TcpSocket(msg);
        socket->setOutputGate(bgpModule->gate("socketOut"));
        Ipv4Address peerAddr = socket->getRemoteAddress().toIpv4();
        SessionId i = findIdFromPeerAddr(_BGPSessions, peerAddr);
        if (i == static_cast<SessionId>(-1)) {
            socket->close();
            _socketMap.removeSocket(socket);
            delete socket;
            socket = nullptr;
            delete msg;
            return;
        }
        socket->setCallback(this);
        socket->setUserData((void *)(uintptr_t)i);

        _socketMap.addSocket(socket);
        _BGPSessions[i]->getSocket()->abort();
        _socketMap.removeSocket(_BGPSessions[i]->getSocket());
        _BGPSessions[i]->setSocket(socket);
    }

    socket->processMessage(msg);
}

void BgpRouter::listenConnectionFromPeer(SessionId sessionID)
{
    if (_BGPSessions[sessionID]->getSocketListen()->getState() == TcpSocket::CLOSED) {
        //session StartDelayTime error, it's anormal that listenSocket is closed.
        _socketMap.removeSocket(_BGPSessions[sessionID]->getSocketListen());
        _BGPSessions[sessionID]->getSocketListen()->abort();
        _BGPSessions[sessionID]->getSocketListen()->renewSocket();
    }

    if (_BGPSessions[sessionID]->getSocketListen()->getState() != TcpSocket::LISTENING) {
        _BGPSessions[sessionID]->getSocketListen()->setOutputGate(bgpModule->gate("socketOut"));
        if(_BGPSessions[sessionID]->getType() == EGP) {
            InterfaceEntry *intf = _BGPSessions[sessionID]->getLinkIntf();
            ASSERT(intf);
            Ipv4Address localAddr = intf->getProtocolData<Ipv4InterfaceData>()->getIPAddress();
            _BGPSessions[sessionID]->getSocketListen()->bind(localAddr, TCP_PORT);
        }
        else
            _BGPSessions[sessionID]->getSocketListen()->bind(TCP_PORT);
        _BGPSessions[sessionID]->getSocketListen()->listen();
        _socketMap.addSocket(_BGPSessions[sessionID]->getSocketListen());

        EV_DEBUG << "Start listening to incoming TCP connections on " <<
                _BGPSessions[sessionID]->getSocketListen()->getLocalAddress() <<
                ":" << (int)TCP_PORT <<
                " for " << BgpSession::getTypeString(_BGPSessions[sessionID]->getType()) << " session" <<
                " to peer " << _BGPSessions[sessionID]->getPeerAddr() << std::endl;
    }
}

void BgpRouter::openTCPConnectionToPeer(SessionId sessionID)
{
    EV_DEBUG << "Opening a TCP connection to " <<
            _BGPSessions[sessionID]->getPeerAddr() <<
            ":" << (int)TCP_PORT << std::endl;

    TcpSocket *socket = _BGPSessions[sessionID]->getSocket();
    if (socket->getState() != TcpSocket::NOT_BOUND) {
        _socketMap.removeSocket(socket);
        socket->abort();
        socket->renewSocket();
    }
    socket->setCallback(this);
    socket->setUserData((void *)(uintptr_t)sessionID);
    socket->setOutputGate(bgpModule->gate("socketOut"));
    if(_BGPSessions[sessionID]->getType() == EGP) {
        InterfaceEntry *intfEntry = _BGPSessions[sessionID]->getLinkIntf();
        if (intfEntry == nullptr)
            throw cRuntimeError("No configuration interface for external peer address: %s", _BGPSessions[sessionID]->getPeerAddr().str().c_str());
        socket->bind(intfEntry->getProtocolData<Ipv4InterfaceData>()->getIPAddress(), 0);

        int ebgpMH = _BGPSessions[sessionID]->getEbgpMultihop();
        if(ebgpMH > 1)
            socket->setTimeToLive(ebgpMH);
        else if(ebgpMH == 1)
            socket->setTimeToLive(1);
        else
            throw cRuntimeError("ebgpMultihop should be >=1");
    }
    else if(_BGPSessions[sessionID]->getType() == IGP) {
        InterfaceEntry *intfEntry = _BGPSessions[sessionID]->getLinkIntf();
        if(!intfEntry)
            intfEntry = rt->getInterfaceForDestAddr(_BGPSessions[sessionID]->getPeerAddr());
        if (intfEntry == nullptr)
            throw cRuntimeError("No configuration interface for internal peer address: %s", _BGPSessions[sessionID]->getPeerAddr().str().c_str());
        _BGPSessions[sessionID]->setlinkIntf(intfEntry);
        if(internalAddress == Ipv4Address::UNSPECIFIED_ADDRESS)
            throw cRuntimeError("Internal address is not specified for router %s", bgpModule->getOwner()->getFullName());
        socket->bind(internalAddress, 0);
    }
    _socketMap.addSocket(socket);

    socket->connect(_BGPSessions[sessionID]->getPeerAddr(), TCP_PORT);
}

void BgpRouter::socketEstablished(TcpSocket *socket)
{
    int connId = socket->getSocketId();
    _currSessionId = findIdFromSocketConnId(_BGPSessions, connId);
    if (_currSessionId == static_cast<SessionId>(-1)) {
        throw cRuntimeError("socket id=%d is not established", connId);
    }

    //if it's an IGP Session, TCPConnectionConfirmed only if all EGP Sessions established
    if (_BGPSessions[_currSessionId]->getType() == IGP &&
        this->findNextSession(EGP) != static_cast<SessionId>(-1))
    {
        _BGPSessions[_currSessionId]->getFSM()->TcpConnectionFails();
    }
    else {
        _BGPSessions[_currSessionId]->getFSM()->TcpConnectionConfirmed();
        _BGPSessions[_currSessionId]->getSocketListen()->abort();
    }

    if (_BGPSessions[_currSessionId]->getSocketListen()->getSocketId() != connId &&
        _BGPSessions[_currSessionId]->getType() == EGP &&
        this->findNextSession(EGP) != static_cast<SessionId>(-1))
    {
        _BGPSessions[_currSessionId]->getSocketListen()->abort();
    }
}

void BgpRouter::socketFailure(TcpSocket *socket, int code)
{
    int connId = socket->getSocketId();
    _currSessionId = findIdFromSocketConnId(_BGPSessions, connId);
    if (_currSessionId != static_cast<SessionId>(-1)) {
        _BGPSessions[_currSessionId]->getFSM()->TcpConnectionFails();
    }
}

void BgpRouter::socketDataArrived(TcpSocket *socket)
{
    auto queue = socket->getReceiveQueue();
    while (queue->has<BgpHeader>()) {
        auto header = queue->pop<BgpHeader>();
        processChunks(*header.get());
    }
}

void BgpRouter::socketDataArrived(TcpSocket *socket, Packet *packet, bool urgent)
{
    _currSessionId = findIdFromSocketConnId(_BGPSessions, socket->getSocketId());
    if (_currSessionId != static_cast<SessionId>(-1))
        ReceiveQueueBasedCallback::socketDataArrived(socket, packet, urgent);
    else
        delete packet;
}

void BgpRouter::processChunks(const BgpHeader& ptrHdr)
{
    switch (ptrHdr.getType()) {
        case BGP_OPEN:
            processMessage(*check_and_cast<const BgpOpenMessage *>(&ptrHdr));
            break;

        case BGP_KEEPALIVE:
            processMessage(*check_and_cast<const BgpKeepAliveMessage *>(&ptrHdr));
            break;

        case BGP_UPDATE:
            processMessage(*check_and_cast<const BgpUpdateMessage *>(&ptrHdr));
            break;

        default:
            throw cRuntimeError("Invalid BGP message type %d", ptrHdr.getType());
    }
}

void BgpRouter::processMessage(const BgpOpenMessage& msg)
{
    BgpSession *session = _BGPSessions[_currSessionId];
    EV_INFO << "Processing BGP OPEN message from " <<
            session->getPeerAddr().str(false) <<
            " with contents: \n";
    printOpenMessage(msg);
    session->getFSM()->OpenMsgEvent();
}

void BgpRouter::processMessage(const BgpKeepAliveMessage& msg)
{
    BgpSession *session = _BGPSessions[_currSessionId];
    EV_INFO << "Processing BGP Keep Alive message from " <<
            session->getPeerAddr().str(false) <<
            " with contents: \n";
    printKeepAliveMessage(msg);
    session->getFSM()->KeepAliveMsgEvent();
}

void BgpRouter::processMessage(const BgpUpdateMessage& msg)
{
    BgpSession *session = _BGPSessions[_currSessionId];
    EV_INFO << "Processing BGP Update message from " <<
            session->getPeerAddr().str(false) <<
            " with contents: \n";
    printUpdateMessage(msg);
    session->getFSM()->UpdateMsgEvent();

    BgpRoutingTableEntry *entry = new BgpRoutingTableEntry();
    entry->setLocalPreference(bgpModule->par("localPreference").intValue());
    entry->setDestination(msg.getNLRI(0).prefix);

    Ipv4Address netMask(Ipv4Address::ALLONES_ADDRESS);
    netMask = Ipv4Address::makeNetmask(msg.getNLRI(0).length);
    entry->setNetmask(netMask);

    for (size_t i = 0; i < msg.getPathAttributesArraySize(); i++) {
        if (msg.getPathAttributes(i)->getTypeCode() == BgpUpdateAttributeTypeCode::AS_PATH) {
            auto& asPath = *check_and_cast<const BgpUpdatePathAttributesAsPath*>(msg.getPathAttributes(i));
            for(uint32_t k = 0; k < asPath.getValueArraySize(); k++) {
                const BgpAsPathSegment& asPathVal = asPath.getValue(k);
                for(uint32_t n = 0; n < asPathVal.getAsValueArraySize(); n++) {
                    entry->addAS(asPathVal.getAsValue(n));
                }
            }
        }
    }

    unsigned char decisionProcessResult = asLoopDetection(entry, myAsId);

    if (decisionProcessResult == ASLOOP_NO_DETECTED) {
        // RFC 4271, 9.1.  Decision Process
        decisionProcessResult = decisionProcess(msg, entry, _currSessionId);
        // RFC 4271, 9.2.  Update-Send Process
        if (decisionProcessResult != 0)
            updateSendProcess(decisionProcessResult, _currSessionId, entry);
    }
    else
        delete entry;
}

unsigned char BgpRouter::asLoopDetection(BgpRoutingTableEntry *entry, AsId myAS)
{
    for (unsigned int i = 1; i < entry->getASCount(); i++) {
        if (myAS == entry->getAS(i))
            return ASLOOP_DETECTED;
    }
    return ASLOOP_NO_DETECTED;
}

/* add entry to routing table, or delete entry */
unsigned char BgpRouter::decisionProcess(const BgpUpdateMessage& msg, BgpRoutingTableEntry *entry, SessionId sessionIndex)
{
    //Don't add the route if it exists in PrefixListINTable or in ASListINTable
    if (isInTable(_prefixListIN, entry) != (unsigned long)-1 || isInASList(_ASListIN, entry)) {
        delete entry;
        return 0;
    }

    /*If the AS_PATH attribute of a BGP route contains an AS loop, the BGP
       route should be excluded from the decision process. */
#if 0
    entry->setPathType(msg.getPathAttributeList(0).getOrigin().getValue());
    entry->setGateway(msg.getPathAttributeList(0).getNextHop().getValue());
    if(msg.getPathAttributeList(0).getLocalPrefArraySize() != 0)
        entry->setLocalPreference(msg.getPathAttributeList(0).getLocalPref(0).getValue());
#else
    for (size_t i = 0; i < msg.getPathAttributesArraySize(); i++) {
        switch (msg.getPathAttributes(i)->getTypeCode()) {
            case BgpUpdateAttributeTypeCode::ORIGIN: {
                auto attr = check_and_cast<const BgpUpdatePathAttributesOrigin *>(msg.getPathAttributes(i));
                entry->setPathType(attr->getValue());
                break;
            }
            case BgpUpdateAttributeTypeCode::NEXT_HOP: {
                auto attr = check_and_cast<const BgpUpdatePathAttributesNextHop *>(msg.getPathAttributes(i));
                entry->setGateway(attr->getValue());
                break;
            }
            case BgpUpdateAttributeTypeCode::LOCAL_PREF: {
                auto attr = check_and_cast<const BgpUpdatePathAttributesLocalPref *>(msg.getPathAttributes(i));
                entry->setLocalPreference(attr->getValue());
                break;
            }
            default:
                break;
        }
    }
#endif

    BgpSessionType type = _BGPSessions[sessionIndex]->getType();
    if(type == IGP) {
        entry->setAdminDist(Ipv4Route::dBGPInternal);
        entry->setIBgpLearned(true);
    }
    else if(type == EGP)
        entry->setAdminDist(Ipv4Route::dBGPExternal);
    else
        entry->setAdminDist(Ipv4Route::dUnknown);

    //if the route already exists in BGP routing table, tieBreakingProcess();
    //(RFC 4271: 9.1.2.2 Breaking Ties)
    unsigned long BGPindex = isInTable(bgpRoutingTable, entry);
    if (BGPindex != (unsigned long)-1) {
        if (tieBreakingProcess(bgpRoutingTable[BGPindex], entry)) {
            delete entry;
            return 0;
        }
        else {
            entry->setInterface(_BGPSessions[sessionIndex]->getLinkIntf());
            bgpRoutingTable.push_back(entry);
            rt->addRoute(entry);
            return ROUTE_DESTINATION_CHANGED;
        }
    }

    int indexIP = isInRoutingTable(rt, entry->getDestination());
    //if the route already exists in the IPv4 routing table
    if (indexIP != -1) {
        // and it was not added by BGP before
        if(rt->getRoute(indexIP)->getSourceType() != IRoute::BGP)
        {
            // and the Update msg is coming from IGP session
            if (_BGPSessions[sessionIndex]->getType() == IGP) {
                Ipv4Route *oldEntry = rt->getRoute(indexIP);
                BgpRoutingTableEntry *BGPEntry = new BgpRoutingTableEntry(oldEntry);
                BGPEntry->addAS(myAsId);
                BGPEntry->setPathType(IGP);
                BGPEntry->setAdminDist(Ipv4Route::dBGPInternal);
                BGPEntry->setIBgpLearned(true);
                BGPEntry->setLocalPreference(entry->getLocalPreference());
                rt->addRoute(BGPEntry);
                // Note: No need to delete the existing route. Let the administrative distance decides.
                // rt->deleteRoute(oldEntry);
            }
            else {
                delete entry;
                return 0;
            }
        }
    }
    else {
        // and the Update msg is coming from IGP session
        if (_BGPSessions[sessionIndex]->getType() == IGP) {
            // if the next hop is reachable
            if(isReachable(entry->getGateway())) {
                entry->setInterface(_BGPSessions[sessionIndex]->getLinkIntf());
                rt->addRoute(entry);
            }
        }
    }

    entry->setInterface(_BGPSessions[sessionIndex]->getLinkIntf());
    bgpRoutingTable.push_back(entry);

    if (_BGPSessions[sessionIndex]->getType() == EGP) {
        if(isReachable(entry->getGateway()))
            rt->addRoute(entry);

        // if redistributeInternal is true, then insert the new external route into the OSPF (if exists).
        // The OSPF module then floods AS-External LSA into the AS and lets other routers know
        // about the new external route.
        if (ospfExist(rt) && redistributeInternal) {
            InterfaceEntry *ie = entry->getInterface();
            if (!ie)
                throw cRuntimeError("Model error: interface entry is nullptr");
            ospfv2::Ipv4AddressRange OSPFnetAddr;
            OSPFnetAddr.address = entry->getDestination();
            OSPFnetAddr.mask = entry->getNetmask();
            if(!ospfModule)
                throw cRuntimeError("Cannot find the OSPF module on router %s", bgpModule->getFullName());
            ospfModule->insertExternalRoute(ie->getInterfaceId(), OSPFnetAddr);
        }
    }
    return NEW_ROUTE_ADDED;     //FIXME model error: When returns NEW_ROUTE_ADDED then entry stored in bgpRoutingTable, but sometimes not stored in rt
}

bool BgpRouter::tieBreakingProcess(BgpRoutingTableEntry *oldEntry, BgpRoutingTableEntry *entry)
{
    if(entry->getLocalPreference() > oldEntry->getLocalPreference()) {
        deleteBGPRoutingEntry(oldEntry);
        return false;
    }

    /* Remove from consideration all routes that are not tied for
         having the smallest number of AS numbers present in their
         AS_PATH attributes.*/
    if (entry->getASCount() < oldEntry->getASCount()) {
        deleteBGPRoutingEntry(oldEntry);
        return false;
    }

    /* Remove from consideration all routes that are not tied for
         having the lowest Origin number in their Origin attribute.*/
    if (entry->getPathType() < oldEntry->getPathType()) {
        deleteBGPRoutingEntry(oldEntry);
        return false;
    }

    return true;
}

void BgpRouter::updateSendProcess(const unsigned char type, SessionId sessionIndex, BgpRoutingTableEntry *entry)
{
    //Don't send the update Message if the route exists in listOUTTable
    //SESSION = EGP : send an update message to all BGP Peer (EGP && IGP)
    //if it is not the currentSession and if the session is already established
    //SESSION = IGP : send an update message to External BGP Peer (EGP) only
    //if it is not the currentSession and if the session is already established
    for (auto & elem : _BGPSessions)
    {
        if (isInTable(_prefixListOUT, entry) != (unsigned long)-1 || isInASList(_ASListOUT, entry) ||
            ((elem).first == sessionIndex && type != NEW_SESSION_ESTABLISHED) ||
            (type == NEW_SESSION_ESTABLISHED && (elem).first != sessionIndex) ||
            !(elem).second->isEstablished())
        {
            continue;
        }

        // if the next hop is not reachable
        if(!isReachable(entry->getGateway()))
            continue;

        BgpSessionType sType = _BGPSessions[sessionIndex]->getType();

        // BGP split horizon: skip if this prefix is learned over I-BGP and we are
        // advertising it to another internal peer.
        if(entry->isIBgpLearned() && sType == IGP && elem.second->getType() == IGP) {
            EV_INFO << "BGP Split Horizon: prevent advertisement of network " <<
                    entry->getDestination() << "\\" << entry->getNetmask();
            continue;
        }

        if ((sType == IGP && (elem).second->getType() == EGP) ||
                sType == EGP ||
                type == ROUTE_DESTINATION_CHANGED ||
                type == NEW_SESSION_ESTABLISHED)
        {
            BgpUpdateNlri NLRI;
            std::vector<BgpUpdatePathAttributes*> content;

            unsigned int nbAS = entry->getASCount();
            auto asPath = new BgpUpdatePathAttributesAsPath();
            content.push_back(asPath);
            asPath->setValueArraySize(1);
            asPath->getValueForUpdate(0).setType(AS_SEQUENCE);
            asPath->getValueForUpdate(0).setLength(0);
            if((elem).second->getType() == EGP) {
                // RFC 4271 : set My AS in first position if it is not already
                if (entry->getAS(0) != myAsId) {
                    asPath->getValueForUpdate(0).setAsValueArraySize(nbAS + 1);
                    asPath->getValueForUpdate(0).setLength(nbAS + 1);
                    asPath->getValueForUpdate(0).setAsValue(0, myAsId);
                    for (unsigned int j = 1; j < nbAS + 1; j++)
                        asPath->getValueForUpdate(0).setAsValue(j, entry->getAS(j - 1));
                }
                else {
                    asPath->getValueForUpdate(0).setAsValueArraySize(nbAS);
                    asPath->getValueForUpdate(0).setLength(nbAS);
                    for (unsigned int j = 0; j < nbAS; j++)
                        asPath->getValueForUpdate(0).setAsValue(j, entry->getAS(j));
                }
            }
            // no AS number is added when the route is being advertised between internal peers
            else if((elem).second->getType() == IGP) {
                asPath->getValueForUpdate(0).setAsValueArraySize(nbAS);
                asPath->getValueForUpdate(0).setLength(nbAS);
                for (unsigned int j = 0; j < nbAS; j++)
                    asPath->getValueForUpdate(0).setAsValue(j, entry->getAS(j));

                auto localPref = new BgpUpdatePathAttributesLocalPref();
                content.push_back(localPref);
                localPref->setLength(4);
                localPref->setValue(_BGPSessions[sessionIndex]->getLocalPreference());
            }
            asPath->setLength(2 + 2 * asPath->getValue(0).getAsValueArraySize());

            auto nextHopAttr = new BgpUpdatePathAttributesNextHop;
            content.push_back(nextHopAttr);
            if(sType == EGP || _BGPSessions[sessionIndex]->getNextHopSelf()) {
                InterfaceEntry *iftEntry = (elem).second->getLinkIntf();
                nextHopAttr->setValue(iftEntry->getProtocolData<Ipv4InterfaceData>()->getIPAddress());
            }
            else
                nextHopAttr->setValue(entry->getGateway());

            auto originAttr = new BgpUpdatePathAttributesOrigin;
            content.push_back(originAttr);
            originAttr->setValue((BgpSessionType)entry->getPathType());

            Ipv4Address netMask = entry->getNetmask();
            NLRI.prefix = entry->getDestination().doAnd(netMask);
            NLRI.length = (unsigned char)netMask.getNetmaskLength();

            (elem).second->sendUpdateMessage(content, NLRI);
        }
    }
}

/*
 *  Delete BGP Routing entry, if the route deleted correctly return true, false else.
 *  Side effects when returns true:
 *      bgpRoutingTable changed, iterators on bgpRoutingTable will be invalid.
 */
bool BgpRouter::deleteBGPRoutingEntry(BgpRoutingTableEntry *entry)
{
    for (auto it = bgpRoutingTable.begin();
         it != bgpRoutingTable.end(); it++)
    {
        if (((*it)->getDestination().getInt() & (*it)->getNetmask().getInt()) ==
            (entry->getDestination().getInt() & entry->getNetmask().getInt()))
        {
            bgpRoutingTable.erase(it);
            rt->deleteRoute(entry);
            return true;
        }
    }
    return false;
}

/*return index of the Ipv4 table if the route is found, -1 else*/
int BgpRouter::isInRoutingTable(IIpv4RoutingTable *rtTable, Ipv4Address addr)
{
    for (int i = 0; i < rtTable->getNumRoutes(); i++) {
        const Ipv4Route *entry = rtTable->getRoute(i);
        if (Ipv4Address::maskedAddrAreEqual(addr, entry->getDestination(), entry->getNetmask())) {
            if(isDefaultRoute(entry) && addr.getInt() != 0)
                continue;
            else
                return i;
        }
    }
    return -1;
}

SessionId BgpRouter::findIdFromSocketConnId(std::map<SessionId, BgpSession *> sessions, int connId)
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
unsigned long BgpRouter::isInTable(std::vector<BgpRoutingTableEntry *> rtTable, BgpRoutingTableEntry *entry)
{
    for (unsigned long i = 0; i < rtTable.size(); i++) {
        BgpRoutingTableEntry *entryCur = rtTable[i];
        if ((entry->getDestination().getInt() & entry->getNetmask().getInt()) ==
            (entryCur->getDestination().getInt() & entryCur->getNetmask().getInt()))
        {
            return i;
        }
    }
    return -1;
}

/*return true if the AS is found, false else*/
bool BgpRouter::isInASList(std::vector<AsId> ASList, BgpRoutingTableEntry *entry)
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
bool BgpRouter::ospfExist(IIpv4RoutingTable *rtTable)
{
    for (int i = 0; i < rtTable->getNumRoutes(); i++) {
        if (rtTable->getRoute(i)->getSourceType() == IRoute::OSPF) {
            return true;
        }
    }
    return false;
}

/*return sessionID if the session is found, -1 else*/
SessionId BgpRouter::findNextSession(BgpSessionType type, bool startSession)
{
    SessionId sessionID = -1;
    for (auto & elem : _BGPSessions)
    {
        if ((elem).second->getType() == type && !(elem).second->isEstablished()) {
            sessionID = (elem).first;
            break;
        }
    }
    if (startSession == true && type == IGP && sessionID != static_cast<SessionId>(-1)) {
        // note: if the internal peer is not directly-connected to us, then we should know how to reach it.
        // this is done with the help of an intra-AS routing protocol (RIP, OSPF, EIGRP).
        InterfaceEntry *linkIntf = rt->getInterfaceForDestAddr(_BGPSessions[sessionID]->getPeerAddr());
        if (linkIntf == nullptr)
            throw cRuntimeError("No configuration interface for peer address: %s", _BGPSessions[sessionID]->getPeerAddr().str().c_str());

        _BGPSessions[sessionID]->setlinkIntf(linkIntf);
        _BGPSessions[sessionID]->startConnection();
    }
    return sessionID;
}

SessionId BgpRouter::findIdFromPeerAddr(std::map<SessionId, BgpSession *> sessions, Ipv4Address peerAddr)
{
    for (auto & session : sessions) {
        if ((session).second->getPeerAddr().equals(peerAddr))
            return (session).first;
    }
    return -1;
}

void BgpRouter::printOpenMessage(const BgpOpenMessage& openMsg)
{
    EV_INFO << "  My AS: " << openMsg.getMyAS() << "\n";
    EV_INFO << "  Hold time: " << openMsg.getHoldTime() << "s \n";
    EV_INFO << "  BGP Id: " << openMsg.getBGPIdentifier() << "\n";
    if(openMsg.getOptionalParameterArraySize() == 0)
        EV_INFO << "  Optional parameters: empty \n";
    for(uint32_t i = 0; i < openMsg.getOptionalParameterArraySize(); i++) {
        auto optParam = openMsg.getOptionalParameter(i);
        ASSERT(optParam != nullptr);
        EV_INFO << "  Optional parameter " << i+1 << ": \n";
        EV_INFO << "    Parameter type: " << optParam->getParameterType() << "\n";
        EV_INFO << "    Parameter length: " << optParam->getParameterValueLength() << "\n";
    }
}

void BgpRouter::printUpdateMessage(const BgpUpdateMessage& updateMsg)
{
    if(updateMsg.getWithdrawnRoutesArraySize() == 0)
        EV_INFO << "  Withdrawn routes: empty \n";
    for(uint32_t i = 0; i < updateMsg.getWithdrawnRoutesArraySize(); i++) {
        const BgpUpdateWithdrawnRoutes& withdrwan = updateMsg.getWithdrawnRoutes(i);
        EV_INFO << "  Withdrawn route " << i+1 << ": \n";
        EV_INFO << "    length: " << (int)withdrwan.length << "\n";
        EV_INFO << "    prefix: " << withdrwan.prefix << "\n";
    }
    if(updateMsg.getPathAttributesArraySize() == 0)
        EV_INFO << "  Path attribute: empty \n";
    for(uint32_t i = 0; i < updateMsg.getPathAttributesArraySize(); i++) {
        EV_INFO << "  Path attribute " << i+1 << ": [len:" << updateMsg.getPathAttributes(i)->getLength() <<"]\n";
        switch (updateMsg.getPathAttributes(i)->getTypeCode()) {
            case BgpUpdateAttributeTypeCode::ORIGIN: {
                auto& attr = *check_and_cast<const BgpUpdatePathAttributesOrigin*>(updateMsg.getPathAttributes(i));
                EV_INFO << "    ORIGIN: " << BgpSession::getTypeString(attr.getValue()) << "\n";
                break;
            }
            case BgpUpdateAttributeTypeCode::AS_PATH: {
                auto& asPath = *check_and_cast<const BgpUpdatePathAttributesAsPath*>(updateMsg.getPathAttributes(i));
                EV_INFO << "    AS_PATH:";
                for(uint32_t k = 0; k < asPath.getValueArraySize(); k++) {
                    const BgpAsPathSegment& asPathVal = asPath.getValue(k);
                    for(uint32_t n = 0; n < asPathVal.getAsValueArraySize(); n++) {
                        EV_INFO << " " << asPathVal.getAsValue(n);
                    }
                }
                EV_INFO << "\n";
                break;
            }
            case BgpUpdateAttributeTypeCode::NEXT_HOP: {
                auto& attr = *check_and_cast<const BgpUpdatePathAttributesNextHop*>(updateMsg.getPathAttributes(i));
                EV_INFO << "    NEXT_HOP: " << attr.getValue().str(false) << "\n";
                break;
            }
            case BgpUpdateAttributeTypeCode::LOCAL_PREF: {
                auto& attr = *check_and_cast<const BgpUpdatePathAttributesLocalPref*>(updateMsg.getPathAttributes(i));
                EV_INFO << "    LOCAL_PREF: " << attr.getValue() << "\n";
                break;
            }
            case BgpUpdateAttributeTypeCode::ATOMIC_AGGREGATE: {
                auto& attr = *check_and_cast<const BgpUpdatePathAttributesAtomicAggregate*>(updateMsg.getPathAttributes(i));
                (void)attr;
                EV_INFO << "    ATOMIC_AGGREGATE" << "\n";
                break;
            }
            case BgpUpdateAttributeTypeCode::AGGREGATOR: {
                auto& attr = *check_and_cast<const BgpUpdatePathAttributesAggregator*>(updateMsg.getPathAttributes(i));
                EV_INFO << "    AGGREGATOR: " << attr.getAsNumber() << ", " << attr.getBgpSpeaker() << "\n";
                break;
            }
            case BgpUpdateAttributeTypeCode::MULTI_EXIT_DISC: {
                auto& attr = *check_and_cast<const BgpUpdatePathAttributesMultiExitDisc*>(updateMsg.getPathAttributes(i));
                EV_INFO << "    MULTI_EXIT_DISC: " << attr.getValue() << "\n";
                break;
            }
        }
    }

    if (updateMsg.getNLRIArraySize() > 0) {
        auto NLRI_Base = updateMsg.getNLRI(0);
        EV_INFO << "  Network Layer Reachability Information (NLRI): \n";
        EV_INFO << "    NLRI length: " << (int)NLRI_Base.length << "\n";
        EV_INFO << "    NLRI prefix: " << NLRI_Base.prefix << "\n";
    }
}

//  void printNotificationMessage(const BgpNotificationMessage& notificationMsg)
//{

//}

void BgpRouter::printKeepAliveMessage(const BgpKeepAliveMessage& keepAliveMsg)
{
    // TODO: add code once implemented
}

bool BgpRouter::isRouteExcluded(const Ipv4Route &rtEntry)
{
    // all host-specific routes are excluded
    if(rtEntry.getNetmask() == Ipv4Address::ALLONES_ADDRESS)
        return true;

    // all static routes are excluded
    if(rtEntry.getSourceType() == IRoute::MANUAL)
        return true;

    // all BGP routes are excluded
    if(rtEntry.getSourceType() == IRoute::BGP)
        return true;

    // all RIP routes are excluded when redistributeRip is false
    if(rtEntry.getSourceType() == IRoute::RIP) {
        if(!redistributeRip)
            return true;
        else
            return false;
    }

    if(rtEntry.getSourceType() == IRoute::OSPF) {
        // all OSPF routes are excluded when redistributeOspf is false
        if(!redistributeOspf)
            return true;

        auto entry = static_cast<const ospfv2::Ospfv2RoutingTableEntry *>(&rtEntry);
        ASSERT(entry);

        if(entry->getPathType() == ospfv2::Ospfv2RoutingTableEntry::INTRAAREA) {
            if(redistributeOspfType.intraArea)
                return false;
            else
                return true;
        }

        if(entry->getPathType() == ospfv2::Ospfv2RoutingTableEntry::INTERAREA) {
            if(redistributeOspfType.interArea)
                return false;
            else
                return true;
        }

        int externalType = checkExternalRoute(&rtEntry);

        if(externalType == 1) {
            if(redistributeOspfType.externalType1)
                return false;
            else
                return true;
        }

        if(externalType == 2) {
            if(redistributeOspfType.externalType2)
                return false;
            else
                return true;
        }

        // exclude all other OSPF route types
        return true;
    }

    if (rtEntry.getSourceType() == IRoute::IFACENETMASK) {
        if(rtEntry.getInterface()->isLoopback())
            return true;
        else if(!redistributeRip && !redistributeOspf)
            return true;
        else
            return isExternalAddress(rtEntry);
    }

    // exclude all other routes
    return true;
}

bool BgpRouter::isExternalAddress(const Ipv4Route &rtEntry)
{
    for(auto &session : _BGPSessions) {
        if(session.second->getType() == EGP) {
            InterfaceEntry *exIntf = rt->getInterfaceForDestAddr(session.second->getPeerAddr());
            if(exIntf == rtEntry.getInterface())
                return true;
        }
    }

    return false;
}

bool BgpRouter::isDefaultRoute(const Ipv4Route *entry) const
{
    if(entry->getDestination().getInt() == 0 && entry->getNetmask().getInt() == 0)
        return true;
    return false;
}

bool BgpRouter::isReachable(const Ipv4Address addr) const
{
    if(addr.isUnspecified())
        return true;

    for(int i = 0; i < rt->getNumRoutes(); i++) {
        Ipv4Route *route = rt->getRoute(i);
        if(!isDefaultRoute(route) && route->getSourceType() != IRoute::BGP) {
            if(addr.doAnd(route->getNetmask()) == route->getDestination().doAnd(route->getNetmask()))
                return true;
        }
    }

    return false;
}

} // namespace bgp

} // namespace inet
