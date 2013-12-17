//
// Copyright (C) 2005 Andras Varga
// Copyright (C) 2005 Wei Yang, Ng
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this program; if not, see <http://www.gnu.org/licenses/>.
//


#include <algorithm>

#include "opp_utils.h"

#include "RoutingTable6.h"

#include "IPv6InterfaceData.h"
#include "InterfaceTableAccess.h"

#include "IPv6TunnelingAccess.h"
#include "NodeOperations.h"

Define_Module(RoutingTable6);


std::string IPv6Route::info() const
{
    std::stringstream out;
    out << getDestPrefix() << "/" << getPrefixLength() << " --> ";
    out << "if=" << getInterfaceId() << " next hop:" << getNextHop(); // FIXME try printing interface name
    out << " " << routeSrcName(getSrc());
    if (getExpiryTime() > SIMTIME_ZERO)
        out << " exp:" << getExpiryTime();
    return out.str();
}

std::string IPv6Route::detailedInfo() const
{
    return std::string();
}

const char *IPv6Route::routeSrcName(RouteSrc src)
{
    switch (src)
    {
        case FROM_RA:         return "FROM_RA";
        case OWN_ADV_PREFIX:  return "OWN_ADV_PREFIX";
        case STATIC:          return "STATIC";
        case ROUTING_PROT:    return "ROUTING_PROT";
        default:              return "???";
    }
}

void IPv6Route::changed(int fieldCode)
{
    if (_rt)
        _rt->routeChanged(this, fieldCode);
}

//----

std::ostream& operator<<(std::ostream& os, const IPv6Route& e)
{
    os << e.info();
    return os;
};

std::ostream& operator<<(std::ostream& os, const RoutingTable6::DestCacheEntry& e)
{
    os << "if=" << e.interfaceId << " " << e.nextHopAddr;  //FIXME try printing interface name
    return os;
};

RoutingTable6::RoutingTable6()
{
}

RoutingTable6::~RoutingTable6()
{
    for (unsigned int i=0; i<routeList.size(); i++)
        delete routeList[i];
}

IPv6Route *RoutingTable6::createNewRoute(IPv6Address destPrefix, int prefixLength, IPv6Route::RouteSrc src)
{
    return new IPv6Route(destPrefix, prefixLength, src);
}

void RoutingTable6::initialize(int stage)
{
    if (stage==1)
    {
        //TODO isNodeUp???

        ift = InterfaceTableAccess().get();
        nb = NotificationBoardAccess().get();

        nb->subscribe(this, NF_INTERFACE_CREATED);
        nb->subscribe(this, NF_INTERFACE_DELETED);
        nb->subscribe(this, NF_INTERFACE_STATE_CHANGED);
        nb->subscribe(this, NF_INTERFACE_CONFIG_CHANGED);
        nb->subscribe(this, NF_INTERFACE_IPv6CONFIG_CHANGED);

        WATCH_PTRVECTOR(routeList);
        WATCH_MAP(destCache); // FIXME commented out for now
        isrouter = par("isRouter");
        multicastForward = par("forwardMulticast");
        WATCH(isrouter);

#ifdef WITH_xMIPv6
        // the following MIPv6 related flags will be overridden by the MIPv6 module (if existing)
        ishome_agent = false;
        WATCH(ishome_agent);

        ismobile_node = false;
        WATCH(ismobile_node);

        mipv6Support = false; // 4.9.07 - CB
#endif /* WITH_xMIPv6 */

        // add IPv6InterfaceData to interfaces
        for (int i=0; i<ift->getNumInterfaces(); i++)
        {
            InterfaceEntry *ie = ift->getInterface(i);
            configureInterfaceForIPv6(ie);
        }

        parseXMLConfigFile();

        // skip hosts
        if (isrouter)
        {
            // add globally routable prefixes to routing table
            for (int x = 0; x < ift->getNumInterfaces(); x++)
            {
                InterfaceEntry *ie = ift->getInterface(x);

                if (ie->isLoopback())
                    continue;

                for (int y = 0; y < ie->ipv6Data()->getNumAdvPrefixes(); y++)
                    if (ie->ipv6Data()->getAdvPrefix(y).prefix.isGlobal())
                        addOrUpdateOwnAdvPrefix(ie->ipv6Data()->getAdvPrefix(y).prefix,
                                                ie->ipv6Data()->getAdvPrefix(y).prefixLength,
                                                ie->getInterfaceId(), SIMTIME_ZERO);
            }
        }
    }
    else if (stage==4)
    {
        // configurator adds routes only in stage==3
        updateDisplayString();
    }
}

