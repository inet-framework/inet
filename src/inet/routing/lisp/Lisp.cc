//
// Copyright (C) 2013 - today Brno University of Technology, Czech Republic
// Copyright (C) OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
// LISP (Locator/ID Separation Protocol, RFC 6830) ported from the ANSAINET project.
// Original author: Vladimir Vesely (Brno University of Technology).
//

#include "inet/routing/lisp/Lisp.h"

#include "inet/common/ModuleAccess.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/networklayer/common/NetworkInterface.h"
#include "inet/networklayer/ipv4/Ipv4Header_m.h"
#include "inet/networklayer/ipv4/Ipv4Route.h"
#include "inet/routing/lisp/LispCommon.h"
#include "inet/routing/lisp/LispMessages_m.h"
#include "inet/routing/lisp/LispTimers_m.h"

namespace inet {
namespace lisp {

Define_Module(Lisp);

void Lisp::initialize(int stage)
{
    ApplicationBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        controlPort = par("controlPort");
        dataPort = par("dataPort");
        ift.reference(this, "interfaceTableModule", true);
        rt.reference(this, "routingTableModule", true);

        acceptMapRequestMapping = par("acceptMapRequestMapping");
        advertOnlyOwnEids = par("advertOnlyOwnEids");
        echoNonceAlgo = par("echoNonceAlgo");
        ciscoStartupDelays = par("ciscoStartupDelays");
        int ttl = par("mapCacheTtl");
        mapCacheTtl = ttl > 0 ? (unsigned short)ttl : DEFAULT_TTL_VAL;
        const char *algo = par("rlocProbingAlgo");
        rlocProbingAlgo = !strcmp(algo, "Simple") ? PROBE_SIMPLE
                          : !strcmp(algo, "Sophisticated") ? PROBE_SOPHISTICATED
                          : PROBE_CISCO;
        mapServerV4 = par("mapServerV4");
        mapServerV6 = par("mapServerV6");
        mapResolverV4 = par("mapResolverV4");
        mapResolverV6 = par("mapResolverV6");
    }
    else if (stage == INITSTAGE_ROUTING_PROTOCOLS) {
        parseConfig(par("configData"));
    }
}

void Lisp::parseMapServerConfig(cXMLElement *config)
{
    for (cXMLElement *m : config->getChildrenByTagName(ETRMAPSERVER_TAG)) {
        if (!m->getAttribute(ADDRESS_ATTR)) {
            EV_WARN << "LISP config: <EtrMapServer> missing 'address'\n";
            continue;
        }
        const char *key = m->getAttribute(KEY_ATTR);
        if (!key || key[0] == '\0') {
            EV_WARN << "LISP config: <EtrMapServer> missing 'key'\n";
            continue;
        }
        bool proxy = m->getAttribute(PROXY_ATTR) && !strcmp(m->getAttribute(PROXY_ATTR), ENABLED_VAL);
        bool notify = m->getAttribute(NOTIFY_ATTR) && !strcmp(m->getAttribute(NOTIFY_ATTR), ENABLED_VAL);
        bool quickreg = m->getAttribute(QUICKREG_ATTR) && !strcmp(m->getAttribute(QUICKREG_ATTR), ENABLED_VAL);
        mapServers.push_back(LispServerEntry(m->getAttribute(ADDRESS_ATTR), key, proxy, notify, quickreg));
    }
}

void Lisp::parseMapResolverConfig(cXMLElement *config)
{
    for (cXMLElement *m : config->getChildrenByTagName(ITRMAPRESOLVER_TAG)) {
        if (!m->getAttribute(ADDRESS_ATTR)) {
            EV_WARN << "LISP config: <ItrMapResolver> missing 'address'\n";
            continue;
        }
        mapResolvers.push_back(LispServerEntry(m->getAttribute(ADDRESS_ATTR)));
    }
    mapResolverQueue = mapResolvers.begin();
}

void Lisp::parseConfig(cXMLElement *config)
{
    if (!config)
        return;
    // navigate to the <LISP> element if a device/Routing wrapper was given
    if (strcmp(config->getTagName(), LISP_TAG)) {
        std::string path = std::string(ROUTING_TAG) + "/" + LISP_TAG;
        if (cXMLElement *lisp = config->getElementByPath(path.c_str()))
            config = lisp;
        else if (cXMLElement *lisp = config->getFirstChildWithTag(LISP_TAG))
            config = lisp;
    }

    parseMapServerConfig(config);
    parseMapResolverConfig(config);

    // Map-Server / Map-Resolver roles from the config tags
    if (cXMLElement *ms = config->getFirstChildWithTag(MAPSERVER_TAG)) {
        if (ms->getAttribute(IPV4_ATTR) && !strcmp(ms->getAttribute(IPV4_ATTR), ENABLED_VAL))
            mapServerV4 = true;
        if (ms->getAttribute(IPV6_ATTR) && !strcmp(ms->getAttribute(IPV6_ATTR), ENABLED_VAL))
            mapServerV6 = true;
        siteDatabase.parseSites(ms);
    }
    if (cXMLElement *mr = config->getFirstChildWithTag(MAPRESOLVER_TAG)) {
        if (mr->getAttribute(IPV4_ATTR) && !strcmp(mr->getAttribute(IPV4_ATTR), ENABLED_VAL))
            mapResolverV4 = true;
        if (mr->getAttribute(IPV6_ATTR) && !strcmp(mr->getAttribute(IPV6_ATTR), ENABLED_VAL))
            mapResolverV6 = true;
    }

    // load this ETR's own EID database and any static map-cache entries
    if (cXMLElement *etrMap = config->getFirstChildWithTag(ETRMAP_TAG))
        mapDatabase.load(etrMap, ift.get(), advertOnlyOwnEids);
    if (cXMLElement *mc = config->getFirstChildWithTag(MAPCACHE_TAG)) {
        mapCache.parseMapEntry(mc);
        // statically configured cache locators are assumed reachable (no RLOC probing yet)
        for (auto& entry : mapCache.getMappingStorage())
            for (auto& rloc : entry.getRlocs())
                rloc.setState(LispRlocator::UP);
    }
}

LispServerEntry *Lisp::findServerEntryByAddress(ServerAddresses& list, const L3Address& addr)
{
    for (auto& entry : list)
        if (entry.getAddress() == addr)
            return &entry;
    return nullptr;
}

void Lisp::handleStartOperation(LifecycleOperation *operation)
{
    controlSocket.setOutputGate(gate("socketOut"));
    controlSocket.setCallback(this);
    controlSocket.bind(L3Address(), controlPort);
    socketMap.addSocket(&controlSocket);

    dataSocket.setOutputGate(gate("socketOut"));
    dataSocket.setCallback(this);
    dataSocket.bind(L3Address(), dataPort);
    socketMap.addSocket(&dataSocket);

    // data plane over the TUN interface (optional: only if the node has one)
    if (NetworkInterface *tunIe = ift->findInterfaceByName(par("tunInterface"))) {
        tunInterfaceId = tunIe->getInterfaceId();
        tunSocket.setOutputGate(gate("socketOut"));
        tunSocket.setCallback(this);
        tunSocket.open(tunInterfaceId);
        socketMap.addSocket(&tunSocket);
        installEidRoutes();
    }
    else
        EV_INFO << "No TUN interface '" << par("tunInterface").stringValue() << "', LISP data plane disabled\n";

    // ETR: register our local EID database with the configured Map-Servers
    if (!mapServers.empty() && !mapDatabase.getMappingStorage().empty())
        scheduleRegistration();
}

void Lisp::installEidRoutes()
{
    // route every cached EID prefix to the TUN interface so that EID-destined
    // traffic is captured and LISP-encapsulated by this ITR
    NetworkInterface *tunIe = ift->getInterfaceById(tunInterfaceId);
    for (auto& entry : mapCache.getMappingStorage()) {
        const L3Address& eid = entry.getEidPrefix().getEidAddr();
        if (eid.getType() != L3Address::IPv4)
            continue; // IPv6 EID routing added with the IPv6 data plane
        Ipv4Route *route = new Ipv4Route();
        route->setDestination(eid.toIpv4());
        route->setNetmask(Ipv4Address::makeNetmask(entry.getEidPrefix().getEidLength()));
        route->setInterface(tunIe);
        route->setSourceType(IRoute::MANUAL);
        rt->addRoute(route);
    }
}

void Lisp::handleMessageWhenUp(cMessage *msg)
{
    if (msg->isSelfMessage())
        handleSelfMessage(msg);
    else if (ISocket *socket = socketMap.findSocketFor(msg))
        socket->processMessage(msg);
    else
        throw cRuntimeError("Unknown message '%s'", msg->getName());
}

void Lisp::handleSelfMessage(cMessage *msg)
{
    if (dynamic_cast<LispRegisterTimer *>(msg))
        expireRegisterTimer(msg);
    else
        throw cRuntimeError("Unknown self message '%s'", msg->getName());
}

void Lisp::socketDataArrived(UdpSocket *socket, Packet *packet)
{
    if (socket == &dataSocket)
        decapsulate(packet);
    else
        handleControlMessage(packet);
}

void Lisp::handleControlMessage(Packet *packet)
{
    const auto& ctrl = packet->peekAtFront<LispControlMessage>();
    switch (ctrl->getType()) {
        case LISP_REGISTER:
            receiveMapRegister(packet);
            break;
        // TODO REPLY / REQUEST / NOTIFY handled with the request/reply flow
        default:
            EV_WARN << "Unhandled LISP control message type " << (int)ctrl->getType() << "\n";
            break;
    }
    delete packet;
}

void Lisp::socketDataArrived(TunSocket *socket, Packet *packet)
{
    encapsulate(packet);
}

void Lisp::socketClosed(TunSocket *socket)
{
}

void Lisp::encapsulate(Packet *packet)
{
    const auto& ipv4Header = packet->peekAtFront<Ipv4Header>();
    L3Address dstEid = ipv4Header->getDestAddress();

    LispMapEntry *me = mapCache.lookupMapEntry(dstEid);
    LispRlocator *rloc = me ? me->getBestUnicastLocator(getRNG(0)) : nullptr;
    if (!rloc) {
        EV_WARN << "ITR: no usable RLOC for EID " << dstEid << ", dropping\n";
        // TODO trigger a Map-Request on a cache miss
        delete packet;
        return;
    }

    auto lispHeader = makeShared<LispHeader>();
    lispHeader->setNonce(0);
    packet->insertAtFront(lispHeader);
    packet->clearTags();
    EV_INFO << "ITR: encapsulating EID " << dstEid << " -> RLOC " << rloc->getRlocAddr() << "\n";
    dataSocket.sendTo(packet, rloc->getRlocAddr(), dataPort);
}

void Lisp::decapsulate(Packet *packet)
{
    packet->popAtFront<LispHeader>();
    packet->clearTags();
    EV_INFO << "ETR: decapsulating, re-injecting inner datagram via TUN\n";
    tunSocket.send(packet);
}

void Lisp::socketErrorArrived(UdpSocket *socket, Indication *indication)
{
    EV_WARN << "LISP UDP error: " << indication->getName() << "\n";
    delete indication;
}

void Lisp::socketClosed(UdpSocket *socket)
{
    // TODO proper async stop completion (added together with the lifecycle work)
}

//
// Control plane: registration (ETR <-> Map-Server)
//

LispMapRecord Lisp::makeMapRecord(const LispMapEntry& entry, bool aBit, int action)
{
    LispMapRecord rec;
    rec.setRecordTTL(mapCacheTtl);
    rec.setEidPrefix(entry.getEidPrefix().getEidAddr());
    rec.setEidMaskLength(entry.getEidPrefix().getEidLength());
    rec.setMapVersion(0);
    rec.setABit(aBit);
    rec.setAct(action);
    rec.setLocatorsArraySize(entry.getRlocs().size());
    size_t i = 0;
    for (const auto& rloc : entry.getRlocs()) {
        LispLocatorRecord loc;
        loc.priority = rloc.getPriority();
        loc.weight = rloc.getWeight();
        loc.mpriority = rloc.getMpriority();
        loc.mweight = rloc.getMweight();
        loc.localLocBit = aBit && rloc.isLocal();
        loc.probedBit = false;
        loc.reachableBit = rloc.getState() == LispRlocator::UP;
        loc.rloc = rloc.getRlocAddr();
        rec.setLocators(i++, loc);
    }
    return rec;
}

void Lisp::scheduleRegistration()
{
    for (auto& server : mapServers) {
        auto timer = new LispRegisterTimer("LISP Register Timer");
        timer->setServerAddress(server.getAddress());
        scheduleAfter(ciscoStartupDelays ? (double)DEFAULT_REGTIMER_VAL : 0.0, timer);
    }
}

void Lisp::expireRegisterTimer(cMessage *timer)
{
    auto regtim = check_and_cast<LispRegisterTimer *>(timer);
    if (LispServerEntry *se = findServerEntryByAddress(mapServers, regtim->getServerAddress())) {
        sendMapRegister(*se);
        scheduleAfter(DEFAULT_REGTIMER_VAL, regtim);
    }
    else
        cancelAndDelete(regtim);
}

void Lisp::sendMapRegister(LispServerEntry& se)
{
    auto lmreg = makeShared<LispMapRegister>();
    lmreg->setNonce(getRNG(0)->intRand());
    lmreg->setProxyBit(se.isProxyReply());
    lmreg->setMapNotifyBit(se.isMapNotify());
    lmreg->setKeyId(LispCommon::KID_NONE);
    lmreg->setAuthDataLen(se.getKey().size());
    lmreg->setAuthData(se.getKey().c_str());

    auto& storage = mapDatabase.getMappingStorage();
    lmreg->setRecordsArraySize(storage.size());
    size_t i = 0;
    for (const auto& entry : storage)
        lmreg->setRecords(i++, makeMapRecord(entry, true, LispCommon::NO_ACTION));
    lmreg->setRecordCount(storage.size());

    auto packet = new Packet("LispMapRegister");
    packet->insertAtBack(lmreg);
    EV_INFO << "ETR: registering " << storage.size() << " EID record(s) with Map-Server " << se.getAddress() << "\n";
    controlSocket.sendTo(packet, se.getAddress(), controlPort);
}

void Lisp::receiveMapRegister(Packet *packet)
{
    const auto& lmreg = packet->peekAtFront<LispMapRegister>();
    L3Address src = packet->getTag<L3AddressInd>()->getSrcAddress();

    if (!isMapServer()) {
        EV_WARN << "Non-Map-Server received a Map-Register, ignoring\n";
        return;
    }
    if (lmreg->getRecordCount() == 0 || lmreg->getKeyId() != 0)
        return;

    std::string authData = lmreg->getAuthData();
    LispSite *si = siteDatabase.findSiteInfoByKey(authData);
    if (!si) {
        EV_WARN << "Map-Register for an unknown LISP site, ignoring\n";
        return;
    }

    LispSiteRecord *srec = siteDatabase.updateSiteEtr(si, src, lmreg->getProxyBit());
    for (size_t i = 0; i < lmreg->getRecordsArraySize(); i++) {
        const LispMapRecord& rec = lmreg->getRecords(i);
        bool isV6 = rec.getEidPrefix().getType() == L3Address::IPv6;
        if ((isV6 && !mapServerV6) || (!isV6 && !mapServerV4))
            continue;
        if (!si->isEidMaintained(rec.getEidPrefix()))
            continue;
        siteDatabase.updateEtrEntries(srec, rec);
    }
    EV_INFO << "MS: registered ETR " << src << " for site '" << si->getSiteName() << "'\n";
}

void Lisp::handleStopOperation(LifecycleOperation *operation)
{
    controlSocket.close();
    dataSocket.close();
    if (tunInterfaceId != -1)
        tunSocket.close();
}

void Lisp::handleCrashOperation(LifecycleOperation *operation)
{
    controlSocket.destroy();
    dataSocket.destroy();
    if (tunInterfaceId != -1)
        tunSocket.destroy();
}

} // namespace lisp
} // namespace inet
