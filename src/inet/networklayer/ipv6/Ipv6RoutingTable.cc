//
// Copyright (C) 2005 OpenSim Ltd.
// Copyright (C) 2005 Wei Yang, Ng
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/networklayer/ipv6/Ipv6RoutingTable.h"

#include <algorithm>

#include "inet/common/INETUtils.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/stlutils.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/ipv6/Ipv6InterfaceData.h"
#include "inet/networklayer/ipv6tunneling/Ipv6Tunneling.h"

namespace inet {

Define_Module(Ipv6RoutingTable);

std::ostream& operator<<(std::ostream& os, const Ipv6Route& e)
{
    os << e.str();
    return os;
};

std::ostream& operator<<(std::ostream& os, const Ipv6RoutingTable::DestCacheEntry& e)
{
    os << "if=" << e.interfaceId << " " << e.nextHopAddr; // FIXME try printing interface name
    return os;
};

Ipv6RoutingTable::Ipv6RoutingTable()
{
}

Ipv6RoutingTable::~Ipv6RoutingTable()
{
    for (auto& elem : routeList)
        delete elem;
}

Ipv6Route *Ipv6RoutingTable::createNewRoute(Ipv6Address destPrefix, int prefixLength, IRoute::SourceType src)
{
    return new Ipv6Route(destPrefix, prefixLength, src);
}

void Ipv6RoutingTable::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        WATCH_PTRVECTOR(routeList);
        WATCH_MAP(destCache); // FIXME commented out for now
        isrouter = par("isRouter");
        multicastForward = par("multicastForwarding");
        useAdminDist = par("useAdminDist");
        WATCH(isrouter);

        ift.reference(this, "interfaceTableModule", true);

#ifdef INET_WITH_xMIPv6
        // the following MIPv6 related flags will be overridden by the MIPv6 module (if existing)
        ishome_agent = false;
        WATCH(ishome_agent);

        ismobile_node = false;
        WATCH(ismobile_node);

        mipv6Support = false; // 4.9.07 - CB
#endif /* INET_WITH_xMIPv6 */

        cModule *host = getContainingNode(this);

        host->subscribe(interfaceCreatedSignal, this);
        host->subscribe(interfaceDeletedSignal, this);
        host->subscribe(interfaceStateChangedSignal, this);
        host->subscribe(interfaceConfigChangedSignal, this);
        host->subscribe(interfaceIpv6ConfigChangedSignal, this);
    }
    // TODO INITSTAGE
    else if (stage == INITSTAGE_LINK_LAYER) {
        // add Ipv6InterfaceData to interfaces
        for (int i = 0; i < ift->getNumInterfaces(); i++) {
            NetworkInterface *ie = ift->getInterface(i);
            configureInterfaceForIpv6(ie);
        }
    }
    // TODO INITSTAGE
    else if (stage == INITSTAGE_NETWORK_CONFIGURATION) {
        parseXmlConfigFile();

        // skip hosts
        if (isrouter) {
            // add globally routable prefixes to routing table
            for (int x = 0; x < ift->getNumInterfaces(); x++) {
                NetworkInterface *ie = ift->getInterface(x);

                if (ie->isLoopback())
                    continue;

                for (int y = 0; y < ie->getProtocolData<Ipv6InterfaceData>()->getNumAdvPrefixes(); y++)
                    if (ie->getProtocolData<Ipv6InterfaceData>()->getAdvPrefix(y).prefix.isGlobal())
                        addOrUpdateOwnAdvPrefix(ie->getProtocolData<Ipv6InterfaceData>()->getAdvPrefix(y).prefix,
                                ie->getProtocolData<Ipv6InterfaceData>()->getAdvPrefix(y).prefixLength,
                                ie->getInterfaceId(), SIMTIME_ZERO);

            }
        }
    }
}

