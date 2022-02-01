//
// Copyright (C) 2006 Andras Babos and Andras Varga
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/routing/ospfv2/router/Ospfv2Router.h"

#include "inet/common/stlutils.h"
#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"
#include "inet/routing/ospfv2/router/Lsa.h"

namespace inet {

namespace ospfv2 {

Router::Router(cSimpleModule *containingModule, IInterfaceTable *ift, IIpv4RoutingTable *rt) :
    ift(ift),
    rt(rt),
    routerID(rt->getRouterId()),
    rfc1583Compatibility(false)
{
    messageHandler = new MessageHandler(this, containingModule);
    ageTimer = new cMessage("Router::DatabaseAgeTimer", DATABASE_AGE_TIMER);
    ageTimer->setContextPointer(this);
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
    long routeCount = ospfRoutingTable.size();
    for (long k = 0; k < routeCount; k++) {
        delete ospfRoutingTable[k];
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
    WATCH_PTRVECTOR(ospfRoutingTable);
}

void Router::addArea(Ospfv2Area *area)
{
    area->setRouter(this);
    areasByID[area->getAreaID()] = area;
    areas.push_back(area);
}

Ospfv2Area *Router::getAreaByID(AreaId areaID)
{
    auto areaIt = areasByID.find(areaID);
    return (areaIt != areasByID.end()) ? areaIt->second : nullptr;
}

Ospfv2Area *Router::getAreaByAddr(Ipv4Address address)
{
    long areaCount = areas.size();

    for (long i = 0; i < areaCount; i++) {
        if (areas[i]->containsAddress(address))
            return areas[i];
    }

    return nullptr;
}

std::vector<AreaId> Router::getAreaIds()
{
    std::vector<AreaId> areaIds;
    for (auto& entry : areas)
        areaIds.push_back(entry->getAreaID());
    return areaIds;
}

Ospfv2Interface *Router::getNonVirtualInterface(unsigned char ifIndex)
{
    long areaCount = areas.size();

    for (long i = 0; i < areaCount; i++) {
        Ospfv2Interface *intf = areas[i]->getInterface(ifIndex);
        if (intf != nullptr) {
            return intf;
        }
    }
    return nullptr;
}

bool Router::installLSA(const Ospfv2Lsa *lsa, AreaId areaID /*= BACKBONE_AREAID*/)
{
    switch (lsa->getHeader().getLsType()) {
        case ROUTERLSA_TYPE: {
            auto areaIt = areasByID.find(areaID);
            if (areaIt != areasByID.end()) {
                const Ospfv2RouterLsa *ospfRouterLSA = check_and_cast<const Ospfv2RouterLsa *>(lsa);
                return areaIt->second->installRouterLSA(ospfRouterLSA);
            }
        }
        break;

        case NETWORKLSA_TYPE: {
            auto areaIt = areasByID.find(areaID);
            if (areaIt != areasByID.end()) {
                const Ospfv2NetworkLsa *ospfNetworkLSA = check_and_cast<const Ospfv2NetworkLsa *>(lsa);
                return areaIt->second->installNetworkLSA(ospfNetworkLSA);
            }
        }
        break;

        case SUMMARYLSA_NETWORKS_TYPE:
        case SUMMARYLSA_ASBOUNDARYROUTERS_TYPE: {
            auto areaIt = areasByID.find(areaID);
            if (areaIt != areasByID.end()) {
                const Ospfv2SummaryLsa *ospfSummaryLSA = check_and_cast<const Ospfv2SummaryLsa *>(lsa);
                return areaIt->second->installSummaryLSA(ospfSummaryLSA);
            }
        }
        break;

        case AS_EXTERNAL_LSA_TYPE: {
            const Ospfv2AsExternalLsa *ospfASExternalLSA = check_and_cast<const Ospfv2AsExternalLsa *>(lsa);
            ASSERT(ospfASExternalLSA->getHeader().getLsaLength() != 0);
            return installASExternalLSA(ospfASExternalLSA);
        }
        break;

        default:
            ASSERT(false);
            break;
    }
    return false;
}

bool Router::installASExternalLSA(const Ospfv2AsExternalLsa *lsa)
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
    // TODO how to solve this problem?

    RouterId advertisingRouter = lsa->getHeader().getAdvertisingRouter();

    bool reachable = false;
    for (uint32_t i = 0; i < ospfRoutingTable.size(); i++) {
        if ((((ospfRoutingTable[i]->getDestinationType() & Ospfv2RoutingTableEntry::AREA_BORDER_ROUTER_DESTINATION) != 0) ||
             ((ospfRoutingTable[i]->getDestinationType() & Ospfv2RoutingTableEntry::AS_BOUNDARY_ROUTER_DESTINATION) != 0)) &&
            (ospfRoutingTable[i]->getDestination() == advertisingRouter))
        {
            reachable = true;
            break;
        }
    }

    bool ownLSAFloodedOut = false;
    LsaKeyType lsaKey;

    lsaKey.linkStateID = lsa->getHeader().getLinkStateID();
    lsaKey.advertisingRouter = routerID;

    auto lsaIt = asExternalLSAsByID.find(lsaKey);
    if ((lsaIt != asExternalLSAsByID.end()) &&
        reachable &&
        (lsaIt->second->getContents().getExternalTOSInfo(0).E_ExternalMetricType == lsa->getContents().getExternalTOSInfo(0).E_ExternalMetricType) &&
        (lsaIt->second->getContents().getExternalTOSInfo(0).tos == lsa->getContents().getExternalTOSInfo(0).tos) &&
        (lsaIt->second->getContents().getExternalTOSInfo(0).routeCost == lsa->getContents().getExternalTOSInfo(0).routeCost) &&
        (lsa->getContents().getExternalTOSInfo(0).forwardingAddress.getInt() != 0) && // forwarding address != 0.0.0.0
        (lsaIt->second->getContents().getExternalTOSInfo(0).forwardingAddress == lsa->getContents().getExternalTOSInfo(0).forwardingAddress))
    {
        if (routerID > advertisingRouter) {
            return false;
        }
        else {
            lsaIt->second->getHeaderForUpdate().setLsAge(MAX_AGE);
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
        AsExternalLsa *lsaCopy = new AsExternalLsa(*lsa);
        asExternalLSAsByID[lsaKey] = lsaCopy;
        ASSERT(lsaCopy->getHeader().getLsaLength() != 0);
        asExternalLSAs.push_back(lsaCopy);
        return true;
    }
}

Ospfv2Lsa *Router::findLSA(Ospfv2LsaType lsaType, LsaKeyType lsaKey, AreaId areaID)
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

AsExternalLsa *Router::findASExternalLSA(LsaKeyType lsaKey)
{
    auto lsaIt = asExternalLSAsByID.find(lsaKey);
    return (lsaIt != asExternalLSAsByID.end()) ? lsaIt->second : nullptr;
}

const AsExternalLsa *Router::findASExternalLSA(LsaKeyType lsaKey) const
{
    auto lsaIt = asExternalLSAsByID.find(lsaKey);
    return (lsaIt != asExternalLSAsByID.end()) ? lsaIt->second : nullptr;
}

void Router::ageDatabase()
{
    long lsaCount = asExternalLSAs.size();
    bool shouldRebuildRoutingTable = false;

    for (long i = 0; i < lsaCount; i++) {
        unsigned short lsAge = asExternalLSAs[i]->getHeader().getLsAge();
        bool selfOriginated = (asExternalLSAs[i]->getHeader().getAdvertisingRouter() == routerID);
        bool unreachable = isDestinationUnreachable(asExternalLSAs[i]);
        AsExternalLsa *lsa = asExternalLSAs[i];

        if ((selfOriginated && (lsAge < (LS_REFRESH_TIME - 1))) || (!selfOriginated && (lsAge < (MAX_AGE - 1)))) {
            lsa->getHeaderForUpdate().setLsAge(lsAge + 1);
            if ((lsAge + 1) % CHECK_AGE == 0) {
                if (!lsa->validateLSChecksum()) {
                    EV_ERROR << "Invalid LS checksum. Memory error detected!\n";
                }
            }
            lsa->incrementInstallTime();
        }
        if (selfOriginated && (lsAge == (LS_REFRESH_TIME - 1))) {
            if (unreachable) {
                lsa->getHeaderForUpdate().setLsAge(MAX_AGE);
                floodLSA(lsa, BACKBONE_AREAID);
                lsa->incrementInstallTime();
            }
            else {
                long sequenceNumber = lsa->getHeader().getLsSequenceNumber();
                if (sequenceNumber == MAX_SEQUENCE_NUMBER) {
                    lsa->getHeaderForUpdate().setLsAge(MAX_AGE);
                    floodLSA(lsa, BACKBONE_AREAID);
                    lsa->incrementInstallTime();
                }
                else {
                    AsExternalLsa *newLSA = originateASExternalLSA(lsa);

                    newLSA->getHeaderForUpdate().setLsSequenceNumber(sequenceNumber + 1);
                    shouldRebuildRoutingTable |= lsa->update(newLSA);
                    delete newLSA;

                    floodLSA(lsa, BACKBONE_AREAID);
                }
            }
        }
        if (!selfOriginated && (lsAge == MAX_AGE - 1)) {
            lsa->getHeaderForUpdate().setLsAge(MAX_AGE);
            floodLSA(lsa, BACKBONE_AREAID);
            lsa->incrementInstallTime();
        }
        if (lsAge == MAX_AGE) {
            LsaKeyType lsaKey;

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
                        AsExternalLsa *newLSA = originateASExternalLSA(lsa);
                        long sequenceNumber = lsa->getHeader().getLsSequenceNumber();

                        newLSA->getHeaderForUpdate().setLsSequenceNumber((sequenceNumber == MAX_SEQUENCE_NUMBER) ? INITIAL_SEQUENCE_NUMBER : sequenceNumber + 1);
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

    for (uint32_t j = 0; j < areas.size(); j++)
        areas[j]->ageDatabase();

    messageHandler->startTimer(ageTimer, 1.0);

    if (shouldRebuildRoutingTable) {
        rebuildRoutingTable();
    }
}

bool Router::hasAnyNeighborInStates(int states) const
{
    for (uint32_t i = 0; i < areas.size(); i++) {
        if (areas[i]->hasAnyNeighborInStates(states))
            return true;
    }
    return false;
}

void Router::removeFromAllRetransmissionLists(LsaKeyType lsaKey)
{
    for (uint32_t i = 0; i < areas.size(); i++)
        areas[i]->removeFromAllRetransmissionLists(lsaKey);
}

bool Router::isOnAnyRetransmissionList(LsaKeyType lsaKey) const
{
    for (uint32_t i = 0; i < areas.size(); i++) {
        if (areas[i]->isOnAnyRetransmissionList(lsaKey))
            return true;
    }
    return false;
}

bool Router::floodLSA(const Ospfv2Lsa *lsa, AreaId areaID /*= BACKBONE_AREAID*/, Ospfv2Interface *intf /*= nullptr*/, Neighbor *neighbor /*= nullptr*/)
{
    bool floodedBackOut = false;

    if (lsa != nullptr) {
        if (lsa->getHeader().getLsType() == AS_EXTERNAL_LSA_TYPE) {
            for (uint32_t i = 0; i < areas.size(); i++) {
                if (areas[i]->getExternalRoutingCapability()) {
                    if (areas[i]->floodLSA(lsa, intf, neighbor)) {
                        floodedBackOut = true;
                    }
                }
                /* RFC 2328, Section 3.6: In order to take advantage of the OSPF stub area support,
                    default routing must be used in the stub area.  This is
                    accomplished as follows.  One or more of the stub area's area
                    border routers must advertise a default route into the stub area
                    via summary-LSAs.  These summary defaults are flooded throughout
                    the stub area, but no further.
                 */
                else {
                    SummaryLsa *summaryLsa = areas[i]->originateSummaryLSA_Stub();
                    if (areas[i]->floodLSA(summaryLsa, intf, neighbor)) {
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

bool Router::isLocalAddress(Ipv4Address address) const
{
    long areaCount = areas.size();
    for (long i = 0; i < areaCount; i++) {
        if (areas[i]->isLocalAddress(address)) {
            return true;
        }
    }
    return false;
}

bool Router::hasAddressRange(const Ipv4AddressRange& addressRange) const
{
    long areaCount = areas.size();
    for (long i = 0; i < areaCount; i++) {
        if (areas[i]->hasAddressRange(addressRange)) {
            return true;
        }
    }
    return false;
}

AsExternalLsa *Router::originateASExternalLSA(AsExternalLsa *lsa)
{
    AsExternalLsa *asExternalLSA = new AsExternalLsa(*lsa);
    Ospfv2LsaHeader& lsaHeader = asExternalLSA->getHeaderForUpdate();
    Ospfv2Options lsaOptions;

    lsaHeader.setLsAge(0);
    lsaOptions.E_ExternalRoutingCapability = true;
    lsaHeader.setLsOptions(lsaOptions);
    lsaHeader.setLsSequenceNumber(INITIAL_SEQUENCE_NUMBER);
    asExternalLSA->setSource(LsaTrackingInfo::ORIGINATED);

    return asExternalLSA;
}

bool Router::isDestinationUnreachable(Ospfv2Lsa *lsa) const
{
    Ipv4Address destination = Ipv4Address(lsa->getHeader().getLinkStateID());

    Ospfv2RouterLsa *routerLSA = dynamic_cast<Ospfv2RouterLsa *>(lsa);
    // TODO verify
    if (routerLSA) {
        RoutingInfo *routingInfo = check_and_cast<RoutingInfo *>(routerLSA);
        if (routerLSA->getHeader().getLinkStateID() == routerID) // this is spfTreeRoot
            return false;

        // get the interface address pointing backwards on the shortest path tree
        unsigned int linkCount = routerLSA->getLinksArraySize();
        RouterLsa *toRouterLSA = dynamic_cast<RouterLsa *>(routingInfo->getParent());
        if (toRouterLSA) {
            bool destinationFound = false;
            bool unnumberedPointToPointLink = false;
            Ipv4Address firstNumberedIfAddress;

            for (unsigned int i = 0; i < linkCount; i++) {
                const auto& link = routerLSA->getLinks(i);

                if (link.getType() == POINTTOPOINT_LINK) {
                    if (link.getLinkID() == Ipv4Address(toRouterLSA->getHeader().getLinkStateID())) {
                        if ((link.getLinkData() & 0xFF000000) == 0) {
                            unnumberedPointToPointLink = true;
                            if (!firstNumberedIfAddress.isUnspecified())
                                break;
                        }
                        else {
                            destination = Ipv4Address(link.getLinkData());
                            destinationFound = true;
                            break;
                        }
                    }
                    else {
                        if (((link.getLinkData() & 0xFF000000) != 0) &&
                            firstNumberedIfAddress.isUnspecified())
                        {
                            firstNumberedIfAddress = Ipv4Address(link.getLinkData());
                        }
                    }
                }
                else if (link.getType() == TRANSIT_LINK) {
                    if (firstNumberedIfAddress.isUnspecified())
                        firstNumberedIfAddress = Ipv4Address(link.getLinkData());
                }
                else if (link.getType() == VIRTUAL_LINK) {
                    if (link.getLinkID() == toRouterLSA->getHeader().getLinkStateID()) {
                        destination = Ipv4Address(link.getLinkData());
                        destinationFound = true;
                        break;
                    }
                    else {
                        if (firstNumberedIfAddress.isUnspecified())
                            firstNumberedIfAddress = Ipv4Address(link.getLinkData());
                    }
                }
                // There's no way to get an interface address for the router from a STUB_LINK
            }

            if (unnumberedPointToPointLink) {
                if (!firstNumberedIfAddress.isUnspecified())
                    destination = firstNumberedIfAddress;
                else
                    return true;
            }

            if (!destinationFound)
                return true;
        }
        else {
            NetworkLsa *toNetworkLSA = dynamic_cast<NetworkLsa *>(routingInfo->getParent());
            if (toNetworkLSA) {
                // get the interface address pointing backwards on the shortest path tree
                bool destinationFound = false;
                for (unsigned int i = 0; i < linkCount; i++) {
                    const auto& link = routerLSA->getLinks(i);

                    if ((link.getType() == TRANSIT_LINK) &&
                        (link.getLinkID() == Ipv4Address(toNetworkLSA->getHeader().getLinkStateID())))
                    {
                        destination = Ipv4Address(link.getLinkData());
                        destinationFound = true;
                        break;
                    }
                }
                if (!destinationFound)
                    return true;
            }
            else
                return true;
        }
    }

    Ospfv2NetworkLsa *networkLSA = dynamic_cast<Ospfv2NetworkLsa *>(lsa);
    if (networkLSA)
        destination = networkLSA->getHeader().getLinkStateID() & networkLSA->getNetworkMask();

    Ospfv2SummaryLsa *summaryLSA = dynamic_cast<Ospfv2SummaryLsa *>(lsa);
    if ((summaryLSA) && (summaryLSA->getHeader().getLsType() == SUMMARYLSA_NETWORKS_TYPE))
        destination = summaryLSA->getHeader().getLinkStateID() & summaryLSA->getNetworkMask();

    Ospfv2AsExternalLsa *asExternalLSA = dynamic_cast<Ospfv2AsExternalLsa *>(lsa);
    if (asExternalLSA)
        destination = asExternalLSA->getHeader().getLinkStateID() & asExternalLSA->getContents().getNetworkMask();

    return lookup(destination) == nullptr;
}

Ospfv2RoutingTableEntry *Router::lookup(Ipv4Address destination, std::vector<Ospfv2RoutingTableEntry *> *table /*= nullptr*/) const
{
    const std::vector<Ospfv2RoutingTableEntry *>& rTable = (table == nullptr) ? ospfRoutingTable : (*table);
    bool unreachable = false;
    std::vector<Ospfv2RoutingTableEntry *> discard;

    for (uint32_t i = 0; i < areas.size(); i++) {
        for (uint32_t j = 0; j < areas[i]->getAddressRangeCount(); j++) {
            Ipv4AddressRange range = areas[i]->getAddressRange(j);
            for (auto entry : rTable) {
                if (entry->getDestinationType() != Ospfv2RoutingTableEntry::NETWORK_DESTINATION)
                    continue;
                if (range.containsRange(entry->getDestination(), entry->getNetmask()) &&
                    (entry->getPathType() == Ospfv2RoutingTableEntry::INTRAAREA))
                {
                    // active area address range
                    Ospfv2RoutingTableEntry *discardEntry = new Ospfv2RoutingTableEntry(ift);
                    discardEntry->setDestination(range.address);
                    discardEntry->setNetmask(range.mask);
                    discardEntry->setDestinationType(Ospfv2RoutingTableEntry::NETWORK_DESTINATION);
                    discardEntry->setPathType(Ospfv2RoutingTableEntry::INTERAREA);
                    discardEntry->setArea(areas[i]->getAreaID());
                    discard.push_back(discardEntry);
                    break;
                }
            }
        }
    }

    Ospfv2RoutingTableEntry *bestMatch = nullptr;
    unsigned long longestMatch = 0;
    unsigned long dest = destination.getInt();

    for (auto entry : rTable) {
        if (entry->getDestinationType() != Ospfv2RoutingTableEntry::NETWORK_DESTINATION)
            continue;
        unsigned long entryAddress = entry->getDestination().getInt();
        unsigned long entryMask = entry->getNetmask().getInt();
        if ((entryAddress & entryMask) == (dest & entryMask)) {
            if ((dest & entryMask) > longestMatch) {
                longestMatch = (dest & entryMask);
                bestMatch = entry;
            }
        }
    }

    if (bestMatch == nullptr)
        unreachable = true;
    else {
        for (auto entry : discard) {
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

    for (uint32_t i = 0; i < discard.size(); i++)
        delete discard[i];

    if (unreachable)
        return nullptr;
    else
        return bestMatch;
}

void Router::rebuildRoutingTable()
{
    unsigned long areaCount = areas.size();
    bool hasTransitAreas = false;
    std::vector<Ospfv2RoutingTableEntry *> newTable;

    EV_INFO << "--> Rebuilding routing table:\n";

    for (uint32_t i = 0; i < areaCount; i++) {
        areas[i]->calculateShortestPathTree(newTable);
        if (areas[i]->getTransitCapability())
            hasTransitAreas = true;
    }

    if (areaCount > 1) {
        Ospfv2Area *backbone = getAreaByID(BACKBONE_AREAID);
        // if this is an ABR and at least one adjacency in FULL state is built over the backbone
        if (backbone && backbone->hasAnyNeighborInStates(Neighbor::FULL_STATE))
            backbone->calculateInterAreaRoutes(newTable);
        else {
            for (auto& area : areas)
                area->calculateInterAreaRoutes(newTable);
        }
    }
    else if (areaCount == 1)
        areas[0]->calculateInterAreaRoutes(newTable);

    if (hasTransitAreas) {
        for (uint32_t i = 0; i < areaCount; i++) {
            if (areas[i]->getTransitCapability())
                areas[i]->recheckSummaryLSAs(newTable);
        }
    }

    calculateASExternalRoutes(newTable);

    // backup the routing table
    std::vector<Ospfv2RoutingTableEntry *> oldTable;
    oldTable.assign(ospfRoutingTable.begin(), ospfRoutingTable.end());
    ospfRoutingTable.clear();
    ospfRoutingTable.assign(newTable.begin(), newTable.end());

    // remove entries from the Ipv4 routing table inserted by the OSPF module
    std::vector<Ipv4Route *> eraseEntries;
    for (int32_t i = 0; i < rt->getNumRoutes(); i++) {
        Ipv4Route *entry = rt->getRoute(i);
        Ospfv2RoutingTableEntry *ospfEntry = dynamic_cast<Ospfv2RoutingTableEntry *>(entry);
        if (ospfEntry != nullptr)
            eraseEntries.push_back(entry);
    }

    // add the new routing entries
    std::vector<Ipv4Route *> addEntries;
    for (auto& tableEntry : ospfRoutingTable) {
        if (tableEntry->getDestinationType() == Ospfv2RoutingTableEntry::NETWORK_DESTINATION) {
            // OSPF never adds direct routes into the IP routing table
            if (!isDirectRoute(*tableEntry)) {
                // ignore advertised loopback addresses with dest=gateway
                if (tableEntry->getDestination() != tableEntry->getGateway()) {
                    Ipv4Route *entry = new Ospfv2RoutingTableEntry(*tableEntry);
                    addEntries.push_back(entry);
                }
            }
        }
    }

    // find the difference between two tables
    std::vector<Ipv4Route *> diffAddEntries;
    std::vector<Ipv4Route *> diffEraseEntries;
    diffEraseEntries.assign(eraseEntries.begin(), eraseEntries.end());
    for (auto& entry : addEntries) {
        auto position = std::find_if(diffEraseEntries.begin(), diffEraseEntries.end(), [&] (const Ipv4Route *m) -> bool {
            return (m->getDestination() == entry->getDestination()) &&
                   (m->getNetmask() == entry->getNetmask()) &&
                   (m->getInterface()->getInterfaceId() == entry->getInterface()->getInterfaceId()) &&
                   (m->getGateway() == entry->getGateway()) &&
                   (m->getMetric() == entry->getMetric());
        });
        if (position != diffEraseEntries.end())
            diffEraseEntries.erase(position);
        else
            diffAddEntries.push_back(entry);
    }

    if (!diffEraseEntries.empty() || !diffAddEntries.empty()) {
        EV_INFO << "OSPF routing table has changed: \n";
        for (auto& entry : diffEraseEntries)
            EV_INFO << "deleted: " << entry << "\n";
        for (auto& entry : diffAddEntries)
            EV_INFO << "added: " << entry << "\n";
    }
    else {
        EV_INFO << "No changes to the OSPF routing table. \n";
    }

    for (auto& entry : eraseEntries)
        rt->deleteRoute(entry);

    for (auto& entry : addEntries)
        rt->addRoute(entry);

    EV_INFO << "<-- Routing table was rebuilt.\n"
            << "Results:\n";

    for (auto& entry : ospfRoutingTable)
        EV_INFO << entry << "\n";

    notifyAboutRoutingTableChanges(oldTable);

    for (auto& entry : oldTable)
        delete entry;
}

bool Router::deleteRoute(Ospfv2RoutingTableEntry *entry)
{
    auto i = find(ospfRoutingTable, entry);
    if (i != ospfRoutingTable.end()) {
        delete *i;
        ospfRoutingTable.erase(i);
        return true;
    }
    return false;
}

bool Router::hasRouteToASBoundaryRouter(const std::vector<Ospfv2RoutingTableEntry *>& inRoutingTable, RouterId asbrRouterID) const
{
    for (uint32_t i = 0; i < inRoutingTable.size(); i++) {
        Ospfv2RoutingTableEntry *routingEntry = inRoutingTable[i];
        if (routingEntry->getDestination() == (asbrRouterID & routingEntry->getNetmask())) {
            if (!routingEntry->getGateway().isUnspecified())
                return true;
            else {
                // ASBR is a directly-connected router
                bool nextHopFound = false;
                for (uint32_t i = 0; i < areas.size(); i++) {
                    Ospfv2Interface *ospfIfEntry = areas[i]->getInterface(routingEntry->getInterface()->getInterfaceId());
                    if (ospfIfEntry) {
                        Neighbor *neighbor = ospfIfEntry->getNeighborById(asbrRouterID);
                        if (neighbor) {
                            nextHopFound = true;
                            break;
                        }
                    }
                }
                return nextHopFound;
            }
        }
    }
    return false;
}

std::vector<Ospfv2RoutingTableEntry *> Router::getRoutesToASBoundaryRouter(const std::vector<Ospfv2RoutingTableEntry *>& fromRoutingTable, RouterId asbrRouterID) const
{
    std::vector<Ospfv2RoutingTableEntry *> results;
    for (uint32_t i = 0; i < fromRoutingTable.size(); i++) {
        Ospfv2RoutingTableEntry *routingEntry = fromRoutingTable[i];
        if (routingEntry->getDestination() == (asbrRouterID & routingEntry->getNetmask())) {
            if (!routingEntry->getGateway().isUnspecified())
                results.push_back(routingEntry);
            else {
                // ASBR is a directly-connected router
                for (uint32_t i = 0; i < areas.size(); i++) {
                    Ospfv2Interface *ospfIfEntry = areas[i]->getInterface(routingEntry->getInterface()->getInterfaceId());
                    if (ospfIfEntry) {
                        Neighbor *neighbor = ospfIfEntry->getNeighborById(asbrRouterID);
                        if (neighbor) {
                            results.push_back(routingEntry);
                            break;
                        }
                    }
                }
            }
        }
    }
    return results;
}

void Router::pruneASBoundaryRouterEntries(std::vector<Ospfv2RoutingTableEntry *>& asbrEntries) const
{
    bool hasNonBackboneIntraAreaPath = false;
    for (auto routingEntry : asbrEntries) {
        if ((routingEntry->getPathType() == Ospfv2RoutingTableEntry::INTRAAREA) &&
            (routingEntry->getArea() != BACKBONE_AREAID))
        {
            hasNonBackboneIntraAreaPath = true;
            break;
        }
    }

    if (hasNonBackboneIntraAreaPath) {
        auto it = asbrEntries.begin();
        while (it != asbrEntries.end()) {
            if (((*it)->getPathType() != Ospfv2RoutingTableEntry::INTRAAREA) ||
                ((*it)->getArea() == BACKBONE_AREAID))
            {
                delete *it;
                it = asbrEntries.erase(it);
            }
            else {
                it++;
            }
        }
    }
}

Ospfv2RoutingTableEntry *Router::selectLeastCostRoutingEntry(std::vector<Ospfv2RoutingTableEntry *>& entries) const
{
    if (entries.empty())
        return nullptr;

    Ospfv2RoutingTableEntry *leastCostEntry = entries[0];
    Metric leastCost = leastCostEntry->getCost();

    for (uint32_t i = 1; i < entries.size(); i++) {
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

Ospfv2RoutingTableEntry *Router::getPreferredEntry(const Ospfv2Lsa& lsa, bool skipSelfOriginated, std::vector<Ospfv2RoutingTableEntry *> *fromRoutingTable /*= nullptr*/)
{
    // see RFC 2328 16.3. and 16.4.
    if (fromRoutingTable == nullptr)
        fromRoutingTable = &ospfRoutingTable;

    const Ospfv2LsaHeader& lsaHeader = lsa.getHeader();
    const Ospfv2AsExternalLsa *asExternalLSA = dynamic_cast<const Ospfv2AsExternalLsa *>(&lsa);
    unsigned long externalCost = (asExternalLSA != nullptr) ? asExternalLSA->getContents().getExternalTOSInfo(0).routeCost : 0;
    unsigned short lsAge = lsaHeader.getLsAge();
    RouterId originatingRouter = lsaHeader.getAdvertisingRouter();
    bool selfOriginated = (originatingRouter == routerID);
    Ipv4Address forwardingAddress; // 0.0.0.0

    if (asExternalLSA != nullptr)
        forwardingAddress = asExternalLSA->getContents().getExternalTOSInfo(0).forwardingAddress;

    if ((externalCost == LS_INFINITY) || (lsAge == MAX_AGE) || (skipSelfOriginated && selfOriginated)) // (1) and(2)
        return nullptr;

    if (!hasRouteToASBoundaryRouter(*fromRoutingTable, originatingRouter)) // (3)
        return nullptr;

    if (forwardingAddress.isUnspecified()) { // (3)
        auto asbrEntries = getRoutesToASBoundaryRouter(*fromRoutingTable, originatingRouter);
        if (!rfc1583Compatibility)
            pruneASBoundaryRouterEntries(asbrEntries);
        return selectLeastCostRoutingEntry(asbrEntries);
    }
    else {
        Ospfv2RoutingTableEntry *forwardEntry = lookup(forwardingAddress, fromRoutingTable);

        if (forwardEntry == nullptr)
            return nullptr;

        if ((forwardEntry->getPathType() != Ospfv2RoutingTableEntry::INTRAAREA) &&
            (forwardEntry->getPathType() != Ospfv2RoutingTableEntry::INTERAREA))
        {
            return nullptr;
        }

        // if a direct delivery, update hop address to point to the forward address
        if (isDirectRoute(*forwardEntry)) {
            forwardEntry->clearNextHops();
            NextHop hop = { forwardEntry->getInterface()->getInterfaceId(), forwardingAddress, routerID };
            forwardEntry->addNextHop(hop);
        }

        return forwardEntry;
    }

    return nullptr;
}

void Router::calculateASExternalRoutes(std::vector<Ospfv2RoutingTableEntry *>& newRoutingTable)
{
    // see RFC 2328 16.4.

    printAsExternalLsa();

    for (uint32_t i = 0; i < asExternalLSAs.size(); i++) {
        AsExternalLsa *currentLSA = asExternalLSAs[i];
        const Ospfv2LsaHeader& currentHeader = currentLSA->getHeader();
        unsigned short externalCost = currentLSA->getContents().getExternalTOSInfo(0).routeCost;
        RouterId originatingRouter = currentHeader.getAdvertisingRouter();

        Ospfv2RoutingTableEntry *preferredEntry = getPreferredEntry(*currentLSA, true, &newRoutingTable);
        if (!preferredEntry)
            continue;

        Ipv4Address destination = currentHeader.getLinkStateID() & currentLSA->getContents().getNetworkMask();

        Metric preferredCost = preferredEntry->getCost();
        Ospfv2RoutingTableEntry *destinationEntry = lookup(destination, &newRoutingTable); // (5)
        if (destinationEntry == nullptr) {
            bool type2ExternalMetric = currentLSA->getContents().getExternalTOSInfo(0).E_ExternalMetricType;
            Ospfv2RoutingTableEntry *newEntry = new Ospfv2RoutingTableEntry(ift);

            newEntry->setDestination(destination);
            newEntry->setNetmask(currentLSA->getContents().getNetworkMask());
            newEntry->setArea(preferredEntry->getArea());
            newEntry->setPathType(type2ExternalMetric ? Ospfv2RoutingTableEntry::TYPE2_EXTERNAL : Ospfv2RoutingTableEntry::TYPE1_EXTERNAL);
            if (type2ExternalMetric) {
                newEntry->setCost(preferredCost);
                newEntry->setType2Cost(externalCost);
            }
            else {
                newEntry->setCost(preferredCost + externalCost);
            }
            newEntry->setDestinationType(Ospfv2RoutingTableEntry::NETWORK_DESTINATION);
            newEntry->setOptionalCapabilities(currentHeader.getLsOptions());
            newEntry->setLinkStateOrigin(currentLSA);

            for (unsigned int j = 0; j < preferredEntry->getNextHopCount(); j++) {
                NextHop nextHop = preferredEntry->getNextHop(j);
                if (!nextHop.hopAddress.isUnspecified()) {
                    nextHop.advertisingRouter = originatingRouter;
                    newEntry->addNextHop(nextHop);
                }
            }

            newRoutingTable.push_back(newEntry);
        }
        else {
            Ospfv2RoutingTableEntry::RoutingPathType destinationPathType = destinationEntry->getPathType();
            bool type2ExternalMetric = currentLSA->getContents().getExternalTOSInfo(0).E_ExternalMetricType;
            unsigned int nextHopCount = preferredEntry->getNextHopCount();

            if ((destinationPathType == Ospfv2RoutingTableEntry::INTRAAREA) ||
                (destinationPathType == Ospfv2RoutingTableEntry::INTERAREA)) // (6) (a)
            {
                continue;
            }

            if (((destinationPathType == Ospfv2RoutingTableEntry::TYPE1_EXTERNAL) &&
                 (type2ExternalMetric)) ||
                ((destinationPathType == Ospfv2RoutingTableEntry::TYPE2_EXTERNAL) &&
                 (type2ExternalMetric) &&
                 (destinationEntry->getType2Cost() < externalCost))) // (6) (b)
            {
                continue;
            }

            Ospfv2RoutingTableEntry *destinationPreferredEntry = getPreferredEntry(*(destinationEntry->getLinkStateOrigin()), false, &newRoutingTable);
            if ((!rfc1583Compatibility) &&
                (destinationPreferredEntry->getPathType() == Ospfv2RoutingTableEntry::INTRAAREA) &&
                (destinationPreferredEntry->getArea() != BACKBONE_AREAID) &&
                ((preferredEntry->getPathType() != Ospfv2RoutingTableEntry::INTRAAREA) ||
                 (preferredEntry->getArea() == BACKBONE_AREAID)))
            {
                continue;
            }

            if ((((destinationPathType == Ospfv2RoutingTableEntry::TYPE1_EXTERNAL) &&
                  (!type2ExternalMetric) &&
                  (destinationEntry->getCost() < preferredCost + externalCost))) ||
                ((destinationPathType == Ospfv2RoutingTableEntry::TYPE2_EXTERNAL) &&
                 (type2ExternalMetric) &&
                 (destinationEntry->getType2Cost() == externalCost) &&
                 (destinationPreferredEntry->getCost() < preferredCost)))
            {
                continue;
            }

            if (((destinationPathType == Ospfv2RoutingTableEntry::TYPE1_EXTERNAL) &&
                 (!type2ExternalMetric) &&
                 (destinationEntry->getCost() == (preferredCost + externalCost))) ||
                ((destinationPathType == Ospfv2RoutingTableEntry::TYPE2_EXTERNAL) &&
                 (type2ExternalMetric) &&
                 (destinationEntry->getType2Cost() == externalCost) &&
                 (destinationPreferredEntry->getCost() == preferredCost))) // equal cost
            {
                for (unsigned int j = 0; j < nextHopCount; j++) {
                    // TODO merge next hops, not add
                    NextHop nextHop = preferredEntry->getNextHop(j);
                    if (!nextHop.hopAddress.isUnspecified()) {
                        nextHop.advertisingRouter = originatingRouter;
                        destinationEntry->addNextHop(nextHop);
                    }
                }
                continue;
            }

            // LSA is better
            destinationEntry->setArea(preferredEntry->getArea());
            destinationEntry->setPathType(type2ExternalMetric ? Ospfv2RoutingTableEntry::TYPE2_EXTERNAL : Ospfv2RoutingTableEntry::TYPE1_EXTERNAL);
            if (type2ExternalMetric) {
                destinationEntry->setCost(preferredCost);
                destinationEntry->setType2Cost(externalCost);
            }
            else {
                destinationEntry->setCost(preferredCost + externalCost);
            }
            destinationEntry->setDestinationType(Ospfv2RoutingTableEntry::NETWORK_DESTINATION);
            destinationEntry->setOptionalCapabilities(currentHeader.getLsOptions());
            destinationEntry->clearNextHops();

            for (unsigned int j = 0; j < nextHopCount; j++) {
                NextHop nextHop = preferredEntry->getNextHop(j);
                if (!nextHop.hopAddress.isUnspecified()) {
                    nextHop.advertisingRouter = originatingRouter;
                    destinationEntry->addNextHop(nextHop);
                }
            }
        }
    }
}

Ipv4AddressRange Router::getContainingAddressRange(const Ipv4AddressRange& addressRange, bool *advertise /*= nullptr*/) const
{
    unsigned long areaCount = areas.size();
    for (unsigned long i = 0; i < areaCount; i++) {
        Ipv4AddressRange containingAddressRange = areas[i]->getContainingAddressRange(addressRange, advertise);
        if (containingAddressRange != NULL_IPV4ADDRESSRANGE) {
            return containingAddressRange;
        }
    }
    if (advertise != nullptr) {
        *advertise = false;
    }
    return NULL_IPV4ADDRESSRANGE;
}

LinkStateId Router::getUniqueLinkStateID(const Ipv4AddressRange& destination,
        Metric destinationCost,
        AsExternalLsa *& lsaToReoriginate,
        bool externalMetricIsType2 /*= false*/) const
{
    if (lsaToReoriginate != nullptr) {
        delete lsaToReoriginate;
        lsaToReoriginate = nullptr;
    }

    LsaKeyType lsaKey;

    lsaKey.linkStateID = destination.address;
    lsaKey.advertisingRouter = routerID;

    const AsExternalLsa *foundLSA = findASExternalLSA(lsaKey);

    if (foundLSA == nullptr) {
        return lsaKey.linkStateID;
    }
    else {
        Ipv4Address existingMask = foundLSA->getContents().getNetworkMask();

        if (destination.mask >= existingMask) {
            return lsaKey.linkStateID.makeBroadcastAddress(destination.mask);
        }
        else {
            AsExternalLsa *asExternalLSA = new AsExternalLsa(*foundLSA);

            long sequenceNumber = asExternalLSA->getHeader().getLsSequenceNumber();

            asExternalLSA->getHeaderForUpdate().setLsAge(0);
            asExternalLSA->getHeaderForUpdate().setLsSequenceNumber((sequenceNumber == MAX_SEQUENCE_NUMBER) ? INITIAL_SEQUENCE_NUMBER : sequenceNumber + 1);
            asExternalLSA->getContentsForUpdate().setNetworkMask(destination.mask);
            asExternalLSA->getContentsForUpdate().getExternalTOSInfoForUpdate(0).E_ExternalMetricType = externalMetricIsType2;
            asExternalLSA->getContentsForUpdate().getExternalTOSInfoForUpdate(0).routeCost = destinationCost;

            lsaToReoriginate = asExternalLSA;

            return lsaKey.linkStateID.makeBroadcastAddress(existingMask);
        }
    }
}

// TODO review this algorithm + add virtual link changes(RFC2328 Section 16.7.).
void Router::notifyAboutRoutingTableChanges(std::vector<Ospfv2RoutingTableEntry *>& oldRoutingTable)
{
    // return if this router is not an ABR
    if (areas.size() <= 1)
        return;
    auto position = std::find_if(areas.begin(), areas.end(),
            [&] (const Ospfv2Area *m) -> bool { return m->getAreaID() == BACKBONE_AREAID; });
    if (position == areas.end())
        return;

    typedef std::map<Ipv4AddressRange, Ospfv2RoutingTableEntry *> RoutingTableEntryMap;
    RoutingTableEntryMap oldTableMap;
    RoutingTableEntryMap newTableMap;

    for (uint32_t i = 0; i < oldRoutingTable.size(); i++) {
        Ipv4AddressRange destination(oldRoutingTable[i]->getDestination() & oldRoutingTable[i]->getNetmask(), oldRoutingTable[i]->getNetmask());
        oldTableMap[destination] = oldRoutingTable[i];
    }

    for (uint32_t i = 0; i < ospfRoutingTable.size(); i++) {
        Ipv4AddressRange destination(ospfRoutingTable[i]->getDestination() & ospfRoutingTable[i]->getNetmask(), ospfRoutingTable[i]->getNetmask());
        newTableMap[destination] = ospfRoutingTable[i];
    }

    for (uint32_t i = 0; i < areas.size(); i++) {
        std::map<LsaKeyType, bool, LsaKeyType_Less> originatedLSAMap;
        std::map<LsaKeyType, bool, LsaKeyType_Less> deletedLSAMap;
        LsaKeyType lsaKey;

        // iterate over the new routing table and look for new or modified entries
        for (uint32_t j = 0; j < ospfRoutingTable.size(); j++) {
            Ipv4AddressRange destination(ospfRoutingTable[j]->getDestination() & ospfRoutingTable[j]->getNetmask(), ospfRoutingTable[j]->getNetmask());
            auto destIt = oldTableMap.find(destination);
            if (destIt == oldTableMap.end()) { // new routing entry
                SummaryLsa *lsaToReoriginate = nullptr;
                SummaryLsa *newLSA = areas[i]->originateSummaryLSA(ospfRoutingTable[j], originatedLSAMap, lsaToReoriginate);

                if (newLSA != nullptr) {
                    if (lsaToReoriginate != nullptr) {
                        areas[i]->installSummaryLSA(lsaToReoriginate);
                        floodLSA(lsaToReoriginate, areas[i]->getAreaID());

                        lsaKey.linkStateID = lsaToReoriginate->getHeader().getLinkStateID();
                        lsaKey.advertisingRouter = routerID;
                        originatedLSAMap[lsaKey] = true;

                        delete lsaToReoriginate;
                    }

                    areas[i]->installSummaryLSA(newLSA);
                    floodLSA(newLSA, areas[i]->getAreaID());

                    lsaKey.linkStateID = newLSA->getHeader().getLinkStateID();
                    lsaKey.advertisingRouter = routerID;
                    originatedLSAMap[lsaKey] = true;

                    delete newLSA;
                }
            }
            else {
                if (*(ospfRoutingTable[j]) != *(destIt->second)) { // modified routing entry
                    SummaryLsa *lsaToReoriginate = nullptr;
                    SummaryLsa *newLSA = areas[i]->originateSummaryLSA(ospfRoutingTable[j], originatedLSAMap, lsaToReoriginate);

                    if (newLSA != nullptr) {
                        if (lsaToReoriginate != nullptr) {
                            areas[i]->installSummaryLSA(lsaToReoriginate);
                            floodLSA(lsaToReoriginate, areas[i]->getAreaID());

                            lsaKey.linkStateID = lsaToReoriginate->getHeader().getLinkStateID();
                            lsaKey.advertisingRouter = routerID;
                            originatedLSAMap[lsaKey] = true;

                            delete lsaToReoriginate;
                        }

                        /*
                         * A router uses initial sequence number the first time it originates any LSA.
                         * Afterwards, the LSA's sequence number is incremented each time the router
                         * originates a new instance of the LSA.
                         */
                        int32_t sequenceNumber = newLSA->getHeader().getLsSequenceNumber();
                        if (sequenceNumber != MAX_SEQUENCE_NUMBER)
                            newLSA->getHeaderForUpdate().setLsSequenceNumber(sequenceNumber + 1);

                        areas[i]->installSummaryLSA(newLSA);
                        floodLSA(newLSA, areas[i]->getAreaID());

                        lsaKey.linkStateID = newLSA->getHeader().getLinkStateID();
                        lsaKey.advertisingRouter = routerID;
                        originatedLSAMap[lsaKey] = true;

                        delete newLSA;
                    }
                    else {
                        Ipv4AddressRange destinationAddressRange(ospfRoutingTable[j]->getDestination(), ospfRoutingTable[j]->getNetmask());

                        if ((ospfRoutingTable[j]->getDestinationType() == Ospfv2RoutingTableEntry::NETWORK_DESTINATION) &&
                            ((ospfRoutingTable[j]->getPathType() == Ospfv2RoutingTableEntry::INTRAAREA) ||
                             (ospfRoutingTable[j]->getPathType() == Ospfv2RoutingTableEntry::INTERAREA)))
                        {
                            Ipv4AddressRange containingAddressRange = getContainingAddressRange(destinationAddressRange);
                            if (containingAddressRange != NULL_IPV4ADDRESSRANGE) {
                                destinationAddressRange = containingAddressRange;
                            }
                        }

                        Metric maxRangeCost = 0;
                        Metric oneLessCost = 0;

                        for (uint32_t k = 0; k < ospfRoutingTable.size(); k++) {
                            if ((ospfRoutingTable[k]->getDestinationType() == Ospfv2RoutingTableEntry::NETWORK_DESTINATION) &&
                                (ospfRoutingTable[k]->getPathType() == Ospfv2RoutingTableEntry::INTRAAREA) &&
                                ((ospfRoutingTable[k]->getDestination().getInt() & ospfRoutingTable[k]->getNetmask().getInt() & destinationAddressRange.mask.getInt()) ==
                                 (destinationAddressRange.address & destinationAddressRange.mask).getInt()) &&
                                (ospfRoutingTable[k]->getCost() > maxRangeCost))
                            {
                                oneLessCost = maxRangeCost;
                                maxRangeCost = ospfRoutingTable[k]->getCost();
                            }
                        }

                        if (maxRangeCost == ospfRoutingTable[j]->getCost()) { // this entry gives the range's cost
                            lsaKey.linkStateID = destinationAddressRange.address;
                            lsaKey.advertisingRouter = routerID;

                            SummaryLsa *summaryLSA = areas[i]->findSummaryLSA(lsaKey);

                            if (summaryLSA != nullptr) {
                                if (oneLessCost != 0) { // there's an other entry in this range
                                    summaryLSA->setRouteCost(oneLessCost);
                                    floodLSA(summaryLSA, areas[i]->getAreaID());

                                    originatedLSAMap[lsaKey] = true;
                                }
                                else { // no more entries in this range -> delete it
                                    if (!containsKey(deletedLSAMap, lsaKey)) {
                                        summaryLSA->getHeaderForUpdate().setLsAge(MAX_AGE);
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

        // iterate over the old routing table and look for deleted entries
        for (uint32_t j = 0; j < oldRoutingTable.size(); j++) {
            Ipv4AddressRange destination(oldRoutingTable[j]->getDestination() & oldRoutingTable[j]->getNetmask(), oldRoutingTable[j]->getNetmask());
            if (!containsKey(newTableMap, destination)) { // deleted routing entry
                Ipv4AddressRange destinationAddressRange(oldRoutingTable[j]->getDestination(), oldRoutingTable[j]->getNetmask());

                if ((oldRoutingTable[j]->getDestinationType() == Ospfv2RoutingTableEntry::NETWORK_DESTINATION) &&
                    ((oldRoutingTable[j]->getPathType() == Ospfv2RoutingTableEntry::INTRAAREA) ||
                     (oldRoutingTable[j]->getPathType() == Ospfv2RoutingTableEntry::INTERAREA)))
                {
                    Ipv4AddressRange containingAddressRange = getContainingAddressRange(destinationAddressRange);
                    if (containingAddressRange != NULL_IPV4ADDRESSRANGE) {
                        destinationAddressRange = containingAddressRange;
                    }
                }

                Metric maxRangeCost = 0;

                unsigned long newRouteCount = ospfRoutingTable.size();
                for (uint32_t k = 0; k < newRouteCount; k++) {
                    if ((ospfRoutingTable[k]->getDestinationType() == Ospfv2RoutingTableEntry::NETWORK_DESTINATION) &&
                        (ospfRoutingTable[k]->getPathType() == Ospfv2RoutingTableEntry::INTRAAREA) &&
                        ((ospfRoutingTable[k]->getDestination().getInt() & ospfRoutingTable[k]->getNetmask().getInt() & destinationAddressRange.mask.getInt()) ==
                         (destinationAddressRange.address & destinationAddressRange.mask).getInt()) && // FIXME correcting network comparison
                        (ospfRoutingTable[k]->getCost() > maxRangeCost))
                    {
                        maxRangeCost = ospfRoutingTable[k]->getCost();
                    }
                }

                if (maxRangeCost < oldRoutingTable[j]->getCost()) { // the range's cost will change
                    lsaKey.linkStateID = destinationAddressRange.address;
                    lsaKey.advertisingRouter = routerID;

                    SummaryLsa *summaryLSA = areas[i]->findSummaryLSA(lsaKey);

                    if (summaryLSA != nullptr) {
                        if (maxRangeCost > 0) { // there's an other entry in this range
                            summaryLSA->setRouteCost(maxRangeCost);
                            floodLSA(summaryLSA, areas[i]->getAreaID());

                            originatedLSAMap[lsaKey] = true;
                        }
                        else { // no more entries in this range -> delete it
                            auto deletedIt = deletedLSAMap.find(lsaKey);
                            if (deletedIt == deletedLSAMap.end()) {
                                summaryLSA->getHeaderForUpdate().setLsAge(MAX_AGE);
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

Ipv4Route *Router::getDefaultRoute()
{
    for (int32_t i = 0; i < rt->getNumRoutes(); i++) {
        Ipv4Route *entry = rt->getRoute(i);
        if (entry->getDestination().isUnspecified() && entry->getNetmask().isUnspecified())
            return entry;
    }
    return nullptr;
}

void Router::updateExternalRoute(Ipv4Address networkAddress, const Ospfv2AsExternalLsaContents& externalRouteContents, int ifIndex)
{
    if (ifIndex != -1) {
        bool inRoutingTable = false;
        Ipv4Route *entry = nullptr;
        // add the external route to the routing table if it was not added by another module
        for (int32_t i = 0; i < rt->getNumRoutes(); i++) {
            entry = rt->getRoute(i);
            if ((entry->getDestination() == networkAddress)
                && (entry->getNetmask() == externalRouteContents.getNetworkMask())) // TODO is it enough?
            {
                inRoutingTable = true;
                break;
            }
        }

        if (!inRoutingTable) {
            Ipv4Route *entry = new Ipv4Route;
            entry->setDestination(networkAddress);
            entry->setNetmask(externalRouteContents.getNetworkMask());
            entry->setInterface(ift->getInterfaceById(ifIndex));
            entry->setSourceType(IRoute::MANUAL);
            entry->setMetric(externalRouteContents.getExternalTOSInfo(0).routeCost);
            rt->addRoute(entry); // IIpv4RoutingTable deletes entry pointer
        }
        else {
            ASSERT(entry);
            entry->setMetric(externalRouteContents.getExternalTOSInfo(0).routeCost);
        }
    }

    AsExternalLsa *asExternalLSA = new AsExternalLsa;
    Ospfv2LsaHeader& lsaHeader = asExternalLSA->getHeaderForUpdate();
    Ospfv2Options lsaOptions;

    lsaHeader.setLsAge(0);
    lsaOptions.E_ExternalRoutingCapability = true;
    lsaHeader.setLsOptions(lsaOptions);
    lsaHeader.setLsType(AS_EXTERNAL_LSA_TYPE);
    lsaHeader.setLinkStateID(networkAddress); // TODO get unique LinkStateId
    lsaHeader.setAdvertisingRouter(Ipv4Address(routerID));
    lsaHeader.setLsSequenceNumber(INITIAL_SEQUENCE_NUMBER);

    asExternalLSA->setContents(externalRouteContents);

    asExternalLSA->setSource(LsaTrackingInfo::ORIGINATED);

    externalRoutes[networkAddress] = externalRouteContents;

    lsaHeader.setLsaLength(B(calculateLsaSize(*asExternalLSA)).get());

    bool rebuild = installASExternalLSA(asExternalLSA);
    floodLSA(asExternalLSA, BACKBONE_AREAID);
    delete asExternalLSA;

    if (rebuild)
        rebuildRoutingTable();
}

void Router::addExternalRouteInIPTable(Ipv4Address networkAddress, const Ospfv2AsExternalLsaContents& externalRouteContents, int ifIndex)
{
    // add the external route to the Ipv4 routing table if it was not added by another module
    bool inRoutingTable = false;
    for (int32_t i = 1; i < rt->getNumRoutes(); i++) {
        const Ipv4Route *entry = rt->getRoute(i);
        if ((entry->getDestination() == networkAddress)
            && (entry->getNetmask() == externalRouteContents.getNetworkMask())) // TODO is it enough?
        {
            inRoutingTable = true;
            break;
        }
    }

    if (!inRoutingTable) {
        Ipv4Route *entry = new Ipv4Route();
        entry->setDestination(networkAddress);
        entry->setNetmask(externalRouteContents.getNetworkMask());
        entry->setInterface(ift->getInterfaceById(ifIndex));
        entry->setSourceType(IRoute::OSPF);
        entry->setMetric(OSPFv2_BGP_DEFAULT_COST);
        rt->addRoute(entry);
    }
}

void Router::removeExternalRoute(Ipv4Address networkAddress)
{
    LsaKeyType lsaKey;

    lsaKey.linkStateID = networkAddress;
    lsaKey.advertisingRouter = routerID;

    auto lsaIt = asExternalLSAsByID.find(lsaKey);
    if (lsaIt != asExternalLSAsByID.end()) {
        lsaIt->second->getHeaderForUpdate().setLsAge(MAX_AGE);
        lsaIt->second->setPurgeable();
        floodLSA(lsaIt->second, BACKBONE_AREAID);
    }

    auto externalIt = externalRoutes.find(networkAddress);
    if (externalIt != externalRoutes.end()) {
        externalRoutes.erase(externalIt);
    }
}

void Router::printAsExternalLsa()
{
    for (uint32_t i = 0; i < asExternalLSAs.size(); i++) {
        Ospfv2AsExternalLsa *entry = check_and_cast<Ospfv2AsExternalLsa *>(asExternalLSAs[i]);

        const Ospfv2LsaHeader& head = entry->getHeader();
        std::string routerId = head.getAdvertisingRouter().str(false);
        EV_DETAIL << "AS External LSA in OSPF router with ID " << routerId << std::endl;

        // print header info
        EV_DETAIL << "    LS age: " << head.getLsAge() << std::endl;
        EV_DETAIL << "    LS type: " << head.getLsType() << std::endl;
        EV_DETAIL << "    Link state ID (IP network): " << head.getLinkStateID() << std::endl;
        EV_DETAIL << "    Advertising router: " << head.getAdvertisingRouter() << std::endl;
        EV_DETAIL << "    Seq number: " << head.getLsSequenceNumber() << std::endl;
        EV_DETAIL << "    Length: " << head.getLsaLength() << std::endl;

        EV_DETAIL << "    Network Mask: " << entry->getContents().getNetworkMask().str(false) << std::endl;
        EV_DETAIL << "    Metric: " << entry->getContents().getExternalTOSInfo(0).routeCost << std::endl;
        EV_DETAIL << "    E flag: " << ((entry->getContents().getExternalTOSInfo(0).E_ExternalMetricType == true) ? "set" : "unset") << std::endl;
        EV_DETAIL << "    Forwarding Address: " << entry->getContents().getExternalTOSInfo(0).forwardingAddress.str(false) << std::endl;
        EV_DETAIL << "    External Route Tag: " << entry->getContents().getExternalTOSInfo(0).externalRouteTag << std::endl;
        // todo: add ExternalTosInfo externalTOSInfo[];
        EV_DETAIL << std::endl;
    }
}

bool Router::isDirectRoute(Ospfv2RoutingTableEntry& entry)
{
    if (entry.getGateway().isUnspecified())
        return true;

    for (int i = 0; i < ift->getNumInterfaces(); i++) {
        NetworkInterface *intf = ift->getInterface(i);
        if (intf && !intf->isLoopback()) {
            const auto& ipv4data = intf->findProtocolData<Ipv4InterfaceData>();
            if (ipv4data) {
                if ((entry.getDestination() & ipv4data->getNetmask()) == (ipv4data->getIPAddress() & ipv4data->getNetmask()))
                    return true;
            }
        }
    }

    return false;
}

} // namespace ospfv2

} // namespace inet