void RoutingTable6::parseXMLConfigFile()
{
    // TODO to be revised by Andras
    // configure interfaces from XML config file
    cXMLElement *config = par("routingTable");
    for (cXMLElement *child=config->getFirstChild(); child; child = child->getNextSibling())
    {
        //std::cout << "configuring interfaces from XML file." << endl;
        //std::cout << "selected element is: " << child->getTagName() << endl;
        // we ensure that the selected element is local.
        if (opp_strcmp(child->getTagName(), "local")!=0) continue;
        //ensure that this is the right parent module we are configuring.
        if (opp_strcmp(child->getAttribute("node"), getParentModule()->getFullName())!=0)
            continue;
        //Go one level deeper.
        //child = child->getFirstChild();
        for (cXMLElement *ifTag=child->getFirstChild(); ifTag; ifTag = ifTag->getNextSibling())
        {
            //The next tag should be "interface".
            if (opp_strcmp(ifTag->getTagName(), "interface")==0)
            {
                //std::cout << "Getting attribute: name" << endl;
                const char *ifname = ifTag->getAttribute("name");
                if (!ifname)
                    error("<interface> without name attribute at %s", child->getSourceLocation());

                InterfaceEntry *ie = ift->getInterfaceByName(ifname);
                if (!ie)
                    error("no interface named %s was registered, %s", ifname, child->getSourceLocation());

                configureInterfaceFromXML(ie, ifTag);
            }
            else if (opp_strcmp(ifTag->getTagName(), "tunnel")==0)
                configureTunnelFromXML(ifTag);
        }
    }
}

void RoutingTable6::updateDisplayString()
{
    if (!ev.isGUI())
        return;

    std::stringstream os;

    os << getNumRoutes() << " routes\n" << destCache.size() << " destcache entries";
    getDisplayString().setTagArg("t", 0, os.str().c_str());
}

void RoutingTable6::handleMessage(cMessage *msg)
{
    throw cRuntimeError("This module doesn't process messages");
}

void RoutingTable6::receiveChangeNotification(int category, const cObject *details)
{
    if (simulation.getContextType()==CTX_INITIALIZE)
        return;  // ignore notifications during initialize

    Enter_Method_Silent();
    printNotificationBanner(category, details);

    if (category==NF_INTERFACE_CREATED)
    {
        //TODO something like this:
        //InterfaceEntry *ie = check_and_cast<InterfaceEntry*>(details);
        //configureInterfaceForIPv6(ie);
    }
    else if (category==NF_INTERFACE_DELETED)
    {
        //TODO remove all routes that point to that interface (?)
        const InterfaceEntry *interfaceEntry = check_and_cast<const InterfaceEntry*>(details);
        int interfaceEntryId = interfaceEntry->getInterfaceId();
        purgeDestCacheForInterfaceID(interfaceEntryId);
    }
    else if (category==NF_INTERFACE_STATE_CHANGED)
    {
        const InterfaceEntry *interfaceEntry = check_and_cast<const InterfaceEntry*>(details);
        int interfaceEntryId = interfaceEntry->getInterfaceId();

        // an interface went down
        if (!interfaceEntry->isUp())
        {
            purgeDestCacheForInterfaceID(interfaceEntryId);
        }
    }
    else if (category==NF_INTERFACE_CONFIG_CHANGED)
    {
        //TODO invalidate routing cache (?)
    }
    else if (category==NF_INTERFACE_IPv6CONFIG_CHANGED)
    {
        //TODO
    }
}

void RoutingTable6::routeChanged(IPv6Route *entry, int fieldCode)
{
    ASSERT(entry != NULL);

    /*XXX: this deletes some cache entries we want to keep, but the node MUST update
     the Destination Cache in such a way that all entries will use the latest
     route information.*/
    if (fieldCode==IPv6Route::F_NEXTHOP || fieldCode==IPv6Route::F_IFACE)
        purgeDestCache();

    updateDisplayString();

    nb->fireChangeNotification(NF_IPv6_ROUTE_CHANGED, entry); // TODO include fieldCode in the notification
}