void Ipv6RoutingTable::parseXmlConfigFile()
{
    cModule *host = getContainingNode(this);

    // configure interfaces from XML config file
    cXMLElement *config = par("routes");
    for (cXMLElement *child = config->getFirstChild(); child; child = child->getNextSibling()) {
//        std::cout << "configuring interfaces from XML file." << endl;
//        std::cout << "selected element is: " << child->getTagName() << endl;
        // we ensure that the selected element is local.
        if (opp_strcmp(child->getTagName(), "local") != 0)
            continue;
        // ensure that this is the right parent module we are configuring.
        if (opp_strcmp(child->getAttribute("node"), host->getFullName()) != 0)
            continue;
        // Go one level deeper.
//        child = child->getFirstChild();
        for (cXMLElement *ifTag = child->getFirstChild(); ifTag; ifTag = ifTag->getNextSibling()) {
            // The next tag should be "interface".
            if (opp_strcmp(ifTag->getTagName(), "interface") == 0) {
//                std::cout << "Getting attribute: name" << endl;
                const char *ifname = ifTag->getAttribute("name");
                if (!ifname)
                    throw cRuntimeError("<interface> without name attribute at %s", child->getSourceLocation());

                NetworkInterface *ie = ift->findInterfaceByName(ifname);
                if (!ie)
                    throw cRuntimeError("no interface named %s was registered, %s", ifname, child->getSourceLocation());

                configureInterfaceFromXml(ie, ifTag);
            }
            else if (opp_strcmp(ifTag->getTagName(), "tunnel") == 0)
                configureTunnelFromXml(ifTag);
        }
    }
}

void Ipv6RoutingTable::refreshDisplay() const
{
    std::stringstream os;

    os << getNumRoutes() << " routes\n" << destCache.size() << " destcache entries";
    getDisplayString().setTagArg("t", 0, os.str().c_str());
}

void Ipv6RoutingTable::handleMessage(cMessage *msg)
{
    throw cRuntimeError("This module doesn't process messages");
}

void Ipv6RoutingTable::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details)
{
#if OMNETPP_BUILDNUM < 2001
    if (getSimulation()->getSimulationStage() == STAGE(INITIALIZE))
#else
    if (getSimulation()->getStage() == STAGE(INITIALIZE))
#endif
        return; // ignore notifications during initialize

    Enter_Method("%s", cComponent::getSignalName(signalID));

    printSignalBanner(signalID, obj, details);

    if (signalID == interfaceCreatedSignal) {
        // TODO something like this:
//        NetworkInterface *ie = check_and_cast<NetworkInterface*>(details);
//        configureInterfaceForIPv6(ie);
    }
    else if (signalID == interfaceDeletedSignal) {
        // remove all routes that point to that interface
        const NetworkInterface *entry = check_and_cast<const NetworkInterface *>(obj);
        deleteInterfaceRoutes(entry);
    }
    else if (signalID == interfaceStateChangedSignal) {
        const NetworkInterface *networkInterface = check_and_cast<const NetworkInterfaceChangeDetails *>(obj)->getNetworkInterface();
        int networkInterfaceId = networkInterface->getInterfaceId();

        // an interface went down
        if (!networkInterface->isUp()) {
            deleteInterfaceRoutes(networkInterface);
            purgeDestCacheForInterfaceId(networkInterfaceId);
        }
    }
    else if (signalID == interfaceConfigChangedSignal) {
        // TODO invalidate routing cache (?)
    }
    else if (signalID == interfaceIpv6ConfigChangedSignal) {
        // TODO
    }
}

void Ipv6RoutingTable::routeChanged(Ipv6Route *entry, int fieldCode)
{
    if (fieldCode == Ipv6Route::F_DESTINATION || fieldCode == Ipv6Route::F_PREFIX_LENGTH || fieldCode == Ipv6Route::F_METRIC) { // our data structures depend on these fields
        entry = internalRemoveRoute(entry);
        ASSERT(entry != nullptr); // failure means inconsistency: route was not found in this routing table
        internalAddRoute(entry);

//        invalidateCache();
    }
    emit(routeChangedSignal, entry); // TODO include fieldCode in the notification
}

void Ipv6RoutingTable::configureInterfaceForIpv6(NetworkInterface *ie)
{
    auto ipv6IfData = ie->addProtocolData<Ipv6InterfaceData>();

    // for routers, turn on advertisements by default
    // FIXME we will use this isRouter flag for now. what if future implementations
    // have 2 interfaces where one interface is configured as a router and the other
    // as a host?
    ipv6IfData->setAdvSendAdvertisements(isrouter); // Added by WEI

    // metric: some hints: OSPF cost (2e9/bps value), MS KB article Q299540, ...
    //d->setMetric((int)ceil(2e9/ie->getDatarate())); // use OSPF cost as default
    // FIXME TODO fill in the rest

    assignRequiredNodeAddresses(ie);

    // add link-local prefix to each interface according to RFC 4861 5.1
    if (!ie->isLoopback())
        addStaticRoute(Ipv6Address::LINKLOCAL_PREFIX, 10, ie->getInterfaceId(), Ipv6Address::UNSPECIFIED_ADDRESS);

    if (ie->isMulticast()) {
        // TODO join other ALL_NODES_x and ALL_ROUTERS_x addresses too?
        ipv6IfData->joinMulticastGroup(Ipv6Address::ALL_NODES_2);
        if (isrouter)
            ipv6IfData->joinMulticastGroup(Ipv6Address::ALL_ROUTERS_2);
    }
}

