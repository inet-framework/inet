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

/**
 * Constructor.
 * Initializes internal variables, adds a MessageHandler and starts the Database Age timer.
 */
OSPF::Router::Router(OSPF::RouterID id, cSimpleModule* containingModule) :
    routerID(id),
    rfc1583Compatibility(false)
{
    messageHandler = new OSPF::MessageHandler(this, containingModule);
    ageTimer = new OSPFTimer;
    ageTimer->setTimerKind(DatabaseAgeTimer);
    ageTimer->setContextPointer(this);
    ageTimer->setName("OSPF::Router::DatabaseAgeTimer");
    messageHandler->StartTimer(ageTimer, 1.0);
}


/**
 * Destructor.
 * Clears all LSA lists and kills the Database Age timer.
 */
OSPF::Router::~Router(void)
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
    messageHandler->ClearTimer(ageTimer);
    delete ageTimer;
    delete messageHandler;
}


/**
 * Adds OMNeT++ watches for the routerID, the list of Areas and the list of AS External LSAs.
 */
void OSPF::Router::AddWatches(void)
{
    WATCH(routerID);
    WATCH_PTRVECTOR(areas);
    WATCH_PTRVECTOR(asExternalLSAs);
}


/**
 * Adds a new Area to the Area list.
 * @param area [in] The Area to add.
 */
void OSPF::Router::AddArea(OSPF::Area* area)
{

    area->SetRouter(this);
    areasByID[area->GetAreaID()] = area;
    areas.push_back(area);
}


/**
 * Returns the pointer to the Area identified by the input areaID, if it's on the Area list,
 * NULL otherwise.
 * @param areaID [in] The Area identifier.
 */
OSPF::Area* OSPF::Router::GetArea(OSPF::AreaID areaID)
{
    std::map<OSPF::AreaID, OSPF::Area*>::iterator areaIt = areasByID.find(areaID);
    if (areaIt != areasByID.end()) {
        return (areaIt->second);
    }
    else {
        return NULL;
    }
}


/**
 * Returns the Area pointer from the Area list which contains the input IP address,
 * NULL if there's no such area connected to the Router.
 * @param address [in] The IP address whose containing Area we're looking for.
 */
OSPF::Area* OSPF::Router::GetArea(OSPF::IPv4Address address)
{
    long areaCount = areas.size();
    for (long i = 0; i < areaCount; i++) {
        if (areas[i]->ContainsAddress(address)) {
            return areas[i];
        }
    }
    return NULL;
}


/**
 * Returns the pointer of the physical Interface identified by the input interface index,
 * NULL if the Router doesn't have such an interface.
 * @param ifIndex [in] The interface index to look for.
 */
OSPF::Interface* OSPF::Router::GetNonVirtualInterface(unsigned char ifIndex)
{
    long areaCount = areas.size();
    for (long i = 0; i < areaCount; i++) {
        OSPF::Interface* intf = areas[i]->GetInterface(ifIndex);
        if (intf != NULL) {
            return intf;
        }
    }
    return NULL;
}


/**
 * Installs a new LSA into the Router database.
 * Checks the input LSA's type and installs it into either the selected Area's database,
 * or if it's an AS External LSA then into the Router's common asExternalLSAs list.
 * @param lsa    [in] The LSA to install. It will be copied into the database.
 * @param areaID [in] Identifies the input Router, Network and Summary LSA's Area.
 * @return True if the routing table needs to be updated, false otherwise.
 */
bool OSPF::Router::InstallLSA(OSPFLSA* lsa, OSPF::AreaID areaID /*= BackboneAreaID*/)
{
    switch (lsa->getHeader().getLsType()) {
        case RouterLSAType:
            {
                std::map<OSPF::AreaID, OSPF::Area*>::iterator areaIt = areasByID.find(areaID);
                if (areaIt != areasByID.end()) {
                    OSPFRouterLSA* ospfRouterLSA = check_and_cast<OSPFRouterLSA*> (lsa);
                    return areaIt->second->InstallRouterLSA(ospfRouterLSA);
                }
            }
            break;
        case NetworkLSAType:
            {
                std::map<OSPF::AreaID, OSPF::Area*>::iterator areaIt = areasByID.find(areaID);
                if (areaIt != areasByID.end()) {
                    OSPFNetworkLSA* ospfNetworkLSA = check_and_cast<OSPFNetworkLSA*> (lsa);
                    return areaIt->second->InstallNetworkLSA(ospfNetworkLSA);
                }
            }
            break;
        case SummaryLSA_NetworksType:
        case SummaryLSA_ASBoundaryRoutersType:
            {
                std::map<OSPF::AreaID, OSPF::Area*>::iterator areaIt = areasByID.find(areaID);
                if (areaIt != areasByID.end()) {
                    OSPFSummaryLSA* ospfSummaryLSA = check_and_cast<OSPFSummaryLSA*> (lsa);
                    return areaIt->second->InstallSummaryLSA(ospfSummaryLSA);
                }
            }
            break;
        case ASExternalLSAType:
            {
                OSPFASExternalLSA* ospfASExternalLSA = check_and_cast<OSPFASExternalLSA*> (lsa);
                return InstallASExternalLSA(ospfASExternalLSA);
            }
            break;
        default:
            ASSERT(false);
            break;
    }
    return false;
}


/**
 * Installs a new AS External LSA into the Router's database.
 * It tries to install keep one of multiple functionally equivalent AS External LSAs in the database.
 * (See the comment in the method implementation.)
 * @param lsa [in] The LSA to install. It will be copied into the database.
 * @return True if the routing table needs to be updated, false otherwise.
 */
bool OSPF::Router::InstallASExternalLSA(OSPFASExternalLSA* lsa)
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
     *    equivalent AS-External-LSA, so we cannot use the IP packet source address.
     * 2. The AS-External-LSA contains only the Router ID of the advertising router, so we
     *    can only look up "router" type routing entries in the routing table(these contain
     *    the Router ID as their Destination ID). However these entries are only inserted into
     *    the routing table for intra-area routers...
     */
     // TODO: how to solve this problem?

    OSPF::RouterID advertisingRouter = lsa->getHeader().getAdvertisingRouter().getInt();
    bool           reachable         = false;
    unsigned int   routeCount        = routingTable.size();

    for (unsigned int i = 0; i < routeCount; i++) {
        if ((((routingTable[i]->GetDestinationType() & OSPF::RoutingTableEntry::AreaBorderRouterDestination) != 0) ||
             ((routingTable[i]->GetDestinationType() & OSPF::RoutingTableEntry::ASBoundaryRouterDestination) != 0)) &&
            (routingTable[i]->GetDestinationID().getInt() == advertisingRouter))
        {
            reachable = true;
            break;
        }
    }

    bool             ownLSAFloodedOut = false;
    OSPF::LSAKeyType lsaKey;

    lsaKey.linkStateID = lsa->getHeader().getLinkStateID();
    lsaKey.advertisingRouter = routerID;

    std::map<OSPF::LSAKeyType, OSPF::ASExternalLSA*, OSPF::LSAKeyType_Less>::iterator lsaIt = asExternalLSAsByID.find(lsaKey);
    if ((lsaIt != asExternalLSAsByID.end()) &&
        reachable &&
        (lsaIt->second->getContents().getE_ExternalMetricType() == lsa->getContents().getE_ExternalMetricType()) &&
        (lsaIt->second->getContents().getRouteCost() == lsa->getContents().getRouteCost()) &&
        (lsa->getContents().getForwardingAddress().getInt() != 0) &&   // forwarding address != 0.0.0.0
        (lsaIt->second->getContents().getForwardingAddress() == lsa->getContents().getForwardingAddress()))
    {
        if (routerID > advertisingRouter) {
            return false;
        } else {
            lsaIt->second->getHeader().setLsAge(MAX_AGE);
            FloodLSA(lsaIt->second, OSPF::BackboneAreaID);
            lsaIt->second->IncrementInstallTime();
            ownLSAFloodedOut = true;
        }
    }

    lsaKey.advertisingRouter = advertisingRouter;

    lsaIt = asExternalLSAsByID.find(lsaKey);
    if (lsaIt != asExternalLSAsByID.end()) {
        unsigned long areaCount = areas.size();
        for (unsigned long i = 0; i < areaCount; i++) {
            areas[i]->RemoveFromAllRetransmissionLists(lsaKey);
        }
        return ((lsaIt->second->Update(lsa)) | ownLSAFloodedOut);
    } else {
        OSPF::ASExternalLSA* lsaCopy = new OSPF::ASExternalLSA(*lsa);
        asExternalLSAsByID[lsaKey] = lsaCopy;
        asExternalLSAs.push_back(lsaCopy);
        return true;
    }
}


