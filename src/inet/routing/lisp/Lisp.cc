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
#include "inet/networklayer/common/NetworkInterface.h"
#include "inet/networklayer/ipv4/Ipv4Header_m.h"
#include "inet/networklayer/ipv4/Ipv4Route.h"
#include "inet/routing/lisp/LispCommon.h"
#include "inet/routing/lisp/LispMessages_m.h"

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
        throw cRuntimeError("Unexpected self message '%s'", msg->getName());
    else if (ISocket *socket = socketMap.findSocketFor(msg))
        socket->processMessage(msg);
    else
        throw cRuntimeError("Unknown message '%s'", msg->getName());
}

void Lisp::socketDataArrived(UdpSocket *socket, Packet *packet)
{
    if (socket == &dataSocket)
        decapsulate(packet);
    else {
        // TODO control-plane messages (Map-Request/Reply/Register/Notify)
        EV_INFO << "LISP control message: " << UdpSocket::getReceivedPacketInfo(packet) << "\n";
        delete packet;
    }
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