void Ipv6RoutingTable::assignRequiredNodeAddresses(NetworkInterface *ie)
{
    // RFC 3513 Section 2.8:A Node's Required Addresses
    /*A host is required to recognize the following addresses as
       identifying itself:*/

    // o  The loopback address.
    if (ie->isLoopback()) {
        ie->getProtocolDataForUpdate<Ipv6InterfaceData>()->assignAddress(Ipv6Address("::1"), false, SIMTIME_ZERO, SIMTIME_ZERO);
        return;
    }
    // o  Its required Link-Local Address for each interface.

#ifndef INET_WITH_xMIPv6
//    Ipv6Address linkLocalAddr = Ipv6Address().formLinkLocalAddress(ie->getInterfaceToken());
//    ie->getProtocolData<Ipv6InterfaceData>()->assignAddress(linkLocalAddr, true, 0, 0);
#else /* INET_WITH_xMIPv6 */
    Ipv6Address linkLocalAddr = Ipv6Address().formLinkLocalAddress(ie->getInterfaceToken());
    ie->getProtocolDataForUpdate<Ipv6InterfaceData>()->assignAddress(linkLocalAddr, true, SIMTIME_ZERO, SIMTIME_ZERO);
#endif /* INET_WITH_xMIPv6 */

    /*o  Any additional Unicast and Anycast Addresses that have been configured
       for the node's interfaces (manually or automatically).*/

    // FIXME FIXME Andras: commented out the following lines, because these addresses
    // are implicitly checked for in isLocalAddress()  (we don't want redundancy,
    // and manually adding solicited-node mcast address for each and every address
    // is very error-prone!)
    //
    // o  The All-Nodes Multicast Addresses defined in section 2.7.1.

    /*o  The Solicited-Node Multicast Address for each of its unicast and anycast
       addresses.*/

    // o  Multicast Addresses of all other groups to which the node belongs.

    /*A router is required to recognize all addresses that a host is
       required to recognize, plus the following addresses as identifying
       itself:*/
    /*o  The Subnet-Router Anycast Addresses for all interfaces for
       which it is configured to act as a router.*/

    // o  All other Anycast Addresses with which the router has been configured.
    // o  The All-Routers Multicast Addresses defined in section 2.7.1.
}

static const char *getRequiredAttr(cXMLElement *elem, const char *attrName)
{
    const char *s = elem->getAttribute(attrName);
    if (!s)
        throw cRuntimeError("Element <%s> misses required attribute %s at %s",
                elem->getTagName(), attrName, elem->getSourceLocation());
    return s;
}

static bool toBool(const char *s, bool defaultValue = false)
{
    if (!s)
        return defaultValue;

    return !strcmp(s, "on") || !strcmp(s, "true") || !strcmp(s, "yes");
}