void RoutingTable6::configureInterfaceForIPv6(InterfaceEntry *ie)
{
    IPv6InterfaceData *ipv6IfData = new IPv6InterfaceData();
    ie->setIPv6Data(ipv6IfData);

    // for routers, turn on advertisements by default
    //FIXME: we will use this isRouter flag for now. what if future implementations
    //have 2 interfaces where one interface is configured as a router and the other
    //as a host?
    ipv6IfData->setAdvSendAdvertisements(isrouter); //Added by WEI

    // metric: some hints: OSPF cost (2e9/bps value), MS KB article Q299540, ...
    //d->setMetric((int)ceil(2e9/ie->getDatarate())); // use OSPF cost as default
    //FIXME TBD fill in the rest

    assignRequiredNodeAddresses(ie);

    // add link-local prefix to each interface according to RFC 4861 5.1
    if (!ie->isLoopback())
        addStaticRoute(IPv6Address::LINKLOCAL_PREFIX, 10, ie->getInterfaceId(), IPv6Address::UNSPECIFIED_ADDRESS);

    if (ie->isMulticast())
    {
        // XXX join other ALL_NODES_x and ALL_ROUTERS_x addresses too?
        ipv6IfData->joinMulticastGroup(IPv6Address::ALL_NODES_2);
        if (isrouter)
            ipv6IfData->joinMulticastGroup(IPv6Address::ALL_ROUTERS_2);
    }
}

