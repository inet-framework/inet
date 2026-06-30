//
// Copyright (C) 2010 Helene Lageber
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/routing/bgpv4/BgpRouter.h"

#include <algorithm>

#include "inet/common/ModuleAccess.h"
#include "inet/networklayer/ipv6/Ipv6InterfaceData.h"
#include "inet/routing/bgpv4/BgpSession.h"

namespace inet {
namespace bgp {

BgpRouter::BgpRouter(cSimpleModule *bgpModule, IInterfaceTable *ift, IRoutingTable *rt, const Protocol *networkProtocol)
{
    this->bgpModule = bgpModule;
    this->ift = ift;
    this->rt = rt;
    this->networkProtocol = networkProtocol;

    const char *routerIdStr = bgpModule->par("routerId");
    if (routerIdStr && *routerIdStr)
        routerIdParam = Ipv4Address(routerIdStr);

    ospfModule = findModuleFromPar<ospfv2::Ospfv2>(bgpModule->par("ospfRoutingModule"), bgpModule);
}

BgpRouter::~BgpRouter(void)
{
    for (auto& elem : _bgpSessions)
        delete (elem).second;

    for (auto& elem : adjRibInTable)
        delete elem;

    // Loc-RIB entries are clones exclusively owned by bgpRoutingTable (the forwarding-table
    // copies in 'rt' are separate objects freed by the routing table), so free them here.
    for (auto& elem : bgpRoutingTable)
        delete elem;

    for (auto& elem : _prefixListINOUT)
        delete elem;

    delete listeningSocket;
}

void BgpRouter::printSessionSummary()
{
    EV_DEBUG << "summary of BGP sessions: \n";
    for (auto& entry : _bgpSessions) {
        BgpSession *session = entry.second;
        BgpSessionType type = session->getType();
        if (type == IGP) {
            EV_DEBUG << "  IGP session to internal peer '" << session->getPeerAddr().str()
                     << "' starts at " << session->getStartEventTime() << "s \n";
        }
        else if (type == EGP) {
            EV_DEBUG << "  EGP session to external peer '" << session->getPeerAddr().str()
                     << "' starts at " << session->getStartEventTime() << "s \n";
        }
        else {
            EV_DEBUG << "  Unknown session to peer '" << session->getPeerAddr().str()
                     << "' starts at " << session->getStartEventTime() << "s \n";
        }
    }
}

void BgpRouter::addWatches()
{
    WATCH(myAsId);
    WATCH(_bgpSessions);
    WATCH(bgpRoutingTable);
    WATCH(adjRibInTable);
    _socketMap.addWatch();
}

void BgpRouter::recordStatistics()
{
    unsigned int statTab[BgpSession::NB_STATS] = {
        0, 0, 0, 0, 0, 0
    };

    for (auto& elem : _bgpSessions)
        (elem).second->getStatistics(statTab);

    bgpModule->recordScalar("OPENMsgSent", statTab[0]);
    bgpModule->recordScalar("OPENMsgRecv", statTab[1]);
    bgpModule->recordScalar("KeepAliveMsgSent", statTab[2]);
    bgpModule->recordScalar("KeepAliveMsgRcv", statTab[3]);
    bgpModule->recordScalar("UpdateMsgSent", statTab[4]);
    bgpModule->recordScalar("UpdateMsgRcv", statTab[5]);
}

void BgpRouter::closeSessions(bool abort)
{
    for (auto& elem : _bgpSessions) {
        TcpSocket *socket = elem.second->getSocket();
        if (socket) {
            _socketMap.removeSocket(socket);
            abort ? socket->abort() : socket->close();
        }
    }
    if (listeningSocket) {
        _socketMap.removeSocket(listeningSocket);
        abort ? listeningSocket->abort() : listeningSocket->close();
    }
}

SessionId BgpRouter::createIbgpSession(const char *peerAddr)
{
    SessionInfo info;
    info.sessionType = IGP;
    info.ASValue = myAsId;
    info.routerId = getRouterId();
    info.peerAddr = L3Address(peerAddr);
    info.sessionId = addressKey(info.peerAddr) + info.routerId.getInt();

    numIgpSessions++;

    SessionId newSessionId;
    newSessionId = info.sessionId;

    BgpSession *newSession = new BgpSession(*this);
    newSession->setInfo(info);

    _bgpSessions[newSessionId] = newSession;

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
    info.routerId = getRouterId();
    info.peerAddr = L3Address(peerAddr);
    info.linkIntf = rt->getOutputInterfaceForDestination(info.peerAddr);
    if (!info.linkIntf) {
        if (info.checkConnection)
            throw cRuntimeError("BGP Error: External BGP neighbor at address %s is not directly connected to BGP router %s", peerAddr, bgpModule->getOwner()->getFullName());
        else
            info.linkIntf = rt->getOutputInterfaceForDestination(info.myAddr);
    }
    ASSERT(info.linkIntf);
    info.sessionId = addressKey(info.peerAddr) + addressKey(getInterfaceAddress(info.linkIntf));
    numEgpSessions++;

    SessionId newSessionId;
    newSessionId = info.sessionId;

    BgpSession *newSession = new BgpSession(*this);
    newSession->setInfo(info);

    _bgpSessions[newSessionId] = newSession;

    return newSessionId;
}

void BgpRouter::setTimer(SessionId id, simtime_t *delayTab)
{
    _bgpSessions[id]->setTimers(delayTab);
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
    for (auto& session : _bgpSessions) {
        session.second->setNextHopSelf(nextHopSelf);
        session.second->setLocalPreference(localPreference);
    }
}

BgpRouteInfo *BgpRouter::createBgpRoutingTableEntry()
{
    if (isIpv6())
        return new BgpRoutingTableEntry6();
    return new BgpRoutingTableEntry();
}

BgpRouteInfo *BgpRouter::createBgpRoutingTableEntry(const IRoute *from)
{
    if (isIpv6())
        return new BgpRoutingTableEntry6(from);
    return new BgpRoutingTableEntry(from);
}

uint32_t BgpRouter::addressKey(const L3Address& addr)
{
    // AF-safe key for the session id. For IPv4 this is exactly the 32-bit address value, so
    // existing IPv4 session ids are unchanged; for IPv6 fold the 128-bit address into 32 bits.
    if (addr.getType() == L3Address::IPv6) {
        const uint32_t *w = addr.toIpv6().words();
        return w[0] ^ w[1] ^ w[2] ^ w[3];
    }
    return addr.toIpv4().getInt();
}

L3Address BgpRouter::getInterfaceAddress(NetworkInterface *ie)
{
    // the local BGP source address on a link: IPv4 interface address, or (for IPv6) the
    // preferred global address -- sessions need a routable address, not link-local.
    if (isIpv6())
        return ie->getProtocolData<Ipv6InterfaceData>()->getPreferredAddress();
    return ie->getProtocolData<Ipv4InterfaceData>()->getIPAddress();
}

void BgpRouter::addToAdvertiseList(const L3Address& address)
{
    bool routeFound = false;
    const IRoute *rtEntry = nullptr;
    for (int i = 0; i < rt->getNumRoutes(); i++) {
        rtEntry = rt->getRoute(i);
        if (rtEntry->getDestinationAsGeneric() == address) {
            routeFound = true;
            break;
        }
    }
    if (!routeFound)
        throw cRuntimeError("Network address '%s' is not found in the routing table of %s", address.str().c_str(), bgpModule->getOwner()->getFullName());

    auto position = std::find_if(advertiseList.begin(), advertiseList.end(),
            [&] (const L3Address& m) -> bool { return m == address; });
    if (position != advertiseList.end())
        throw cRuntimeError("Network address '%s' is already added to the advertised list of %s", address.str().c_str(), bgpModule->getOwner()->getFullName());
    advertiseList.push_back(address);

    BgpRouteInfo *bgpEntry = createBgpRoutingTableEntry(rtEntry);
    bgpEntry->addAS(myAsId);
    bgpEntry->setPathType(IGP);
    bgpEntry->setLocalPreference(bgpModule->par("localPreference").intValue());
    bgpEntry->setSourceSessionId(0);
    bgpRoutingTable.push_back(bgpEntry);
}

void BgpRouter::addToPrefixList(std::string nodeName, BgpRouteInfo *entry)
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
    for (auto& session : _bgpSessions) {
        if (session.second->getPeerAddr() == peer) {
            found = true;
            session.second->setNextHopSelf(nextHopSelf);
            break;
        }
    }
    if (!found)
        throw cRuntimeError("Neighbor address '%s' cannot be found in BGP router %s", peer.str(false).c_str(), bgpModule->getOwner()->getFullName());
}

void BgpRouter::setLocalPreference(Ipv4Address peer, int localPref)
{
    bool found = false;
    for (auto& session : _bgpSessions) {
        if (session.second->getPeerAddr() == peer) {
            found = true;
            session.second->setLocalPreference(localPref);
            break;
        }
    }
    if (!found)
        throw cRuntimeError("Neighbor address '%s' cannot be found in BGP router %s", peer.str(false).c_str(), bgpModule->getOwner()->getFullName());
}

void BgpRouter::setRedistributeOspf(std::string str)
{
    if (str == "") {
        redistributeOspf = false;
        return;
    }

    redistributeOspf = true;
    std::vector<std::string> tokens = cStringTokenizer(str.c_str()).asVector();

    for (auto& Ospfv2RouteType : tokens) {
        std::transform(Ospfv2RouteType.begin(), Ospfv2RouteType.end(), Ospfv2RouteType.begin(), ::tolower);
        if (Ospfv2RouteType == "o")
            redistributeOspfType.intraArea = true;
        else if (Ospfv2RouteType == "ia")
            redistributeOspfType.interArea = true;
        else if (Ospfv2RouteType == "e1")
            redistributeOspfType.externalType1 = true;
        else if (Ospfv2RouteType == "e2")
            redistributeOspfType.externalType2 = true;
        else
            throw cRuntimeError("Unknown OSPF redistribute type '%s' in BGP router %s", Ospfv2RouteType.c_str(), bgpModule->getOwner()->getFullName());
    }
}

void BgpRouter::processMessageFromTcp(cMessage *msg)
{
    TcpSocket *socket = check_and_cast_nullable<TcpSocket *>(_socketMap.findSocketFor(msg));
    if (!socket) {
        if (dynamic_cast<Packet *>(msg) != nullptr ||
            (msg->getKind() != TCP_I_AVAILABLE && msg->getKind() != TCP_I_ESTABLISHED))
        {
            // A restarted peer may still deliver late TCP payloads or control indications for a
            // socket id that has already been replaced locally. Ignore those stale messages
            // instead of rebuilding a new TcpSocket for the defunct connection.
            delete msg;
            return;
        }
        socket = new TcpSocket(msg);
        socket->setOutputGate(bgpModule->gate("socketOut"));
        L3Address peerAddr = socket->getRemoteAddress();
        SessionId i = findIdFromPeerAddr(_bgpSessions, peerAddr);
        if (i == static_cast<SessionId>(-1)) {
            socket->close();
            _socketMap.removeSocket(socket);
            delete socket;
            socket = nullptr;
            delete msg;
            return;
        }

        // RFC 4271 §6.8 connection collision detection: if we already have a live connection to
        // this peer, the peer connected too — keep exactly one. The BGP Identifier is the
        // connection's local address (see BgpSession::sendOpenMessage): ours is our source
        // address for the session, the peer's is this incoming connection's remote address
        // (== peerAddr). The connection opened by the higher-Identifier speaker survives; both
        // ends compute the same winner, so they agree.
        TcpSocket *current = _bgpSessions[i]->getSocket();
        if (current && (current->getState() == TcpSocket::CONNECTING || current->getState() == TcpSocket::CONNECTED)) {
            L3Address ourId = (_bgpSessions[i]->getType() == EGP)
                ? getInterfaceAddress(_bgpSessions[i]->getLinkIntf())
                : internalAddress;
            // higher BGP Identifier wins; keep the IPv4 comparison exact (byte-identical)
            bool weWin = isIpv6() ? (ourId > peerAddr)
                                  : (ourId.toIpv4().getInt() > peerAddr.toIpv4().getInt());
            if (weWin) {
                // we win: keep our connection, reject the peer's colliding one
                socket->abort();
                delete socket;
                delete msg;
                return;
            }
            // else peer wins: fall through and adopt the incoming connection (dropping ours)
        }

        socket->setCallback(this);
        socket->setUserData((void *)(uintptr_t)i);

        _socketMap.addSocket(socket);
        _bgpSessions[i]->getSocket()->abort();
        _socketMap.removeSocket(_bgpSessions[i]->getSocket());
        _bgpSessions[i]->setSocket(socket);
    }

    socket->processMessage(msg);
}

void BgpRouter::listenConnectionFromPeer(SessionId sessionId)
{
    // Ensure the single shared listening socket is up. One wildcard listener per router
    // accepts all incoming BGP connections on TCP_PORT; processMessageFromTcp() demuxes
    // accepted connections to the right session by peer address. This replaces the former
    // per-session listeners, which collided on the shared wildcard port when several
    // sessions reconnected at once (see plan §4 Phase 0 result).
    if (listeningSocket == nullptr)
        listeningSocket = new TcpSocket();

    if (listeningSocket->getState() == TcpSocket::CLOSED) {
        _socketMap.removeSocket(listeningSocket);
        listeningSocket->abort();
        listeningSocket->renewSocket();
    }

    if (listeningSocket->getState() != TcpSocket::LISTENING) {
        listeningSocket->setOutputGate(bgpModule->gate("socketOut"));
        listeningSocket->bind(TCP_PORT); // wildcard (address-family-unspecified) bind accepts IPv4 and IPv6
        listeningSocket->listen();
        _socketMap.addSocket(listeningSocket);

        EV_DEBUG << "Start listening for incoming BGP connections on *:" << (int)TCP_PORT
                 << " on " << bgpModule->getOwner()->getFullName() << std::endl;
    }
}

void BgpRouter::openTcpConnectionToPeer(SessionId sessionId)
{
    EV_DEBUG << "Opening a TCP connection to "
             << _bgpSessions[sessionId]->getPeerAddr()
             << ":" << (int)TCP_PORT << std::endl;

    TcpSocket *socket = _bgpSessions[sessionId]->getSocket();
    if (socket->getState() != TcpSocket::NOT_BOUND) {
        _socketMap.removeSocket(socket);
        socket->abort();
        socket->renewSocket();
    }
    socket->setCallback(this);
    socket->setUserData((void *)(uintptr_t)sessionId);
    socket->setOutputGate(bgpModule->gate("socketOut"));
    if (_bgpSessions[sessionId]->getType() == EGP) {
        NetworkInterface *intfEntry = _bgpSessions[sessionId]->getLinkIntf();
        if (intfEntry == nullptr)
            throw cRuntimeError("No configuration interface for external peer address: %s", _bgpSessions[sessionId]->getPeerAddr().str().c_str());
        // note: port=-1 stands for ephemeral port (=0 would be literally port 0)
        socket->bind(_bgpSessions[sessionId]->getMyAddr(), -1);

        int ebgpMH = _bgpSessions[sessionId]->getEbgpMultihop();
        if (ebgpMH > 1)
            socket->setTimeToLive(ebgpMH);
        else if (ebgpMH == 1)
            socket->setTimeToLive(1);
        else
            throw cRuntimeError("ebgpMultihop should be >=1");
    }
    else if (_bgpSessions[sessionId]->getType() == IGP) {
        NetworkInterface *intfEntry = _bgpSessions[sessionId]->getLinkIntf();
        if (!intfEntry)
            intfEntry = rt->getOutputInterfaceForDestination(_bgpSessions[sessionId]->getPeerAddr());
        if (intfEntry == nullptr)
            throw cRuntimeError("No configuration interface for internal peer address: %s", _bgpSessions[sessionId]->getPeerAddr().str().c_str());
        _bgpSessions[sessionId]->setlinkIntf(intfEntry);
        if (internalAddress.isUnspecified())
            throw cRuntimeError("Internal address is not specified for router %s", bgpModule->getOwner()->getFullName());
        // note: port=-1 stands for ephemeral port (=0 would be literally port 0)
        socket->bind(internalAddress, -1);
    }
    _socketMap.addSocket(socket);

    socket->connect(_bgpSessions[sessionId]->getPeerAddr(), TCP_PORT);
}

void BgpRouter::socketEstablished(TcpSocket *socket)
{
    int connId = socket->getSocketId();
    _currSessionId = findIdFromSocketConnId(_bgpSessions, connId);
    if (_currSessionId == static_cast<SessionId>(-1)) {
        throw cRuntimeError("socket id=%d is not established", connId);
    }

    // if it's an IGP Session, TCPConnectionConfirmed only if all EGP Sessions established
    if (_bgpSessions[_currSessionId]->getType() == IGP &&
        this->findNextSession(EGP) != static_cast<SessionId>(-1))
    {
        _bgpSessions[_currSessionId]->getFsm()->TcpConnectionFails();
    }
    else {
        _bgpSessions[_currSessionId]->getFsm()->TcpConnectionConfirmed();
    }
    // Note: the shared listening socket is intentionally left listening (it is not a
    // per-session resource and must stay up to accept connections for other sessions).
}

void BgpRouter::socketFailure(TcpSocket *socket, int code)
{
    int connId = socket->getSocketId();
    _currSessionId = findIdFromSocketConnId(_bgpSessions, connId);
    if (_currSessionId != static_cast<SessionId>(-1)) {
        _bgpSessions[_currSessionId]->getFsm()->TcpConnectionFails();
    }
}

void BgpRouter::socketPeerClosed(TcpSocket *socket)
{
    socket->close();
    int connId = socket->getSocketId();
    _currSessionId = findIdFromSocketConnId(_bgpSessions, connId);
    if (_currSessionId != static_cast<SessionId>(-1))
        _bgpSessions[_currSessionId]->getFsm()->TcpConnectionFails();
}

void BgpRouter::socketDataArrived(TcpSocket *socket)
{
    auto queue = socket->getReadBuffer();
    while (queue->has<BgpHeader>()) {
        auto header = queue->pop<BgpHeader>();
        processChunks(*header.get());
    }
}

void BgpRouter::socketDataArrived(TcpSocket *socket, Packet *packet, bool urgent)
{
    _currSessionId = findIdFromSocketConnId(_bgpSessions, socket->getSocketId());
    if (_currSessionId != static_cast<SessionId>(-1))
        BufferingCallback::socketDataArrived(socket, packet, urgent);
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
    BgpSession *session = _bgpSessions[_currSessionId];
    EV_INFO << "Processing BGP OPEN message from "
            << session->getPeerAddr().str()
            << " with contents: \n";
    printOpenMessage(msg);
    session->getFsm()->OpenMsgEvent();
}

void BgpRouter::processMessage(const BgpKeepAliveMessage& msg)
{
    BgpSession *session = _bgpSessions[_currSessionId];
    EV_INFO << "Processing BGP Keep Alive message from "
            << session->getPeerAddr().str()
            << " with contents: \n";
    printKeepAliveMessage(msg);
    session->getFsm()->KeepAliveMsgEvent();
}

void BgpRouter::processMessage(const BgpUpdateMessage& msg)
{
    BgpSession *session = _bgpSessions[_currSessionId];
    EV_INFO << "Processing BGP Update message from "
            << session->getPeerAddr().str()
            << " with contents: \n";
    printUpdateMessage(msg);
    session->getFsm()->UpdateMsgEvent();

    std::vector<PrefixKey> affectedPrefixes;  // prefixes whose best path may need to be reselected
    std::vector<PrefixKey> withdrawnPrefixes; // prefixes explicitly withdrawn by this peer in this UPDATE

    for (size_t i = 0; i < msg.getWithdrawnRoutesArraySize(); ++i)
        addAffectedPrefix(withdrawnPrefixes, msg.getWithdrawnRoutes(i).prefix, msg.getWithdrawnRoutes(i).length);

    for (size_t i = 0; i < msg.getPathAttributesArraySize(); i++) {
        if (msg.getPathAttributes(i)->getTypeCode() == BgpUpdateAttributeTypeCode::MP_UNREACH_NLRI) {
            auto& mp = *check_and_cast<const BgpUpdatePathAttributesMpUnreachNlri *>(msg.getPathAttributes(i));
            for (size_t n = 0; n < mp.getWithdrawnRoutesArraySize(); ++n)
                addAffectedPrefix(withdrawnPrefixes, mp.getWithdrawnRoutes(n).prefix, mp.getWithdrawnRoutes(n).length);
        }
    }

    if (isIpv6()) {
        for (size_t i = 0; i < msg.getPathAttributesArraySize(); i++) {
            if (msg.getPathAttributes(i)->getTypeCode() == BgpUpdateAttributeTypeCode::MP_REACH_NLRI) {
                auto& mp = *check_and_cast<const BgpUpdatePathAttributesMpReachNlri *>(msg.getPathAttributes(i));
                for (size_t n = 0; n < mp.getNlriArraySize(); ++n) {
                    BgpRouteInfo *entry = createBgpRoutingTableEntry();
                    entry->setDestination(mp.getNlri(n).prefix);
                    entry->setPrefixLength(mp.getNlri(n).length);
                    BgpProcessResult result = decisionProcess(msg, entry, _currSessionId);
                    if (result == NEW_ROUTE_ADDED || result == ROUTE_DESTINATION_CHANGED)
                        addAffectedPrefix(affectedPrefixes, entry->getDestinationAsGeneric(), entry->getPrefixLength());
                }
            }
        }
    }
    else {
        for (size_t i = 0; i < msg.getNlriArraySize(); ++i) {
            BgpRouteInfo *entry = createBgpRoutingTableEntry();
            entry->setDestination(msg.getNlri(i).prefix);
            entry->setPrefixLength(msg.getNlri(i).length);
            BgpProcessResult result = decisionProcess(msg, entry, _currSessionId);
            if (result == NEW_ROUTE_ADDED || result == ROUTE_DESTINATION_CHANGED)
                addAffectedPrefix(affectedPrefixes, entry->getDestinationAsGeneric(), entry->getPrefixLength());
        }
    }

    for (const auto& prefix : withdrawnPrefixes) {
        removeAdjRibInRoute(_currSessionId, prefix.prefix, prefix.prefixLength);
        addAffectedPrefix(affectedPrefixes, prefix.prefix, prefix.prefixLength);
    }

    // Re-run best-path selection only for prefixes touched by this UPDATE.
    for (const auto& prefix : affectedPrefixes)
        installBestRouteForPrefix(prefix.prefix, prefix.prefixLength, _currSessionId);
}

BgpProcessResult BgpRouter::asLoopDetection(BgpRouteInfo *entry, AsId myAS)
{
    for (unsigned int i = 1; i < entry->getASCount(); i++) {
        if (myAS == entry->getAS(i))
            return ASLOOP_DETECTED;
    }
    return ASLOOP_NO_DETECTED;
}

BgpProcessResult BgpRouter::decisionProcess(const BgpUpdateMessage& msg, BgpRouteInfo *entry, SessionId sessionIndex)
{
    for (size_t i = 0; i < msg.getPathAttributesArraySize(); i++) {
        switch (msg.getPathAttributes(i)->getTypeCode()) {
            case BgpUpdateAttributeTypeCode::ORIGIN: {
                auto attr = check_and_cast<const BgpUpdatePathAttributesOrigin *>(msg.getPathAttributes(i));
                entry->setPathType(attr->getValue());
                break;
            }
            case BgpUpdateAttributeTypeCode::NEXT_HOP: {
                auto attr = check_and_cast<const BgpUpdatePathAttributesNextHop *>(msg.getPathAttributes(i));
                entry->setNextHop(attr->getValue());
                break;
            }
            case BgpUpdateAttributeTypeCode::LOCAL_PREF: {
                auto attr = check_and_cast<const BgpUpdatePathAttributesLocalPref *>(msg.getPathAttributes(i));
                entry->setLocalPreference(attr->getValue());
                break;
            }
            case BgpUpdateAttributeTypeCode::AS_PATH: {
                auto& asPath = *check_and_cast<const BgpUpdatePathAttributesAsPath *>(msg.getPathAttributes(i));
                for (size_t k = 0; k < asPath.getValueArraySize(); k++) {
                    const BgpAsPathSegment& asPathVal = asPath.getValue(k);
                    for (size_t n = 0; n < asPathVal.getAsValueArraySize(); n++)
                        entry->addAS(asPathVal.getAsValue(n));
                }
                break;
            }
            case BgpUpdateAttributeTypeCode::MP_REACH_NLRI: {
                auto attr = check_and_cast<const BgpUpdatePathAttributesMpReachNlri *>(msg.getPathAttributes(i));
                entry->setNextHop(attr->getNextHop());
                break;
            }
            default:
                break;
        }
    }

    if (entry->getLocalPreference() == 0)
        entry->setLocalPreference(bgpModule->par("localPreference").intValue());

    if (asLoopDetection(entry, myAsId) != ASLOOP_NO_DETECTED) {
        delete entry;
        return ASLOOP_DETECTED;
    }

    // Don't add the route if it exists in PrefixListINTable or in ASListINTable
    if (isInTable(_prefixListIN, entry) != (unsigned long)-1 || isInASList(_ASListIN, entry)) {
        delete entry;
        return RESULT0;
    }

    BgpSessionType type = _bgpSessions[sessionIndex]->getType();
    if (type == IGP) {
        entry->setAdminDist(Ipv4Route::dBGPInternal);
        entry->setIBgpLearned(true);
    }
    else if (type == EGP)
        entry->setAdminDist(Ipv4Route::dBGPExternal);
    else
        entry->setAdminDist(Ipv4Route::dUnknown);
    entry->setSourceSessionId(sessionIndex);
    entry->setSourcePeerAddress(_bgpSessions[sessionIndex]->getPeerAddr());
    entry->setInterface(_bgpSessions[sessionIndex]->getLinkIntf());

    // Adj-RIB-In keeps every path learned from every peer; best-path selection is deferred
    // to installBestRouteForPrefix(), which populates the Loc-RIB and the routing table.
    upsertAdjRibInRoute(entry);
    return NEW_ROUTE_ADDED;
}

bool BgpRouter::tieBreakingProcess(BgpRouteInfo *oldEntry, BgpRouteInfo *entry)
{
    return !isBetterRoute(entry, oldEntry);
}

void BgpRouter::updateSendProcess(BgpProcessResult type, SessionId sessionIndex, BgpRouteInfo *entry)
{
    // Don't send the update Message if the route exists in listOUTTable
    // SESSION = EGP : send an update message to all BGP Peer (EGP && IGP)
    // if it is not the currentSession and if the session is already established
    // SESSION = IGP : send an update message to External BGP Peer (EGP) only
    // if it is not the currentSession and if the session is already established
    for (auto& elem : _bgpSessions) {
        if (isInTable(_prefixListOUT, entry) != (unsigned long)-1 || isInASList(_ASListOUT, entry) ||
            ((elem).first == sessionIndex && type != NEW_SESSION_ESTABLISHED) ||
            (type == NEW_SESSION_ESTABLISHED && (elem).first != sessionIndex) ||
            !(elem).second->isEstablished() ||
            elem.second->getSocket() == nullptr ||
            elem.second->getSocket()->getState() != TcpSocket::CONNECTED)
        {
            continue;
        }

        // if the next hop is not reachable
        if (!isReachable(entry->getNextHopAsGeneric()))
            continue;

        BgpSessionType sType = _bgpSessions[sessionIndex]->getType();

        // BGP split horizon: skip if this prefix is learned over I-BGP and we are
        // advertising it to another internal peer.
        if (entry->isIBgpLearned() && sType == IGP && elem.second->getType() == IGP) {
            EV_INFO << "BGP Split Horizon: prevent advertisement of network "
                    << entry->getDestinationAsGeneric() << "/" << entry->getPrefixLength();
            continue;
        }

        if ((sType == IGP && (elem).second->getType() == EGP) ||
            sType == EGP ||
            type == ROUTE_DESTINATION_CHANGED ||
            type == NEW_SESSION_ESTABLISHED)
        {
            std::vector<BgpUpdatePathAttributes *> content;

            unsigned int nbAS = entry->getASCount();
            auto asPath = new BgpUpdatePathAttributesAsPath();
            content.push_back(asPath);
            asPath->setValueArraySize(1);
            asPath->getValueForUpdate(0).setType(AS_SEQUENCE);
            asPath->getValueForUpdate(0).setLength(0);
            if ((elem).second->getType() == EGP) {
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
            else if ((elem).second->getType() == IGP) {
                asPath->getValueForUpdate(0).setAsValueArraySize(nbAS);
                asPath->getValueForUpdate(0).setLength(nbAS);
                for (unsigned int j = 0; j < nbAS; j++)
                    asPath->getValueForUpdate(0).setAsValue(j, entry->getAS(j));

                auto localPref = new BgpUpdatePathAttributesLocalPref();
                content.push_back(localPref);
                localPref->setLength(4);
                localPref->setValue(_bgpSessions[sessionIndex]->getLocalPreference());
            }
            asPath->setLength(2 + 2 * asPath->getValue(0).getAsValueArraySize());

            // next hop: our own address on the link for EGP / next-hop-self, else the learned next hop
            L3Address nextHop = (sType == EGP || _bgpSessions[sessionIndex]->getNextHopSelf())
                ? getInterfaceAddress((elem).second->getLinkIntf())
                : entry->getNextHopAsGeneric();

            if (isIpv6()) {
                // a locally-originated route is built from a connected route and carries no
                // learned next hop; advertise our own address on the link so that iBGP peers
                // can resolve it via the IGP (RFC 4271: a router uses itself as the next hop
                // for the networks it originates)
                if (nextHop.isUnspecified())
                    nextHop = getInterfaceAddress((elem).second->getLinkIntf());
                // RFC 4760: carry the IPv6 next hop + reachability in MP_REACH_NLRI; ORIGIN stays
                // a normal attribute. The legacy NEXT_HOP attribute and NLRI field are not used.
                auto originAttr = new BgpUpdatePathAttributesOrigin;
                content.push_back(originAttr);
                originAttr->setValue((BgpSessionType)entry->getPathType());

                auto mpReach = new BgpUpdatePathAttributesMpReachNlri();
                mpReach->setAfi(2);  // IPv6
                mpReach->setSafi(1); // unicast
                mpReach->setNextHop(nextHop);
                mpReach->setNextHopLength(16);
                BgpUpdateNlri6 nlri6;
                nlri6.prefix = entry->getDestinationAsGeneric().getPrefix(entry->getPrefixLength());
                nlri6.length = entry->getPrefixLength();
                mpReach->setNlriArraySize(1);
                mpReach->setNlri(0, nlri6);
                mpReach->setLength(computePathAttributeBytes(*mpReach) - 3); // attribute value length (1-octet length field)
                content.push_back(mpReach);

                (elem).second->sendUpdateMessage(content);
            }
            else {
                auto nextHopAttr = new BgpUpdatePathAttributesNextHop;
                content.push_back(nextHopAttr);
                nextHopAttr->setValue(nextHop.toIpv4());

                auto originAttr = new BgpUpdatePathAttributesOrigin;
                content.push_back(originAttr);
                originAttr->setValue((BgpSessionType)entry->getPathType());

                BgpUpdateNlri nlri;
                nlri.prefix = entry->getDestinationAsGeneric().getPrefix(entry->getPrefixLength()).toIpv4();
                nlri.length = (unsigned char)entry->getPrefixLength();

                (elem).second->sendUpdateMessage(content, nlri);
            }
        }
    }
}

/*
 *  Delete BGP Routing entry, if the route deleted correctly return true, false else.
 *  Side effects when returns true:
 *      bgpRoutingTable changed, iterators on bgpRoutingTable will be invalid.
 */
bool BgpRouter::deleteBgpRoutingEntry(BgpRouteInfo *entry)
{
    if (deleteLocRibEntry(entry))
        return true;

    for (auto it = adjRibInTable.begin(); it != adjRibInTable.end(); ++it) {
        if (*it == entry) {
            delete *it;
            adjRibInTable.erase(it);
            return true;
        }
    }
    return false;
}

bool BgpRouter::deleteLocRibEntry(BgpRouteInfo *entry)
{
    for (auto it = bgpRoutingTable.begin();
         it != bgpRoutingTable.end(); it++) {
        if (*it == entry) {
            bgpRoutingTable.erase(it);
            for (int i = 0; i < rt->getNumRoutes(); ++i) {
                IRoute *route = rt->getRoute(i);
                if (route->getSourceType() == IRoute::BGP &&
                    route->getDestinationAsGeneric().getPrefix(route->getPrefixLength()) == entry->getDestinationAsGeneric().getPrefix(entry->getPrefixLength()) &&
                    route->getPrefixLength() == entry->getPrefixLength())
                {
                    rt->deleteRoute(route);
                    break;
                }
            }
            delete entry;
            return true;
        }
    }
    return false;
}

BgpRouteInfo *BgpRouter::findLocRibEntry(const L3Address& prefix, int prefixLength) const
{
    for (auto entry : bgpRoutingTable) {
        if (prefixMatches(entry, prefix, prefixLength))
            return entry;
    }
    return nullptr;
}

BgpRouteInfo *BgpRouter::selectBestAdjRibInRoute(const L3Address& prefix, int prefixLength) const
{
    BgpRouteInfo *best = nullptr;
    for (auto entry : adjRibInTable) {
        if (!prefixMatches(entry, prefix, prefixLength))
            continue;
        if (entry->isIBgpLearned() && !isReachable(entry->getNextHopAsGeneric()))
            continue;
        if (!entry->isIBgpLearned() && !isReachable(entry->getNextHopAsGeneric()))
            continue;
        if (best == nullptr || isBetterRoute(entry, best))
            best = entry;
    }
    return best;
}

void BgpRouter::upsertAdjRibInRoute(BgpRouteInfo *entry)
{
    for (auto it = adjRibInTable.begin(); it != adjRibInTable.end(); ++it) {
        BgpRouteInfo *current = *it;
        if (current->getSourceSessionId() == entry->getSourceSessionId() && samePrefix(current, entry)) {
            delete current;
            *it = entry;
            return;
        }
    }
    adjRibInTable.push_back(entry);
}

bool BgpRouter::removeAdjRibInRoute(SessionId sessionId, const L3Address& prefix, int prefixLength)
{
    for (auto it = adjRibInTable.begin(); it != adjRibInTable.end(); ++it) {
        BgpRouteInfo *entry = *it;
        if (entry->getSourceSessionId() == sessionId && prefixMatches(entry, prefix, prefixLength)) {
            delete entry;
            adjRibInTable.erase(it);
            return true;
        }
    }
    return false;
}

std::vector<BgpRouter::PrefixKey> BgpRouter::removeAdjRibInRoutesFromSession(SessionId sessionId)
{
    std::vector<PrefixKey> affectedPrefixes;
    for (auto it = adjRibInTable.begin(); it != adjRibInTable.end(); ) {
        BgpRouteInfo *entry = *it;
        if (entry->getSourceSessionId() == sessionId) {
            addAffectedPrefix(affectedPrefixes, entry->getDestinationAsGeneric(), entry->getPrefixLength());
            delete entry;
            it = adjRibInTable.erase(it);
        }
        else
            ++it;
    }
    return affectedPrefixes;
}

void BgpRouter::installBestRouteForPrefix(const L3Address& prefix, int prefixLength, SessionId triggeringSessionId)
{
    BgpRouteInfo *current = findLocRibEntry(prefix, prefixLength);
    if (current != nullptr && current->getSourceSessionId() == 0)
        return;

    BgpRouteInfo *best = selectBestAdjRibInRoute(prefix, prefixLength);
    if (current != nullptr && best != nullptr && samePath(current, best))
        return;

    // Keep a snapshot of the old Loc-RIB route so we can send an explicit withdraw if
    // no alternative remains after re-running the decision process.
    BgpRouteInfo *oldEntry = current;
    bool hadPreviousRoute = (oldEntry != nullptr);
    BgpRouteInfo *oldSnapshot = oldEntry != nullptr ? cloneRoute(oldEntry) : nullptr;
    if (oldEntry != nullptr)
        deleteLocRibEntry(oldEntry); // frees *oldEntry; do not dereference oldEntry past this point

    if (best == nullptr) {
        EV_INFO << "Prefix " << prefix.getPrefix(prefixLength) << "/" << prefixLength
                << ": no alternative path in Adj-RIB-In, sending explicit BGP withdrawal\n";
        if (oldSnapshot != nullptr)
            withdrawSendProcess(triggeringSessionId, oldSnapshot);
        delete oldSnapshot;
        return;
    }

    BgpRouteInfo *selected = cloneRoute(best);
    SessionId sessionId = selected->getSourceSessionId();
    if (sessionId != 0) {
        selected->setInterface(_bgpSessions.at(sessionId)->getLinkIntf());
        if (selected->isIBgpLearned()) {
            L3Address onLinkNextHop = rt->getNextHopForDestination(selected->getNextHopAsGeneric());
            if (!onLinkNextHop.isUnspecified())
                selected->setNextHop(onLinkNextHop);
        }
    }
    bgpRoutingTable.push_back(selected);

    int indexIp = isInRoutingTable(rt, selected->getDestinationAsGeneric());
    if (indexIp != -1 && rt->getRoute(indexIp)->getSourceType() == IRoute::BGP)
        rt->deleteRoute(rt->getRoute(indexIp));

    // The Loc-RIB entry and the forwarding-table entry are kept separate so removing
    // one cannot invalidate pointers stored in the other structure.
    if (selected->getSourceSessionId() != 0 && isReachable(selected->getNextHopAsGeneric())) {
        BgpRouteInfo *routeForRt = cloneRoute(selected);
        rt->addRoute(routeForRt->asRoute());
    }

    auto selectedSession = _bgpSessions.find(selected->getSourceSessionId());
    if (selectedSession != _bgpSessions.end() && selectedSession->second->getType() == EGP && ospfExist(rt) && redistributeInternal) {
        NetworkInterface *ie = selected->getInterface();
        if (!ie)
            throw cRuntimeError("Model error: interface entry is nullptr");
        ospfv2::Ipv4AddressRange ospfNetAddr;
        ospfNetAddr.address = selected->getDestinationAsGeneric().toIpv4();
        ospfNetAddr.mask = Ipv4Address::makeNetmask(selected->getPrefixLength());
        if (!ospfModule)
            throw cRuntimeError("Cannot find the OSPF module on router %s", bgpModule->getFullName());
        ospfModule->insertExternalRoute(ie->getInterfaceId(), ospfNetAddr);
    }

    updateSendProcess(hadPreviousRoute ? ROUTE_DESTINATION_CHANGED : NEW_ROUTE_ADDED, selected->getSourceSessionId(), selected);
    delete oldSnapshot;
}

void BgpRouter::withdrawRoutesFromSession(SessionId sessionId)
{
    auto affectedPrefixes = removeAdjRibInRoutesFromSession(sessionId);
    auto sessionIt = _bgpSessions.find(sessionId);
    EV_INFO << "BGP session " << sessionId << " to peer "
            << (sessionIt != _bgpSessions.end() ? sessionIt->second->getPeerAddr().str() : std::string("?"))
            << " lost: removed its Adj-RIB-In routes, re-running best-path selection for "
            << affectedPrefixes.size() << " affected prefix(es)\n";
    for (const auto& prefix : affectedPrefixes)
        installBestRouteForPrefix(prefix.prefix, prefix.prefixLength, sessionId);
}

void BgpRouter::withdrawSendProcess(SessionId sessionIndex, const BgpRouteInfo *entry)
{
    if (entry == nullptr)
        return;

    for (auto& elem : _bgpSessions) {
        if (isInTable(_prefixListOUT, const_cast<BgpRouteInfo *>(entry)) != (unsigned long)-1 || isInASList(_ASListOUT, const_cast<BgpRouteInfo *>(entry)) ||
            elem.first == sessionIndex || !elem.second->isEstablished() ||
            elem.second->getSocket() == nullptr ||
            elem.second->getSocket()->getState() != TcpSocket::CONNECTED)
        {
            continue;
        }

        auto sourceSession = _bgpSessions.find(sessionIndex);
        if (sourceSession == _bgpSessions.end())
            continue;
        BgpSessionType sourceType = sourceSession->second->getType();
        if (entry->isIBgpLearned() && sourceType == IGP && elem.second->getType() == IGP)
            continue;

        if ((sourceType == IGP && elem.second->getType() == EGP) || sourceType == EGP || sourceType == INCOMPLETE)
            elem.second->sendWithdrawMessage(entry->getDestinationAsGeneric(), entry->getPrefixLength());
    }
}

void BgpRouter::addAffectedPrefix(std::vector<PrefixKey>& prefixes, const L3Address& prefix, int prefixLength)
{
    for (const auto& entry : prefixes) {
        if (entry.prefix == prefix && entry.prefixLength == prefixLength)
            return;
    }
    prefixes.push_back({prefix.getPrefix(prefixLength), prefixLength});
}

bool BgpRouter::prefixMatches(const BgpRouteInfo *entry, const L3Address& prefix, int prefixLength)
{
    return entry->getPrefixLength() == prefixLength &&
           entry->getDestinationAsGeneric().getPrefix(prefixLength) == prefix.getPrefix(prefixLength);
}

bool BgpRouter::samePrefix(const BgpRouteInfo *first, const BgpRouteInfo *second)
{
    return prefixMatches(first, second->getDestinationAsGeneric(), second->getPrefixLength());
}

bool BgpRouter::samePath(const BgpRouteInfo *first, const BgpRouteInfo *second)
{
    if (!samePrefix(first, second))
        return false;
    if (first->getLocalPreference() != second->getLocalPreference() ||
        first->getPathType() != second->getPathType() ||
        first->isIBgpLearned() != second->isIBgpLearned() ||
        first->getSourceSessionId() != second->getSourceSessionId() ||
        first->getASCount() != second->getASCount())
    {
        return false;
    }
    for (unsigned int i = 0; i < first->getASCount(); ++i) {
        if (first->getAS(i) != second->getAS(i))
            return false;
    }
    return true;
}

bool BgpRouter::isBetterRoute(const BgpRouteInfo *candidate, const BgpRouteInfo *current) const
{
    if (candidate->getLocalPreference() != current->getLocalPreference())
        return candidate->getLocalPreference() > current->getLocalPreference();
    if (candidate->getASCount() != current->getASCount())
        return candidate->getASCount() < current->getASCount();
    if (candidate->getPathType() != current->getPathType())
        return candidate->getPathType() < current->getPathType();
    return candidate->getSourcePeerAddress() < current->getSourcePeerAddress();
}

BgpRouteInfo *BgpRouter::cloneRoute(const BgpRouteInfo *entry) const
{
    return entry->clone();
}

/*return index of the Ipv4 table if the route is found, -1 else*/
int BgpRouter::isInRoutingTable(IRoutingTable *rtTable, const L3Address& addr)
{
    for (int i = 0; i < rtTable->getNumRoutes(); i++) {
        const IRoute *entry = rtTable->getRoute(i);
        if (addr.getPrefix(entry->getPrefixLength()) == entry->getDestinationAsGeneric().getPrefix(entry->getPrefixLength())) {
            if (isDefaultRoute(entry) && !addr.isUnspecified())
                continue;
            else
                return i;
        }
    }
    return -1;
}

SessionId BgpRouter::findIdFromSocketConnId(std::map<SessionId, BgpSession *> sessions, int connId)
{
    for (auto& session : sessions) {
        TcpSocket *socket = (session).second->getSocket();
        if (socket->getSocketId() == connId) {
            return (session).first;
        }
    }
    return -1;
}

/*return index of the table if the route is found, -1 else*/
unsigned long BgpRouter::isInTable(std::vector<BgpRouteInfo *> rtTable, BgpRouteInfo *entry)
{
    for (unsigned long i = 0; i < rtTable.size(); i++) {
        BgpRouteInfo *entryCur = rtTable[i];
        if (entry->getDestinationAsGeneric().getPrefix(entry->getPrefixLength()) ==
            entryCur->getDestinationAsGeneric().getPrefix(entryCur->getPrefixLength()))
        {
            return i;
        }
    }
    return -1;
}

/*return true if the AS is found, false else*/
bool BgpRouter::isInASList(std::vector<AsId> ASList, BgpRouteInfo *entry)
{
    for (auto& elem : ASList) {
        for (unsigned int i = 0; i < entry->getASCount(); i++) {
            if ((elem) == entry->getAS(i)) {
                return true;
            }
        }
    }
    return false;
}

/*return true if OSPF exists, false else*/
bool BgpRouter::ospfExist(IRoutingTable *rtTable)
{
    for (int i = 0; i < rtTable->getNumRoutes(); i++) {
        if (rtTable->getRoute(i)->getSourceType() == IRoute::OSPF) {
            return true;
        }
    }
    return false;
}

/*return sessionId if the session is found, -1 else*/
SessionId BgpRouter::findNextSession(BgpSessionType type, bool startSession)
{
    SessionId sessionId = -1;
    for (auto& elem : _bgpSessions) {
        if ((elem).second->getType() == type && !(elem).second->isEstablished()) {
            sessionId = (elem).first;
            break;
        }
    }
    if (startSession == true && type == IGP && sessionId != static_cast<SessionId>(-1)) {
        // note: if the internal peer is not directly-connected to us, then we should know how to reach it.
        // this is done with the help of an intra-AS routing protocol (RIP, OSPF, EIGRP).
        NetworkInterface *linkIntf = rt->getOutputInterfaceForDestination(_bgpSessions[sessionId]->getPeerAddr());
        if (linkIntf == nullptr) {
            // The IGP has not installed a route to this internal peer yet (it may still be
            // converging). Defer the session start and retry, instead of failing.
            _bgpSessions[sessionId]->scheduleStartRetry();
            return sessionId;
        }

        _bgpSessions[sessionId]->setlinkIntf(linkIntf);
        _bgpSessions[sessionId]->startConnection();
    }
    return sessionId;
}

SessionId BgpRouter::findIdFromPeerAddr(std::map<SessionId, BgpSession *> sessions, const L3Address& peerAddr)
{
    for (auto& session : sessions) {
        if ((session).second->getPeerAddr() == peerAddr)
            return (session).first;
    }
    return -1;
}

void BgpRouter::printOpenMessage(const BgpOpenMessage& openMsg)
{
    EV_INFO << "  My AS: " << openMsg.getMyAS() << "\n";
    EV_INFO << "  Hold time: " << openMsg.getHoldTime() << "s \n";
    EV_INFO << "  BGP Id: " << openMsg.getBgpIdentifier() << "\n";
    if (openMsg.getOptionalParameterArraySize() == 0)
        EV_INFO << "  Optional parameters: empty \n";
    for (size_t i = 0; i < openMsg.getOptionalParameterArraySize(); i++) {
        auto optParam = openMsg.getOptionalParameter(i);
        ASSERT(optParam != nullptr);
        EV_INFO << "  Optional parameter " << i + 1 << ": \n";
        EV_INFO << "    Parameter type: " << optParam->getParameterType() << "\n";
        EV_INFO << "    Parameter length: " << optParam->getParameterValueLength() << "\n";
        if (auto caps = dynamic_cast<const BgpOptionalParameterCapabilities *>(optParam)) {
            for (size_t c = 0; c < caps->getCapabilityArraySize(); c++) {
                auto cap = caps->getCapability(c);
                if (auto mp = dynamic_cast<const BgpCapabilityMultiprotocol *>(cap))
                    EV_INFO << "    Capability: Multiprotocol Extensions, AFI=" << mp->getAfi()
                            << " SAFI=" << (int)mp->getSafi() << "\n";
                else
                    EV_INFO << "    Capability: code " << (int)cap->getCapabilityCode() << "\n";
            }
        }
    }
}

void BgpRouter::printUpdateMessage(const BgpUpdateMessage& updateMsg)
{
    if (updateMsg.getWithdrawnRoutesArraySize() == 0)
        EV_INFO << "  Withdrawn routes: empty \n";
    for (size_t i = 0; i < updateMsg.getWithdrawnRoutesArraySize(); i++) {
        const BgpUpdateWithdrawnRoutes& withdrwan = updateMsg.getWithdrawnRoutes(i);
        EV_INFO << "  Withdrawn route " << i + 1 << ": \n";
        EV_INFO << "    length: " << (int)withdrwan.length << "\n";
        EV_INFO << "    prefix: " << withdrwan.prefix << "\n";
    }
    if (updateMsg.getPathAttributesArraySize() == 0)
        EV_INFO << "  Path attribute: empty \n";
    for (size_t i = 0; i < updateMsg.getPathAttributesArraySize(); i++) {
        EV_INFO << "  Path attribute " << i + 1 << ": [len:" << updateMsg.getPathAttributes(i)->getLength() << "]\n";
        switch (updateMsg.getPathAttributes(i)->getTypeCode()) {
            case BgpUpdateAttributeTypeCode::ORIGIN: {
                auto& attr = *check_and_cast<const BgpUpdatePathAttributesOrigin *>(updateMsg.getPathAttributes(i));
                EV_INFO << "    ORIGIN: " << BgpSession::getTypeString(attr.getValue()) << "\n";
                break;
            }
            case BgpUpdateAttributeTypeCode::AS_PATH: {
                auto& asPath = *check_and_cast<const BgpUpdatePathAttributesAsPath *>(updateMsg.getPathAttributes(i));
                EV_INFO << "    AS_PATH:";
                for (size_t k = 0; k < asPath.getValueArraySize(); k++) {
                    const BgpAsPathSegment& asPathVal = asPath.getValue(k);
                    for (size_t n = 0; n < asPathVal.getAsValueArraySize(); n++) {
                        EV_INFO << " " << asPathVal.getAsValue(n);
                    }
                }
                EV_INFO << "\n";
                break;
            }
            case BgpUpdateAttributeTypeCode::NEXT_HOP: {
                auto& attr = *check_and_cast<const BgpUpdatePathAttributesNextHop *>(updateMsg.getPathAttributes(i));
                EV_INFO << "    NEXT_HOP: " << attr.getValue().str(false) << "\n";
                break;
            }
            case BgpUpdateAttributeTypeCode::LOCAL_PREF: {
                auto& attr = *check_and_cast<const BgpUpdatePathAttributesLocalPref *>(updateMsg.getPathAttributes(i));
                EV_INFO << "    LOCAL_PREF: " << attr.getValue() << "\n";
                break;
            }
            case BgpUpdateAttributeTypeCode::ATOMIC_AGGREGATE: {
                auto& attr = *check_and_cast<const BgpUpdatePathAttributesAtomicAggregate *>(updateMsg.getPathAttributes(i));
                (void)attr;
                EV_INFO << "    ATOMIC_AGGREGATE" << "\n";
                break;
            }
            case BgpUpdateAttributeTypeCode::AGGREGATOR: {
                auto& attr = *check_and_cast<const BgpUpdatePathAttributesAggregator *>(updateMsg.getPathAttributes(i));
                EV_INFO << "    AGGREGATOR: " << attr.getAsNumber() << ", " << attr.getBgpSpeaker() << "\n";
                break;
            }
            case BgpUpdateAttributeTypeCode::MULTI_EXIT_DISC: {
                auto& attr = *check_and_cast<const BgpUpdatePathAttributesMultiExitDisc *>(updateMsg.getPathAttributes(i));
                EV_INFO << "    MULTI_EXIT_DISC: " << attr.getValue() << "\n";
                break;
            }
            case BgpUpdateAttributeTypeCode::MP_REACH_NLRI: {
                auto& attr = *check_and_cast<const BgpUpdatePathAttributesMpReachNlri *>(updateMsg.getPathAttributes(i));
                EV_INFO << "    MP_REACH_NLRI: afi=" << attr.getAfi() << " safi=" << (int)attr.getSafi()
                        << " nextHop=" << attr.getNextHop().str() << " (" << attr.getNlriArraySize() << " prefixes)\n";
                break;
            }
            case BgpUpdateAttributeTypeCode::MP_UNREACH_NLRI: {
                auto& attr = *check_and_cast<const BgpUpdatePathAttributesMpUnreachNlri *>(updateMsg.getPathAttributes(i));
                EV_INFO << "    MP_UNREACH_NLRI: afi=" << attr.getAfi() << " safi=" << (int)attr.getSafi()
                        << " (" << attr.getWithdrawnRoutesArraySize() << " prefixes)\n";
                break;
            }
        }
    }

    if (updateMsg.getNlriArraySize() > 0) {
        auto nlriBase = updateMsg.getNlri(0);
        EV_INFO << "  Network Layer Reachability Information (NLRI): \n";
        EV_INFO << "    NLRI length: " << (int)nlriBase.length << "\n";
        EV_INFO << "    NLRI prefix: " << nlriBase.prefix << "\n";
    }
}

//void printNotificationMessage(const BgpNotificationMessage& notificationMsg)
//{
//
//}

void BgpRouter::printKeepAliveMessage(const BgpKeepAliveMessage& keepAliveMsg)
{
    // TODO add code once implemented
}

bool BgpRouter::isRouteExcluded(const IRoute& rtEntry)
{
    // all host-specific routes are excluded (/32 for IPv4, /128 for IPv6)
    if (rtEntry.getPrefixLength() == (rtEntry.getDestinationAsGeneric().getType() == L3Address::IPv6 ? 128 : 32))
        return true;

    // all static routes are excluded
    if (rtEntry.getSourceType() == IRoute::MANUAL)
        return true;

    // all BGP routes are excluded
    if (rtEntry.getSourceType() == IRoute::BGP)
        return true;

    // all RIP routes are excluded when redistributeRip is false
    if (rtEntry.getSourceType() == IRoute::RIP) {
        if (!redistributeRip)
            return true;
        else
            return false;
    }

    if (rtEntry.getSourceType() == IRoute::OSPF) {
        // all OSPF routes are excluded when redistributeOspf is false
        if (!redistributeOspf)
            return true;

        auto entry = check_and_cast<const ospfv2::Ospfv2RoutingTableEntry *>(&rtEntry);
        ASSERT(entry);

        if (entry->getPathType() == ospfv2::Ospfv2RoutingTableEntry::INTRAAREA) {
            if (redistributeOspfType.intraArea)
                return false;
            else
                return true;
        }

        if (entry->getPathType() == ospfv2::Ospfv2RoutingTableEntry::INTERAREA) {
            if (redistributeOspfType.interArea)
                return false;
            else
                return true;
        }

        int externalType = checkExternalRoute(entry);

        if (externalType == 1) {
            if (redistributeOspfType.externalType1)
                return false;
            else
                return true;
        }

        if (externalType == 2) {
            if (redistributeOspfType.externalType2)
                return false;
            else
                return true;
        }

        // exclude all other OSPF route types
        return true;
    }

    if (rtEntry.getSourceType() == IRoute::IFACENETMASK) {
        if (rtEntry.getInterface()->isLoopback())
            return true;
        else if (!redistributeRip && !redistributeOspf)
            return true;
        else
            return isExternalAddress(rtEntry);
    }

    // exclude all other routes
    return true;
}

bool BgpRouter::isExternalAddress(const IRoute& rtEntry)
{
    for (auto& session : _bgpSessions) {
        if (session.second->getType() == EGP) {
            NetworkInterface *exIntf = rt->getOutputInterfaceForDestination(session.second->getPeerAddr());
            if (exIntf == rtEntry.getInterface())
                return true;
        }
    }

    return false;
}

bool BgpRouter::isDefaultRoute(const IRoute *entry) const
{
    return entry->getDestinationAsGeneric().isUnspecified() && entry->getPrefixLength() == 0;
}

bool BgpRouter::isReachable(const L3Address& addr) const
{
    if (addr.isUnspecified())
        return true;

    for (int i = 0; i < rt->getNumRoutes(); i++) {
        IRoute *route = rt->getRoute(i);
        if (!isDefaultRoute(route) && route->getSourceType() != IRoute::BGP) {
            if (addr.getPrefix(route->getPrefixLength()) == route->getDestinationAsGeneric().getPrefix(route->getPrefixLength()))
                return true;
        }
    }

    return false;
}

} // namespace bgp

} // namespace inet