void Ipv6RoutingTable::configureInterfaceFromXml(NetworkInterface *ie, cXMLElement *cfg)
{
    /*XML parsing capabilities tweaked by WEI. For now, we can configure a specific
       node's interface. We can set advertising prefixes and other variables to be used
       in RAs. The Ipv6 interface data gets overwritten if lines 249 to 262 is uncommented.
       The fix is to create an XML file with all the default values. Customised XML files
       can be used for future protocols that requires different values. (MIPv6)*/
    auto d = ie->getProtocolDataForUpdate<Ipv6InterfaceData>();

    // parse basic config (attributes)
    d->setAdvSendAdvertisements(toBool(getRequiredAttr(cfg, "AdvSendAdvertisements")));
    // TODO leave this off first!! They overwrite stuff!

    /* TODO Wei commented out the stuff below. To be checked why (Andras).
       d->setMaxRtrAdvInterval(utils::atod(getRequiredAttr(cfg, "MaxRtrAdvInterval")));
       d->setMinRtrAdvInterval(utils::atod(getRequiredAttr(cfg, "MinRtrAdvInterval")));
       d->setAdvManagedFlag(toBool(getRequiredAttr(cfg, "AdvManagedFlag")));
       d->setAdvOtherConfigFlag(toBool(getRequiredAttr(cfg, "AdvOtherConfigFlag")));
       d->setAdvLinkMTU(utils::atoul(getRequiredAttr(cfg, "AdvLinkMTU")));
       d->setAdvReachableTime(utils::atoul(getRequiredAttr(cfg, "AdvReachableTime")));
       d->setAdvRetransTimer(utils::atoul(getRequiredAttr(cfg, "AdvRetransTimer")));
       d->setAdvCurHopLimit(utils::atoul(getRequiredAttr(cfg, "AdvCurHopLimit")));
       d->setAdvDefaultLifetime(utils::atoul(getRequiredAttr(cfg, "AdvDefaultLifetime")));
       ie->setMtu(utils::atoul(getRequiredAttr(cfg, "HostLinkMTU")));
       d->setCurHopLimit(utils::atoul(getRequiredAttr(cfg, "HostCurHopLimit")));
       d->setBaseReachableTime(utils::atoul(getRequiredAttr(cfg, "HostBaseReachableTime")));
       d->setRetransTimer(utils::atoul(getRequiredAttr(cfg, "HostRetransTimer")));
       d->setDupAddrDetectTransmits(utils::atoul(getRequiredAttr(cfg, "HostDupAddrDetectTransmits")));
     */

    // parse prefixes (AdvPrefix elements; they should be inside an AdvPrefixList
    // element, but we don't check that)
    cXMLElementList prefixList = cfg->getElementsByTagName("AdvPrefix");
    for (auto& elem : prefixList) {
        cXMLElement *node = elem;
        Ipv6InterfaceData::AdvPrefix prefix;

        // FIXME todo implement: advValidLifetime, advPreferredLifetime can
        // store (absolute) expiry time (if >0) or lifetime (delta) (if <0);
        // 0 should be treated as infinity
        int pfxLen;
        if (!prefix.prefix.tryParseAddrWithPrefix(node->getNodeValue(), pfxLen))
            throw cRuntimeError("Element <%s> at %s: wrong Ipv6Address/prefix syntax %s",
                    node->getTagName(), node->getSourceLocation(), node->getNodeValue());

        prefix.prefixLength = pfxLen;
        prefix.advValidLifetime = utils::atoul(getRequiredAttr(node, "AdvValidLifetime"));
        prefix.advOnLinkFlag = toBool(getRequiredAttr(node, "AdvOnLinkFlag"));
        prefix.advPreferredLifetime = utils::atoul(getRequiredAttr(node, "AdvPreferredLifetime"));
        prefix.advAutonomousFlag = toBool(getRequiredAttr(node, "AdvAutonomousFlag"));
        d->addAdvPrefix(prefix);
    }

    // parse addresses
    cXMLElementList addrList = cfg->getChildrenByTagName("inetAddr");
    for (auto& elem : addrList) {
        cXMLElement *node = elem;
        Ipv6Address address = Ipv6Address(node->getNodeValue());
        // We can now decide if the address is tentative or not.
        d->assignAddress(address, toBool(getRequiredAttr(node, "tentative")), SIMTIME_ZERO, SIMTIME_ZERO); // set up with infinite lifetimes
    }
}

void Ipv6RoutingTable::configureTunnelFromXml(cXMLElement *cfg)
{
    Ipv6Tunneling *tunneling = getModuleFromPar<Ipv6Tunneling>(par("ipv6TunnelingModule"), this);

    // parse basic config (attributes)
    cXMLElementList tunnelList = cfg->getElementsByTagName("tunnelEntry");
    for (auto& elem : tunnelList) {
        cXMLElement *node = elem;

        Ipv6Address entry, exit, trigger;
        entry.set(getRequiredAttr(node, "entryPoint"));
        exit.set(getRequiredAttr(node, "exitPoint"));

        cXMLElementList triggerList = node->getElementsByTagName("triggers");

        if (triggerList.size() != 1)
            throw cRuntimeError("element <%s> at %s: Only exactly one trigger allowed",
                    node->getTagName(), node->getSourceLocation());

        cXMLElement *triggerNode = triggerList[0];
        trigger.set(getRequiredAttr(triggerNode, "destination"));

        EV_INFO << "New tunnel: " << "entry=" << entry << ",exit=" << exit << ",trigger=" << trigger << endl;
        tunneling->createTunnel(Ipv6Tunneling::NORMAL, entry, exit, trigger);
    }
}

