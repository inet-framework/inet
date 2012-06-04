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


#include "OSPFRouter.h"

#include "RoutingTableAccess.h"


OSPF::Router::Router(OSPF::RouterID id, cSimpleModule* containingModule) :
    routerID(id),
    rfc1583Compatibility(false)
{
    messageHandler = new OSPF::MessageHandler(this, containingModule);
    ageTimer = new OSPFTimer();
    ageTimer->setTimerKind(DATABASE_AGE_TIMER);
    ageTimer->setContextPointer(this);
    ageTimer->setName("OSPF::Router::DatabaseAgeTimer");
    messageHandler->startTimer(ageTimer, 1.0);
}


OSPF::Router::~Router()
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


void OSPF::Router::addWatches()
{
    WATCH(routerID);
    WATCH_PTRVECTOR(areas);
    WATCH_PTRVECTOR(asExternalLSAs);
    WATCH_PTRVECTOR(routingTable);
}


void OSPF::Router::addArea(OSPF::Area* area)
{

    area->setRouter(this);
    areasByID[area->getAreaID()] = area;
    areas.push_back(area);
}


OSPF::Area* OSPF::Router::getAreaByID(OSPF::AreaID areaID)
{
    std::map<OSPF::AreaID, OSPF::Area*>::iterator areaIt = areasByID.find(areaID);
    if (areaIt != areasByID.end()) {
        return (areaIt->second);
    }
    else {
        return NULL;
    }
}


OSPF::Area* OSPF::Router::getAreaByAddr(IPv4Address address)
{
    long areaCount = areas.size();

    for (long i = 0; i < areaCount; i++)
    {
        if (areas[i]->containsAddress(address))
            return areas[i];
    }

    return NULL;
}


OSPF::Interface* OSPF::Router::getNonVirtualInterface(unsigned char ifIndex)
{
    long areaCount = areas.size();

    for (long i = 0; i < areaCount; i++)
    {
        OSPF::Interface* intf = areas[i]->getInterface(ifIndex);
        if (intf != NULL) {
            return intf;
        }
    }
    return NULL;
}


bool OSPF::Router::installLSA(OSPFLSA* lsa, OSPF::AreaID areaID /*= BACKBONE_AREAID*/)
{
    switch (lsa->getHeader().getLsType()) {
        case ROUTERLSA_TYPE:
            {
                std::map<OSPF::AreaID, OSPF::Area*>::iterator areaIt = areasByID.find(areaID);
                if (areaIt != areasByID.end()) {
                    OSPFRouterLSA* ospfRouterLSA = check_and_cast<OSPFRouterLSA*> (lsa);
                    return areaIt->second->installRouterLSA(ospfRouterLSA);
                }
            }
            break;
        case NETWORKLSA_TYPE:
            {
                std::map<OSPF::AreaID, OSPF::Area*>::iterator areaIt = areasByID.find(areaID);
                if (areaIt != areasByID.end()) {
                    OSPFNetworkLSA* ospfNetworkLSA = check_and_cast<OSPFNetworkLSA*> (lsa);
                    return areaIt->second->installNetworkLSA(ospfNetworkLSA);
                }
            }
            break;
        case SUMMARYLSA_NETWORKS_TYPE:
        case SUMMARYLSA_ASBOUNDARYROUTERS_TYPE:
            {
                std::map<OSPF::AreaID, OSPF::Area*>::iterator areaIt = areasByID.find(areaID);
                if (areaIt != areasByID.end()) {
                    OSPFSummaryLSA* ospfSummaryLSA = check_and_cast<OSPFSummaryLSA*> (lsa);
                    return areaIt->second->installSummaryLSA(ospfSummaryLSA);
                }
            }
            break;
        case AS_EXTERNAL_LSA_TYPE:
            {
                OSPFASExternalLSA* ospfASExternalLSA = check_and_cast<OSPFASExternalLSA*> (lsa);
                return installASExternalLSA(ospfASExternalLSA);
            }
            break;
        default:
            ASSERT(false);
            break;
    }
    return false;
}


bool OSPF::Router::installASExternalLSA(OSPFASExternalLSA* lsa)
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

    OSPF::RouterID advertisingRouter = lsa->getHeader().getAdvertisingRouter();
    bool reachable = false;
    unsigned int routeCount = routingTable.size();

    for (unsigned int i = 0; i < routeCount; i++) {
        if ((((routingTable[i]->getDestinationType() & OSPF::RoutingTableEntry::AREA_BORDER_ROUTER_DESTINATION) != 0) ||
             ((routingTable[i]->getDestinationType() & OSPF::RoutingTableEntry::AS_BOUNDARY_ROUTER_DESTINATION) != 0)) &&
            (routingTable[i]->getDestination() == advertisingRouter))
        {
            reachable = true;
            break;
        }
    }

    bool ownLSAFloodedOut = false;
    OSPF::LSAKeyType lsaKey;

    lsaKey.linkStateID = lsa->getHeader().getLinkStateID();
    lsaKey.advertisingRouter = routerID;

    std::map<OSPF::LSAKeyType, OSPF::ASExternalLSA*, OSPF::LSAKeyType_Less>::iterator lsaIt = asExternalLSAsByID.find(lsaKey);
    if ((lsaIt != asExternalLSAsByID.end()) &&
        reachable &&
        (lsaIt->second->getContents().getE_ExternalMetricType() == lsa->getContents().getE_ExternalMetricType()) &&
        (lsaIt->second->getContents().getRouteCost() == lsa->getContents().getRouteCost()) &&
        (lsa->getContents().getForwardingAddress().getInt() != 0) && // forwarding address != 0.0.0.0
        (lsaIt->second->getContents().getForwardingAddress() == lsa->getContents().getForwardingAddress()))
    {
        if (routerID > advertisingRouter) {
            return false;
        } else {
            lsaIt->second->getHeader().setLsAge(MAX_AGE);
            floodLSA(lsaIt->second, OSPF::BACKBONE_AREAID);
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
        return ((lsaIt->second->update(lsa)) | ownLSAFloodedOut);
    } else {
        OSPF::ASExternalLSA* lsaCopy = new OSPF::ASExternalLSA(*lsa);
        asExternalLSAsByID[lsaKey] = lsaCopy;
        asExternalLSAs.push_back(lsaCopy);
        return true;
    }
}