void RoutingTable6::assignRequiredNodeAddresses(InterfaceEntry *ie)
{
    //RFC 3513 Section 2.8:A Node's Required Addresses
    /*A host is required to recognize the following addresses as
    identifying itself:*/

    //o  The loopback address.
    if (ie->isLoopback())
    {
        ie->ipv6Data()->assignAddress(IPv6Address("::1"), false, SIMTIME_ZERO, SIMTIME_ZERO);
        return;
    }
    //o  Its required Link-Local Address for each interface.

#ifndef WITH_xMIPv6
    //IPv6Address linkLocalAddr = IPv6Address().formLinkLocalAddress(ie->getInterfaceToken());
    //ie->ipv6Data()->assignAddress(linkLocalAddr, true, 0, 0);
#else /* WITH_xMIPv6 */
    IPv6Address linkLocalAddr = IPv6Address().formLinkLocalAddress(ie->getInterfaceToken());
    ie->ipv6Data()->assignAddress(linkLocalAddr, true, SIMTIME_ZERO, SIMTIME_ZERO);
#endif /* WITH_xMIPv6 */

    /*o  Any additional Unicast and Anycast Addresses that have been configured
    for the node's interfaces (manually or automatically).*/

    // FIXME FIXME Andras: commented out the following lines, because these addresses
    // are implicitly checked for in isLocalAddress()  (we don't want redundancy,
    // and manually adding solicited-node mcast address for each and every address
    // is very error-prone!)
    //
    //o  The All-Nodes Multicast Addresses defined in section 2.7.1.

    /*o  The Solicited-Node Multicast Address for each of its unicast and anycast
    addresses.*/

    //o  Multicast Addresses of all other groups to which the node belongs.

    /*A router is required to recognize all addresses that a host is
    required to recognize, plus the following addresses as identifying
    itself:*/
    /*o  The Subnet-Router Anycast Addresses for all interfaces for
    which it is configured to act as a router.*/

    //o  All other Anycast Addresses with which the router has been configured.
    //o  The All-Routers Multicast Addresses defined in section 2.7.1.
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

void RoutingTable6::configureInterfaceFromXML(InterfaceEntry *ie, cXMLElement *cfg)
{
    /*XML parsing capabilities tweaked by WEI. For now, we can configure a specific
    node's interface. We can set advertising prefixes and other variables to be used
    in RAs. The IPv6 interface data gets overwritten if lines 249 to 262 is uncommented.
    The fix is to create an XML file with all the default values. Customised XML files
    can be used for future protocols that requires different values. (MIPv6)*/
    IPv6InterfaceData *d = ie->ipv6Data();

    // parse basic config (attributes)
    d->setAdvSendAdvertisements(toBool(getRequiredAttr(cfg, "AdvSendAdvertisements")));
    //TODO: leave this off first!! They overwrite stuff!

    /* TODO: Wei commented out the stuff below. To be checked why (Andras).
    d->setMaxRtrAdvInterval(OPP_Global::atod(getRequiredAttr(cfg, "MaxRtrAdvInterval")));
    d->setMinRtrAdvInterval(OPP_Global::atod(getRequiredAttr(cfg, "MinRtrAdvInterval")));
    d->setAdvManagedFlag(toBool(getRequiredAttr(cfg, "AdvManagedFlag")));
    d->setAdvOtherConfigFlag(toBool(getRequiredAttr(cfg, "AdvOtherConfigFlag")));
    d->setAdvLinkMTU(OPP_Global::atoul(getRequiredAttr(cfg, "AdvLinkMTU")));
    d->setAdvReachableTime(OPP_Global::atoul(getRequiredAttr(cfg, "AdvReachableTime")));
    d->setAdvRetransTimer(OPP_Global::atoul(getRequiredAttr(cfg, "AdvRetransTimer")));
    d->setAdvCurHopLimit(OPP_Global::atoul(getRequiredAttr(cfg, "AdvCurHopLimit")));
    d->setAdvDefaultLifetime(OPP_Global::atoul(getRequiredAttr(cfg, "AdvDefaultLifetime")));
    ie->setMtu(OPP_Global::atoul(getRequiredAttr(cfg, "HostLinkMTU")));
    d->setCurHopLimit(OPP_Global::atoul(getRequiredAttr(cfg, "HostCurHopLimit")));
    d->setBaseReachableTime(OPP_Global::atoul(getRequiredAttr(cfg, "HostBaseReachableTime")));
    d->setRetransTimer(OPP_Global::atoul(getRequiredAttr(cfg, "HostRetransTimer")));
    d->setDupAddrDetectTransmits(OPP_Global::atoul(getRequiredAttr(cfg, "HostDupAddrDetectTransmits")));
    */

    // parse prefixes (AdvPrefix elements; they should be inside an AdvPrefixList
    // element, but we don't check that)
    cXMLElementList prefixList = cfg->getElementsByTagName("AdvPrefix");
    for (unsigned int i=0; i<prefixList.size(); i++)
    {
        cXMLElement *node = prefixList[i];
        IPv6InterfaceData::AdvPrefix prefix;

        // FIXME todo implement: advValidLifetime, advPreferredLifetime can
        // store (absolute) expiry time (if >0) or lifetime (delta) (if <0);
        // 0 should be treated as infinity
        int pfxLen;
        if (!prefix.prefix.tryParseAddrWithPrefix(node->getNodeValue(), pfxLen))
            throw cRuntimeError("Element <%s> at %s: wrong IPv6Address/prefix syntax %s",
                      node->getTagName(), node->getSourceLocation(), node->getNodeValue());

        prefix.prefixLength = pfxLen;
        prefix.advValidLifetime = OPP_Global::atoul(getRequiredAttr(node, "AdvValidLifetime"));
        prefix.advOnLinkFlag = toBool(getRequiredAttr(node, "AdvOnLinkFlag"));
        prefix.advPreferredLifetime = OPP_Global::atoul(getRequiredAttr(node, "AdvPreferredLifetime"));
        prefix.advAutonomousFlag = toBool(getRequiredAttr(node, "AdvAutonomousFlag"));
        d->addAdvPrefix(prefix);
    }

    // parse addresses
    cXMLElementList addrList = cfg->getChildrenByTagName("inetAddr");
    for (unsigned int k=0; k<addrList.size(); k++)
    {
        cXMLElement *node = addrList[k];
        IPv6Address address = IPv6Address(node->getNodeValue());
        //We can now decide if the address is tentative or not.
        d->assignAddress(address, toBool(getRequiredAttr(node, "tentative")), SIMTIME_ZERO, SIMTIME_ZERO);  // set up with infinite lifetimes
    }
}

void RoutingTable6::configureTunnelFromXML(cXMLElement* cfg)
{
    IPv6Tunneling* tunneling = IPv6TunnelingAccess().get();

    // parse basic config (attributes)
    cXMLElementList tunnelList = cfg->getElementsByTagName("tunnelEntry");
    for (unsigned int i=0; i<tunnelList.size(); i++)
    {
        cXMLElement *node = tunnelList[i];

        IPv6Address entry, exit, trigger;
        entry.set( getRequiredAttr(node, "entryPoint") );
        exit.set( getRequiredAttr(node, "exitPoint") );

        cXMLElementList triggerList = node->getElementsByTagName("triggers");

        if (triggerList.size() != 1)
            opp_error("element <%s> at %s: Only exactly one trigger allowed",
                    node->getTagName(), node->getSourceLocation());

        cXMLElement *triggerNode = triggerList[0];
        trigger.set( getRequiredAttr(triggerNode, "destination") );

        EV << "New tunnel: " << "entry=" << entry << ",exit=" << exit << ",trigger=" << trigger << endl;
        tunneling->createTunnel(IPv6Tunneling::NORMAL, entry, exit, trigger);
    }
}

InterfaceEntry *RoutingTable6::getInterfaceByAddress(const IPv6Address& addr)
{
    Enter_Method("getInterfaceByAddress(%s)=?", addr.str().c_str());

    if (addr.isUnspecified())
        return NULL;

    for (int i=0; i<ift->getNumInterfaces(); ++i)
    {
        InterfaceEntry *ie = ift->getInterface(i);
        if (ie->ipv6Data()->hasAddress(addr))
            return ie;
    }
    return NULL;
}

bool RoutingTable6::isLocalAddress(const IPv6Address& dest) const
{
    Enter_Method("isLocalAddress(%s) y/n", dest.str().c_str());

    // first, check if we have an interface with this address
    for (int i=0; i<ift->getNumInterfaces(); i++)
    {
        InterfaceEntry *ie = ift->getInterface(i);
        if (ie->ipv6Data()->hasAddress(dest))
            return true;
    }

    // then check for special, preassigned multicast addresses
    // (these addresses occur more rarely than specific interface addresses,
    // that's why we check for them last)

    if (dest==IPv6Address::ALL_NODES_1 || dest==IPv6Address::ALL_NODES_2)
        return true;

    if (isRouter() && (dest==IPv6Address::ALL_ROUTERS_1 || dest==IPv6Address::ALL_ROUTERS_2 || dest==IPv6Address::ALL_ROUTERS_5))
        return true;

    // check for solicited-node multicast address
    if (dest.matches(IPv6Address::SOLICITED_NODE_PREFIX, 104))
    {
        for (int i=0; i<ift->getNumInterfaces(); i++)
        {
            InterfaceEntry *ie = ift->getInterface(i);
            if (ie->ipv6Data()->matchesSolicitedNodeMulticastAddress(dest))
                return true;
        }
    }
    return false;
}

const IPv6Address& RoutingTable6::lookupDestCache(const IPv6Address& dest, int& outInterfaceId)
{
    Enter_Method("lookupDestCache(%s)", dest.str().c_str());

    DestCache::iterator it = destCache.find(dest);
    if (it == destCache.end())
    {
        outInterfaceId = -1;
        return IPv6Address::UNSPECIFIED_ADDRESS;
    }
    DestCacheEntry &entry = it->second;
    if (entry.expiryTime > 0 && simTime() > entry.expiryTime)
    {
        destCache.erase(it);
        outInterfaceId = -1;
        return IPv6Address::UNSPECIFIED_ADDRESS;
    }

    outInterfaceId = entry.interfaceId;
    return entry.nextHopAddr;
}

const IPv6Route *RoutingTable6::doLongestPrefixMatch(const IPv6Address& dest)
{
    Enter_Method("doLongestPrefixMatch(%s)", dest.str().c_str());

    // we'll just stop at the first match, because the table is sorted
    // by prefix lengths and metric (see addRoute())

    // bugfix - CB
    RouteList::iterator it = routeList.begin();
    while (it!=routeList.end())
    {
        if (dest.matches((*it)->getDestPrefix(), (*it)->getPrefixLength()))
        {
            if (simTime() > (*it)->getExpiryTime() && (*it)->getExpiryTime() != 0)//since 0 represents infinity.
            {
                if ( (*it)->getSrc()==IPv6Route::FROM_RA )
                {
                    EV << "Expired prefix detected!!" << endl;
                    it = routeList.erase(it);
                    //RouteList::iterator oldIt = it++;
                    //removeOnLinkPrefix((*oldIt)->getDestPrefix(), (*oldIt)->getPrefixLength());
                }
            }
            else
                return *it;
        }
        else
            ++it;
    }
    // FIXME todo: if we selected an expired route, throw it out and select again!
    return NULL;
}

bool RoutingTable6::isPrefixPresent(const IPv6Address& prefix) const
{
    for (RouteList::const_iterator it=routeList.begin(); it!=routeList.end(); it++)
        if (prefix.matches((*it)->getDestPrefix(), 128))
            return true;
    return false;
}

void RoutingTable6::updateDestCache(const IPv6Address& dest, const IPv6Address& nextHopAddr, int interfaceId, simtime_t expiryTime)
{
    DestCacheEntry &entry = destCache[dest];
    entry.nextHopAddr = nextHopAddr;
    entry.interfaceId = interfaceId;
    entry.expiryTime = expiryTime;

    updateDisplayString();
}

void RoutingTable6::purgeDestCache()
{
    destCache.clear();
    updateDisplayString();
}

void RoutingTable6::purgeDestCacheEntriesToNeighbour(const IPv6Address& nextHopAddr, int interfaceId)
{
    for (DestCache::iterator it=destCache.begin(); it!=destCache.end(); )
    {
        if (it->second.interfaceId==interfaceId && it->second.nextHopAddr==nextHopAddr)
        {
            // move the iterator past this element before removing it
            destCache.erase(it++);
        }
        else
        {
            it++;
        }
    }

    updateDisplayString();
}

void RoutingTable6::purgeDestCacheForInterfaceID(int interfaceId)
{
    for (DestCache::iterator it=destCache.begin(); it!=destCache.end(); )
    {
        if (it->second.interfaceId==interfaceId)
        {
            // move the iterator past this element before removing it
            destCache.erase(it++);
        }
        else
        {
            ++it;
        }
    }

    updateDisplayString();
}

void RoutingTable6::addOrUpdateOnLinkPrefix(const IPv6Address& destPrefix, int prefixLength,
        int interfaceId, simtime_t expiryTime)
{
    // see if prefix exists in table
    IPv6Route *route = NULL;
    for (RouteList::iterator it=routeList.begin(); it!=routeList.end(); it++)
    {
        if ((*it)->getSrc()==IPv6Route::FROM_RA && (*it)->getDestPrefix()==destPrefix && (*it)->getPrefixLength()==prefixLength)
        {
            route = *it;
            break;
        }
    }

    if (route==NULL)
    {
        // create new route object
        IPv6Route *route = createNewRoute(destPrefix, prefixLength, IPv6Route::FROM_RA);
        route->setInterfaceId(interfaceId);
        route->setExpiryTime(expiryTime);
        route->setMetric(0);
        route->setAdminDist(IPv6Route::dDirectlyConnected);

        // then add it
        addRoute(route);
    }
    else
    {
        // update existing one; notification-wise, we pretend the route got removed then re-added
        nb->fireChangeNotification(NF_IPv6_ROUTE_DELETED, route);
        route->setInterfaceId(interfaceId);
        route->setExpiryTime(expiryTime);
        nb->fireChangeNotification(NF_IPv6_ROUTE_ADDED, route);
    }

    updateDisplayString();
}

void RoutingTable6::addOrUpdateOwnAdvPrefix(const IPv6Address& destPrefix, int prefixLength,
        int interfaceId, simtime_t expiryTime)
{
    // FIXME this is very similar to the one above -- refactor!!

    // see if prefix exists in table
    IPv6Route *route = NULL;
    for (RouteList::iterator it=routeList.begin(); it!=routeList.end(); it++)
    {
        if ((*it)->getSrc()==IPv6Route::OWN_ADV_PREFIX && (*it)->getDestPrefix()==destPrefix && (*it)->getPrefixLength()==prefixLength)
        {
            route = *it;
            break;
        }
    }

    if (route==NULL)
    {
        // create new route object
        IPv6Route *route = createNewRoute(destPrefix, prefixLength, IPv6Route::OWN_ADV_PREFIX);
        route->setInterfaceId(interfaceId);
        route->setExpiryTime(expiryTime);
        route->setMetric(0);
        route->setAdminDist(IPv6Route::dDirectlyConnected);

        // then add it
        addRoute(route);
    }
    else
    {
        // update existing one; notification-wise, we pretend the route got removed then re-added
        nb->fireChangeNotification(NF_IPv6_ROUTE_DELETED, route);
        route->setInterfaceId(interfaceId);
        route->setExpiryTime(expiryTime);
        nb->fireChangeNotification(NF_IPv6_ROUTE_ADDED, route);
    }

    updateDisplayString();
}

void RoutingTable6::removeOnLinkPrefix(const IPv6Address& destPrefix, int prefixLength)
{
    // scan the routing table for this prefix and remove it
    for (RouteList::iterator it=routeList.begin(); it!=routeList.end(); it++)
    {
        if ((*it)->getSrc()==IPv6Route::FROM_RA && (*it)->getDestPrefix()==destPrefix && (*it)->getPrefixLength()==prefixLength)
        {
            routeList.erase(it);
            return; // there can be only one such route, addOrUpdateOnLinkPrefix() guarantees that
        }
    }

    updateDisplayString();
}

void RoutingTable6::addStaticRoute(const IPv6Address& destPrefix, int prefixLength,
                    unsigned int interfaceId, const IPv6Address& nextHop,
                    int metric)
{
    // create route object
    IPv6Route *route = createNewRoute(destPrefix, prefixLength, IPv6Route::STATIC);
    route->setInterfaceId(interfaceId);
    route->setNextHop(nextHop);
    if (metric==0)
        metric = 10; // TBD should be filled from interface metric
    route->setMetric(metric);
    route->setAdminDist(IPv6Route::dStatic);

    // then add it
    addRoute(route);
}

void RoutingTable6::addDefaultRoute(const IPv6Address& nextHop, unsigned int ifID,
        simtime_t routerLifetime)
{
    // create route object
    IPv6Route *route = createNewRoute(IPv6Address(), 0, IPv6Route::FROM_RA);
    route->setInterfaceId(ifID);
    route->setNextHop(nextHop);
    route->setMetric(10); //FIXME:should be filled from interface metric
    route->setAdminDist(IPv6Route::dStatic);

#ifdef WITH_xMIPv6
    route->setExpiryTime(routerLifetime); // lifetime useful after transitioning to new AR // 27.07.08 - CB
#endif /* WITH_xMIPv6 */

    // then add it
    addRoute(route);
}

void RoutingTable6::addRoutingProtocolRoute(IPv6Route *route)
{
    ASSERT(route->getSrc()==IPv6Route::ROUTING_PROT);
    addRoute(route);
}

bool RoutingTable6::routeLessThan(const IPv6Route *a, const IPv6Route *b)
{
    // helper for sort() in addRoute(). We want routes with longer
    // prefixes to be at front, so we compare them as "less".
    // For metric, a smaller value is better (we report that as "less").
    if (a->getPrefixLength()!=b->getPrefixLength())
        return a->getPrefixLength() > b->getPrefixLength();

    // smaller administrative distance is better
    if (a->getAdminDist() != b->getAdminDist())
        return a->getAdminDist() < b->getAdminDist();

    // smaller metric is better
    return a->getMetric() < b->getMetric();
}

void RoutingTable6::addRoute(IPv6Route *route)
{
    route->setRoutingTable(this);
    routeList.push_back(route);

    // we keep entries sorted by prefix length in routeList, so that we can
    // stop at the first match when doing the longest prefix matching
    std::sort(routeList.begin(), routeList.end(), routeLessThan);

    /*XXX: this deletes some cache entries we want to keep, but the node MUST update
     the Destination Cache in such a way that the latest route information are used.*/
    purgeDestCache();
    updateDisplayString();

    nb->fireChangeNotification(NF_IPv6_ROUTE_ADDED, route);
}

void RoutingTable6::removeRoute(IPv6Route *route)
{
    RouteList::iterator it = std::find(routeList.begin(), routeList.end(), route);
    ASSERT(it!=routeList.end());

    nb->fireChangeNotification(NF_IPv6_ROUTE_DELETED, route); // rather: going to be deleted

    routeList.erase(it);
    delete route;

    /*XXX: this deletes some cache entries we want to keep, but the node MUST update
     the Destination Cache in such a way that all entries using the next-hop from
     the deleted route perform next-hop determination again rather than continue
     sending traffic using that deleted route next-hop.*/
    purgeDestCache();
    updateDisplayString();
}

int RoutingTable6::getNumRoutes() const
{
    return routeList.size();
}

IPv6Route *RoutingTable6::getRoute(int i)
{
    ASSERT(i>=0 && i<(int)routeList.size());
    return routeList[i];
}

#ifdef WITH_xMIPv6
//#####Added by Zarrar Yousaf##################################################################

const IPv6Address& RoutingTable6::getHomeAddress()
{
    for (int i=0; i<ift->getNumInterfaces(); ++i)
    {
        InterfaceEntry *ie = ift->getInterface(i);

        return ie->ipv6Data()->getMNHomeAddress();
    }

    return IPv6Address::UNSPECIFIED_ADDRESS;
}

// Added by CB
bool RoutingTable6::isHomeAddress(const IPv6Address& addr)
{
    // check all interfaces whether they have the
    // provided address as HoA
    for (int i=0; i<ift->getNumInterfaces(); ++i)
    {
        InterfaceEntry *ie = ift->getInterface(i);
        if ( ie->ipv6Data()->getMNHomeAddress() == addr )
            return true;
    }

    return false;
}

// Added by CB
void RoutingTable6::removeDefaultRoutes(int interfaceID)
{
    EV << "/// Removing default route for interface=" << interfaceID << endl;

    for (RouteList::iterator it=routeList.begin(); it!=routeList.end(); )
    {
        // default routes have prefix length 0
        if ( (((*it)->getInterfaceId()) == interfaceID) && ((*it)->getPrefixLength() == 0)  )
            it = routeList.erase(it);
        else
            ++it;
    }

    updateDisplayString();
}

// Added by CB
void RoutingTable6::removeAllRoutes()
{
    EV << "/// Removing all routes from rt6 " << endl;

    for (unsigned int i=0; i<routeList.size(); i++)
        delete routeList[i];

    routeList.clear();

    updateDisplayString();
}

// 4.9.07 - Added by CB
void RoutingTable6::removePrefixes(int interfaceID)
{
    for (RouteList::iterator it=routeList.begin(); it!=routeList.end(); )
    {
        // "real" prefixes have a length of larger then 0
        if ( (((*it)->getInterfaceId()) == interfaceID) && ((*it)->getPrefixLength() > 0)  )
            it = routeList.erase(it);
        else
            ++it;
    }

    updateDisplayString();
}

bool RoutingTable6::isOnLinkAddress(const IPv6Address& address)
{
    for (int j = 0; j < ift->getNumInterfaces(); j++)
    {
        InterfaceEntry *ie = ift->getInterface(j);

        for (int i = 0; i < ie->ipv6Data()->getNumAdvPrefixes(); i++)
            if ( address.matches( ie->ipv6Data()->getAdvPrefix(i).prefix,  ie->ipv6Data()->getAdvPrefix(i).prefixLength) )
                return true;
    }

    return false;
}
#endif /* WITH_xMIPv6 */


bool RoutingTable6::handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback)
{
    Enter_Method_Silent();
    if (dynamic_cast<NodeStartOperation *>(operation)) {
        if (stage == NodeStartOperation::STAGE_NETWORK_LAYER)
            ; // TODO:
    }
    else if (dynamic_cast<NodeShutdownOperation *>(operation)) {
        if (stage == NodeShutdownOperation::STAGE_NETWORK_LAYER)
            while (!routeList.empty())
                removeRoute(routeList[0]);
    }
    else if (dynamic_cast<NodeCrashOperation *>(operation)) {
        if (stage == NodeCrashOperation::STAGE_CRASH)
            while (!routeList.empty())
                removeRoute(routeList[0]);
    }
    return true;
}