NetworkInterface *Ipv6RoutingTable::getInterfaceByAddress(const Ipv6Address& addr) const
{
    Enter_Method("getInterfaceByAddress(%s)=?", addr.str().c_str());

    return ift->findInterfaceByAddress(addr);
}

NetworkInterface *Ipv6RoutingTable::getInterfaceByAddress(const L3Address& address) const
{
    return getInterfaceByAddress(address.toIpv6());
}

bool Ipv6RoutingTable::isLocalAddress(const Ipv6Address& dest) const
{
    Enter_Method("isLocalAddress(%s)", dest.str().c_str());

    // first, check if we have an interface with this address
    if (ift->isLocalAddress(dest))
        return true;

    // then check for special, preassigned multicast addresses
    // (these addresses occur more rarely than specific interface addresses,
    // that's why we check for them last)

    if (dest == Ipv6Address::ALL_NODES_1 || dest == Ipv6Address::ALL_NODES_2)
        return true;

    if (isRouter() && (dest == Ipv6Address::ALL_ROUTERS_1 || dest == Ipv6Address::ALL_ROUTERS_2 || dest == Ipv6Address::ALL_ROUTERS_5 ||
                       dest == Ipv6Address::ALL_OSPF_ROUTERS_MCAST || dest == Ipv6Address::ALL_OSPF_DESIGNATED_ROUTERS_MCAST))
        return true;

    // check for solicited-node multicast address
    if (dest.matches(Ipv6Address::SOLICITED_NODE_PREFIX, 104)) {
        for (int i = 0; i < ift->getNumInterfaces(); i++) {
            NetworkInterface *ie = ift->getInterface(i);
            if (ie->getProtocolData<Ipv6InterfaceData>()->matchesSolicitedNodeMulticastAddress(dest))
                return true;
        }
    }
    return false;
}

const Ipv6Address& Ipv6RoutingTable::lookupDestCache(const Ipv6Address& dest, int& outInterfaceId)
{
    Enter_Method("lookupDestCache(%s)", dest.str().c_str());

    auto it = destCache.find(dest);
    if (it == destCache.end()) {
        outInterfaceId = -1;
        return Ipv6Address::UNSPECIFIED_ADDRESS;
    }
    DestCacheEntry& entry = it->second;
    if (entry.expiryTime > 0 && simTime() > entry.expiryTime) {
        destCache.erase(it);
        outInterfaceId = -1;
        return Ipv6Address::UNSPECIFIED_ADDRESS;
    }

    outInterfaceId = entry.interfaceId;
    return entry.nextHopAddr;
}

const Ipv6Route *Ipv6RoutingTable::doLongestPrefixMatch(const Ipv6Address& dest)
{
    Enter_Method("doLongestPrefixMatch(%s)", dest.str().c_str());

    // we'll just stop at the first match, because the table is sorted
    // by prefix lengths and metric (see addRoute())

    auto it = routeList.begin();
    while (it != routeList.end()) {
        if (dest.matches((*it)->getDestPrefix(), (*it)->getPrefixLength())) {
            if (simTime() > (*it)->getExpiryTime() && (*it)->getExpiryTime() != 0) { // since 0 represents infinity.
                if ((*it)->getSourceType() == IRoute::ROUTER_ADVERTISEMENT) {
                    EV_INFO << "Expired prefix detected!!" << endl;
                    it = internalDeleteRoute(it); // TODO update display string
                }
            }
            else
                return *it;
        }
        else
            ++it;
    }
    // FIXME todo: if we selected an expired route, throw it out and select again!
    return nullptr;
}

bool Ipv6RoutingTable::isPrefixPresent(const Ipv6Address& prefix) const
{
    for (const auto& elem : routeList)
        if (prefix.matches((elem)->getDestPrefix(), 128))
            return true;

    return false;
}

void Ipv6RoutingTable::updateDestCache(const Ipv6Address& dest, const Ipv6Address& nextHopAddr, int interfaceId, simtime_t expiryTime)
{
    DestCacheEntry& entry = destCache[dest];
    entry.nextHopAddr = nextHopAddr;
    entry.interfaceId = interfaceId;
    entry.expiryTime = expiryTime;
}

void Ipv6RoutingTable::purgeDestCache()
{
    destCache.clear();
}