OSPFLSA* OSPF::Router::findLSA(LSAType lsaType, OSPF::LSAKeyType lsaKey, OSPF::AreaID areaID)
{
    switch (lsaType) {
        case ROUTERLSA_TYPE:
            {
                std::map<OSPF::AreaID, OSPF::Area*>::iterator areaIt = areasByID.find(areaID);
                if (areaIt != areasByID.end()) {
                    return areaIt->second->findRouterLSA(lsaKey.linkStateID);
                }
            }
            break;
        case NETWORKLSA_TYPE:
            {
                std::map<OSPF::AreaID, OSPF::Area*>::iterator areaIt = areasByID.find(areaID);
                if (areaIt != areasByID.end()) {
                    return areaIt->second->findNetworkLSA(lsaKey.linkStateID);
                }
            }
            break;
        case SUMMARYLSA_NETWORKS_TYPE:
        case SUMMARYLSA_ASBOUNDARYROUTERS_TYPE:
            {
                std::map<OSPF::AreaID, OSPF::Area*>::iterator areaIt = areasByID.find(areaID);
                if (areaIt != areasByID.end()) {
                    return areaIt->second->findSummaryLSA(lsaKey);
                }
            }
            break;
        case AS_EXTERNAL_LSA_TYPE:
            {
                return findASExternalLSA(lsaKey);
            }
            break;
        default:
            ASSERT(false);
            break;
    }
    return NULL;
}


OSPF::ASExternalLSA* OSPF::Router::findASExternalLSA(OSPF::LSAKeyType lsaKey)
{
    std::map<OSPF::LSAKeyType, OSPF::ASExternalLSA*, OSPF::LSAKeyType_Less>::iterator lsaIt = asExternalLSAsByID.find(lsaKey);
    if (lsaIt != asExternalLSAsByID.end()) {
        return lsaIt->second;
    } else {
        return NULL;
    }
}


const OSPF::ASExternalLSA* OSPF::Router::findASExternalLSA(OSPF::LSAKeyType lsaKey) const
{
    std::map<OSPF::LSAKeyType, OSPF::ASExternalLSA*, OSPF::LSAKeyType_Less>::const_iterator lsaIt = asExternalLSAsByID.find(lsaKey);
    if (lsaIt != asExternalLSAsByID.end()) {
        return lsaIt->second;
    } else {
        return NULL;
    }
}


