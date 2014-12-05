//
// Copyright (C) 2006 Andras Babos and Andras Varga
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

#include "inet/routing/ospfv2/router/OSPFRouter.h"

namespace inet {

namespace ospf {

Router::Router(RouterID id, cSimpleModule *containingModule, IInterfaceTable *ift, IIPv4RoutingTable *rt) :
    ift(ift),
    rt(rt),
    routerID(id),
    rfc1583Compatibility(false)
{
    messageHandler = new MessageHandler(this, containingModule);
    ageTimer = new cMessage();
    ageTimer->setKind(DATABASE_AGE_TIMER);
    ageTimer->setContextPointer(this);
    ageTimer->setName("Router::DatabaseAgeTimer");
    messageHandler->startTimer(ageTimer, 1.0);
}

Router::~Router()
{
    long areaCount = areas.size();
    for (long i = 0; i < areaCount; i++) {
        delete areas[i];
    }
    long lsaCount = asExternalLSAs.size();
    for (long j = 0; j < lsaCount; j++) {
        delete asExternalLSAs[j];
    }
    long routeCount = routingTable.size();
    for (long k = 0; k < routeCount; k++) {
        delete routingTable[k];
    }
    messageHandler->clearTimer(ageTimer);
    delete ageTimer;
    delete messageHandler;
}

void Router::addWatches()
{
    WATCH(routerID);
    WATCH_PTRVECTOR(areas);
    WATCH_PTRVECTOR(asExternalLSAs);
    WATCH_PTRVECTOR(routingTable);
}

void Router::addArea(Area *area)
{
    area->setRouter(this);
    areasByID[area->getAreaID()] = area;
    areas.push_back(area);
}

Area *Router::getAreaByID(AreaID areaID)
{
    auto areaIt = areasByID.find(areaID);
    if (areaIt != areasByID.end()) {
        return areaIt->second;
    }
    else {
        return nullptr;
    }
}

Area *Router::getAreaByAddr(IPv4Address address)
{
    long areaCount = areas.size();

    for (long i = 0; i < areaCount; i++) {
        if (areas[i]->containsAddress(address))
            return areas[i];
    }

    return nullptr;
}

Interface *Router::getNonVirtualInterface(unsigned char ifIndex)
{
    long areaCount = areas.size();

    for (long i = 0; i < areaCount; i++) {
        Interface *intf = areas[i]->getInterface(ifIndex);
        if (intf != nullptr) {
            return intf;
        }
    }
    return nullptr;
}

bool Router::installLSA(OSPFLSA *lsa, AreaID areaID    /*= BACKBONE_AREAID*/)
{
    switch (lsa->getHeader().getLsType()) {
        case ROUTERLSA_TYPE: {
            auto areaIt = areasByID.find(areaID);
            if (areaIt != areasByID.end()) {
                OSPFRouterLSA *ospfRouterLSA = check_and_cast<OSPFRouterLSA *>(lsa);
                return areaIt->second->installRouterLSA(ospfRouterLSA);
            }
        }
        break;

        case NETWORKLSA_TYPE: {
            auto areaIt = areasByID.find(areaID);
            if (areaIt != areasByID.end()) {
                OSPFNetworkLSA *ospfNetworkLSA = check_and_cast<OSPFNetworkLSA *>(lsa);
                return areaIt->second->installNetworkLSA(ospfNetworkLSA);
            }
        }
        break;

        case SUMMARYLSA_NETWORKS_TYPE:
        case SUMMARYLSA_ASBOUNDARYROUTERS_TYPE: {
            auto areaIt = areasByID.find(areaID);
            if (areaIt != areasByID.end()) {
                OSPFSummaryLSA *ospfSummaryLSA = check_and_cast<OSPFSummaryLSA *>(lsa);
                return areaIt->second->installSummaryLSA(ospfSummaryLSA);
            }
        }
        break;

        case AS_EXTERNAL_LSA_TYPE: {
            OSPFASExternalLSA *ospfASExternalLSA = check_and_cast<OSPFASExternalLSA *>(lsa);
            return installASExternalLSA(ospfASExternalLSA);
        }
        break;

        default:
            ASSERT(false);
            break;
    }
    return false;
}

bool Router::installASExternalLSA(OSPFASExternalLSA *lsa)
{
    /**
     * From RFC2328 Section 12.4.4.1.:
     * "If two routers, both reachable from one another, originate functionally equivalent
     * AS-External-LSAs(i.e., same destination, cost and non-zero forwarding address), then
     * the LSA originated by the router having the highest OSPF Router ID is used. The router
     * having the lower OSPF Router ID can then flush its LSA."
     * The problem is: how do we tell whether two routers are reachable from one another based
     * on a Link State Update packet?
     * 0. We can assume that if this LSA reached this router, then this router is reachable
     *    from the other router. But what about the other direction?
     * 1. The update packet is most likely not sent by the router originating the functionally
     *    equivalent AS-External-LSA, so we cannot use the IPv4 packet source address.
     * 2. The AS-External-LSA contains only the Router ID of the advertising router, so we
     *    can only look up "router" type routing entries in the routing table(these contain
     *    the Router ID as their Destination ID). However these entries are only inserted into
     *    the routing table for intra-area routers...
     */
    // TODO: how to solve this problem?

    RouterID advertisingRouter = lsa->getHeader().getAdvertisingRouter();
    bool reachable = false;
    unsigned int routeCount = routingTable.size();

    for (unsigned int i = 0; i < routeCount; i++) {
        if ((((routingTable[i]->getDestinationType() & RoutingTableEntry::AREA_BORDER_ROUTER_DESTINATION) != 0) ||
             ((routingTable[i]->getDestinationType() & RoutingTableEntry::AS_BOUNDARY_ROUTER_DESTINATION) != 0)) &&
            (routingTable[i]->getDestination() == advertisingRouter))
        {
            reachable = true;
            break;
        }
    }

    bool ownLSAFloodedOut = false;
    LSAKeyType lsaKey;

    lsaKey.linkStateID = lsa->getHeader().getLinkStateID();
    lsaKey.advertisingRouter = routerID;

    auto lsaIt = asExternalLSAsByID.find(lsaKey);
    if ((lsaIt != asExternalLSAsByID.end()) &&
        reachable &&
        (lsaIt->second->getContents().getE_ExternalMetricType() == lsa->getContents().getE_ExternalMetricType()) &&
        (lsaIt->second->getContents().getRouteCost() == lsa->getContents().getRouteCost()) &&
        (lsa->getContents().getForwardingAddress().getInt() != 0) &&    // forwarding address != 0.0.0.0
        (lsaIt->second->getContents().getForwardingAddress() == lsa->getContents().getForwardingAddress()))
    {
        if (routerID > advertisingRouter) {
            return false;
        }
        else {
            lsaIt->second->getHeader().setLsAge(MAX_AGE);
            floodLSA(lsaIt->second, BACKBONE_AREAID);
            lsaIt->second->incrementInstallTime();
            ownLSAFloodedOut = true;
        }
    }

    lsaKey.advertisingRouter = advertisingRouter;

    lsaIt = asExternalLSAsByID.find(lsaKey);
    if (lsaIt != asExternalLSAsByID.end()) {
        unsigned long areaCount = areas.size();
        for (unsigned long i = 0; i < areaCount; i++) {
            areas[i]->removeFromAllRetransmissionLists(lsaKey);
        }
        return (lsaIt->second->update(lsa)) | ownLSAFloodedOut;
    }
    else {
        ASExternalLSA *lsaCopy = new ASExternalLSA(*lsa);
        asExternalLSAsByID[lsaKey] = lsaCopy;
        asExternalLSAs.push_back(lsaCopy);
        return true;
    }
}

OSPFLSA *Router::findLSA(LSAType lsaType, LSAKeyType lsaKey, AreaID areaID)
{
    switch (lsaType) {
        case ROUTERLSA_TYPE: {
            auto areaIt = areasByID.find(areaID);
            if (areaIt != areasByID.end()) {
                return areaIt->second->findRouterLSA(lsaKey.linkStateID);
            }
        }
        break;

        case NETWORKLSA_TYPE: {
            auto areaIt = areasByID.find(areaID);
            if (areaIt != areasByID.end()) {
                return areaIt->second->findNetworkLSA(lsaKey.linkStateID);
            }
        }
        break;

        case SUMMARYLSA_NETWORKS_TYPE:
        case SUMMARYLSA_ASBOUNDARYROUTERS_TYPE: {
            auto areaIt = areasByID.find(areaID);
            if (areaIt != areasByID.end()) {
                return areaIt->second->findSummaryLSA(lsaKey);
            }
        }
        break;

        case AS_EXTERNAL_LSA_TYPE: {
            return findASExternalLSA(lsaKey);
        }
        break;

        default:
            ASSERT(false);
            break;
    }
    return nullptr;
}

ASExternalLSA *Router::findASExternalLSA(LSAKeyType lsaKey)
{
    auto lsaIt = asExternalLSAsByID.find(lsaKey);
    if (lsaIt != asExternalLSAsByID.end()) {
        return lsaIt->second;
    }
    else {
        return nullptr;
    }
}

const ASExternalLSA *Router::findASExternalLSA(LSAKeyType lsaKey) const
{
    std::map<LSAKeyType, ASExternalLSA *, LSAKeyType_Less>::const_iterator lsaIt = asExternalLSAsByID.find(lsaKey);
    if (lsaIt != asExternalLSAsByID.end()) {
        return lsaIt->second;
    }
    else {
        return nullptr;
    }
}

void Router::ageDatabase()
{
    long lsaCount = asExternalLSAs.size();
    bool shouldRebuildRoutingTable = false;

    for (long i = 0; i < lsaCount; i++) {
        unsigned short lsAge = asExternalLSAs[i]->getHeader().getLsAge();
        bool selfOriginated = (asExternalLSAs[i]->getHeader().getAdvertisingRouter() == routerID);
        bool unreachable = isDestinationUnreachable(asExternalLSAs[i]);
        ASExternalLSA *lsa = asExternalLSAs[i];

        if ((selfOriginated && (lsAge < (LS_REFRESH_TIME - 1))) || (!selfOriginated && (lsAge < (MAX_AGE - 1)))) {
            lsa->getHeader().setLsAge(lsAge + 1);
            if ((lsAge + 1) % CHECK_AGE == 0) {
                if (!lsa->validateLSChecksum()) {
                    EV_ERROR << "Invalid LS checksum. Memory error detected!\n";
                }
            }
            lsa->incrementInstallTime();
        }
        if (selfOriginated && (lsAge == (LS_REFRESH_TIME - 1))) {
            if (unreachable) {
                lsa->getHeader().setLsAge(MAX_AGE);
                floodLSA(lsa, BACKBONE_AREAID);
                lsa->incrementInstallTime();
            }
            else {
                long sequenceNumber = lsa->getHeader().getLsSequenceNumber();
                if (sequenceNumber == MAX_SEQUENCE_NUMBER) {
                    lsa->getHeader().setLsAge(MAX_AGE);
                    floodLSA(lsa, BACKBONE_AREAID);
                    lsa->incrementInstallTime();
                }
                else {
                    ASExternalLSA *newLSA = originateASExternalLSA(lsa);

                    newLSA->getHeader().setLsSequenceNumber(sequenceNumber + 1);
                    shouldRebuildRoutingTable |= lsa->update(newLSA);
                    delete newLSA;

                    floodLSA(lsa, BACKBONE_AREAID);
                }
            }
        }
        if (!selfOriginated && (lsAge == MAX_AGE - 1)) {
            lsa->getHeader().setLsAge(MAX_AGE);
            floodLSA(lsa, BACKBONE_AREAID);
            lsa->incrementInstallTime();
        }
        if (lsAge == MAX_AGE) {
            LSAKeyType lsaKey;

            lsaKey.linkStateID = lsa->getHeader().getLinkStateID();
            lsaKey.advertisingRouter = lsa->getHeader().getAdvertisingRouter();

            if (!isOnAnyRetransmissionList(lsaKey) &&
                !hasAnyNeighborInStates(Neighbor::EXCHANGE_STATE | Neighbor::LOADING_STATE))
            {
                if (!selfOriginated || unreachable) {
                    asExternalLSAsByID.erase(lsaKey);
                    delete lsa;
                    asExternalLSAs[i] = nullptr;
                    shouldRebuildRoutingTable = true;
                }
                else {
                    if (lsa->getPurgeable()) {
                        asExternalLSAsByID.erase(lsaKey);
                        delete lsa;
                        asExternalLSAs[i] = nullptr;
                        shouldRebuildRoutingTable = true;
                    }
                    else {
                        ASExternalLSA *newLSA = originateASExternalLSA(lsa);
                        long sequenceNumber = lsa->getHeader().getLsSequenceNumber();

                        newLSA->getHeader().setLsSequenceNumber((sequenceNumber == MAX_SEQUENCE_NUMBER) ? INITIAL_SEQUENCE_NUMBER : sequenceNumber + 1);
                        shouldRebuildRoutingTable |= lsa->update(newLSA);
                        delete newLSA;

                        floodLSA(lsa, BACKBONE_AREAID);
                    }
                }
            }
        }
    }

    auto it = asExternalLSAs.begin();
    while (it != asExternalLSAs.end()) {
        if ((*it) == nullptr) {
            it = asExternalLSAs.erase(it);
        }
        else {
            it++;
        }
    }

    long areaCount = areas.size();
    for (long j = 0; j < areaCount; j++) {
        areas[j]->ageDatabase();
    }
    messageHandler->startTimer(ageTimer, 1.0);

    if (shouldRebuildRoutingTable) {
        rebuildRoutingTable();
    }
}

bool Router::hasAnyNeighborInStates(int states) const
{
    long areaCount = areas.size();
    for (long i = 0; i < areaCount; i++) {
        if (areas[i]->hasAnyNeighborInStates(states)) {
            return true;
        }
    }
    return false;
}

void Router::removeFromAllRetransmissionLists(LSAKeyType lsaKey)
{
    long areaCount = areas.size();
    for (long i = 0; i < areaCount; i++) {
        areas[i]->removeFromAllRetransmissionLists(lsaKey);
    }
}

bool Router::isOnAnyRetransmissionList(LSAKeyType lsaKey) const
{
    long areaCount = areas.size();
    for (long i = 0; i < areaCount; i++) {
        if (areas[i]->isOnAnyRetransmissionList(lsaKey)) {
            return true;
        }
    }
    return false;
}

bool Router::floodLSA(OSPFLSA *lsa, AreaID areaID    /*= BACKBONE_AREAID*/, Interface *intf    /*= nullptr*/, Neighbor *neighbor    /*= nullptr*/)
{
    bool floodedBackOut = false;

    if (lsa != nullptr) {
        if (lsa->getHeader().getLsType() == AS_EXTERNAL_LSA_TYPE) {
            long areaCount = areas.size();
            for (long i = 0; i < areaCount; i++) {
                if (areas[i]->getExternalRoutingCapability()) {
                    if (areas[i]->floodLSA(lsa, intf, neighbor)) {
                        floodedBackOut = true;
                    }
                }
            }
        }
        else {
            auto areaIt = areasByID.find(areaID);
            if (areaIt != areasByID.end()) {
                floodedBackOut = areaIt->second->floodLSA(lsa, intf, neighbor);
            }
        }
    }

    return floodedBackOut;
}

bool Router::isLocalAddress(IPv4Address address) const
{
    long areaCount = areas.size();
    for (long i = 0; i < areaCount; i++) {
        if (areas[i]->isLocalAddress(address)) {
            return true;
        }
    }
    return false;
}

bool Router::hasAddressRange(const IPv4AddressRange& addressRange) const
{
    long areaCount = areas.size();
    for (long i = 0; i < areaCount; i++) {
        if (areas[i]->hasAddressRange(addressRange)) {
            return true;
        }
    }
    return false;
}

ASExternalLSA *Router::originateASExternalLSA(ASExternalLSA *lsa)
{
    ASExternalLSA *asExternalLSA = new ASExternalLSA(*lsa);
    OSPFLSAHeader& lsaHeader = asExternalLSA->getHeader();
    OSPFOptions lsaOptions;

    lsaHeader.setLsAge(0);
    memset(&lsaOptions, 0, sizeof(OSPFOptions));
    lsaOptions.E_ExternalRoutingCapability = true;
    lsaHeader.setLsOptions(lsaOptions);
    lsaHeader.setLsSequenceNumber(INITIAL_SEQUENCE_NUMBER);
    asExternalLSA->setSource(LSATrackingInfo::ORIGINATED);

    return asExternalLSA;
}

bool Router::isDestinationUnreachable(OSPFLSA *lsa) const
{
    IPv4Address destination = IPv4Address(lsa->getHeader().getLinkStateID());

    OSPFRouterLSA *routerLSA = dynamic_cast<OSPFRouterLSA *>(lsa);
    OSPFNetworkLSA *networkLSA = dynamic_cast<OSPFNetworkLSA *>(lsa);
    OSPFSummaryLSA *summaryLSA = dynamic_cast<OSPFSummaryLSA *>(lsa);
    OSPFASExternalLSA *asExternalLSA = dynamic_cast<OSPFASExternalLSA *>(lsa);
    // TODO: verify
    if (routerLSA != nullptr) {
        RoutingInfo *routingInfo = check_and_cast<RoutingInfo *>(routerLSA);
        if (routerLSA->getHeader().getLinkStateID() == routerID) {    // this is spfTreeRoot
            return false;
        }

        // get the interface address pointing backwards on the shortest path tree
        unsigned int linkCount = routerLSA->getLinksArraySize();
        RouterLSA *toRouterLSA = dynamic_cast<RouterLSA *>(routingInfo->getParent());
        if (toRouterLSA != nullptr) {
            bool destinationFound = false;
            bool unnumberedPointToPointLink = false;
            IPv4Address firstNumberedIfAddress;

            for (unsigned int i = 0; i < linkCount; i++) {
                Link& link = routerLSA->getLinks(i);

                if (link.getType() == POINTTOPOINT_LINK) {
                    if (link.getLinkID() == IPv4Address(toRouterLSA->getHeader().getLinkStateID())) {
                        if ((link.getLinkData() & 0xFF000000) == 0) {
                            unnumberedPointToPointLink = true;
                            if (!firstNumberedIfAddress.isUnspecified()) {
                                break;
                            }
                        }
                        else {
                            destination = IPv4Address(link.getLinkData());
                            destinationFound = true;
                            break;
                        }
                    }
                    else {
                        if (((link.getLinkData() & 0xFF000000) != 0) &&
                            firstNumberedIfAddress.isUnspecified())
                        {
                            firstNumberedIfAddress = IPv4Address(link.getLinkData());
                        }
                    }
                }
                else if (link.getType() == TRANSIT_LINK) {
                    if (firstNumberedIfAddress.isUnspecified()) {
                        firstNumberedIfAddress = IPv4Address(link.getLinkData());
                    }
                }
                else if (link.getType() == VIRTUAL_LINK) {
                    if (link.getLinkID() == toRouterLSA->getHeader().getLinkStateID()) {
                        destination = IPv4Address(link.getLinkData());
                        destinationFound = true;
                        break;
                    }
                    else {
                        if (firstNumberedIfAddress.isUnspecified()) {
                            firstNumberedIfAddress = IPv4Address(link.getLinkData());
                        }
                    }
                }
                // There's no way to get an interface address for the router from a STUB_LINK
            }
            if (unnumberedPointToPointLink) {
                if (!firstNumberedIfAddress.isUnspecified()) {
                    destination = firstNumberedIfAddress;
                }
                else {
                    return true;
                }
            }
            if (!destinationFound) {
                return true;
            }
        }
        else {
            NetworkLSA *toNetworkLSA = dynamic_cast<NetworkLSA *>(routingInfo->getParent());
            if (toNetworkLSA != nullptr) {
                // get the interface address pointing backwards on the shortest path tree
                bool destinationFound = false;
                for (unsigned int i = 0; i < linkCount; i++) {
                    Link& link = routerLSA->getLinks(i);

                    if ((link.getType() == TRANSIT_LINK) &&
                        (link.getLinkID() == IPv4Address(toNetworkLSA->getHeader().getLinkStateID())))
                    {
                        destination = IPv4Address(link.getLinkData());
                        destinationFound = true;
                        break;
                    }
                }
                if (!destinationFound) {
                    return true;
                }
            }
            else {
                return true;
            }
        }
    }
    if (networkLSA != nullptr) {
        destination = networkLSA->getHeader().getLinkStateID() & networkLSA->getNetworkMask();
    }
    if ((summaryLSA != nullptr) && (summaryLSA->getHeader().getLsType() == SUMMARYLSA_NETWORKS_TYPE)) {
        destination = summaryLSA->getHeader().getLinkStateID() & summaryLSA->getNetworkMask();
    }
    if (asExternalLSA != nullptr) {
        destination = asExternalLSA->getHeader().getLinkStateID() & asExternalLSA->getContents().getNetworkMask();
    }

    if (lookup(destination) == nullptr) {
        return true;
    }
    else {
        return false;
    }
}

RoutingTableEntry *Router::lookup(IPv4Address destination, std::vector<RoutingTableEntry *> *table    /*= nullptr*/) const
{
    const std::vector<RoutingTableEntry *>& rTable = (table == nullptr) ? routingTable : (*table);
    unsigned long dest = destination.getInt();
    unsigned long routingTableSize = rTable.size();
    bool unreachable = false;
    std::vector<RoutingTableEntry *> discard;
    unsigned long i;

    unsigned long areaCount = areas.size();
    for (i = 0; i < areaCount; i++) {
        unsigned int addressRangeCount = areas[i]->getAddressRangeCount();
        for (unsigned int j = 0; j < addressRangeCount; j++) {
            IPv4AddressRange range = areas[i]->getAddressRange(j);

            for (unsigned int k = 0; k < routingTableSize; k++) {
                RoutingTableEntry *entry = rTable[k];

                if (entry->getDestinationType() != RoutingTableEntry::NETWORK_DESTINATION) {
                    continue;
                }
//                if (((entry->getDestination().getInt() & entry->getNetmask().getInt() & range.mask.getInt()) == (range.address & range.mask).getInt()) &&
//                    (entry->getPathType() == RoutingTableEntry::INTRAAREA))
                if (range.containsRange(entry->getDestination(), entry->getNetmask()) &&
                    (entry->getPathType() == RoutingTableEntry::INTRAAREA))
                {
                    // active area address range
                    RoutingTableEntry *discardEntry = new RoutingTableEntry(ift);
                    discardEntry->setDestination(range.address);
                    discardEntry->setNetmask(range.mask);
                    discardEntry->setDestinationType(RoutingTableEntry::NETWORK_DESTINATION);
                    discardEntry->setPathType(RoutingTableEntry::INTERAREA);
                    discardEntry->setArea(areas[i]->getAreaID());
                    discard.push_back(discardEntry);
                    break;
                }
            }
        }
    }

    RoutingTableEntry *bestMatch = nullptr;
    unsigned long longestMatch = 0;

    for (i = 0; i < routingTableSize; i++) {
        if (rTable[i]->getDestinationType() == RoutingTableEntry::NETWORK_DESTINATION) {
            RoutingTableEntry *entry = rTable[i];
            unsigned long entryAddress = entry->getDestination().getInt();
            unsigned long entryMask = entry->getNetmask().getInt();

            if ((entryAddress & entryMask) == (dest & entryMask)) {
                if ((dest & entryMask) > longestMatch) {
                    longestMatch = (dest & entryMask);
                    bestMatch = entry;
                }
            }
        }
    }

    unsigned int discardCount = discard.size();
    if (bestMatch == nullptr) {
        unreachable = true;
    }
    else {
        for (i = 0; i < discardCount; i++) {
            RoutingTableEntry *entry = discard[i];
            unsigned long entryAddress = entry->getDestination().getInt();
            unsigned long entryMask = entry->getNetmask().getInt();

            if ((entryAddress & entryMask) == (dest & entryMask)) {
                if ((dest & entryMask) > longestMatch) {
                    unreachable = true;
                    break;
                }
            }
        }
    }

    for (i = 0; i < discardCount; i++) {
        delete discard[i];
    }

    if (unreachable) {
        return nullptr;
    }
    else {
        return bestMatch;
    }
}

void Router::rebuildRoutingTable()
{
    unsigned long areaCount = areas.size();
    bool hasTransitAreas = false;
    std::vector<RoutingTableEntry *> newTable;
    unsigned long i;

    EV_INFO << "Rebuilding routing table:\n";

    for (i = 0; i < areaCount; i++) {
        areas[i]->calculateShortestPathTree(newTable);
        if (areas[i]->getTransitCapability()) {
            hasTransitAreas = true;
        }
    }
    if (areaCount > 1) {
        Area *backbone = getAreaByID(BACKBONE_AREAID);
        if (backbone != nullptr) {
            backbone->calculateInterAreaRoutes(newTable);
        }
    }
    else {
        if (areaCount == 1) {
            areas[0]->calculateInterAreaRoutes(newTable);
        }
    }
    if (hasTransitAreas) {
        for (i = 0; i < areaCount; i++) {
            if (areas[i]->getTransitCapability()) {
                areas[i]->recheckSummaryLSAs(newTable);
            }
        }
    }
    calculateASExternalRoutes(newTable);

    // backup the routing table
    unsigned long routeCount = routingTable.size();
    std::vector<RoutingTableEntry *> oldTable;

    oldTable.assign(routingTable.begin(), routingTable.end());
    routingTable.clear();
    routingTable.assign(newTable.begin(), newTable.end());

    std::vector<IPv4Route *> eraseEntries;
    unsigned long routingEntryNumber = rt->getNumRoutes();
    // remove entries from the IPv4 routing table inserted by the OSPF module
    for (i = 0; i < routingEntryNumber; i++) {
        IPv4Route *entry = rt->getRoute(i);
        RoutingTableEntry *ospfEntry = dynamic_cast<RoutingTableEntry *>(entry);
        if (ospfEntry != nullptr) {
            eraseEntries.push_back(entry);
        }
    }

    unsigned int eraseCount = eraseEntries.size();
    for (i = 0; i < eraseCount; i++) {
        rt->deleteRoute(eraseEntries[i]);
    }

    // add the new routing entries
    routeCount = routingTable.size();
    for (i = 0; i < routeCount; i++) {
        if (routingTable[i]->getDestinationType() == RoutingTableEntry::NETWORK_DESTINATION) {
            rt->addRoute(new RoutingTableEntry(*(routingTable[i])));
        }
    }

    notifyAboutRoutingTableChanges(oldTable);

    routeCount = oldTable.size();
    for (i = 0; i < routeCount; i++) {
        delete (oldTable[i]);
    }

    EV_INFO << "Routing table was rebuilt.\n"
            << "Results:\n";

    routeCount = routingTable.size();
    for (i = 0; i < routeCount; i++) {
        EV_INFO << *routingTable[i]
                << "\n";
    }
}

bool Router::hasRouteToASBoundaryRouter(const std::vector<RoutingTableEntry *>& inRoutingTable, RouterID asbrRouterID) const
{
    long routeCount = inRoutingTable.size();
    for (long i = 0; i < routeCount; i++) {
        RoutingTableEntry *routingEntry = inRoutingTable[i];
        if (((routingEntry->getDestinationType() & RoutingTableEntry::AS_BOUNDARY_ROUTER_DESTINATION) != 0) &&
            (routingEntry->getDestination() == asbrRouterID))
        {
            return true;
        }
    }
    return false;
}

std::vector<RoutingTableEntry *> Router::getRoutesToASBoundaryRouter(const std::vector<RoutingTableEntry *>& fromRoutingTable, RouterID asbrRouterID) const
{
    std::vector<RoutingTableEntry *> results;
    long routeCount = fromRoutingTable.size();

    for (long i = 0; i < routeCount; i++) {
        RoutingTableEntry *routingEntry = fromRoutingTable[i];
        if (((routingEntry->getDestinationType() & RoutingTableEntry::AS_BOUNDARY_ROUTER_DESTINATION) != 0) &&
            (routingEntry->getDestination() == asbrRouterID))
        {
            results.push_back(routingEntry);
        }
    }
    return results;
}

void Router::pruneASBoundaryRouterEntries(std::vector<RoutingTableEntry *>& asbrEntries) const
{
    bool hasNonBackboneIntraAreaPath = false;
    for (auto it = asbrEntries.begin(); it != asbrEntries.end(); it++) {
        RoutingTableEntry *routingEntry = *it;
        if ((routingEntry->getPathType() == RoutingTableEntry::INTRAAREA) &&
            (routingEntry->getArea() != BACKBONE_AREAID))
        {
            hasNonBackboneIntraAreaPath = true;
            break;
        }
    }

    if (hasNonBackboneIntraAreaPath) {
        auto it = asbrEntries.begin();
        while (it != asbrEntries.end()) {
            if (((*it)->getPathType() != RoutingTableEntry::INTRAAREA) ||
                ((*it)->getArea() == BACKBONE_AREAID))
            {
                it = asbrEntries.erase(it);
            }
            else {
                it++;
            }
        }
    }
}

RoutingTableEntry *Router::selectLeastCostRoutingEntry(std::vector<RoutingTableEntry *>& entries) const
{
    if (entries.empty()) {
        return nullptr;
    }

    RoutingTableEntry *leastCostEntry = entries[0];
    Metric leastCost = leastCostEntry->getCost();
    long routeCount = entries.size();

    for (long i = 1; i < routeCount; i++) {
        Metric currentCost = entries[i]->getCost();
        if ((currentCost < leastCost) ||
            ((currentCost == leastCost) && (entries[i]->getArea() > leastCostEntry->getArea())))
        {
            leastCostEntry = entries[i];
            leastCost = currentCost;
        }
    }

    return leastCostEntry;
}

RoutingTableEntry *Router::getPreferredEntry(const OSPFLSA& lsa, bool skipSelfOriginated, std::vector<RoutingTableEntry *> *fromRoutingTable    /*= nullptr*/)
{
    // see RFC 2328 16.3. and 16.4.
    if (fromRoutingTable == nullptr) {
        fromRoutingTable = &routingTable;
    }

    const OSPFLSAHeader& lsaHeader = lsa.getHeader();
    const OSPFASExternalLSA *asExternalLSA = dynamic_cast<const OSPFASExternalLSA *>(&lsa);
    unsigned long externalCost = (asExternalLSA != nullptr) ? asExternalLSA->getContents().getRouteCost() : 0;
    unsigned short lsAge = lsaHeader.getLsAge();
    RouterID originatingRouter = lsaHeader.getAdvertisingRouter();
    bool selfOriginated = (originatingRouter == routerID);
    IPv4Address forwardingAddress;    // 0.0.0.0

    if (asExternalLSA != nullptr) {
        forwardingAddress = asExternalLSA->getContents().getForwardingAddress();
    }

    if ((externalCost == LS_INFINITY) || (lsAge == MAX_AGE) || (skipSelfOriginated && selfOriginated)) {    // (1) and(2)
        return nullptr;
    }

    if (!hasRouteToASBoundaryRouter(*fromRoutingTable, originatingRouter)) {    // (3)
        return nullptr;
    }

    if (forwardingAddress.isUnspecified()) {    // (3)
        std::vector<RoutingTableEntry *> asbrEntries = getRoutesToASBoundaryRouter(*fromRoutingTable, originatingRouter);
        if (!rfc1583Compatibility) {
            pruneASBoundaryRouterEntries(asbrEntries);
        }
        return selectLeastCostRoutingEntry(asbrEntries);
    }
    else {
        RoutingTableEntry *forwardEntry = lookup(forwardingAddress, fromRoutingTable);

        if (forwardEntry == nullptr) {
            return nullptr;
        }

        if ((forwardEntry->getPathType() != RoutingTableEntry::INTRAAREA) &&
            (forwardEntry->getPathType() != RoutingTableEntry::INTERAREA))
        {
            return nullptr;
        }

        return forwardEntry;
    }

    return nullptr;
}

void Router::calculateASExternalRoutes(std::vector<RoutingTableEntry *>& newRoutingTable)
{
    // see RFC 2328 16.4.
    unsigned long lsaCount = asExternalLSAs.size();
    unsigned long i;

    for (i = 0; i < lsaCount; i++) {
        ASExternalLSA *currentLSA = asExternalLSAs[i];
        OSPFLSAHeader& currentHeader = currentLSA->getHeader();
        unsigned short externalCost = currentLSA->getContents().getRouteCost();
        RouterID originatingRouter = currentHeader.getAdvertisingRouter();

        RoutingTableEntry *preferredEntry = getPreferredEntry(*currentLSA, true, &newRoutingTable);
        if (preferredEntry == nullptr) {
            continue;
        }

        IPv4Address destination = currentHeader.getLinkStateID() & currentLSA->getContents().getNetworkMask();

        Metric preferredCost = preferredEntry->getCost();
        RoutingTableEntry *destinationEntry = lookup(destination, &newRoutingTable);    // (5)
        if (destinationEntry == nullptr) {
            bool type2ExternalMetric = currentLSA->getContents().getE_ExternalMetricType();
            unsigned int nextHopCount = preferredEntry->getNextHopCount();
            RoutingTableEntry *newEntry = new RoutingTableEntry(ift);

            newEntry->setDestination(destination);
            newEntry->setNetmask(currentLSA->getContents().getNetworkMask());
            newEntry->setArea(preferredEntry->getArea());
            newEntry->setPathType(type2ExternalMetric ? RoutingTableEntry::TYPE2_EXTERNAL : RoutingTableEntry::TYPE1_EXTERNAL);
            if (type2ExternalMetric) {
                newEntry->setCost(preferredCost);
                newEntry->setType2Cost(externalCost);
            }
            else {
                newEntry->setCost(preferredCost + externalCost);
            }
            newEntry->setDestinationType(RoutingTableEntry::NETWORK_DESTINATION);
            newEntry->setOptionalCapabilities(currentHeader.getLsOptions());
            newEntry->setLinkStateOrigin(currentLSA);

            for (unsigned int j = 0; j < nextHopCount; j++) {
                NextHop nextHop = preferredEntry->getNextHop(j);

                nextHop.advertisingRouter = originatingRouter;
                newEntry->addNextHop(nextHop);
            }

            newRoutingTable.push_back(newEntry);
        }
        else {
            RoutingTableEntry::RoutingPathType destinationPathType = destinationEntry->getPathType();
            bool type2ExternalMetric = currentLSA->getContents().getE_ExternalMetricType();
            unsigned int nextHopCount = preferredEntry->getNextHopCount();

            if ((destinationPathType == RoutingTableEntry::INTRAAREA) ||
                (destinationPathType == RoutingTableEntry::INTERAREA))    // (6) (a)
            {
                continue;
            }

            if (((destinationPathType == RoutingTableEntry::TYPE1_EXTERNAL) &&
                 (type2ExternalMetric)) ||
                ((destinationPathType == RoutingTableEntry::TYPE2_EXTERNAL) &&
                 (type2ExternalMetric) &&
                 (destinationEntry->getType2Cost() < externalCost)))    // (6) (b)
            {
                continue;
            }

            RoutingTableEntry *destinationPreferredEntry = getPreferredEntry(*(destinationEntry->getLinkStateOrigin()), false, &newRoutingTable);
            if ((!rfc1583Compatibility) &&
                (destinationPreferredEntry->getPathType() == RoutingTableEntry::INTRAAREA) &&
                (destinationPreferredEntry->getArea() != BACKBONE_AREAID) &&
                ((preferredEntry->getPathType() != RoutingTableEntry::INTRAAREA) ||
                 (preferredEntry->getArea() == BACKBONE_AREAID)))
            {
                continue;
            }

            if ((((destinationPathType == RoutingTableEntry::TYPE1_EXTERNAL) &&
                  (!type2ExternalMetric) &&
                  (destinationEntry->getCost() < preferredCost + externalCost))) ||
                ((destinationPathType == RoutingTableEntry::TYPE2_EXTERNAL) &&
                 (type2ExternalMetric) &&
                 (destinationEntry->getType2Cost() == externalCost) &&
                 (destinationPreferredEntry->getCost() < preferredCost)))
            {
                continue;
            }

            if (((destinationPathType == RoutingTableEntry::TYPE1_EXTERNAL) &&
                 (!type2ExternalMetric) &&
                 (destinationEntry->getCost() == (preferredCost + externalCost))) ||
                ((destinationPathType == RoutingTableEntry::TYPE2_EXTERNAL) &&
                 (type2ExternalMetric) &&
                 (destinationEntry->getType2Cost() == externalCost) &&
                 (destinationPreferredEntry->getCost() == preferredCost)))    // equal cost
            {
                for (unsigned int j = 0; j < nextHopCount; j++) {
                    // TODO: merge next hops, not add
                    NextHop nextHop = preferredEntry->getNextHop(j);

                    nextHop.advertisingRouter = originatingRouter;
                    destinationEntry->addNextHop(nextHop);
                }
                continue;
            }

            // LSA is better
            destinationEntry->setArea(preferredEntry->getArea());
            destinationEntry->setPathType(type2ExternalMetric ? RoutingTableEntry::TYPE2_EXTERNAL : RoutingTableEntry::TYPE1_EXTERNAL);
            if (type2ExternalMetric) {
                destinationEntry->setCost(preferredCost);
                destinationEntry->setType2Cost(externalCost);
            }
            else {
                destinationEntry->setCost(preferredCost + externalCost);
            }
            destinationEntry->setDestinationType(RoutingTableEntry::NETWORK_DESTINATION);
            destinationEntry->setOptionalCapabilities(currentHeader.getLsOptions());
            destinationEntry->clearNextHops();

            for (unsigned int j = 0; j < nextHopCount; j++) {
                NextHop nextHop = preferredEntry->getNextHop(j);

                nextHop.advertisingRouter = originatingRouter;
                destinationEntry->addNextHop(nextHop);
            }
        }
    }
}

IPv4AddressRange Router::getContainingAddressRange(const IPv4AddressRange& addressRange, bool *advertise    /*= nullptr*/) const
{
    unsigned long areaCount = areas.size();
    for (unsigned long i = 0; i < areaCount; i++) {
        IPv4AddressRange containingAddressRange = areas[i]->getContainingAddressRange(addressRange, advertise);
        if (containingAddressRange != NULL_IPV4ADDRESSRANGE) {
            return containingAddressRange;
        }
    }
    if (advertise != nullptr) {
        *advertise = false;
    }
    return NULL_IPV4ADDRESSRANGE;
}

LinkStateID Router::getUniqueLinkStateID(const IPv4AddressRange& destination,
        Metric destinationCost,
        ASExternalLSA *& lsaToReoriginate,
        bool externalMetricIsType2    /*= false*/) const
{
    if (lsaToReoriginate != nullptr) {
        delete lsaToReoriginate;
        lsaToReoriginate = nullptr;
    }

    LSAKeyType lsaKey;

    lsaKey.linkStateID = destination.address;
    lsaKey.advertisingRouter = routerID;

    const ASExternalLSA *foundLSA = findASExternalLSA(lsaKey);

    if (foundLSA == nullptr) {
        return lsaKey.linkStateID;
    }
    else {
        IPv4Address existingMask = foundLSA->getContents().getNetworkMask();

        if (destination.mask >= existingMask) {
            return lsaKey.linkStateID.makeBroadcastAddress(destination.mask);
        }
        else {
            ASExternalLSA *asExternalLSA = new ASExternalLSA(*foundLSA);

            long sequenceNumber = asExternalLSA->getHeader().getLsSequenceNumber();

            asExternalLSA->getHeader().setLsAge(0);
            asExternalLSA->getHeader().setLsSequenceNumber((sequenceNumber == MAX_SEQUENCE_NUMBER) ? INITIAL_SEQUENCE_NUMBER : sequenceNumber + 1);
            asExternalLSA->getContents().setNetworkMask(destination.mask);
            asExternalLSA->getContents().setE_ExternalMetricType(externalMetricIsType2);
            asExternalLSA->getContents().setRouteCost(destinationCost);

            lsaToReoriginate = asExternalLSA;

            return lsaKey.linkStateID.makeBroadcastAddress(existingMask);
        }
    }
}

// TODO: review this algorithm + add virtual link changes(RFC2328 Section 16.7.).
void Router::notifyAboutRoutingTableChanges(std::vector<RoutingTableEntry *>& oldRoutingTable)
{
    if (areas.size() <= 1) {
        return;
    }

    typedef std::map<IPv4AddressRange, RoutingTableEntry *> RoutingTableEntryMap;
    unsigned long routeCount = oldRoutingTable.size();
    RoutingTableEntryMap oldTableMap;
    RoutingTableEntryMap newTableMap;
    unsigned long i, j, k;

    for (i = 0; i < routeCount; i++) {
        IPv4AddressRange destination(oldRoutingTable[i]->getDestination() & oldRoutingTable[i]->getNetmask(), oldRoutingTable[i]->getNetmask());
        oldTableMap[destination] = oldRoutingTable[i];
    }

    routeCount = routingTable.size();
    for (i = 0; i < routeCount; i++) {
        IPv4AddressRange destination(routingTable[i]->getDestination() & routingTable[i]->getNetmask(), routingTable[i]->getNetmask());
        newTableMap[destination] = routingTable[i];
    }

    unsigned long areaCount = areas.size();
    for (i = 0; i < areaCount; i++) {
        std::map<LSAKeyType, bool, LSAKeyType_Less> originatedLSAMap;
        std::map<LSAKeyType, bool, LSAKeyType_Less> deletedLSAMap;
        LSAKeyType lsaKey;

        routeCount = routingTable.size();
        for (j = 0; j < routeCount; j++) {
            IPv4AddressRange destination(routingTable[j]->getDestination() & routingTable[j]->getNetmask(), routingTable[j]->getNetmask());
            auto destIt = oldTableMap.find(destination);
            if (destIt == oldTableMap.end()) {    // new routing entry
                SummaryLSA *lsaToReoriginate = nullptr;
                SummaryLSA *newLSA = areas[i]->originateSummaryLSA(routingTable[j], originatedLSAMap, lsaToReoriginate);

                if (newLSA != nullptr) {
                    if (lsaToReoriginate != nullptr) {
                        areas[i]->installSummaryLSA(lsaToReoriginate);
//                        floodLSA(lsaToReoriginate, BACKBONE_AREAID);
                        floodLSA(lsaToReoriginate, areas[i]->getAreaID());

                        lsaKey.linkStateID = lsaToReoriginate->getHeader().getLinkStateID();
                        lsaKey.advertisingRouter = routerID;
                        originatedLSAMap[lsaKey] = true;

                        delete lsaToReoriginate;
                    }

                    areas[i]->installSummaryLSA(newLSA);
//                    floodLSA(newLSA, BACKBONE_AREAID);
                    floodLSA(newLSA, areas[i]->getAreaID());

                    lsaKey.linkStateID = newLSA->getHeader().getLinkStateID();
                    lsaKey.advertisingRouter = routerID;
                    originatedLSAMap[lsaKey] = true;

                    delete newLSA;
                }
            }
            else {
                if (*(routingTable[j]) != *(destIt->second)) {    // modified routing entry
                    SummaryLSA *lsaToReoriginate = nullptr;
                    SummaryLSA *newLSA = areas[i]->originateSummaryLSA(routingTable[j], originatedLSAMap, lsaToReoriginate);

                    if (newLSA != nullptr) {
                        if (lsaToReoriginate != nullptr) {
                            areas[i]->installSummaryLSA(lsaToReoriginate);
//                            floodLSA(lsaToReoriginate, BACKBONE_AREAID);
                            floodLSA(lsaToReoriginate, areas[i]->getAreaID());

                            lsaKey.linkStateID = lsaToReoriginate->getHeader().getLinkStateID();
                            lsaKey.advertisingRouter = routerID;
                            originatedLSAMap[lsaKey] = true;

                            delete lsaToReoriginate;
                        }

                        areas[i]->installSummaryLSA(newLSA);
//                        floodLSA(newLSA, BACKBONE_AREAID);
                        floodLSA(newLSA, areas[i]->getAreaID());

                        lsaKey.linkStateID = newLSA->getHeader().getLinkStateID();
                        lsaKey.advertisingRouter = routerID;
                        originatedLSAMap[lsaKey] = true;

                        delete newLSA;
                    }
                    else {
                        IPv4AddressRange destinationAddressRange(routingTable[j]->getDestination(), routingTable[j]->getNetmask());

                        if ((routingTable[j]->getDestinationType() == RoutingTableEntry::NETWORK_DESTINATION) &&
                            ((routingTable[j]->getPathType() == RoutingTableEntry::INTRAAREA) ||
                             (routingTable[j]->getPathType() == RoutingTableEntry::INTERAREA)))
                        {
                            IPv4AddressRange containingAddressRange = getContainingAddressRange(destinationAddressRange);
                            if (containingAddressRange != NULL_IPV4ADDRESSRANGE) {
                                destinationAddressRange = containingAddressRange;
                            }
                        }

                        Metric maxRangeCost = 0;
                        Metric oneLessCost = 0;

                        for (k = 0; k < routeCount; k++) {
                            if ((routingTable[k]->getDestinationType() == RoutingTableEntry::NETWORK_DESTINATION) &&
                                (routingTable[k]->getPathType() == RoutingTableEntry::INTRAAREA) &&
                                ((routingTable[k]->getDestination().getInt() & routingTable[k]->getNetmask().getInt() & destinationAddressRange.mask.getInt()) ==
                                 (destinationAddressRange.address & destinationAddressRange.mask).getInt()) &&
                                (routingTable[k]->getCost() > maxRangeCost))
                            {
                                oneLessCost = maxRangeCost;
                                maxRangeCost = routingTable[k]->getCost();
                            }
                        }

                        if (maxRangeCost == routingTable[j]->getCost()) {    // this entry gives the range's cost
                            lsaKey.linkStateID = destinationAddressRange.address;
                            lsaKey.advertisingRouter = routerID;

                            SummaryLSA *summaryLSA = areas[i]->findSummaryLSA(lsaKey);

                            if (summaryLSA != nullptr) {
                                if (oneLessCost != 0) {    // there's an other entry in this range
                                    summaryLSA->setRouteCost(oneLessCost);
//                                    floodLSA(summaryLSA, BACKBONE_AREAID);
                                    floodLSA(summaryLSA, areas[i]->getAreaID());

                                    originatedLSAMap[lsaKey] = true;
                                }
                                else {    // no more entries in this range -> delete it
                                    std::map<LSAKeyType, bool, LSAKeyType_Less>::const_iterator deletedIt = deletedLSAMap.find(lsaKey);
                                    if (deletedIt == deletedLSAMap.end()) {
                                        summaryLSA->getHeader().setLsAge(MAX_AGE);
//                                        floodLSA(summaryLSA, BACKBONE_AREAID);
                                        floodLSA(summaryLSA, areas[i]->getAreaID());

                                        deletedLSAMap[lsaKey] = true;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        routeCount = oldRoutingTable.size();
        for (j = 0; j < routeCount; j++) {
            IPv4AddressRange destination(oldRoutingTable[j]->getDestination() & oldRoutingTable[j]->getNetmask(), oldRoutingTable[j]->getNetmask());
            auto destIt = newTableMap.find(destination);
            if (destIt == newTableMap.end()) {    // deleted routing entry
                IPv4AddressRange destinationAddressRange(oldRoutingTable[j]->getDestination(), oldRoutingTable[j]->getNetmask());

                if ((oldRoutingTable[j]->getDestinationType() == RoutingTableEntry::NETWORK_DESTINATION) &&
                    ((oldRoutingTable[j]->getPathType() == RoutingTableEntry::INTRAAREA) ||
                     (oldRoutingTable[j]->getPathType() == RoutingTableEntry::INTERAREA)))
                {
                    IPv4AddressRange containingAddressRange = getContainingAddressRange(destinationAddressRange);
                    if (containingAddressRange != NULL_IPV4ADDRESSRANGE) {
                        destinationAddressRange = containingAddressRange;
                    }
                }

                Metric maxRangeCost = 0;

                unsigned long newRouteCount = routingTable.size();
                for (k = 0; k < newRouteCount; k++) {
                    if ((routingTable[k]->getDestinationType() == RoutingTableEntry::NETWORK_DESTINATION) &&
                        (routingTable[k]->getPathType() == RoutingTableEntry::INTRAAREA) &&
                        ((routingTable[k]->getDestination().getInt() & routingTable[k]->getNetmask().getInt() & destinationAddressRange.mask.getInt()) ==
                         (destinationAddressRange.address & destinationAddressRange.mask).getInt()) &&    //FIXME correcting network comparison
                        (routingTable[k]->getCost() > maxRangeCost))
                    {
                        maxRangeCost = routingTable[k]->getCost();
                    }
                }

                if (maxRangeCost < oldRoutingTable[j]->getCost()) {    // the range's cost will change
                    lsaKey.linkStateID = destinationAddressRange.address;
                    lsaKey.advertisingRouter = routerID;

                    SummaryLSA *summaryLSA = areas[i]->findSummaryLSA(lsaKey);

                    if (summaryLSA != nullptr) {
                        if (maxRangeCost > 0) {    // there's an other entry in this range
                            summaryLSA->setRouteCost(maxRangeCost);
                            floodLSA(summaryLSA, BACKBONE_AREAID);

                            originatedLSAMap[lsaKey] = true;
                        }
                        else {    // no more entries in this range -> delete it
                            std::map<LSAKeyType, bool, LSAKeyType_Less>::const_iterator deletedIt = deletedLSAMap.find(lsaKey);
                            if (deletedIt == deletedLSAMap.end()) {
                                summaryLSA->getHeader().setLsAge(MAX_AGE);
                                floodLSA(summaryLSA, BACKBONE_AREAID);

                                deletedLSAMap[lsaKey] = true;
                            }
                        }
                    }
                }
            }
        }
    }
}

void Router::updateExternalRoute(IPv4Address networkAddress, const OSPFASExternalLSAContents& externalRouteContents, int ifIndex)
{
    ASExternalLSA *asExternalLSA = new ASExternalLSA;
    OSPFLSAHeader& lsaHeader = asExternalLSA->getHeader();
    OSPFOptions lsaOptions;
    //LSAKeyType lsaKey;

    unsigned long routingEntryNumber = rt->getNumRoutes();
    bool inRoutingTable = false;
    // add the external route to the routing table if it was not added by another module
    for (unsigned long i = 0; i < routingEntryNumber; i++) {
        const IPv4Route *entry = rt->getRoute(i);
        if ((entry->getDestination() == networkAddress)
            && (entry->getNetmask() == externalRouteContents.getNetworkMask()))    //TODO is it enough?
        {
            inRoutingTable = true;
        }
    }

    if (!inRoutingTable) {
        IPv4Route *entry = new IPv4Route;
        entry->setDestination(networkAddress);
        entry->setNetmask(externalRouteContents.getNetworkMask());
        entry->setInterface(ift->getInterfaceById(ifIndex));
        entry->setSourceType(IRoute::MANUAL);
        entry->setMetric(externalRouteContents.getRouteCost());
        rt->addRoute(entry);    // IIPv4RoutingTable deletes entry pointer
    }

    lsaHeader.setLsAge(0);
    memset(&lsaOptions, 0, sizeof(OSPFOptions));
    lsaOptions.E_ExternalRoutingCapability = true;
    lsaHeader.setLsOptions(lsaOptions);
    lsaHeader.setLsType(AS_EXTERNAL_LSA_TYPE);
    lsaHeader.setLinkStateID(networkAddress);    // TODO: get unique LinkStateID
    lsaHeader.setAdvertisingRouter(IPv4Address(routerID));
    lsaHeader.setLsSequenceNumber(INITIAL_SEQUENCE_NUMBER);

    asExternalLSA->setContents(externalRouteContents);

    asExternalLSA->setSource(LSATrackingInfo::ORIGINATED);

    externalRoutes[networkAddress] = externalRouteContents;

    bool rebuild = installASExternalLSA(asExternalLSA);
    floodLSA(asExternalLSA, BACKBONE_AREAID);
    delete asExternalLSA;

    if (rebuild) {
        rebuildRoutingTable();
    }
}

void Router::addExternalRouteInIPTable(IPv4Address networkAddress, const OSPFASExternalLSAContents& externalRouteContents, int ifIndex)
{
    int routingEntryNumber = rt->getNumRoutes();
    bool inRoutingTable = false;

    // add the external route to the IPv4 routing table if it was not added by another module
    for (int i = 1; i < routingEntryNumber; i++) {
        const IPv4Route *entry = rt->getRoute(i);
        if ((entry->getDestination() == networkAddress)
            && (entry->getNetmask() == externalRouteContents.getNetworkMask()))    //TODO is it enough?
        {
            inRoutingTable = true;
            break;
        }
    }

    if (!inRoutingTable) {
        IPv4Route *entry = new IPv4Route();
        entry->setDestination(networkAddress);
        entry->setNetmask(externalRouteContents.getNetworkMask());
        entry->setInterface(ift->getInterfaceById(ifIndex));
        entry->setSourceType(IRoute::OSPF);
        entry->setMetric(OSPF_BGP_DEFAULT_COST);
        rt->addRoute(entry);
    }
}

void Router::removeExternalRoute(IPv4Address networkAddress)
{
    LSAKeyType lsaKey;

    lsaKey.linkStateID = networkAddress;
    lsaKey.advertisingRouter = routerID;

    auto lsaIt = asExternalLSAsByID.find(lsaKey);
    if (lsaIt != asExternalLSAsByID.end()) {
        lsaIt->second->getHeader().setLsAge(MAX_AGE);
        lsaIt->second->setPurgeable();
        floodLSA(lsaIt->second, BACKBONE_AREAID);
    }

    auto externalIt = externalRoutes.find(networkAddress);
    if (externalIt != externalRoutes.end()) {
        externalRoutes.erase(externalIt);
    }
}

} // namespace ospf

} // namespace inet