void Ipv6RoutingTable::purgeDestCacheEntriesToNeighbour(const Ipv6Address& nextHopAddr, int interfaceId)
{
    for (auto it = destCache.begin(); it != destCache.end();) {
        if (it->second.interfaceId == interfaceId && it->second.nextHopAddr == nextHopAddr) {
            // move the iterator past this element before removing it
            destCache.erase(it++);
        }
        else {
            it++;
        }
    }
}

void Ipv6RoutingTable::purgeDestCacheForInterfaceId(int interfaceId)
{
    for (auto it = destCache.begin(); it != destCache.end();) {
        if (it->second.interfaceId == interfaceId) {
            // move the iterator past this element before removing it
            destCache.erase(it++);
        }
        else {
            ++it;
        }
    }
}

void Ipv6RoutingTable::addOrUpdateOnLinkPrefix(const Ipv6Address& destPrefix, int prefixLength,
        int interfaceId, simtime_t expiryTime)
{
    // see if prefix exists in table
    Ipv6Route *route = nullptr;
    for (auto& elem : routeList) {
        if ((elem)->getSourceType() == IRoute::ROUTER_ADVERTISEMENT && (elem)->getDestPrefix() == destPrefix && (elem)->getPrefixLength() == prefixLength) {
            route = elem;
            break;
        }
    }

    if (route == nullptr) {
        // create new route object
        Ipv6Route *route = createNewRoute(destPrefix, prefixLength, IRoute::ROUTER_ADVERTISEMENT);
        route->setInterface(ift->getInterfaceById(interfaceId));
        route->setExpiryTime(expiryTime);
        route->setMetric(0);
        route->setAdminDist(Ipv6Route::dDirectlyConnected);

        // then add it
        addRoute(route);
    }
    else {
        // update existing one; notification-wise, we pretend the route got removed then re-added
        emit(routeDeletedSignal, route);
        route->setInterface(ift->getInterfaceById(interfaceId));
        route->setExpiryTime(expiryTime);
        emit(routeAddedSignal, route);
    }
}

void Ipv6RoutingTable::addOrUpdateOwnAdvPrefix(const Ipv6Address& destPrefix, int prefixLength,
        int interfaceId, simtime_t expiryTime)
{
    // FIXME this is very similar to the one above -- refactor!!

    // see if prefix exists in table
    Ipv6Route *route = nullptr;
    for (auto& elem : routeList) {
        if ((elem)->getSourceType() == IRoute::OWN_ADV_PREFIX && (elem)->getDestPrefix() == destPrefix && (elem)->getPrefixLength() == prefixLength) {
            route = elem;
            break;
        }
    }

    if (route == nullptr) {
        // create new route object
        Ipv6Route *route = createNewRoute(destPrefix, prefixLength, IRoute::OWN_ADV_PREFIX);
        route->setInterface(ift->getInterfaceById(interfaceId));
        route->setExpiryTime(expiryTime);
        route->setMetric(0);
        route->setAdminDist(Ipv6Route::dDirectlyConnected);

        // then add it
        addRoute(route);
    }
    else {
        // update existing one; notification-wise, we pretend the route got removed then re-added
        emit(routeDeletedSignal, route);
        route->setInterface(ift->getInterfaceById(interfaceId));
        route->setExpiryTime(expiryTime);
        emit(routeAddedSignal, route);
    }
}

void Ipv6RoutingTable::deleteOnLinkPrefix(const Ipv6Address& destPrefix, int prefixLength)
{
    // scan the routing table for this prefix and remove it
    for (auto it = routeList.begin(); it != routeList.end(); it++) {
        if ((*it)->getSourceType() == IRoute::ROUTER_ADVERTISEMENT && (*it)->getDestPrefix() == destPrefix && (*it)->getPrefixLength() == prefixLength) {
            internalDeleteRoute(it);
            return; // there can be only one such route, addOrUpdateOnLinkPrefix() guarantees that
        }
    }
}

void Ipv6RoutingTable::addStaticRoute(const Ipv6Address& destPrefix, int prefixLength,
        unsigned int interfaceId, const Ipv6Address& nextHop,
        int metric)
{
    // create route object
    Ipv6Route *route = createNewRoute(destPrefix, prefixLength, IRoute::MANUAL);
    route->setInterface(ift->getInterfaceById(interfaceId));
    route->setNextHop(nextHop);
    if (metric == 0)
        metric = 10; // TODO should be filled from interface metric
    route->setMetric(metric);
    route->setAdminDist(Ipv6Route::dStatic);

    // then add it
    addRoute(route);
}