void OSPF::Router::ageDatabase()
{
    long lsaCount = asExternalLSAs.size();
    bool shouldRebuildRoutingTable = false;

    for (long i = 0; i < lsaCount; i++) {
        unsigned short lsAge = asExternalLSAs[i]->getHeader().getLsAge();
        bool selfOriginated = (asExternalLSAs[i]->getHeader().getAdvertisingRouter() == routerID);
        bool unreachable = isDestinationUnreachable(asExternalLSAs[i]);
        OSPF::ASExternalLSA* lsa = asExternalLSAs[i];

        if ((selfOriginated && (lsAge < (LS_REFRESH_TIME - 1))) || (!selfOriginated && (lsAge < (MAX_AGE - 1)))) {
            lsa->getHeader().setLsAge(lsAge + 1);
            if ((lsAge + 1) % CHECK_AGE == 0) {
                if (!lsa->validateLSChecksum()) {
                    EV << "Invalid LS checksum. Memory error detected!\n";
                }
            }
            lsa->incrementInstallTime();
        }
        if (selfOriginated && (lsAge == (LS_REFRESH_TIME - 1))) {
            if (unreachable) {
                lsa->getHeader().setLsAge(MAX_AGE);
                floodLSA(lsa, OSPF::BACKBONE_AREAID);
                lsa->incrementInstallTime();
            } else {
                long sequenceNumber = lsa->getHeader().getLsSequenceNumber();
                if (sequenceNumber == MAX_SEQUENCE_NUMBER) {
                    lsa->getHeader().setLsAge(MAX_AGE);
                    floodLSA(lsa, OSPF::BACKBONE_AREAID);
                    lsa->incrementInstallTime();
                } else {
                    OSPF::ASExternalLSA* newLSA = originateASExternalLSA(lsa);

                    newLSA->getHeader().setLsSequenceNumber(sequenceNumber + 1);
                    shouldRebuildRoutingTable |= lsa->update(newLSA);
                    delete newLSA;

                    floodLSA(lsa, OSPF::BACKBONE_AREAID);
                }
            }
        }
        if (!selfOriginated && (lsAge == MAX_AGE - 1)) {
            lsa->getHeader().setLsAge(MAX_AGE);
            floodLSA(lsa, OSPF::BACKBONE_AREAID);
            lsa->incrementInstallTime();
        }
        if (lsAge == MAX_AGE) {
            OSPF::LSAKeyType lsaKey;

            lsaKey.linkStateID = lsa->getHeader().getLinkStateID();
            lsaKey.advertisingRouter = lsa->getHeader().getAdvertisingRouter();

            if (!isOnAnyRetransmissionList(lsaKey) &&
                !hasAnyNeighborInStates(OSPF::Neighbor::EXCHANGE_STATE | OSPF::Neighbor::LOADING_STATE))
            {
                if (!selfOriginated || unreachable) {
                    asExternalLSAsByID.erase(lsaKey);
                    delete lsa;
                    asExternalLSAs[i] = NULL;
                    shouldRebuildRoutingTable = true;
                } else {
                    if (lsa->getPurgeable()) {
                        asExternalLSAsByID.erase(lsaKey);
                        delete lsa;
                        asExternalLSAs[i] = NULL;
                        shouldRebuildRoutingTable = true;
                    } else {
                        OSPF::ASExternalLSA* newLSA = originateASExternalLSA(lsa);
                        long sequenceNumber = lsa->getHeader().getLsSequenceNumber();

                        newLSA->getHeader().setLsSequenceNumber((sequenceNumber == MAX_SEQUENCE_NUMBER) ? INITIAL_SEQUENCE_NUMBER : sequenceNumber + 1);
                        shouldRebuildRoutingTable |= lsa->update(newLSA);
                        delete newLSA;

                        floodLSA(lsa, OSPF::BACKBONE_AREAID);
                    }
                }
            }
        }
    }

    std::vector<ASExternalLSA*>::iterator it = asExternalLSAs.begin();
    while (it != asExternalLSAs.end()) {
        if ((*it) == NULL) {
            it = asExternalLSAs.erase(it);
        } else {
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


bool OSPF::Router::hasAnyNeighborInStates(int states) const
{
    long areaCount = areas.size();
    for (long i = 0; i < areaCount; i++) {
        if (areas[i]->hasAnyNeighborInStates(states)) {
            return true;
        }
    }
    return false;
}


void OSPF::Router::removeFromAllRetransmissionLists(OSPF::LSAKeyType lsaKey)
{
    long areaCount = areas.size();
    for (long i = 0; i < areaCount; i++) {
        areas[i]->removeFromAllRetransmissionLists(lsaKey);
    }
}


bool OSPF::Router::isOnAnyRetransmissionList(OSPF::LSAKeyType lsaKey) const
{
    long areaCount = areas.size();
    for (long i = 0; i < areaCount; i++) {
        if (areas[i]->isOnAnyRetransmissionList(lsaKey)) {
            return true;
        }
    }
    return false;
}


bool OSPF::Router::floodLSA(OSPFLSA* lsa, OSPF::AreaID areaID /*= BACKBONE_AREAID*/, OSPF::Interface* intf /*= NULL*/, OSPF::Neighbor* neighbor /*= NULL*/)
{
    bool floodedBackOut = false;

    if (lsa != NULL) {
        if (lsa->getHeader().getLsType() == AS_EXTERNAL_LSA_TYPE) {
            long areaCount = areas.size();
            for (long i = 0; i < areaCount; i++) {
                if (areas[i]->getExternalRoutingCapability()) {
                    if (areas[i]->floodLSA(lsa, intf, neighbor)) {
                        floodedBackOut = true;
                    }
                }
            }
        } else {
            std::map<OSPF::AreaID, OSPF::Area*>::iterator areaIt = areasByID.find(areaID);
            if (areaIt != areasByID.end()) {
                floodedBackOut = areaIt->second->floodLSA(lsa, intf, neighbor);
            }
        }
    }

    return floodedBackOut;
}


bool OSPF::Router::isLocalAddress(IPv4Address address) const
{
    long areaCount = areas.size();
    for (long i = 0; i < areaCount; i++) {
        if (areas[i]->isLocalAddress(address)) {
            return true;
        }
    }
    return false;
}


bool OSPF::Router::hasAddressRange(const OSPF::IPv4AddressRange& addressRange) const
{
    long areaCount = areas.size();
    for (long i = 0; i < areaCount; i++) {
        if (areas[i]->hasAddressRange(addressRange)) {
            return true;
        }
    }
    return false;
}


OSPF::ASExternalLSA* OSPF::Router::originateASExternalLSA(OSPF::ASExternalLSA* lsa)
{
    OSPF::ASExternalLSA* asExternalLSA = new OSPF::ASExternalLSA(*lsa);
    OSPFLSAHeader& lsaHeader = asExternalLSA->getHeader();
    OSPFOptions lsaOptions;

    lsaHeader.setLsAge(0);
    memset(&lsaOptions, 0, sizeof(OSPFOptions));
    lsaOptions.E_ExternalRoutingCapability = true;
    lsaHeader.setLsOptions(lsaOptions);
    lsaHeader.setLsSequenceNumber(INITIAL_SEQUENCE_NUMBER);
    asExternalLSA->setSource(OSPF::LSATrackingInfo::ORIGINATED);

    return asExternalLSA;
}


bool OSPF::Router::isDestinationUnreachable(OSPFLSA* lsa) const
{
    IPv4Address destination = IPv4Address(lsa->getHeader().getLinkStateID());

    OSPFRouterLSA* routerLSA = dynamic_cast<OSPFRouterLSA*> (lsa);
    OSPFNetworkLSA* networkLSA = dynamic_cast<OSPFNetworkLSA*> (lsa);
    OSPFSummaryLSA* summaryLSA = dynamic_cast<OSPFSummaryLSA*> (lsa);
    OSPFASExternalLSA* asExternalLSA = dynamic_cast<OSPFASExternalLSA*> (lsa);
    // TODO: verify
    if (routerLSA != NULL) {
        OSPF::RoutingInfo* routingInfo = check_and_cast<OSPF::RoutingInfo*> (routerLSA);
        if (routerLSA->getHeader().getLinkStateID() == routerID) { // this is spfTreeRoot
            return false;
        }

        // get the interface address pointing backwards on the shortest path tree
        unsigned int linkCount = routerLSA->getLinksArraySize();
        OSPF::RouterLSA* toRouterLSA = dynamic_cast<OSPF::RouterLSA*> (routingInfo->getParent());
        if (toRouterLSA != NULL) {
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
                        } else {
                            destination = IPv4Address(link.getLinkData());
                            destinationFound = true;
                            break;
                        }
                    } else {
                        if (((link.getLinkData() & 0xFF000000) != 0) &&
                             firstNumberedIfAddress.isUnspecified())
                        {
                            firstNumberedIfAddress = IPv4Address(link.getLinkData());
                        }
                    }
                } else if (link.getType() == TRANSIT_LINK) {
                    if (firstNumberedIfAddress.isUnspecified()) {
                        firstNumberedIfAddress = IPv4Address(link.getLinkData());
                    }
                } else if (link.getType() == VIRTUAL_LINK) {
                    if (link.getLinkID() == toRouterLSA->getHeader().getLinkStateID()) {
                        destination = IPv4Address(link.getLinkData());
                        destinationFound = true;
                        break;
                    } else {
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
                } else {
                    return true;
                }
            }
            if (!destinationFound) {
                return true;
            }
        } else {
            OSPF::NetworkLSA* toNetworkLSA = dynamic_cast<OSPF::NetworkLSA*> (routingInfo->getParent());
            if (toNetworkLSA != NULL) {
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
            } else {
                return true;
            }
        }
    }
    if (networkLSA != NULL) {
        destination = networkLSA->getHeader().getLinkStateID() & networkLSA->getNetworkMask();
    }
    if ((summaryLSA != NULL) && (summaryLSA->getHeader().getLsType() == SUMMARYLSA_NETWORKS_TYPE)) {
        destination = summaryLSA->getHeader().getLinkStateID() & summaryLSA->getNetworkMask();
    }
    if (asExternalLSA != NULL) {
        destination = asExternalLSA->getHeader().getLinkStateID() & asExternalLSA->getContents().getNetworkMask();
    }

    if (lookup(destination) == NULL) {
        return true;
    } else {
        return false;
    }
}