/**
 * Find the LSA identified by the input lsaKey in the database.
 * @param lsaType [in] Look for an LSA of this type.
 * @param lsaKey  [in] Look for the LSA which is identified by this key.
 * @param areaID  [in] In case of Router, Network and Summary LSAs, look in the Area's database
 *                     identified by this parameter.
 * @return The pointer to the LSA if it was found, NULL otherwise.
 */
OSPFLSA* OSPF::Router::FindLSA(LSAType lsaType, OSPF::LSAKeyType lsaKey, OSPF::AreaID areaID)
{
    switch (lsaType) {
        case RouterLSAType:
            {
                std::map<OSPF::AreaID, OSPF::Area*>::iterator areaIt = areasByID.find(areaID);
                if (areaIt != areasByID.end()) {
                    return areaIt->second->FindRouterLSA(lsaKey.linkStateID);
                }
            }
            break;
        case NetworkLSAType:
            {
                std::map<OSPF::AreaID, OSPF::Area*>::iterator areaIt = areasByID.find(areaID);
                if (areaIt != areasByID.end()) {
                    return areaIt->second->FindNetworkLSA(lsaKey.linkStateID);
                }
            }
            break;
        case SummaryLSA_NetworksType:
        case SummaryLSA_ASBoundaryRoutersType:
            {
                std::map<OSPF::AreaID, OSPF::Area*>::iterator areaIt = areasByID.find(areaID);
                if (areaIt != areasByID.end()) {
                    return areaIt->second->FindSummaryLSA(lsaKey);
                }
            }
            break;
        case ASExternalLSAType:
            {
                return FindASExternalLSA(lsaKey);
            }
            break;
        default:
            ASSERT(false);
            break;
    }
    return NULL;
}


/**
 * Find the AS External LSA identified by the input lsaKey in the database.
 * @param lsaKey [in] Look for the AS External LSA which is identified by this key.
 * @return The pointer to the AS External LSA if it was found, NULL otherwise.
 */
OSPF::ASExternalLSA* OSPF::Router::FindASExternalLSA(OSPF::LSAKeyType lsaKey)
{
    std::map<OSPF::LSAKeyType, OSPF::ASExternalLSA*, OSPF::LSAKeyType_Less>::iterator lsaIt = asExternalLSAsByID.find(lsaKey);
    if (lsaIt != asExternalLSAsByID.end()) {
        return lsaIt->second;
    } else {
        return NULL;
    }
}


/**
 * Find the AS External LSA identified by the input lsaKey in the database.
 * @param lsaKey [in] Look for the AS External LSA which is identified by this key.
 * @return The const pointer to the AS External LSA if it was found, NULL otherwise.
 */
const OSPF::ASExternalLSA* OSPF::Router::FindASExternalLSA(OSPF::LSAKeyType lsaKey) const
{
    std::map<OSPF::LSAKeyType, OSPF::ASExternalLSA*, OSPF::LSAKeyType_Less>::const_iterator lsaIt = asExternalLSAsByID.find(lsaKey);
    if (lsaIt != asExternalLSAsByID.end()) {
        return lsaIt->second;
    } else {
        return NULL;
    }
}


/**
 * Ages the LSAs in the Router's database.
 * This method is called on every firing of the DatabaseAgeTimer(every second).
 * @sa RFC2328 Section 14.
 */