void Ipv6RoutingTable::addDefaultRoute(const Ipv6Address& nextHop, unsigned int ifID,
        simtime_t routerLifetime)
{
    // create route object
    Ipv6Route *route = createNewRoute(Ipv6Address(), 0, IRoute::ROUTER_ADVERTISEMENT);
    route->setInterface(ift->getInterfaceById(ifID));
    route->setNextHop(nextHop);
    route->setMetric(10); // FIXMEshould be filled from interface metric
    route->setAdminDist(Ipv6Route::dStatic);

#ifdef INET_WITH_xMIPv6
    route->setExpiryTime(routerLifetime); // lifetime useful after transitioning to new AR // 27.07.08 - CB
#endif /* INET_WITH_xMIPv6 */

    // then add it
    addRoute(route);
}

void Ipv6RoutingTable::addRoutingProtocolRoute(Ipv6Route *route)
{
    // TODO ASSERT(route->getSrc()==Ipv6Route::ROUTING_PROT);
    addRoute(route);
}

bool Ipv6RoutingTable::routeLessThan(const Ipv6Route *a, const Ipv6Route *b) const
{
    // helper for sort() in addRoute(). We want routes with longer
    // prefixes to be at front, so we compare them as "less".
    // For metric, a smaller value is better (we report that as "less").
    if (a->getPrefixLength() != b->getPrefixLength())
        return a->getPrefixLength() > b->getPrefixLength();

    // smaller administrative distance is better
    if (useAdminDist && (a->getAdminDist() != b->getAdminDist()))
        return a->getAdminDist() < b->getAdminDist();

    // smaller metric is better
    return a->getMetric() < b->getMetric();
}

void Ipv6RoutingTable::addRoute(Ipv6Route *route)
{
    internalAddRoute(route);

    /*TODO this deletes some cache entries we want to keep, but the node MUST update
       the Destination Cache in such a way that the latest route information are used.*/
    purgeDestCache();

    emit(routeAddedSignal, route);
}

Ipv6Route *Ipv6RoutingTable::removeRoute(Ipv6Route *route)
{
    route = internalRemoveRoute(route);
    if (route) {
        // TODO purge cache?

        emit(routeDeletedSignal, route); // rather: going to be deleted
    }
    return route;
}

void Ipv6RoutingTable::internalAddRoute(Ipv6Route *route)
{
    ASSERT(route->getRoutingTable() == nullptr);

    routeList.push_back(route);
    route->setRoutingTable(this);

    // we keep entries sorted by prefix length in routeList, so that we can
    // stop at the first match when doing the longest prefix matching
    std::stable_sort(routeList.begin(), routeList.end(), RouteLessThan(*this));
}

Ipv6Route *Ipv6RoutingTable::internalRemoveRoute(Ipv6Route *route)
{
    auto i = find(routeList, route);
    if (i != routeList.end()) {
        ASSERT(route->getRoutingTable() == this);
        routeList.erase(i);
        route->setRoutingTable(nullptr);
        return route;
    }
    return nullptr;
}

Ipv6RoutingTable::RouteList::iterator Ipv6RoutingTable::internalDeleteRoute(RouteList::iterator it)
{
    ASSERT(it != routeList.end());
    Ipv6Route *route = *it;
    it = routeList.erase(it);
    emit(routeDeletedSignal, route);
    // TODO purge cache?
    delete route;
    return it;
}

bool Ipv6RoutingTable::deleteRoute(Ipv6Route *route)
{
    auto it = find(routeList, route);
    if (it == routeList.end())
        return false;

    internalDeleteRoute(it);
    return true;
}

int Ipv6RoutingTable::getNumRoutes() const
{
    return routeList.size();
}

Ipv6Route *Ipv6RoutingTable::getRoute(int i) const
{
    ASSERT(i >= 0 && i < (int)routeList.size());
    return routeList[i];
}

#ifdef INET_WITH_xMIPv6
//#####Added by Zarrar Yousaf##################################################################

const Ipv6Address& Ipv6RoutingTable::getHomeAddress()
{
    for (int i = 0; i < ift->getNumInterfaces(); ++i) {
        NetworkInterface *ie = ift->getInterface(i);

        return ie->getProtocolData<Ipv6InterfaceData>()->getMNHomeAddress();
    }

    return Ipv6Address::UNSPECIFIED_ADDRESS;
}