OSPF::RoutingTableEntry* OSPF::Router::lookup(IPv4Address destination, std::vector<OSPF::RoutingTableEntry*>* table /*= NULL*/) const
{
    const std::vector<OSPF::RoutingTableEntry*>& rTable = (table == NULL) ? routingTable : (*table);
    unsigned long dest = destination.getInt();
    unsigned long routingTableSize = rTable.size();
    bool unreachable = false;
    std::vector<OSPF::RoutingTableEntry*> discard;
    unsigned long i;

    unsigned long areaCount = areas.size();
    for (i = 0; i < areaCount; i++) {
        unsigned int addressRangeCount = areas[i]->getAddressRangeCount();
        for (unsigned int j = 0; j < addressRangeCount; j++) {
            OSPF::IPv4AddressRange range = areas[i]->getAddressRange(j);

            for (unsigned int k = 0; k < routingTableSize; k++) {
                OSPF::RoutingTableEntry* entry = rTable[k];

                if (entry->getDestinationType() != OSPF::RoutingTableEntry::NETWORK_DESTINATION) {
                    continue;
                }
//                if (((entry->getDestination().getInt() & entry->getNetmask().getInt() & range.mask.getInt()) == (range.address & range.mask).getInt()) &&
//                    (entry->getPathType() == OSPF::RoutingTableEntry::INTRAAREA))
                if (range.containsRange(entry->getDestination(), entry->getNetmask()) &&
                    (entry->getPathType() == OSPF::RoutingTableEntry::INTRAAREA))
                {
                    // active area address range
                    OSPF::RoutingTableEntry* discardEntry = new OSPF::RoutingTableEntry;
                    discardEntry->setDestination(range.address);
                    discardEntry->setNetmask(range.mask);
                    discardEntry->setDestinationType(OSPF::RoutingTableEntry::NETWORK_DESTINATION);
                    discardEntry->setPathType(OSPF::RoutingTableEntry::INTERAREA);
                    discardEntry->setArea(areas[i]->getAreaID());
                    discard.push_back(discardEntry);
                    break;
                }
            }
        }
    }

    OSPF::RoutingTableEntry* bestMatch = NULL;
    unsigned long longestMatch = 0;

    for (i = 0; i < routingTableSize; i++) {
        if (rTable[i]->getDestinationType() == OSPF::RoutingTableEntry::NETWORK_DESTINATION) {
            OSPF::RoutingTableEntry* entry = rTable[i];
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
    if (bestMatch == NULL) {
        unreachable = true;
    } else {
        for (i = 0; i < discardCount; i++) {
            OSPF::RoutingTableEntry* entry = discard[i];
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
        return NULL;
    } else {
        return bestMatch;
    }
}


void OSPF::Router::rebuildRoutingTable()
{
    unsigned long areaCount = areas.size();
    bool hasTransitAreas = false;
    std::vector<OSPF::RoutingTableEntry*> newTable;
    unsigned long i;

    EV << "Rebuilding routing table:\n";

    for (i = 0; i < areaCount; i++) {
        areas[i]->calculateShortestPathTree(newTable);
        if (areas[i]->getTransitCapability()) {
            hasTransitAreas = true;
        }
    }
    if (areaCount > 1) {
        OSPF::Area* backbone = getAreaByID(OSPF::BACKBONE_AREAID);
        if (backbone != NULL) {
            backbone->calculateInterAreaRoutes(newTable);
        }
    } else {
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
    std::vector<OSPF::RoutingTableEntry*> oldTable;

    oldTable.assign(routingTable.begin(), routingTable.end());
    routingTable.clear();
    routingTable.assign(newTable.begin(), newTable.end());

    RoutingTableAccess routingTableAccess;
    std::vector<IPv4Route*> eraseEntries;
    IRoutingTable* simRoutingTable = routingTableAccess.get();
    unsigned long routingEntryNumber = simRoutingTable->getNumRoutes();
    // remove entries from the IPv4 routing table inserted by the OSPF module
    for (i = 0; i < routingEntryNumber; i++) {
        IPv4Route *entry = simRoutingTable->getRoute(i);
        OSPF::RoutingTableEntry* ospfEntry = dynamic_cast<OSPF::RoutingTableEntry*>(entry);
        if (ospfEntry != NULL) {
            eraseEntries.push_back(entry);
        }
    }

    unsigned int eraseCount = eraseEntries.size();
    for (i = 0; i < eraseCount; i++) {
        simRoutingTable->deleteRoute(eraseEntries[i]);
    }

    // add the new routing entries
    routeCount = routingTable.size();
    for (i = 0; i < routeCount; i++) {
        if (routingTable[i]->getDestinationType() == OSPF::RoutingTableEntry::NETWORK_DESTINATION) {
            simRoutingTable->addRoute(new OSPF::RoutingTableEntry(*(routingTable[i])));
        }
    }

    notifyAboutRoutingTableChanges(oldTable);

    routeCount = oldTable.size();
    for (i = 0; i < routeCount; i++) {
        delete (oldTable[i]);
    }

    EV << "Routing table was rebuilt.\n"
       << "Results:\n";

    routeCount = routingTable.size();
    for (i = 0; i < routeCount; i++) {
        EV << *routingTable[i]
           << "\n";
    }
}


bool OSPF::Router::hasRouteToASBoundaryRouter(const std::vector<OSPF::RoutingTableEntry*>& inRoutingTable, OSPF::RouterID asbrRouterID) const
{
    long routeCount = inRoutingTable.size();
    for (long i = 0; i < routeCount; i++) {
        OSPF::RoutingTableEntry* routingEntry = inRoutingTable[i];
        if (((routingEntry->getDestinationType() & OSPF::RoutingTableEntry::AS_BOUNDARY_ROUTER_DESTINATION) != 0) &&
            (routingEntry->getDestination() == asbrRouterID))
        {
            return true;
        }
    }
    return false;
}


std::vector<OSPF::RoutingTableEntry*> OSPF::Router::getRoutesToASBoundaryRouter(const std::vector<OSPF::RoutingTableEntry*>& fromRoutingTable, OSPF::RouterID asbrRouterID) const
{
    std::vector<OSPF::RoutingTableEntry*> results;
    long routeCount = fromRoutingTable.size();

    for (long i = 0; i < routeCount; i++) {
        OSPF::RoutingTableEntry* routingEntry = fromRoutingTable[i];
        if (((routingEntry->getDestinationType() & OSPF::RoutingTableEntry::AS_BOUNDARY_ROUTER_DESTINATION) != 0) &&
            (routingEntry->getDestination() == asbrRouterID))
        {
            results.push_back(routingEntry);
        }
    }
    return results;
}


void OSPF::Router::pruneASBoundaryRouterEntries(std::vector<OSPF::RoutingTableEntry*>& asbrEntries) const
{
    bool hasNonBackboneIntraAreaPath = false;
    for (std::vector<OSPF::RoutingTableEntry*>::iterator it = asbrEntries.begin(); it != asbrEntries.end(); it++) {
        OSPF::RoutingTableEntry* routingEntry = *it;
        if ((routingEntry->getPathType() == OSPF::RoutingTableEntry::INTRAAREA) &&
            (routingEntry->getArea() != OSPF::BACKBONE_AREAID))
        {
            hasNonBackboneIntraAreaPath = true;
            break;
        }
    }

    if (hasNonBackboneIntraAreaPath) {
        std::vector<OSPF::RoutingTableEntry*>::iterator it = asbrEntries.begin();
        while (it != asbrEntries.end()) {
            if (((*it)->getPathType() != OSPF::RoutingTableEntry::INTRAAREA) ||
                ((*it)->getArea() == OSPF::BACKBONE_AREAID))
            {
                it = asbrEntries.erase(it);
            } else {
                it++;
            }
        }
    }
}


OSPF::RoutingTableEntry* OSPF::Router::selectLeastCostRoutingEntry(std::vector<OSPF::RoutingTableEntry*>& entries) const
{
    if (entries.empty()) {
        return NULL;
    }

    OSPF::RoutingTableEntry* leastCostEntry = entries[0];
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


OSPF::RoutingTableEntry* OSPF::Router::getPreferredEntry(const OSPFLSA& lsa, bool skipSelfOriginated, std::vector<OSPF::RoutingTableEntry*>* fromRoutingTable /*= NULL*/)
{
    // see RFC 2328 16.3. and 16.4.
    if (fromRoutingTable == NULL) {
        fromRoutingTable = &routingTable;
    }

    const OSPFLSAHeader& lsaHeader = lsa.getHeader();
    const OSPFASExternalLSA* asExternalLSA = dynamic_cast<const OSPFASExternalLSA*> (&lsa);
    unsigned long externalCost = (asExternalLSA != NULL) ? asExternalLSA->getContents().getRouteCost() : 0;
    unsigned short lsAge = lsaHeader.getLsAge();
    OSPF::RouterID originatingRouter = lsaHeader.getAdvertisingRouter();
    bool selfOriginated = (originatingRouter == routerID);
    IPv4Address forwardingAddress; // 0.0.0.0

    if (asExternalLSA != NULL) {
        forwardingAddress = asExternalLSA->getContents().getForwardingAddress();
    }

    if ((externalCost == LS_INFINITY) || (lsAge == MAX_AGE) || (skipSelfOriginated && selfOriginated)) { // (1) and(2)
        return NULL;
    }

    if (!hasRouteToASBoundaryRouter(*fromRoutingTable, originatingRouter)) { // (3)
        return NULL;
    }

    if (forwardingAddress.isUnspecified()) {   // (3)
        std::vector<OSPF::RoutingTableEntry*> asbrEntries = getRoutesToASBoundaryRouter(*fromRoutingTable, originatingRouter);
        if (!rfc1583Compatibility) {
            pruneASBoundaryRouterEntries(asbrEntries);
        }
        return selectLeastCostRoutingEntry(asbrEntries);
    } else {
        OSPF::RoutingTableEntry* forwardEntry = lookup(forwardingAddress, fromRoutingTable);

        if (forwardEntry == NULL) {
            return NULL;
        }

        if ((forwardEntry->getPathType() != OSPF::RoutingTableEntry::INTRAAREA) &&
            (forwardEntry->getPathType() != OSPF::RoutingTableEntry::INTERAREA))
        {
            return NULL;
        }

        return forwardEntry;
    }

    return NULL;
}

void OSPF::Router::calculateASExternalRoutes(std::vector<OSPF::RoutingTableEntry*>& newRoutingTable)
{
    // see RFC 2328 16.4.
    unsigned long lsaCount = asExternalLSAs.size();
    unsigned long i;

    for (i = 0; i < lsaCount; i++) {
        OSPF::ASExternalLSA* currentLSA = asExternalLSAs[i];
        OSPFLSAHeader& currentHeader = currentLSA->getHeader();
        unsigned short externalCost = currentLSA->getContents().getRouteCost();
        OSPF::RouterID originatingRouter = currentHeader.getAdvertisingRouter();

        OSPF::RoutingTableEntry* preferredEntry = getPreferredEntry(*currentLSA, true, &newRoutingTable);
        if (preferredEntry == NULL) {
            continue;
        }

        IPv4Address destination = currentHeader.getLinkStateID() & currentLSA->getContents().getNetworkMask();

        Metric preferredCost = preferredEntry->getCost();
        OSPF::RoutingTableEntry* destinationEntry = lookup(destination, &newRoutingTable);   // (5)
        if (destinationEntry == NULL) {
            bool type2ExternalMetric = currentLSA->getContents().getE_ExternalMetricType();
            unsigned int nextHopCount = preferredEntry->getNextHopCount();
            OSPF::RoutingTableEntry* newEntry = new OSPF::RoutingTableEntry;

            newEntry->setDestination(destination);
            newEntry->setNetmask(currentLSA->getContents().getNetworkMask());
            newEntry->setArea(preferredEntry->getArea());
            newEntry->setPathType(type2ExternalMetric ? OSPF::RoutingTableEntry::TYPE2_EXTERNAL : OSPF::RoutingTableEntry::TYPE1_EXTERNAL);
            if (type2ExternalMetric) {
                newEntry->setCost(preferredCost);
                newEntry->setType2Cost(externalCost);
            } else {
                newEntry->setCost(preferredCost + externalCost);
            }
            newEntry->setDestinationType(OSPF::RoutingTableEntry::NETWORK_DESTINATION);
            newEntry->setOptionalCapabilities(currentHeader.getLsOptions());
            newEntry->setLinkStateOrigin(currentLSA);

            for (unsigned int j = 0; j < nextHopCount; j++) {
                NextHop nextHop = preferredEntry->getNextHop(j);

                nextHop.advertisingRouter = originatingRouter;
                newEntry->addNextHop(nextHop);
            }

            newRoutingTable.push_back(newEntry);
        } else {
            OSPF::RoutingTableEntry::RoutingPathType destinationPathType = destinationEntry->getPathType();
            bool type2ExternalMetric = currentLSA->getContents().getE_ExternalMetricType();
            unsigned int nextHopCount = preferredEntry->getNextHopCount();

            if ((destinationPathType == OSPF::RoutingTableEntry::INTRAAREA) ||
                (destinationPathType == OSPF::RoutingTableEntry::INTERAREA))   // (6) (a)
            {
                continue;
            }

            if (((destinationPathType == OSPF::RoutingTableEntry::TYPE1_EXTERNAL) &&
                 (type2ExternalMetric)) ||
                ((destinationPathType == OSPF::RoutingTableEntry::TYPE2_EXTERNAL) &&
                 (type2ExternalMetric) &&
                 (destinationEntry->getType2Cost() < externalCost))) // (6) (b)
            {
                continue;
            }

            OSPF::RoutingTableEntry* destinationPreferredEntry = getPreferredEntry(*(destinationEntry->getLinkStateOrigin()), false, &newRoutingTable);
            if ((!rfc1583Compatibility) &&
                (destinationPreferredEntry->getPathType() == OSPF::RoutingTableEntry::INTRAAREA) &&
                (destinationPreferredEntry->getArea() != OSPF::BACKBONE_AREAID) &&
                ((preferredEntry->getPathType() != OSPF::RoutingTableEntry::INTRAAREA) ||
                 (preferredEntry->getArea() == OSPF::BACKBONE_AREAID)))
            {
                continue;
            }

            if ((((destinationPathType == OSPF::RoutingTableEntry::TYPE1_EXTERNAL) &&
                  (!type2ExternalMetric) &&
                  (destinationEntry->getCost() < preferredCost + externalCost))) ||
                ((destinationPathType == OSPF::RoutingTableEntry::TYPE2_EXTERNAL) &&
                 (type2ExternalMetric) &&
                 (destinationEntry->getType2Cost() == externalCost) &&
                 (destinationPreferredEntry->getCost() < preferredCost)))
            {
                continue;
            }

            if (((destinationPathType == OSPF::RoutingTableEntry::TYPE1_EXTERNAL) &&
                 (!type2ExternalMetric) &&
                 (destinationEntry->getCost() == (preferredCost + externalCost))) ||
                ((destinationPathType == OSPF::RoutingTableEntry::TYPE2_EXTERNAL) &&
                 (type2ExternalMetric) &&
                 (destinationEntry->getType2Cost() == externalCost) &&
                 (destinationPreferredEntry->getCost() == preferredCost)))   // equal cost
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
            destinationEntry->setPathType(type2ExternalMetric ? OSPF::RoutingTableEntry::TYPE2_EXTERNAL : OSPF::RoutingTableEntry::TYPE1_EXTERNAL);
            if (type2ExternalMetric) {
                destinationEntry->setCost(preferredCost);
                destinationEntry->setType2Cost(externalCost);
            } else {
                destinationEntry->setCost(preferredCost + externalCost);
            }
            destinationEntry->setDestinationType(OSPF::RoutingTableEntry::NETWORK_DESTINATION);
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


OSPF::IPv4AddressRange OSPF::Router::getContainingAddressRange(const OSPF::IPv4AddressRange& addressRange, bool* advertise /*= NULL*/) const
{
    unsigned long areaCount = areas.size();
    for (unsigned long i = 0; i < areaCount; i++) {
        OSPF::IPv4AddressRange containingAddressRange = areas[i]->getContainingAddressRange(addressRange, advertise);
        if (containingAddressRange != OSPF::NULL_IPV4ADDRESSRANGE) {
            return containingAddressRange;
        }
    }
    if (advertise != NULL) {
        *advertise = false;
    }
    return OSPF::NULL_IPV4ADDRESSRANGE;
}


OSPF::LinkStateID OSPF::Router::getUniqueLinkStateID(const OSPF::IPv4AddressRange& destination,
                                                      OSPF::Metric destinationCost,
                                                      OSPF::ASExternalLSA*& lsaToReoriginate,
                                                      bool externalMetricIsType2 /*= false*/) const
{
    if (lsaToReoriginate != NULL) {
        delete lsaToReoriginate;
        lsaToReoriginate = NULL;
    }

    OSPF::LSAKeyType lsaKey;

    lsaKey.linkStateID = destination.address;
    lsaKey.advertisingRouter = routerID;

    const OSPF::ASExternalLSA* foundLSA = findASExternalLSA(lsaKey);

    if (foundLSA == NULL) {
        return lsaKey.linkStateID;
    } else {
        IPv4Address existingMask = foundLSA->getContents().getNetworkMask();

        if (destination.mask >= existingMask) {
            return lsaKey.linkStateID.getBroadcastAddress(destination.mask);
        } else {
            OSPF::ASExternalLSA* asExternalLSA = new OSPF::ASExternalLSA(*foundLSA);

            long sequenceNumber = asExternalLSA->getHeader().getLsSequenceNumber();

            asExternalLSA->getHeader().setLsAge(0);
            asExternalLSA->getHeader().setLsSequenceNumber((sequenceNumber == MAX_SEQUENCE_NUMBER) ? INITIAL_SEQUENCE_NUMBER : sequenceNumber + 1);
            asExternalLSA->getContents().setNetworkMask(destination.mask);
            asExternalLSA->getContents().setE_ExternalMetricType(externalMetricIsType2);
            asExternalLSA->getContents().setRouteCost(destinationCost);

            lsaToReoriginate = asExternalLSA;

            return lsaKey.linkStateID.getBroadcastAddress(existingMask);
        }
    }
}


// TODO: review this algorithm + add virtual link changes(RFC2328 Section 16.7.).
void OSPF::Router::notifyAboutRoutingTableChanges(std::vector<OSPF::RoutingTableEntry*>& oldRoutingTable)
{
    if (areas.size() <= 1) {
        return;
    }

    typedef std::map<IPv4AddressRange, RoutingTableEntry*> RoutingTableEntryMap;
    unsigned long routeCount = oldRoutingTable.size();
    RoutingTableEntryMap oldTableMap;
    RoutingTableEntryMap newTableMap;
    unsigned long                                 i, j, k;

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
        std::map<OSPF::LSAKeyType, bool, OSPF::LSAKeyType_Less> originatedLSAMap;
        std::map<OSPF::LSAKeyType, bool, OSPF::LSAKeyType_Less> deletedLSAMap;
        OSPF::LSAKeyType lsaKey;

        routeCount = routingTable.size();
        for (j = 0; j < routeCount; j++) {
            IPv4AddressRange destination(routingTable[j]->getDestination() & routingTable[j]->getNetmask(), routingTable[j]->getNetmask());
            RoutingTableEntryMap::iterator destIt = oldTableMap.find(destination);
            if (destIt == oldTableMap.end()) { // new routing entry
                OSPF::SummaryLSA* lsaToReoriginate = NULL;
                OSPF::SummaryLSA* newLSA = areas[i]->originateSummaryLSA(routingTable[j], originatedLSAMap, lsaToReoriginate);

                if (newLSA != NULL) {
                    if (lsaToReoriginate != NULL) {
                        areas[i]->installSummaryLSA(lsaToReoriginate);
//                        floodLSA(lsaToReoriginate, OSPF::BACKBONE_AREAID);
                        floodLSA(lsaToReoriginate, areas[i]->getAreaID());

                        lsaKey.linkStateID = lsaToReoriginate->getHeader().getLinkStateID();
                        lsaKey.advertisingRouter = routerID;
                        originatedLSAMap[lsaKey] = true;

                        delete lsaToReoriginate;
                    }

                    areas[i]->installSummaryLSA(newLSA);
//                    floodLSA(newLSA, OSPF::BACKBONE_AREAID);
                    floodLSA(newLSA, areas[i]->getAreaID());

                    lsaKey.linkStateID = newLSA->getHeader().getLinkStateID();
                    lsaKey.advertisingRouter = routerID;
                    originatedLSAMap[lsaKey] = true;

                    delete newLSA;
                }
            } else {
                if (*(routingTable[j]) != *(destIt->second)) {  // modified routing entry
                    OSPF::SummaryLSA* lsaToReoriginate = NULL;
                    OSPF::SummaryLSA* newLSA = areas[i]->originateSummaryLSA(routingTable[j], originatedLSAMap, lsaToReoriginate);

                    if (newLSA != NULL) {
                        if (lsaToReoriginate != NULL) {
                            areas[i]->installSummaryLSA(lsaToReoriginate);
//                            floodLSA(lsaToReoriginate, OSPF::BACKBONE_AREAID);
                            floodLSA(lsaToReoriginate, areas[i]->getAreaID());

                            lsaKey.linkStateID = lsaToReoriginate->getHeader().getLinkStateID();
                            lsaKey.advertisingRouter = routerID;
                            originatedLSAMap[lsaKey] = true;

                            delete lsaToReoriginate;
                        }

                        areas[i]->installSummaryLSA(newLSA);
//                        floodLSA(newLSA, OSPF::BACKBONE_AREAID);
                        floodLSA(newLSA, areas[i]->getAreaID());

                        lsaKey.linkStateID = newLSA->getHeader().getLinkStateID();
                        lsaKey.advertisingRouter = routerID;
                        originatedLSAMap[lsaKey] = true;

                        delete newLSA;
                    } else {
                        OSPF::IPv4AddressRange destinationAddressRange(routingTable[j]->getDestination(), routingTable[j]->getNetmask());

                        if ((routingTable[j]->getDestinationType() == OSPF::RoutingTableEntry::NETWORK_DESTINATION) &&
                            ((routingTable[j]->getPathType() == OSPF::RoutingTableEntry::INTRAAREA) ||
                             (routingTable[j]->getPathType() == OSPF::RoutingTableEntry::INTERAREA)))
                        {
                            OSPF::IPv4AddressRange containingAddressRange = getContainingAddressRange(destinationAddressRange);
                            if (containingAddressRange != OSPF::NULL_IPV4ADDRESSRANGE) {
                                destinationAddressRange = containingAddressRange;
                            }
                        }

                        Metric maxRangeCost = 0;
                        Metric oneLessCost = 0;

                        for (k = 0; k < routeCount; k++) {
                            if ((routingTable[k]->getDestinationType() == OSPF::RoutingTableEntry::NETWORK_DESTINATION) &&
                                (routingTable[k]->getPathType() == OSPF::RoutingTableEntry::INTRAAREA) &&
                                ((routingTable[k]->getDestination().getInt() & routingTable[k]->getNetmask().getInt() & destinationAddressRange.mask.getInt()) ==
                                 (destinationAddressRange.address & destinationAddressRange.mask).getInt()) &&
                                (routingTable[k]->getCost() > maxRangeCost))
                            {
                                oneLessCost = maxRangeCost;
                                maxRangeCost = routingTable[k]->getCost();
                            }
                        }

                        if (maxRangeCost == routingTable[j]->getCost()) {  // this entry gives the range's cost
                            lsaKey.linkStateID = destinationAddressRange.address;
                            lsaKey.advertisingRouter = routerID;

                            OSPF::SummaryLSA* summaryLSA = areas[i]->findSummaryLSA(lsaKey);

                            if (summaryLSA != NULL) {
                                if (oneLessCost != 0) { // there's an other entry in this range
                                    summaryLSA->setRouteCost(oneLessCost);
//                                    floodLSA(summaryLSA, OSPF::BACKBONE_AREAID);
                                    floodLSA(summaryLSA, areas[i]->getAreaID());

                                    originatedLSAMap[lsaKey] = true;
                                } else {    // no more entries in this range -> delete it
                                    std::map<OSPF::LSAKeyType, bool, OSPF::LSAKeyType_Less>::const_iterator deletedIt = deletedLSAMap.find(lsaKey);
                                    if (deletedIt == deletedLSAMap.end()) {
                                        summaryLSA->getHeader().setLsAge(MAX_AGE);
//                                        floodLSA(summaryLSA, OSPF::BACKBONE_AREAID);
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
            RoutingTableEntryMap::iterator destIt = newTableMap.find(destination);
            if (destIt == newTableMap.end()) { // deleted routing entry
                OSPF::IPv4AddressRange destinationAddressRange(oldRoutingTable[j]->getDestination(), oldRoutingTable[j]->getNetmask());

                if ((oldRoutingTable[j]->getDestinationType() == OSPF::RoutingTableEntry::NETWORK_DESTINATION) &&
                    ((oldRoutingTable[j]->getPathType() == OSPF::RoutingTableEntry::INTRAAREA) ||
                     (oldRoutingTable[j]->getPathType() == OSPF::RoutingTableEntry::INTERAREA)))
                {
                    OSPF::IPv4AddressRange containingAddressRange = getContainingAddressRange(destinationAddressRange);
                    if (containingAddressRange != OSPF::NULL_IPV4ADDRESSRANGE) {
                        destinationAddressRange = containingAddressRange;
                    }
                }

                Metric maxRangeCost = 0;

                unsigned long newRouteCount = routingTable.size();
                for (k = 0; k < newRouteCount; k++) {
                    if ((routingTable[k]->getDestinationType() == OSPF::RoutingTableEntry::NETWORK_DESTINATION) &&
                        (routingTable[k]->getPathType() == OSPF::RoutingTableEntry::INTRAAREA) &&
                        ((routingTable[k]->getDestination().getInt() & routingTable[k]->getNetmask().getInt() & destinationAddressRange.mask.getInt()) ==
                         (destinationAddressRange.address & destinationAddressRange.mask).getInt()) &&     //FIXME correcting network comparison
                        (routingTable[k]->getCost() > maxRangeCost))
                    {
                        maxRangeCost = routingTable[k]->getCost();
                    }
                }

                if (maxRangeCost < oldRoutingTable[j]->getCost()) {  // the range's cost will change
                    lsaKey.linkStateID = destinationAddressRange.address;
                    lsaKey.advertisingRouter = routerID;

                    OSPF::SummaryLSA* summaryLSA = areas[i]->findSummaryLSA(lsaKey);

                    if (summaryLSA != NULL) {
                        if (maxRangeCost > 0) { // there's an other entry in this range
                            summaryLSA->setRouteCost(maxRangeCost);
                            floodLSA(summaryLSA, OSPF::BACKBONE_AREAID);

                            originatedLSAMap[lsaKey] = true;
                        } else {    // no more entries in this range -> delete it
                            std::map<OSPF::LSAKeyType, bool, OSPF::LSAKeyType_Less>::const_iterator deletedIt = deletedLSAMap.find(lsaKey);
                            if (deletedIt == deletedLSAMap.end()) {
                                summaryLSA->getHeader().setLsAge(MAX_AGE);
                                floodLSA(summaryLSA, OSPF::BACKBONE_AREAID);

                                deletedLSAMap[lsaKey] = true;
                            }
                        }
                    }
                }
            }
        }
    }
}


void OSPF::Router::updateExternalRoute(IPv4Address networkAddress, const OSPFASExternalLSAContents& externalRouteContents, int ifIndex)
{
    OSPF::ASExternalLSA* asExternalLSA = new OSPF::ASExternalLSA;
    OSPFLSAHeader& lsaHeader = asExternalLSA->getHeader();
    OSPFOptions lsaOptions;
    //OSPF::LSAKeyType lsaKey;

    IRoutingTable* simRoutingTable = RoutingTableAccess().get();
    unsigned long routingEntryNumber = simRoutingTable->getNumRoutes();
    bool inRoutingTable = false;
    // add the external route to the routing table if it was not added by another module
    for (unsigned long i = 0; i < routingEntryNumber; i++)
    {
        const IPv4Route *entry = simRoutingTable->getRoute(i);
        if ((entry->getDestination() == networkAddress)
                && (entry->getNetmask() == externalRouteContents.getNetworkMask())) //TODO is it enough?
        {
            inRoutingTable = true;
        }
    }

    if (!inRoutingTable)
    {
        IPv4Route* entry = new IPv4Route;
        entry->setDestination(networkAddress);
        entry->setNetmask(externalRouteContents.getNetworkMask());
        entry->setInterface(InterfaceTableAccess().get()->getInterfaceById(ifIndex));
        entry->setSource(IPv4Route::MANUAL);
        entry->setMetric(externalRouteContents.getRouteCost());
        simRoutingTable->addRoute(entry);   // IRoutingTable deletes entry pointer
    }

    lsaHeader.setLsAge(0);
    memset(&lsaOptions, 0, sizeof(OSPFOptions));
    lsaOptions.E_ExternalRoutingCapability = true;
    lsaHeader.setLsOptions(lsaOptions);
    lsaHeader.setLsType(AS_EXTERNAL_LSA_TYPE);
    lsaHeader.setLinkStateID(networkAddress);   // TODO: get unique LinkStateID
    lsaHeader.setAdvertisingRouter(IPv4Address(routerID));
    lsaHeader.setLsSequenceNumber(INITIAL_SEQUENCE_NUMBER);

    asExternalLSA->setContents(externalRouteContents);

    asExternalLSA->setSource(OSPF::LSATrackingInfo::ORIGINATED);

    externalRoutes[networkAddress] = externalRouteContents;

    bool rebuild = installASExternalLSA(asExternalLSA);
    floodLSA(asExternalLSA, OSPF::BACKBONE_AREAID);
    delete asExternalLSA;

    if (rebuild) {
        rebuildRoutingTable();
    }
}


void OSPF::Router::addExternalRouteInIPTable(IPv4Address networkAddress, const OSPFASExternalLSAContents& externalRouteContents, int ifIndex)
{
    IRoutingTable* simRoutingTable = RoutingTableAccess().get();
    IInterfaceTable* simInterfaceTable = InterfaceTableAccess().get();
    int routingEntryNumber = simRoutingTable->getNumRoutes();
    bool inRoutingTable = false;

    // add the external route to the IPv4 routing table if it was not added by another module
    for (int i = 1; i < routingEntryNumber; i++) {
        const IPv4Route *entry = simRoutingTable->getRoute(i);
        if ((entry->getDestination() == networkAddress)
                && (entry->getNetmask() == externalRouteContents.getNetworkMask())) //TODO is it enough?
        {
            inRoutingTable = true;
            break;
        }
    }

    if (!inRoutingTable)
    {
        IPv4Route* entry = new IPv4Route();
        entry->setDestination(networkAddress);
        entry->setNetmask(externalRouteContents.getNetworkMask());
        entry->setInterface(simInterfaceTable->getInterfaceById(ifIndex));
        entry->setSource(IPv4Route::OSPF);
        entry->setMetric(OSPF_BGP_DEFAULT_COST);
        simRoutingTable->addRoute(entry);
    }
}

void OSPF::Router::removeExternalRoute(IPv4Address networkAddress)
{
    OSPF::LSAKeyType lsaKey;

    lsaKey.linkStateID = networkAddress;
    lsaKey.advertisingRouter = routerID;

    std::map<OSPF::LSAKeyType, OSPF::ASExternalLSA*, OSPF::LSAKeyType_Less>::iterator lsaIt = asExternalLSAsByID.find(lsaKey);
    if (lsaIt != asExternalLSAsByID.end()) {
        lsaIt->second->getHeader().setLsAge(MAX_AGE);
        lsaIt->second->setPurgeable();
        floodLSA(lsaIt->second, OSPF::BACKBONE_AREAID);
    }

    std::map<IPv4Address, OSPFASExternalLSAContents>::iterator externalIt = externalRoutes.find(networkAddress);
    if (externalIt != externalRoutes.end()) {
        externalRoutes.erase(externalIt);
    }
}