void OSPF::Router::AgeDatabase(void)
{
    long lsaCount            = asExternalLSAs.size();
    bool rebuildRoutingTable = false;

    for (long i = 0; i < lsaCount; i++) {
        unsigned short       lsAge          = asExternalLSAs[i]->getHeader().getLsAge();
        bool                 selfOriginated = (asExternalLSAs[i]->getHeader().getAdvertisingRouter().getInt() == routerID);
        bool                 unreachable    = IsDestinationUnreachable(asExternalLSAs[i]);
        OSPF::ASExternalLSA* lsa            = asExternalLSAs[i];

        if ((selfOriginated && (lsAge < (LS_REFRESH_TIME - 1))) || (!selfOriginated && (lsAge < (MAX_AGE - 1)))) {
            lsa->getHeader().setLsAge(lsAge + 1);
            if ((lsAge + 1) % CHECK_AGE == 0) {
                if (!lsa->ValidateLSChecksum()) {
                    EV << "Invalid LS checksum. Memory error detected!\n";
                }
            }
            lsa->IncrementInstallTime();
        }
        if (selfOriginated && (lsAge == (LS_REFRESH_TIME - 1))) {
            if (unreachable) {
                lsa->getHeader().setLsAge(MAX_AGE);
                FloodLSA(lsa, OSPF::BackboneAreaID);
                lsa->IncrementInstallTime();
            } else {
                long sequenceNumber = lsa->getHeader().getLsSequenceNumber();
                if (sequenceNumber == MAX_SEQUENCE_NUMBER) {
                    lsa->getHeader().setLsAge(MAX_AGE);
                    FloodLSA(lsa, OSPF::BackboneAreaID);
                    lsa->IncrementInstallTime();
                } else {
                    OSPF::ASExternalLSA* newLSA = OriginateASExternalLSA(lsa);

                    newLSA->getHeader().setLsSequenceNumber(sequenceNumber + 1);
                    newLSA->getHeader().setLsChecksum(0);    // TODO: calculate correct LS checksum
                    rebuildRoutingTable |= lsa->Update(newLSA);
                    delete newLSA;

                    FloodLSA(lsa, OSPF::BackboneAreaID);
                }
            }
        }
        if (!selfOriginated && (lsAge == MAX_AGE - 1)) {
            lsa->getHeader().setLsAge(MAX_AGE);
            FloodLSA(lsa, OSPF::BackboneAreaID);
            lsa->IncrementInstallTime();
        }
        if (lsAge == MAX_AGE) {
            OSPF::LSAKeyType lsaKey;

            lsaKey.linkStateID       = lsa->getHeader().getLinkStateID();
            lsaKey.advertisingRouter = lsa->getHeader().getAdvertisingRouter().getInt();

            if (!IsOnAnyRetransmissionList(lsaKey) &&
                !HasAnyNeighborInStates(OSPF::Neighbor::ExchangeState | OSPF::Neighbor::LoadingState))
            {
                if (!selfOriginated || unreachable) {
                    asExternalLSAsByID.erase(lsaKey);
                    delete lsa;
                    asExternalLSAs[i] = NULL;
                    rebuildRoutingTable = true;
                } else {
                    if (lsa->GetPurgeable()) {
                        asExternalLSAsByID.erase(lsaKey);
                        delete lsa;
                        asExternalLSAs[i] = NULL;
                        rebuildRoutingTable = true;
                    } else {
                        OSPF::ASExternalLSA* newLSA              = OriginateASExternalLSA(lsa);
                        long                 sequenceNumber      = lsa->getHeader().getLsSequenceNumber();

                        newLSA->getHeader().setLsSequenceNumber((sequenceNumber == MAX_SEQUENCE_NUMBER) ? INITIAL_SEQUENCE_NUMBER : sequenceNumber + 1);
                        newLSA->getHeader().setLsChecksum(0);    // TODO: calculate correct LS checksum
                        rebuildRoutingTable |= lsa->Update(newLSA);
                        delete newLSA;

                        FloodLSA(lsa, OSPF::BackboneAreaID);
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
        areas[j]->AgeDatabase();
    }
    messageHandler->StartTimer(ageTimer, 1.0);

    if (rebuildRoutingTable) {
        RebuildRoutingTable();
    }
}


/**
 * Returns true if any Neighbor on any Interface in any of the Router's Areas is
 * in any of the input states, false otherwise.
 * @param states [in] A bitfield combination of NeighborStateType values.
 */
bool OSPF::Router::HasAnyNeighborInStates(int states) const
{
    long areaCount = areas.size();
    for (long i = 0; i < areaCount; i++) {
        if (areas[i]->HasAnyNeighborInStates(states)) {
            return true;
        }
    }
    return false;
}


/**
 * Removes all LSAs from all Neighbor's retransmission lists which are identified by
 * the input lsaKey.
 * @param lsaKey [in] Identifies the LSAs to remove from the retransmission lists.
 */
void OSPF::Router::RemoveFromAllRetransmissionLists(OSPF::LSAKeyType lsaKey)
{
    long areaCount = areas.size();
    for (long i = 0; i < areaCount; i++) {
        areas[i]->RemoveFromAllRetransmissionLists(lsaKey);
    }
}


/**
 * Returns true if there's at least one LSA on any Neighbor's retransmission list
 * identified by the input lsaKey, false otherwise.
 * @param lsaKey [in] Identifies the LSAs to look for on the retransmission lists.
 */
bool OSPF::Router::IsOnAnyRetransmissionList(OSPF::LSAKeyType lsaKey) const
{
    long areaCount = areas.size();
    for (long i = 0; i < areaCount; i++) {
        if (areas[i]->IsOnAnyRetransmissionList(lsaKey)) {
            return true;
        }
    }
    return false;
}


/**
 * Floods out the input lsa on a set of Interfaces.
 * @sa RFC2328 Section 13.3.
 * @param lsa      [in] The LSA to be flooded out.
 * @param areaID   [in] If the lsa is a Router, Network or Summary LSA, then flood it only in this Area.
 * @param intf     [in] The Interface this LSA arrived on.
 * @param neighbor [in] The Nieghbor this LSA arrived from.
 * @return True if the LSA was floooded back out on the receiving Interface, false otherwise.
 */
bool OSPF::Router::FloodLSA(OSPFLSA* lsa, OSPF::AreaID areaID /*= BackboneAreaID*/, OSPF::Interface* intf /*= NULL*/, OSPF::Neighbor* neighbor /*= NULL*/)
{
    bool floodedBackOut = false;

    if (lsa != NULL) {
        if (lsa->getHeader().getLsType() == ASExternalLSAType) {
            long areaCount = areas.size();
            for (long i = 0; i < areaCount; i++) {
                if (areas[i]->GetExternalRoutingCapability()) {
                    if (areas[i]->FloodLSA(lsa, intf, neighbor)) {
                        floodedBackOut = true;
                    }
                }
            }
        } else {
            std::map<OSPF::AreaID, OSPF::Area*>::iterator areaIt = areasByID.find(areaID);
            if (areaIt != areasByID.end()) {
                floodedBackOut = areaIt->second->FloodLSA(lsa, intf, neighbor);
            }
        }
    }

    return floodedBackOut;
}


/**
 * Returns true if the input IP address falls into any of the Router's Areas' configured
 * IP address ranges, false otherwise.
 * @param address [in] The IP address to look for.
 */
bool OSPF::Router::IsLocalAddress(OSPF::IPv4Address address) const
{
    long areaCount = areas.size();
    for (long i = 0; i < areaCount; i++) {
        if (areas[i]->IsLocalAddress(address)) {
            return true;
        }
    }
    return false;
}


/**
 * Returns true if one of the Router's Areas the same IP address range configured as the
 * input IP address range, false otherwise.
 * @param addressRange [in] The IP address range to look for.
 */
bool OSPF::Router::HasAddressRange(OSPF::IPv4AddressRange addressRange) const
{
    long areaCount = areas.size();
    for (long i = 0; i < areaCount; i++) {
        if (areas[i]->HasAddressRange(addressRange)) {
            return true;
        }
    }
    return false;
}


/**
 * Originates a new AS External LSA based on the input lsa.
 * @param lsa [in] The LSA whose contents should be copied into the newly originated LSA.
 * @return The newly originated LSA.
 */
OSPF::ASExternalLSA* OSPF::Router::OriginateASExternalLSA(OSPF::ASExternalLSA* lsa)
{
    OSPF::ASExternalLSA* asExternalLSA = new OSPF::ASExternalLSA(*lsa);
    OSPFLSAHeader& lsaHeader = asExternalLSA->getHeader();
    OSPFOptions    lsaOptions;

    lsaHeader.setLsAge(0);
    memset(&lsaOptions, 0, sizeof(OSPFOptions));
    lsaOptions.E_ExternalRoutingCapability = true;
    lsaHeader.setLsOptions(lsaOptions);
    lsaHeader.setLsSequenceNumber(INITIAL_SEQUENCE_NUMBER);
    asExternalLSA->SetSource(OSPF::LSATrackingInfo::Originated);

    return asExternalLSA;
}


/**
 * Returns true if the destination described by the input lsa is in the routing table, false otherwise.
 * @param lsa [in] The LSA which describes the destination to look for.
 */
bool OSPF::Router::IsDestinationUnreachable(OSPFLSA* lsa) const
{
    IPAddress destination = lsa->getHeader().getLinkStateID();

    OSPFRouterLSA* routerLSA         = dynamic_cast<OSPFRouterLSA*> (lsa);
    OSPFNetworkLSA* networkLSA       = dynamic_cast<OSPFNetworkLSA*> (lsa);
    OSPFSummaryLSA* summaryLSA       = dynamic_cast<OSPFSummaryLSA*> (lsa);
    OSPFASExternalLSA* asExternalLSA = dynamic_cast<OSPFASExternalLSA*> (lsa);
    // TODO: verify
    if (routerLSA != NULL) {
        OSPF::RoutingInfo* routingInfo = check_and_cast<OSPF::RoutingInfo*> (routerLSA);
        if (routerLSA->getHeader().getLinkStateID() == routerID) { // this is spfTreeRoot
            return false;
        }

        // get the interface address pointing backwards on the shortest path tree
        unsigned int     linkCount   = routerLSA->getLinksArraySize();
        OSPF::RouterLSA* toRouterLSA = dynamic_cast<OSPF::RouterLSA*> (routingInfo->GetParent());
        if (toRouterLSA != NULL) {
            bool      destinationFound           = false;
            bool      unnumberedPointToPointLink = false;
            IPAddress firstNumberedIfAddress;

            for (unsigned int i = 0; i < linkCount; i++) {
                Link& link = routerLSA->getLinks(i);

                if (link.getType() == PointToPointLink) {
                    if (link.getLinkID() == toRouterLSA->getHeader().getLinkStateID()) {
                        if ((link.getLinkData() & 0xFF000000) == 0) {
                            unnumberedPointToPointLink = true;
                            if (!firstNumberedIfAddress.isUnspecified()) {
                                break;
                            }
                        } else {
                            destination = link.getLinkData();
                            destinationFound = true;
                            break;
                        }
                    } else {
                        if (((link.getLinkData() & 0xFF000000) != 0) &&
                             firstNumberedIfAddress.isUnspecified())
                        {
                            firstNumberedIfAddress = link.getLinkData();
                        }
                    }
                } else if (link.getType() == TransitLink) {
                    if (firstNumberedIfAddress.isUnspecified()) {
                        firstNumberedIfAddress = link.getLinkData();
                    }
                } else if (link.getType() == VirtualLink) {
                    if (link.getLinkID() == toRouterLSA->getHeader().getLinkStateID()) {
                        destination = link.getLinkData();
                        destinationFound = true;
                        break;
                    } else {
                        if (firstNumberedIfAddress.isUnspecified()) {
                            firstNumberedIfAddress = link.getLinkData();
                        }
                    }
                }
                // There's no way to get an interface address for the router from a StubLink
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
            OSPF::NetworkLSA* toNetworkLSA = dynamic_cast<OSPF::NetworkLSA*> (routingInfo->GetParent());
            if (toNetworkLSA != NULL) {
                // get the interface address pointing backwards on the shortest path tree
                bool destinationFound = false;
                for (unsigned int i = 0; i < linkCount; i++) {
                    Link& link = routerLSA->getLinks(i);

                    if ((link.getType() == TransitLink) &&
                        (link.getLinkID() == toNetworkLSA->getHeader().getLinkStateID()))
                    {
                        destination = link.getLinkData();
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
        destination = networkLSA->getHeader().getLinkStateID() & networkLSA->getNetworkMask().getInt();
    }
    if ((summaryLSA != NULL) && (summaryLSA->getHeader().getLsType() == SummaryLSA_NetworksType)) {
        destination = summaryLSA->getHeader().getLinkStateID() & summaryLSA->getNetworkMask().getInt();
    }
    if (asExternalLSA != NULL) {
        destination = asExternalLSA->getHeader().getLinkStateID() & asExternalLSA->getContents().getNetworkMask().getInt();
    }

    if (Lookup(destination) == NULL) {
        return true;
    } else {
        return false;
    }
}


/**
 * Do a lookup in either the input OSPF routing table, or if it's NULL then in the Router's own routing table.
 * @sa RFC2328 Section 11.1.
 * @param destination [in] The destination to look up in the routing table.
 * @param table       [in] The routing table to do the lookup in.
 * @return The RoutingTableEntry describing the input destination if there's one, false otherwise.
 */
OSPF::RoutingTableEntry* OSPF::Router::Lookup(IPAddress destination, std::vector<OSPF::RoutingTableEntry*>* table /*= NULL*/) const
{
    const std::vector<OSPF::RoutingTableEntry*>& rTable           = (table == NULL) ? routingTable : (*table);
    unsigned long                                dest             = destination.getInt();
    unsigned long                                routingTableSize = rTable.size();
    bool                                         unreachable      = false;
    std::vector<OSPF::RoutingTableEntry*>        discard;
    unsigned long                                i;

    unsigned long areaCount = areas.size();
    for (i = 0; i < areaCount; i++) {
        unsigned int addressRangeCount = areas[i]->GetAddressRangeCount();
        for (unsigned int j = 0; j < addressRangeCount; j++) {
            OSPF::IPv4AddressRange range = areas[i]->GetAddressRange(j);

            for (unsigned int k = 0; k < routingTableSize; k++) {
                OSPF::RoutingTableEntry* entry = rTable[k];

                if (entry->GetDestinationType() != OSPF::RoutingTableEntry::NetworkDestination) {
                    continue;
                }
                if (((entry->GetDestinationID().getInt() & entry->GetAddressMask().getInt() & ULongFromIPv4Address(range.mask)) == ULongFromIPv4Address(range.address & range.mask)) &&
                    (entry->GetPathType() == OSPF::RoutingTableEntry::IntraArea))
                {
                    // active area address range
                    OSPF::RoutingTableEntry* discardEntry = new OSPF::RoutingTableEntry;
                    discardEntry->SetDestinationID(ULongFromIPv4Address(range.address));
                    discardEntry->SetAddressMask(ULongFromIPv4Address(range.mask));
                    discardEntry->SetDestinationType(OSPF::RoutingTableEntry::NetworkDestination);
                    discardEntry->SetPathType(OSPF::RoutingTableEntry::InterArea);
                    discardEntry->SetArea(areas[i]->GetAreaID());
                    discard.push_back(discardEntry);
                    break;
                }
            }
        }
    }

    OSPF::RoutingTableEntry* bestMatch = NULL;
    unsigned long            longestMatch = 0;

    for (i = 0; i < routingTableSize; i++) {
        if (rTable[i]->GetDestinationType() == OSPF::RoutingTableEntry::NetworkDestination) {
            OSPF::RoutingTableEntry* entry        = rTable[i];
            unsigned long            entryAddress = entry->GetDestinationID().getInt();
            unsigned long            entryMask    = entry->GetAddressMask().getInt();

            if ((entryAddress & entryMask) == (dest & entryMask)) {
                if ((dest & entryMask) > longestMatch) {
                    longestMatch = (dest & entryMask);
                    bestMatch    = entry;
                }
            }
        }
    }

    unsigned int discardCount = discard.size();
    if (bestMatch == NULL) {
        unreachable = true;
    } else {
        for (i = 0; i < discardCount; i++) {
            OSPF::RoutingTableEntry* entry        = discard[i];
            unsigned long            entryAddress = entry->GetDestinationID().getInt();
            unsigned long            entryMask    = entry->GetAddressMask().getInt();

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


/**
 * Rebuilds the routing table from scratch(based on the LSA database).
 * @sa RFC2328 Section 16.
 */
void OSPF::Router::RebuildRoutingTable(void)
{
    unsigned long                         areaCount       = areas.size();
    bool                                  hasTransitAreas = false;
    std::vector<OSPF::RoutingTableEntry*> newTable;
    unsigned long                         i;

    EV << "Rebuilding routing table:\n";

    for (i = 0; i < areaCount; i++) {
        areas[i]->CalculateShortestPathTree(newTable);
        if (areas[i]->GetTransitCapability()) {
            hasTransitAreas = true;
        }
    }
    if (areaCount > 1) {
        OSPF::Area* backbone = GetArea(OSPF::BackboneAreaID);
        if (backbone != NULL) {
            backbone->CalculateInterAreaRoutes(newTable);
        }
    } else {
        if (areaCount == 1) {
            areas[0]->CalculateInterAreaRoutes(newTable);
        }
    }
    if (hasTransitAreas) {
        for (i = 0; i < areaCount; i++) {
            if (areas[i]->GetTransitCapability()) {
                areas[i]->ReCheckSummaryLSAs(newTable);
            }
        }
    }
    CalculateASExternalRoutes(newTable);

    // backup the routing table
    unsigned long                         routeCount = routingTable.size();
    std::vector<OSPF::RoutingTableEntry*> oldTable;

    oldTable.assign(routingTable.begin(), routingTable.end());
    routingTable.clear();
    routingTable.assign(newTable.begin(), newTable.end());

    RoutingTableAccess         routingTableAccess;
    std::vector<const IPRoute*> eraseEntries;
    IRoutingTable*              simRoutingTable    = routingTableAccess.get();
    unsigned long              routingEntryNumber = simRoutingTable->getNumRoutes();
    // remove entries from the IP routing table inserted by the OSPF module
    for (i = 0; i < routingEntryNumber; i++) {
        const IPRoute *entry = simRoutingTable->getRoute(i);
        const OSPF::RoutingTableEntry* ospfEntry = dynamic_cast<const OSPF::RoutingTableEntry*>(entry);
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
        if (routingTable[i]->GetDestinationType() == OSPF::RoutingTableEntry::NetworkDestination) {
            simRoutingTable->addRoute(new OSPF::RoutingTableEntry(*(routingTable[i])));
        }
    }

    NotifyAboutRoutingTableChanges(oldTable);

    routeCount = oldTable.size();
    for (i = 0; i < routeCount; i++) {
        delete(oldTable[i]);
    }

    EV << "Routing table was rebuilt.\n"
       << "Results:\n";

    routeCount = routingTable.size();
    for (i = 0; i < routeCount; i++) {
        EV << *routingTable[i]
           << "\n";
    }
}


/**
 * Returns true if there is a route to the AS Boundary Router identified by
 * asbrRouterID in the input inRoutingTable, false otherwise.
 * @param inRoutingTable [in] The routing table to look in.
 * @param asbrRouterID   [in] The ID of the AS Boundary Router to look for.
 */
bool OSPF::Router::HasRouteToASBoundaryRouter(const std::vector<OSPF::RoutingTableEntry*>& inRoutingTable, OSPF::RouterID asbrRouterID) const
{
    long routeCount = inRoutingTable.size();
    for (long i = 0; i < routeCount; i++) {
        OSPF::RoutingTableEntry* routingEntry = inRoutingTable[i];
        if (((routingEntry->GetDestinationType() & OSPF::RoutingTableEntry::ASBoundaryRouterDestination) != 0) &&
            (routingEntry->GetDestinationID().getInt() == asbrRouterID))
        {
            return true;
        }
    }
    return false;
}


/**
 * Returns an std::vector of routes leading to the AS Boundary Router
 * identified by asbrRouterID from the input fromRoutingTable. If there are no
 * routes leading to the AS Boundary Router, the returned std::vector is empty.
 * @param fromRoutingTable [in] The routing table to look in.
 * @param asbrRouterID     [in] The ID of the AS Boundary Router to look for.
 */
std::vector<OSPF::RoutingTableEntry*> OSPF::Router::GetRoutesToASBoundaryRouter(const std::vector<OSPF::RoutingTableEntry*>& fromRoutingTable, OSPF::RouterID asbrRouterID) const
{
    std::vector<OSPF::RoutingTableEntry*> results;
    long                                  routeCount = fromRoutingTable.size();

    for (long i = 0; i < routeCount; i++) {
        OSPF::RoutingTableEntry* routingEntry = fromRoutingTable[i];
        if (((routingEntry->GetDestinationType() & OSPF::RoutingTableEntry::ASBoundaryRouterDestination) != 0) &&
            (routingEntry->GetDestinationID().getInt() == asbrRouterID))
        {
            results.push_back(routingEntry);
        }
    }
    return results;
}


/**
 * Prunes the input std::vector of RoutingTableEntries according to the RFC2328
 * Section 16.4.1.
 * @param asbrEntries [in/out] The list of RoutingTableEntries to prune.
 * @sa RFC2328 Section 16.4.1.
 */
void OSPF::Router::PruneASBoundaryRouterEntries(std::vector<OSPF::RoutingTableEntry*>& asbrEntries) const
{
    bool hasNonBackboneIntraAreaPath = false;
    for (std::vector<OSPF::RoutingTableEntry*>::iterator it = asbrEntries.begin(); it != asbrEntries.end(); it++) {
        OSPF::RoutingTableEntry* routingEntry = *it;
        if ((routingEntry->GetPathType() == OSPF::RoutingTableEntry::IntraArea) &&
            (routingEntry->GetArea() != OSPF::BackboneAreaID))
        {
            hasNonBackboneIntraAreaPath = true;
            break;
        }
    }

    if (hasNonBackboneIntraAreaPath) {
        std::vector<OSPF::RoutingTableEntry*>::iterator it = asbrEntries.begin();
        while (it != asbrEntries.end()) {
            if (((*it)->GetPathType() != OSPF::RoutingTableEntry::IntraArea) ||
                ((*it)->GetArea() == OSPF::BackboneAreaID))
            {
                it = asbrEntries.erase(it);
            } else {
                it++;
            }
        }
    }
}


/**
 * Selects the least cost RoutingTableEntry from the input std::vector of
 * RoutingTableEntries.
 * @param entries [in] The RoutingTableEntries to choose the least cost one from.
 * @return The least cost entry or NULL if entries is empty.
 */
OSPF::RoutingTableEntry* OSPF::Router::SelectLeastCostRoutingEntry(std::vector<OSPF::RoutingTableEntry*>& entries) const
{
    if (entries.empty()) {
        return NULL;
    }

    OSPF::RoutingTableEntry* leastCostEntry = entries[0];
    Metric                   leastCost      = leastCostEntry->GetCost();
    long                     routeCount     = entries.size();

    for (long i = 1; i < routeCount; i++) {
        Metric currentCost = entries[i]->GetCost();
        if ((currentCost < leastCost) ||
            ((currentCost == leastCost) && (entries[i]->GetArea() > leastCostEntry->GetArea())))
        {
            leastCostEntry = entries[i];
            leastCost = currentCost;
        }
    }

    return leastCostEntry;
}


/**
 * Selects the preferred routing table entry for the input LSA(which is either
 * an ASExternalLSA or a SummaryLSA) according to the algorithm defined in
 * RFC2328 Section 16.4. points(1) through(3). This method is used when
 * calculating the AS external routes and also when originating an SummaryLSA
 * for an AS Boundary Router.
 * @param lsa                [in] The LSA describing the destination for which
 *                                the preferred Routing Entry is sought for.
 * @param skipSelfOriginated [in] Whether to disregard this LSA if it was
 *                                self-originated.
 * @param fromRoutingTable   [in] The Routing Table from which to select the
 *                                preferred RoutingTableEntry. If it is NULL
 *                                then the router's current routing table is
 *                                used instead.
 * @return The preferred RoutingTableEntry, or NULL if no such entry exists.
 * @sa RFC2328 Section 16.4. points(1) through(3)
 * @sa OSPF::Area::OriginateSummaryLSA
 */
OSPF::RoutingTableEntry* OSPF::Router::GetPreferredEntry(const OSPFLSA& lsa, bool skipSelfOriginated, std::vector<OSPF::RoutingTableEntry*>* fromRoutingTable /*= NULL*/)
{
    if (fromRoutingTable == NULL) {
        fromRoutingTable = &routingTable;
    }

    const OSPFLSAHeader&     lsaHeader         = lsa.getHeader();
    const OSPFASExternalLSA* asExternalLSA     = dynamic_cast<const OSPFASExternalLSA*> (&lsa);
    unsigned long            externalCost      = (asExternalLSA != NULL) ? asExternalLSA->getContents().getRouteCost() : 0;
    unsigned short           lsAge             = lsaHeader.getLsAge();
    OSPF::RouterID           originatingRouter = lsaHeader.getAdvertisingRouter().getInt();
    bool                     selfOriginated    = (originatingRouter == routerID);
    IPAddress                forwardingAddress; // 0.0.0.0

    if (asExternalLSA != NULL) {
        forwardingAddress = asExternalLSA->getContents().getForwardingAddress();
    }

    if ((externalCost == LS_INFINITY) || (lsAge == MAX_AGE) || (skipSelfOriginated && selfOriginated)) { // (1) and(2)
        return NULL;
    }

    if (!HasRouteToASBoundaryRouter(*fromRoutingTable, originatingRouter)) { // (3)
        return NULL;
    }

    if (forwardingAddress.isUnspecified()) {   // (3)
        std::vector<OSPF::RoutingTableEntry*> asbrEntries = GetRoutesToASBoundaryRouter(*fromRoutingTable, originatingRouter);
        if (!rfc1583Compatibility) {
            PruneASBoundaryRouterEntries(asbrEntries);
        }
        return SelectLeastCostRoutingEntry(asbrEntries);
    } else {
        OSPF::RoutingTableEntry* forwardEntry = Lookup(forwardingAddress, fromRoutingTable);

        if (forwardEntry == NULL) {
            return NULL;
        }

        if ((forwardEntry->GetPathType() != OSPF::RoutingTableEntry::IntraArea) &&
            (forwardEntry->GetPathType() != OSPF::RoutingTableEntry::InterArea))
        {
            return NULL;
        }

        return forwardEntry;
    }

    return NULL;
}


/**
 * Calculate the AS External Routes from the ASExternalLSAs in the database.
 * @param newRoutingTable [in/out] Push the new RoutingTableEntries into this
 *                                 routing table, and also use this for path
 *                                 calculations.
 * @sa RFC2328 Section 16.4.
 */
void OSPF::Router::CalculateASExternalRoutes(std::vector<OSPF::RoutingTableEntry*>& newRoutingTable)
{
    unsigned long lsaCount = asExternalLSAs.size();
    unsigned long i;

    for (i = 0; i < lsaCount; i++) {
        OSPF::ASExternalLSA* currentLSA        = asExternalLSAs[i];
        OSPFLSAHeader&       currentHeader     = currentLSA->getHeader();
        unsigned short       externalCost      = currentLSA->getContents().getRouteCost();
        OSPF::RouterID       originatingRouter = currentHeader.getAdvertisingRouter().getInt();

        OSPF::RoutingTableEntry* preferredEntry = GetPreferredEntry(*currentLSA, true, &newRoutingTable);
        if (preferredEntry == NULL) {
            continue;
        }

        IPAddress destination = currentHeader.getLinkStateID() & currentLSA->getContents().getNetworkMask().getInt();

        Metric                   preferredCost    = preferredEntry->GetCost();
        OSPF::RoutingTableEntry* destinationEntry = Lookup(destination, &newRoutingTable);   // (5)
        if (destinationEntry == NULL) {
            bool                     type2ExternalMetric = currentLSA->getContents().getE_ExternalMetricType();
            unsigned int             nextHopCount        = preferredEntry->GetNextHopCount();
            OSPF::RoutingTableEntry* newEntry            = new OSPF::RoutingTableEntry;

            newEntry->SetDestinationID(destination);
            newEntry->SetAddressMask(currentLSA->getContents().getNetworkMask().getInt());
            newEntry->SetArea(preferredEntry->GetArea());
            newEntry->SetPathType(type2ExternalMetric ? OSPF::RoutingTableEntry::Type2External : OSPF::RoutingTableEntry::Type1External);
            if (type2ExternalMetric) {
                newEntry->SetCost(preferredCost);
                newEntry->SetType2Cost(externalCost);
            } else {
                newEntry->SetCost(preferredCost + externalCost);
            }
            newEntry->SetDestinationType(OSPF::RoutingTableEntry::NetworkDestination);
            newEntry->SetOptionalCapabilities(currentHeader.getLsOptions());
            newEntry->SetLinkStateOrigin(currentLSA);

            for (unsigned int j = 0; j < nextHopCount; j++) {
                NextHop nextHop = preferredEntry->GetNextHop(j);

                nextHop.advertisingRouter = originatingRouter;
                newEntry->AddNextHop(nextHop);
            }

            newRoutingTable.push_back(newEntry);
        } else {
            OSPF::RoutingTableEntry::RoutingPathType destinationPathType = destinationEntry->GetPathType();
            bool                                     type2ExternalMetric = currentLSA->getContents().getE_ExternalMetricType();
            unsigned int                             nextHopCount        = preferredEntry->GetNextHopCount();

            if ((destinationPathType == OSPF::RoutingTableEntry::IntraArea) ||
                (destinationPathType == OSPF::RoutingTableEntry::InterArea))   // (6) (a)
            {
                continue;
            }

            if (((destinationPathType == OSPF::RoutingTableEntry::Type1External) &&
                 (type2ExternalMetric)) ||
                ((destinationPathType == OSPF::RoutingTableEntry::Type2External) &&
                 (type2ExternalMetric) &&
                 (destinationEntry->GetType2Cost() < externalCost))) // (6) (b)
            {
                continue;
            }

            OSPF::RoutingTableEntry* destinationPreferredEntry = GetPreferredEntry(*(destinationEntry->GetLinkStateOrigin()), false, &newRoutingTable);
            if ((!rfc1583Compatibility) &&
                (destinationPreferredEntry->GetPathType() == OSPF::RoutingTableEntry::IntraArea) &&
                (destinationPreferredEntry->GetArea() != OSPF::BackboneAreaID) &&
                ((preferredEntry->GetPathType() != OSPF::RoutingTableEntry::IntraArea) ||
                 (preferredEntry->GetArea() == OSPF::BackboneAreaID)))
            {
                continue;
            }

            if ((((destinationPathType == OSPF::RoutingTableEntry::Type1External) &&
                  (!type2ExternalMetric) &&
                  (destinationEntry->GetCost() < preferredCost + externalCost))) ||
                ((destinationPathType == OSPF::RoutingTableEntry::Type2External) &&
                 (type2ExternalMetric) &&
                 (destinationEntry->GetType2Cost() == externalCost) &&
                 (destinationPreferredEntry->GetCost() < preferredCost)))
            {
                continue;
            }

            if (((destinationPathType == OSPF::RoutingTableEntry::Type1External) &&
                 (!type2ExternalMetric) &&
                 (destinationEntry->GetCost() == (preferredCost + externalCost))) ||
                ((destinationPathType == OSPF::RoutingTableEntry::Type2External) &&
                 (type2ExternalMetric) &&
                 (destinationEntry->GetType2Cost() == externalCost) &&
                 (destinationPreferredEntry->GetCost() == preferredCost)))   // equal cost
            {
                for (unsigned int j = 0; j < nextHopCount; j++) {
                    // TODO: merge next hops, not add
                    NextHop nextHop = preferredEntry->GetNextHop(j);

                    nextHop.advertisingRouter = originatingRouter;
                    destinationEntry->AddNextHop(nextHop);
                }
                continue;
            }

            // LSA is better
            destinationEntry->SetArea(preferredEntry->GetArea());
            destinationEntry->SetPathType(type2ExternalMetric ? OSPF::RoutingTableEntry::Type2External : OSPF::RoutingTableEntry::Type1External);
            if (type2ExternalMetric) {
                destinationEntry->SetCost(preferredCost);
                destinationEntry->SetType2Cost(externalCost);
            } else {
                destinationEntry->SetCost(preferredCost + externalCost);
            }
            destinationEntry->SetDestinationType(OSPF::RoutingTableEntry::NetworkDestination);
            destinationEntry->SetOptionalCapabilities(currentHeader.getLsOptions());
            destinationEntry->ClearNextHops();

            for (unsigned int j = 0; j < nextHopCount; j++) {
                NextHop nextHop = preferredEntry->GetNextHop(j);

                nextHop.advertisingRouter = originatingRouter;
                destinationEntry->AddNextHop(nextHop);
            }
        }
    }
}


/**
 * Scans through the router's areas' preconfigured address ranges and returns
 * the one containing the input addressRange.
 * @param addressRange [in] The address range to look for.
 * @param advertise    [out] Whether the advertise flag is set in the returned
 *                           preconfigured address range.
 * @return The containing preconfigured address range if found,
 *         OSPF::NullIPv4AddressRange otherwise.
 */
OSPF::IPv4AddressRange OSPF::Router::GetContainingAddressRange(OSPF::IPv4AddressRange addressRange, bool* advertise /*= NULL*/) const
{
    unsigned long areaCount = areas.size();
    for (unsigned long i = 0; i < areaCount; i++) {
        OSPF::IPv4AddressRange containingAddressRange = areas[i]->GetContainingAddressRange(addressRange, advertise);
        if (containingAddressRange != OSPF::NullIPv4AddressRange) {
            return containingAddressRange;
        }
    }
    if (advertise != NULL) {
        *advertise = false;
    }
    return OSPF::NullIPv4AddressRange;
}


/**
 * Generates a unique LinkStateID for a given destination. This may require the
 * reorigination of an LSA already in the database(with a different
 * LinkStateID).
 * @param destination           [in] The destination for which a unique
 *                                   LinkStateID is required.
 * @param destinationCost       [in] The path cost to the destination.
 * @param lsaToReoriginate      [out] The LSA to reoriginate(which was already
 *                                    in the database, and had to be changed).
 * @param externalMetricIsType2 [in] True if the destinationCost is given as a
 *                                   Type2 external metric.
 * @return the LinkStateID for the destination.
 * @sa RFC2328 Appendix E.
 * @sa OSPF::Area::GetUniqueLinkStateID
 */
OSPF::LinkStateID OSPF::Router::GetUniqueLinkStateID(OSPF::IPv4AddressRange destination,
                                                      OSPF::Metric destinationCost,
                                                      OSPF::ASExternalLSA*& lsaToReoriginate,
                                                      bool externalMetricIsType2 /*= false*/) const
{
    if (lsaToReoriginate != NULL) {
        delete lsaToReoriginate;
        lsaToReoriginate = NULL;
    }

    OSPF::LSAKeyType lsaKey;

    lsaKey.linkStateID = ULongFromIPv4Address(destination.address);
    lsaKey.advertisingRouter = routerID;

    const OSPF::ASExternalLSA* foundLSA = FindASExternalLSA(lsaKey);

    if (foundLSA == NULL) {
        return lsaKey.linkStateID;
    } else {
        OSPF::IPv4Address existingMask = IPv4AddressFromULong(foundLSA->getContents().getNetworkMask().getInt());

        if (destination.mask >= existingMask) {
            return (lsaKey.linkStateID | (~(ULongFromIPv4Address(destination.mask))));
        } else {
            OSPF::ASExternalLSA* asExternalLSA = new OSPF::ASExternalLSA(*foundLSA);

            long sequenceNumber = asExternalLSA->getHeader().getLsSequenceNumber();

            asExternalLSA->getHeader().setLsAge(0);
            asExternalLSA->getHeader().setLsSequenceNumber((sequenceNumber == MAX_SEQUENCE_NUMBER) ? INITIAL_SEQUENCE_NUMBER : sequenceNumber + 1);
            asExternalLSA->getContents().setNetworkMask(ULongFromIPv4Address(destination.mask));
            asExternalLSA->getContents().setE_ExternalMetricType(externalMetricIsType2);
            asExternalLSA->getContents().setRouteCost(destinationCost);
            asExternalLSA->getHeader().setLsChecksum(0);    // TODO: calculate correct LS checksum

            lsaToReoriginate = asExternalLSA;

            return (lsaKey.linkStateID | (~(ULongFromIPv4Address(existingMask))));
        }
    }
}


/**
 * After a routing table rebuild the changes in the routing table are
 * identified and new SummaryLSAs are originated or old ones are flooded out
 * in each area as necessary.
 * @param oldRoutingTable [in] The previous version of the routing table(which
 *                             is then compared with the one in routingTable).
 * @sa RFC2328 Section 12.4. points(5) through(6).
 */
// TODO: review this algorithm + add virtual link changes(RFC2328 Section 16.7.).
void OSPF::Router::NotifyAboutRoutingTableChanges(std::vector<OSPF::RoutingTableEntry*>& oldRoutingTable)
{
    if (areas.size() <= 1) {
        return;
    }

    unsigned long                                 routeCount = oldRoutingTable.size();
    std::map<unsigned long, RoutingTableEntry*>   oldTableMap;
    std::map<unsigned long, RoutingTableEntry*>   newTableMap;
    unsigned long                                 i, j, k;

    for (i = 0; i < routeCount; i++) {
        unsigned long destination = oldRoutingTable[i]->GetDestinationID().getInt() & oldRoutingTable[i]->GetAddressMask().getInt();
        oldTableMap[destination] = oldRoutingTable[i];
    }

    routeCount = routingTable.size();
    for (i = 0; i < routeCount; i++) {
        unsigned long   destination = routingTable[i]->GetDestinationID().getInt() & routingTable[i]->GetAddressMask().getInt();
        newTableMap[destination] = routingTable[i];
    }

    unsigned long areaCount = areas.size();
    for (i = 0; i < areaCount; i++) {
        std::map<OSPF::LSAKeyType, bool, OSPF::LSAKeyType_Less> originatedLSAMap;
        std::map<OSPF::LSAKeyType, bool, OSPF::LSAKeyType_Less> deletedLSAMap;
        OSPF::LSAKeyType                                        lsaKey;

        routeCount = routingTable.size();
        for (j = 0; j < routeCount; j++) {
            unsigned long                                         destination = routingTable[j]->GetDestinationID().getInt() & routingTable[j]->GetAddressMask().getInt();
            std::map<unsigned long, RoutingTableEntry*>::iterator destIt      = oldTableMap.find(destination);
            if (destIt == oldTableMap.end()) { // new routing entry
                OSPF::SummaryLSA* lsaToReoriginate = NULL;
                OSPF::SummaryLSA* newLSA           = areas[i]->OriginateSummaryLSA(routingTable[j], originatedLSAMap, lsaToReoriginate);

                if (newLSA != NULL) {
                    if (lsaToReoriginate != NULL) {
                        areas[i]->InstallSummaryLSA(lsaToReoriginate);
//                        FloodLSA(lsaToReoriginate, OSPF::BackboneAreaID);
                        FloodLSA(lsaToReoriginate, areas[i]->GetAreaID());

                        lsaKey.linkStateID       = lsaToReoriginate->getHeader().getLinkStateID();
                        lsaKey.advertisingRouter = routerID;
                        originatedLSAMap[lsaKey] = true;

                        delete lsaToReoriginate;
                    }

                    areas[i]->InstallSummaryLSA(newLSA);
//                    FloodLSA(newLSA, OSPF::BackboneAreaID);
                    FloodLSA(newLSA, areas[i]->GetAreaID());

                    lsaKey.linkStateID       = newLSA->getHeader().getLinkStateID();
                    lsaKey.advertisingRouter = routerID;
                    originatedLSAMap[lsaKey] = true;

                    delete newLSA;
                }
            } else {
                if (*(routingTable[j]) != *(destIt->second)) {  // modified routing entry
                    OSPF::SummaryLSA* lsaToReoriginate = NULL;
                    OSPF::SummaryLSA* newLSA           = areas[i]->OriginateSummaryLSA(routingTable[j], originatedLSAMap, lsaToReoriginate);

                    if (newLSA != NULL) {
                        if (lsaToReoriginate != NULL) {
                            areas[i]->InstallSummaryLSA(lsaToReoriginate);
//                            FloodLSA(lsaToReoriginate, OSPF::BackboneAreaID);
                            FloodLSA(lsaToReoriginate, areas[i]->GetAreaID());

                            lsaKey.linkStateID       = lsaToReoriginate->getHeader().getLinkStateID();
                            lsaKey.advertisingRouter = routerID;
                            originatedLSAMap[lsaKey] = true;

                            delete lsaToReoriginate;
                        }

                        areas[i]->InstallSummaryLSA(newLSA);
//                        FloodLSA(newLSA, OSPF::BackboneAreaID);
                        FloodLSA(newLSA, areas[i]->GetAreaID());

                        lsaKey.linkStateID       = newLSA->getHeader().getLinkStateID();
                        lsaKey.advertisingRouter = routerID;
                        originatedLSAMap[lsaKey] = true;

                        delete newLSA;
                    } else {
                        OSPF::IPv4AddressRange destinationAddressRange;

                        destinationAddressRange.address = IPv4AddressFromULong(routingTable[j]->GetDestinationID().getInt());
                        destinationAddressRange.mask = IPv4AddressFromULong(routingTable[j]->GetAddressMask().getInt());

                        if ((routingTable[j]->GetDestinationType() == OSPF::RoutingTableEntry::NetworkDestination) &&
                            ((routingTable[j]->GetPathType() == OSPF::RoutingTableEntry::IntraArea) ||
                             (routingTable[j]->GetPathType() == OSPF::RoutingTableEntry::InterArea)))
                        {
                            OSPF::IPv4AddressRange containingAddressRange = GetContainingAddressRange(destinationAddressRange);
                            if (containingAddressRange != OSPF::NullIPv4AddressRange) {
                                destinationAddressRange = containingAddressRange;
                            }
                        }

                        Metric maxRangeCost = 0;
                        Metric oneLessCost  = 0;

                        for (k = 0; k < routeCount; k++) {
                            if ((routingTable[k]->GetDestinationType() == OSPF::RoutingTableEntry::NetworkDestination) &&
                                (routingTable[k]->GetPathType() == OSPF::RoutingTableEntry::IntraArea) &&
                                ((routingTable[k]->GetDestinationID().getInt() & routingTable[k]->GetAddressMask().getInt() & ULongFromIPv4Address(destinationAddressRange.mask)) ==
                                 ULongFromIPv4Address(destinationAddressRange.address & destinationAddressRange.mask)) &&
                                (routingTable[k]->GetCost() > maxRangeCost))
                            {
                                oneLessCost  = maxRangeCost;
                                maxRangeCost = routingTable[k]->GetCost();
                            }
                        }

                        if (maxRangeCost == routingTable[j]->GetCost()) {  // this entry gives the range's cost
                            lsaKey.linkStateID       = ULongFromIPv4Address(destinationAddressRange.address);
                            lsaKey.advertisingRouter = routerID;

                            OSPF::SummaryLSA* summaryLSA = areas[i]->FindSummaryLSA(lsaKey);

                            if (summaryLSA != NULL) {
                                if (oneLessCost != 0) { // there's an other entry in this range
                                    summaryLSA->setRouteCost(oneLessCost);
//                                    FloodLSA(summaryLSA, OSPF::BackboneAreaID);
                                    FloodLSA(summaryLSA, areas[i]->GetAreaID());

                                    originatedLSAMap[lsaKey] = true;
                                } else {    // no more entries in this range -> delete it
                                    std::map<OSPF::LSAKeyType, bool, OSPF::LSAKeyType_Less>::const_iterator deletedIt = deletedLSAMap.find(lsaKey);
                                    if (deletedIt == deletedLSAMap.end()) {
                                        summaryLSA->getHeader().setLsAge(MAX_AGE);
//                                        FloodLSA(summaryLSA, OSPF::BackboneAreaID);
                                        FloodLSA(summaryLSA, areas[i]->GetAreaID());

                                        deletedLSAMap[lsaKey]    = true;
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
            unsigned long                                         destination = oldRoutingTable[j]->GetDestinationID().getInt() & oldRoutingTable[j]->GetAddressMask().getInt();
            std::map<unsigned long, RoutingTableEntry*>::iterator destIt      = newTableMap.find(destination);
            if (destIt == newTableMap.end()) { // deleted routing entry
                OSPF::IPv4AddressRange destinationAddressRange;

                destinationAddressRange.address = IPv4AddressFromULong(oldRoutingTable[j]->GetDestinationID().getInt());
                destinationAddressRange.mask = IPv4AddressFromULong(oldRoutingTable[j]->GetAddressMask().getInt());

                if ((oldRoutingTable[j]->GetDestinationType() == OSPF::RoutingTableEntry::NetworkDestination) &&
                    ((oldRoutingTable[j]->GetPathType() == OSPF::RoutingTableEntry::IntraArea) ||
                     (oldRoutingTable[j]->GetPathType() == OSPF::RoutingTableEntry::InterArea)))
                {
                    OSPF::IPv4AddressRange containingAddressRange = GetContainingAddressRange(destinationAddressRange);
                    if (containingAddressRange != OSPF::NullIPv4AddressRange) {
                        destinationAddressRange = containingAddressRange;
                    }
                }

                Metric maxRangeCost = 0;

                unsigned long newRouteCount = routingTable.size();
                for (k = 0; k < newRouteCount; k++) {
                    if ((routingTable[k]->GetDestinationType() == OSPF::RoutingTableEntry::NetworkDestination) &&
                        (routingTable[k]->GetPathType() == OSPF::RoutingTableEntry::IntraArea) &&
                        ((routingTable[k]->GetDestinationID().getInt() & routingTable[k]->GetAddressMask().getInt() & ULongFromIPv4Address(destinationAddressRange.mask)) ==
                         ULongFromIPv4Address(destinationAddressRange.address & destinationAddressRange.mask)) &&
                        (routingTable[k]->GetCost() > maxRangeCost))
                    {
                        maxRangeCost = routingTable[k]->GetCost();
                    }
                }

                if (maxRangeCost < oldRoutingTable[j]->GetCost()) {  // the range's cost will change
                    lsaKey.linkStateID       = ULongFromIPv4Address(destinationAddressRange.address);
                    lsaKey.advertisingRouter = routerID;

                    OSPF::SummaryLSA* summaryLSA = areas[i]->FindSummaryLSA(lsaKey);

                    if (summaryLSA != NULL) {
                        if (maxRangeCost > 0) { // there's an other entry in this range
                            summaryLSA->setRouteCost(maxRangeCost);
                            FloodLSA(summaryLSA, OSPF::BackboneAreaID);

                            originatedLSAMap[lsaKey] = true;
                        } else {    // no more entries in this range -> delete it
                            std::map<OSPF::LSAKeyType, bool, OSPF::LSAKeyType_Less>::const_iterator deletedIt = deletedLSAMap.find(lsaKey);
                            if (deletedIt == deletedLSAMap.end()) {
                                summaryLSA->getHeader().setLsAge(MAX_AGE);
                                FloodLSA(summaryLSA, OSPF::BackboneAreaID);

                                deletedLSAMap[lsaKey]    = true;
                            }
                        }
                    }
                }
            }
        }
    }
}


/**
 * Stores information on an AS External Route in externalRoutes and intalls(or
 * updates) a new ASExternalLSA into the database.
 * @param networkAddress        [in] The external route's network address.
 * @param externalRouteContents [in] Route configuration data for the external route.
 * @param ifIndex               [in]
 */
void OSPF::Router::UpdateExternalRoute(OSPF::IPv4Address networkAddress, const OSPFASExternalLSAContents& externalRouteContents, int ifIndex)
{
    OSPF::ASExternalLSA* asExternalLSA = new OSPF::ASExternalLSA;
    OSPFLSAHeader&       lsaHeader     = asExternalLSA->getHeader();
    OSPFOptions          lsaOptions;
    //OSPF::LSAKeyType     lsaKey;

    IRoutingTable*      simRoutingTable    = RoutingTableAccess().get();
    unsigned long      routingEntryNumber = simRoutingTable->getNumRoutes();
    bool               inRoutingTable     = false;
    // add the external route to the routing table if it was not added by another module
    for (unsigned long i = 0; i < routingEntryNumber; i++) {
        const IPRoute *entry = simRoutingTable->getRoute(i);
        if ((entry->getHost().getInt() & entry->getNetmask().getInt()) ==
            (ULongFromIPv4Address(networkAddress) & externalRouteContents.getNetworkMask().getInt()))
        {
            inRoutingTable = true;
        }
    }
    if (!inRoutingTable) {
        IPRoute* entry = new IPRoute;
        entry->setHost(ULongFromIPv4Address(networkAddress));
        entry->setNetmask(externalRouteContents.getNetworkMask());
        entry->setInterface(InterfaceTableAccess().get()->getInterfaceById(ifIndex));
        entry->setType(IPRoute::REMOTE);
        entry->setSource(IPRoute::MANUAL);
        entry->setMetric(externalRouteContents.getRouteCost());
        simRoutingTable->addRoute(entry);   // IRoutingTable deletes entry pointer
    }

    lsaHeader.setLsAge(0);
    memset(&lsaOptions, 0, sizeof(OSPFOptions));
    lsaOptions.E_ExternalRoutingCapability = true;
    lsaHeader.setLsOptions(lsaOptions);
    lsaHeader.setLsType(ASExternalLSAType);
    lsaHeader.setLinkStateID(ULongFromIPv4Address(networkAddress));   // TODO: get unique LinkStateID
    lsaHeader.setAdvertisingRouter(routerID);
    lsaHeader.setLsSequenceNumber(INITIAL_SEQUENCE_NUMBER);

    asExternalLSA->setContents(externalRouteContents);

    lsaHeader.setLsChecksum(0);    // TODO: calculate correct LS checksum

    asExternalLSA->SetSource(OSPF::LSATrackingInfo::Originated);

    externalRoutes[networkAddress] = externalRouteContents;

    bool rebuild = InstallASExternalLSA(asExternalLSA);
    FloodLSA(asExternalLSA, OSPF::BackboneAreaID);
    delete asExternalLSA;

    if (rebuild) {
        RebuildRoutingTable();
    }
}


/**
 * Removes an AS External Route from the database.
 * @param networkAddress [in] The network address of the external route which
 *                            needs to be removed.
 */
void OSPF::Router::RemoveExternalRoute(OSPF::IPv4Address networkAddress)
{
    OSPF::LSAKeyType     lsaKey;

    lsaKey.linkStateID = ULongFromIPv4Address(networkAddress);
    lsaKey.advertisingRouter = routerID;

    std::map<OSPF::LSAKeyType, OSPF::ASExternalLSA*, OSPF::LSAKeyType_Less>::iterator lsaIt = asExternalLSAsByID.find(lsaKey);
    if (lsaIt != asExternalLSAsByID.end()) {
        lsaIt->second->getHeader().setLsAge(MAX_AGE);
        lsaIt->second->SetPurgeable();
        FloodLSA(lsaIt->second, OSPF::BackboneAreaID);
    }

    std::map<OSPF::IPv4Address, OSPFASExternalLSAContents, OSPF::IPv4Address_Less>::iterator externalIt = externalRoutes.find(networkAddress);
    if (externalIt != externalRoutes.end()) {
        externalRoutes.erase(externalIt);
    }
}