// Added by CB
bool Ipv6RoutingTable::isHomeAddress(const Ipv6Address& addr)
{
    // check all interfaces whether they have the
    // provided address as HoA
    for (int i = 0; i < ift->getNumInterfaces(); ++i) {
        NetworkInterface *ie = ift->getInterface(i);
        if (ie->getProtocolData<Ipv6InterfaceData>()->getMNHomeAddress() == addr)
            return true;
    }

    return false;
}

// Added by CB
void Ipv6RoutingTable::deleteDefaultRoutes(int interfaceID)
{
    ASSERT(interfaceID >= 0);

    EV_INFO << "/// Removing default route for interface=" << interfaceID << endl;

    for (auto it = routeList.begin(); it != routeList.end();) {
        // default routes have prefix length 0
        if ((*it)->getInterface() && (*it)->getInterface()->getInterfaceId() == interfaceID &&
            (*it)->getPrefixLength() == 0)
            it = internalDeleteRoute(it);
        else
            ++it;
    }
}

// Added by CB
void Ipv6RoutingTable::deleteAllRoutes()
{
    EV_INFO << "/// Removing all routes from rt6 " << endl;

    for (auto& elem : routeList) {
        emit(routeDeletedSignal, elem);
        delete elem;
    }

    routeList.clear();
    // TODO purge cache?
}

// 4.9.07 - Added by CB
void Ipv6RoutingTable::deletePrefixes(int interfaceID)
{
    ASSERT(interfaceID >= 0);

    for (auto it = routeList.begin(); it != routeList.end();) {
        // "real" prefixes have a length of larger then 0
        if ((*it)->getInterface() && (*it)->getInterface()->getInterfaceId() == interfaceID &&
            (*it)->getPrefixLength() > 0)
            it = internalDeleteRoute(it);
        else
            ++it;
    }
}

bool Ipv6RoutingTable::isOnLinkAddress(const Ipv6Address& address)
{
    for (int j = 0; j < ift->getNumInterfaces(); j++) {
        NetworkInterface *ie = ift->getInterface(j);

        for (int i = 0; i < ie->getProtocolData<Ipv6InterfaceData>()->getNumAdvPrefixes(); i++)
            if (address.matches(ie->getProtocolData<Ipv6InterfaceData>()->getAdvPrefix(i).prefix, ie->getProtocolData<Ipv6InterfaceData>()->getAdvPrefix(i).prefixLength))
                return true;

    }

    return false;
}

#endif /* INET_WITH_xMIPv6 */

void Ipv6RoutingTable::deleteInterfaceRoutes(const NetworkInterface *entry)
{
    bool changed = false;

    // delete unicast routes using this interface
    for (auto it = routeList.begin(); it != routeList.end();) {
        Ipv6Route *route = *it;
        if (route->getInterface() == entry) {
            it = internalDeleteRoute(it);
            changed = true;
        }
        else
            ++it;
    }

    // TODO delete or update multicast routes:
    //   1. delete routes has entry as parent
    //   2. remove entry from children list

    if (changed) {
//        invalidateCache();
    }
}

bool Ipv6RoutingTable::handleOperationStage(LifecycleOperation *operation, IDoneCallback *doneCallback)
{
    Enter_Method("handleOperationStage");
    int stage = operation->getCurrentStage();
    if (dynamic_cast<ModuleStartOperation *>(operation)) {
        if (static_cast<ModuleStartOperation::Stage>(stage) == ModuleStartOperation::STAGE_NETWORK_LAYER)
            ; // TODO
    }
    else if (dynamic_cast<ModuleStopOperation *>(operation)) {
        if (static_cast<ModuleStopOperation::Stage>(stage) == ModuleStopOperation::STAGE_NETWORK_LAYER)
            while (!routeList.empty())
                delete removeRoute(routeList[0]);

    }
    else if (dynamic_cast<ModuleCrashOperation *>(operation)) {
        if (static_cast<ModuleCrashOperation::Stage>(stage) == ModuleCrashOperation::STAGE_CRASH)
            while (!routeList.empty())
                delete removeRoute(routeList[0]);

    }
    return true;
}

void Ipv6RoutingTable::printRoutingTable() const
{
    for (const auto& elem : routeList)
        EV_INFO << (elem)->getInterface()->getInterfaceFullPath() << " -> " << (elem)->getDestinationAsGeneric().str() << " as " << (elem)->str() << endl;
}

} // namespace inet

