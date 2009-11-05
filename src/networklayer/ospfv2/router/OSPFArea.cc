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

#include "OSPFArea.h"
#include "OSPFRouter.h"
#include <memory.h>

OSPF::Area::Area(OSPF::AreaID id) :
    areaID(id),
    transitCapability(false),
    externalRoutingCapability(true),
    stubDefaultCost(1),
    spfTreeRoot(NULL),
    parentRouter(NULL)
{
}

OSPF::Area::~Area(void)
{
    int interfaceNum = associatedInterfaces.size();
    for (int i = 0; i < interfaceNum; i++) {
        delete(associatedInterfaces[i]);
    }
    long lsaCount = routerLSAs.size();
    for (long j = 0; j < lsaCount; j++) {
        delete routerLSAs[j];
    }
    routerLSAs.clear();
    lsaCount = networkLSAs.size();
    for (long k = 0; k < lsaCount; k++) {
        delete networkLSAs[k];
    }
    networkLSAs.clear();
    lsaCount = summaryLSAs.size();
    for (long m = 0; m < lsaCount; m++) {
        delete summaryLSAs[m];
    }
    summaryLSAs.clear();
}

void OSPF::Area::AddInterface(OSPF::Interface* intf)
{
    intf->SetArea(this);
    associatedInterfaces.push_back(intf);
}

void OSPF::Area::info(char *buffer)
{
    std::stringstream out;
    char areaString[16];
    out << "areaID: " << AddressStringFromULong(areaString, 16, areaID);
    strcpy(buffer, out.str().c_str());
}

std::string OSPF::Area::detailedInfo(void) const
{
    std::stringstream out;
    char addressString[16];
    int i;
    out << "\n    areaID: " << AddressStringFromULong(addressString, 16, areaID) << ", ";
    out << "transitCapability: " << (transitCapability ? "true" : "false") << ", ";
    out << "externalRoutingCapability: " << (externalRoutingCapability ? "true" : "false") << ", ";
    out << "stubDefaultCost: " << stubDefaultCost << "\n";
    int addressRangeNum = areaAddressRanges.size();
    for (i = 0; i < addressRangeNum; i++) {
        out << "    addressRanges[" << i << "]: ";
        out << AddressStringFromIPv4Address(addressString, 16, areaAddressRanges[i].address);
        out << "/" << AddressStringFromIPv4Address(addressString, 16, areaAddressRanges[i].mask) << "\n";
    }
    int interfaceNum = associatedInterfaces.size();
    for (i = 0; i < interfaceNum; i++) {
        out << "    interface[" << i << "]: addressRange: ";
        out << AddressStringFromIPv4Address(addressString, 16, associatedInterfaces[i]->GetAddressRange().address);
        out << "/" << AddressStringFromIPv4Address(addressString, 16, associatedInterfaces[i]->GetAddressRange().mask) << "\n";
    }

    out << "\n";
    out << "    Database:\n";
    out << "      RouterLSAs:\n";
    long lsaCount = routerLSAs.size();
    for (i = 0; i < lsaCount; i++) {
        out << "        " << *routerLSAs[i] << "\n";
    }
    out << "      NetworkLSAs:\n";
    lsaCount = networkLSAs.size();
    for (i = 0; i < lsaCount; i++) {
        out << "        " << *networkLSAs[i] << "\n";
    }
    out << "      SummaryLSAs:\n";
    lsaCount = summaryLSAs.size();
    for (i = 0; i < lsaCount; i++) {
        out << "        " << *summaryLSAs[i] << "\n";
    }

    out << "--------------------------------------------------------------------------------";

    return out.str();
}

bool OSPF::Area::ContainsAddress(OSPF::IPv4Address address) const
{
    int addressRangeNum = areaAddressRanges.size();
    for (int i = 0; i < addressRangeNum; i++) {
        if ((areaAddressRanges[i].address & areaAddressRanges[i].mask) == (address & areaAddressRanges[i].mask)) {
            return true;
        }
    }
    return false;
}

bool OSPF::Area::HasAddressRange(OSPF::IPv4AddressRange addressRange) const
{
    int addressRangeNum = areaAddressRanges.size();
    for (int i = 0; i < addressRangeNum; i++) {
        if ((areaAddressRanges[i].address == addressRange.address) &&
            (areaAddressRanges[i].mask == addressRange.mask))
        {
            return true;
        }
    }
    return false;
}

OSPF::IPv4AddressRange OSPF::Area::GetContainingAddressRange(OSPF::IPv4AddressRange addressRange, bool* advertise /*= NULL*/) const
{
    int addressRangeNum = areaAddressRanges.size();
    for (int i = 0; i < addressRangeNum; i++) {
        if ((areaAddressRanges[i].address & areaAddressRanges[i].mask) == (addressRange.address & areaAddressRanges[i].mask)) {
            if (advertise != NULL) {
                std::map<OSPF::IPv4AddressRange, bool, OSPF::IPv4AddressRange_Less>::const_iterator rangeIt = advertiseAddressRanges.find(areaAddressRanges[i]);
                if (rangeIt != advertiseAddressRanges.end()) {
                    *advertise = rangeIt->second;
                } else {
                    *advertise = true;
                }
            }
            return areaAddressRanges[i];
        }
    }
    if (advertise != NULL) {
        *advertise =  false;
    }
    return NullIPv4AddressRange;
}

OSPF::Interface*  OSPF::Area::GetInterface(unsigned char ifIndex)
{
    int interfaceNum = associatedInterfaces.size();
    for (int i = 0; i < interfaceNum; i++) {
        if ((associatedInterfaces[i]->GetType() != OSPF::Interface::Virtual) &&
            (associatedInterfaces[i]->GetIfIndex() == ifIndex))
        {
            return associatedInterfaces[i];
        }
    }
    return NULL;
}

OSPF::Interface*  OSPF::Area::GetInterface(OSPF::IPv4Address address)
{
    int interfaceNum = associatedInterfaces.size();
    for (int i = 0; i < interfaceNum; i++) {
        if ((associatedInterfaces[i]->GetType() != OSPF::Interface::Virtual) &&
            (associatedInterfaces[i]->GetAddressRange().address == address))
        {
            return associatedInterfaces[i];
        }
    }
    return NULL;
}

bool OSPF::Area::HasVirtualLink(OSPF::AreaID withTransitArea) const
{
    if ((areaID != OSPF::BackboneAreaID) || (withTransitArea == OSPF::BackboneAreaID)) {
        return false;
    }

    int interfaceNum = associatedInterfaces.size();
    for (int i = 0; i < interfaceNum; i++) {
        if ((associatedInterfaces[i]->GetType() == OSPF::Interface::Virtual) &&
            (associatedInterfaces[i]->GetTransitAreaID() == withTransitArea))
        {
            return true;
        }
    }
    return false;
}


OSPF::Interface*  OSPF::Area::FindVirtualLink(OSPF::RouterID routerID)
{
    int interfaceNum = associatedInterfaces.size();
    for (int i = 0; i < interfaceNum; i++) {
        if ((associatedInterfaces[i]->GetType() == OSPF::Interface::Virtual) &&
            (associatedInterfaces[i]->GetNeighborByID(routerID) != NULL))
        {
            return associatedInterfaces[i];
        }
    }
    return NULL;
}

bool OSPF::Area::InstallRouterLSA(OSPFRouterLSA* lsa)
{
    OSPF::LinkStateID linkStateID = lsa->getHeader().getLinkStateID();
    std::map<OSPF::LinkStateID, OSPF::RouterLSA*>::iterator lsaIt = routerLSAsByID.find(linkStateID);
    if (lsaIt != routerLSAsByID.end()) {
        OSPF::LSAKeyType lsaKey;

        lsaKey.linkStateID = lsa->getHeader().getLinkStateID();
        lsaKey.advertisingRouter = lsa->getHeader().getAdvertisingRouter().getInt();

        RemoveFromAllRetransmissionLists(lsaKey);
        return lsaIt->second->Update(lsa);
    } else {
        OSPF::RouterLSA* lsaCopy = new OSPF::RouterLSA(*lsa);
        routerLSAsByID[linkStateID] = lsaCopy;
        routerLSAs.push_back(lsaCopy);
        return true;
    }
}

bool OSPF::Area::InstallNetworkLSA(OSPFNetworkLSA* lsa)
{
    OSPF::LinkStateID linkStateID = lsa->getHeader().getLinkStateID();
    std::map<OSPF::LinkStateID, OSPF::NetworkLSA*>::iterator lsaIt = networkLSAsByID.find(linkStateID);
    if (lsaIt != networkLSAsByID.end()) {
        OSPF::LSAKeyType lsaKey;

        lsaKey.linkStateID = lsa->getHeader().getLinkStateID();
        lsaKey.advertisingRouter = lsa->getHeader().getAdvertisingRouter().getInt();

        RemoveFromAllRetransmissionLists(lsaKey);
        return lsaIt->second->Update(lsa);
    } else {
        OSPF::NetworkLSA* lsaCopy = new OSPF::NetworkLSA(*lsa);
        networkLSAsByID[linkStateID] = lsaCopy;
        networkLSAs.push_back(lsaCopy);
        return true;
    }
}

bool OSPF::Area::InstallSummaryLSA(OSPFSummaryLSA* lsa)
{
    OSPF::LSAKeyType lsaKey;

    lsaKey.linkStateID = lsa->getHeader().getLinkStateID();
    lsaKey.advertisingRouter = lsa->getHeader().getAdvertisingRouter().getInt();

    std::map<OSPF::LSAKeyType, OSPF::SummaryLSA*, OSPF::LSAKeyType_Less>::iterator lsaIt = summaryLSAsByID.find(lsaKey);
    if (lsaIt != summaryLSAsByID.end()) {
        OSPF::LSAKeyType lsaKey;

        lsaKey.linkStateID = lsa->getHeader().getLinkStateID();
        lsaKey.advertisingRouter = lsa->getHeader().getAdvertisingRouter().getInt();

        RemoveFromAllRetransmissionLists(lsaKey);
        return lsaIt->second->Update(lsa);
    } else {
        OSPF::SummaryLSA* lsaCopy = new OSPF::SummaryLSA(*lsa);
        summaryLSAsByID[lsaKey] = lsaCopy;
        summaryLSAs.push_back(lsaCopy);
        return true;
    }
}

OSPF::RouterLSA* OSPF::Area::FindRouterLSA(OSPF::LinkStateID linkStateID)
{
    std::map<OSPF::LinkStateID, OSPF::RouterLSA*>::iterator lsaIt = routerLSAsByID.find(linkStateID);
    if (lsaIt != routerLSAsByID.end()) {
        return lsaIt->second;
    } else {
        return NULL;
    }
}

const OSPF::RouterLSA* OSPF::Area::FindRouterLSA(OSPF::LinkStateID linkStateID) const
{
    std::map<OSPF::LinkStateID, OSPF::RouterLSA*>::const_iterator lsaIt = routerLSAsByID.find(linkStateID);
    if (lsaIt != routerLSAsByID.end()) {
        return lsaIt->second;
    } else {
        return NULL;
    }
}

OSPF::NetworkLSA* OSPF::Area::FindNetworkLSA(OSPF::LinkStateID linkStateID)
{
    std::map<OSPF::LinkStateID, OSPF::NetworkLSA*>::iterator lsaIt = networkLSAsByID.find(linkStateID);
    if (lsaIt != networkLSAsByID.end()) {
        return lsaIt->second;
    } else {
        return NULL;
    }
}

const OSPF::NetworkLSA* OSPF::Area::FindNetworkLSA(OSPF::LinkStateID linkStateID) const
{
    std::map<OSPF::LinkStateID, OSPF::NetworkLSA*>::const_iterator lsaIt = networkLSAsByID.find(linkStateID);
    if (lsaIt != networkLSAsByID.end()) {
        return lsaIt->second;
    } else {
        return NULL;
    }
}

OSPF::SummaryLSA* OSPF::Area::FindSummaryLSA(OSPF::LSAKeyType lsaKey)
{
    std::map<OSPF::LSAKeyType, OSPF::SummaryLSA*, OSPF::LSAKeyType_Less>::iterator lsaIt = summaryLSAsByID.find(lsaKey);
    if (lsaIt != summaryLSAsByID.end()) {
        return lsaIt->second;
    } else {
        return NULL;
    }
}

const OSPF::SummaryLSA* OSPF::Area::FindSummaryLSA(OSPF::LSAKeyType lsaKey) const
{
    std::map<OSPF::LSAKeyType, OSPF::SummaryLSA*, OSPF::LSAKeyType_Less>::const_iterator lsaIt = summaryLSAsByID.find(lsaKey);
    if (lsaIt != summaryLSAsByID.end()) {
        return lsaIt->second;
    } else {
        return NULL;
    }
}

void OSPF::Area::AgeDatabase(void)
{
    long            lsaCount            = routerLSAs.size();
    bool            rebuildRoutingTable = false;
    long            i;

    for (i = 0; i < lsaCount; i++) {
        unsigned short   lsAge          = routerLSAs[i]->getHeader().getLsAge();
        bool             selfOriginated = (routerLSAs[i]->getHeader().getAdvertisingRouter().getInt() == parentRouter->GetRouterID());
        bool             unreachable    = parentRouter->IsDestinationUnreachable(routerLSAs[i]);
        OSPF::RouterLSA* lsa            = routerLSAs[i];

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
                FloodLSA(lsa);
                lsa->IncrementInstallTime();
            } else {
                long sequenceNumber = lsa->getHeader().getLsSequenceNumber();
                if (sequenceNumber == MAX_SEQUENCE_NUMBER) {
                    lsa->getHeader().setLsAge(MAX_AGE);
                    FloodLSA(lsa);
                    lsa->IncrementInstallTime();
                } else {
                    OSPF::RouterLSA* newLSA = OriginateRouterLSA();

                    newLSA->getHeader().setLsSequenceNumber(sequenceNumber + 1);
                    newLSA->getHeader().setLsChecksum(0);    // TODO: calculate correct LS checksum
                    rebuildRoutingTable |= lsa->Update(newLSA);
                    delete newLSA;

                    FloodLSA(lsa);
                }
            }
        }
        if (!selfOriginated && (lsAge == MAX_AGE - 1)) {
            lsa->getHeader().setLsAge(MAX_AGE);
            FloodLSA(lsa);
            lsa->IncrementInstallTime();
        }
        if (lsAge == MAX_AGE) {
            OSPF::LSAKeyType lsaKey;

            lsaKey.linkStateID = lsa->getHeader().getLinkStateID();
            lsaKey.advertisingRouter = lsa->getHeader().getAdvertisingRouter().getInt();

            if (!IsOnAnyRetransmissionList(lsaKey) &&
                !HasAnyNeighborInStates(OSPF::Neighbor::ExchangeState | OSPF::Neighbor::LoadingState))
            {
                if (!selfOriginated || unreachable) {
                    routerLSAsByID.erase(lsa->getHeader().getLinkStateID());
                    delete lsa;
                    routerLSAs[i] = NULL;
                    rebuildRoutingTable = true;
                } else {
                    OSPF::RouterLSA* newLSA              = OriginateRouterLSA();
                    long             sequenceNumber      = lsa->getHeader().getLsSequenceNumber();

                    newLSA->getHeader().setLsSequenceNumber((sequenceNumber == MAX_SEQUENCE_NUMBER) ? INITIAL_SEQUENCE_NUMBER : sequenceNumber + 1);
                    newLSA->getHeader().setLsChecksum(0);    // TODO: calculate correct LS checksum
                    rebuildRoutingTable |= lsa->Update(newLSA);
                    delete newLSA;

                    FloodLSA(lsa);
                }
            }
        }
    }

    std::vector<RouterLSA*>::iterator routerIt = routerLSAs.begin();
    while (routerIt != routerLSAs.end()) {
        if ((*routerIt) == NULL) {
            routerIt = routerLSAs.erase(routerIt);
        } else {
            routerIt++;
        }
    }

    lsaCount = networkLSAs.size();
    for (i = 0; i < lsaCount; i++) {
        unsigned short    lsAge          = networkLSAs[i]->getHeader().getLsAge();
        bool              unreachable    = parentRouter->IsDestinationUnreachable(networkLSAs[i]);
        OSPF::NetworkLSA* lsa            = networkLSAs[i];
        OSPF::Interface*  localIntf      = GetInterface(IPv4AddressFromULong(lsa->getHeader().getLinkStateID()));
        bool              selfOriginated = false;

        if ((localIntf != NULL) &&
            (localIntf->GetState() == OSPF::Interface::DesignatedRouterState) &&
            (localIntf->GetNeighborCount() > 0) &&
            (localIntf->HasAnyNeighborInStates(OSPF::Neighbor::FullState)))
        {
            selfOriginated = true;
        }

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
                FloodLSA(lsa);
                lsa->IncrementInstallTime();
            } else {
                long sequenceNumber = lsa->getHeader().getLsSequenceNumber();
                if (sequenceNumber == MAX_SEQUENCE_NUMBER) {
                    lsa->getHeader().setLsAge(MAX_AGE);
                    FloodLSA(lsa);
                    lsa->IncrementInstallTime();
                } else {
                    OSPF::NetworkLSA* newLSA = OriginateNetworkLSA(localIntf);

                    if (newLSA != NULL) {
                        newLSA->getHeader().setLsSequenceNumber(sequenceNumber + 1);
                        newLSA->getHeader().setLsChecksum(0);    // TODO: calculate correct LS checksum
                        rebuildRoutingTable |= lsa->Update(newLSA);
                        delete newLSA;
                    } else {    // no neighbors on the network -> old NetworkLSA must be flushed
                        lsa->getHeader().setLsAge(MAX_AGE);
                        lsa->IncrementInstallTime();
                    }

                    FloodLSA(lsa);
                }
            }
        }
        if (!selfOriginated && (lsAge == MAX_AGE - 1)) {
            lsa->getHeader().setLsAge(MAX_AGE);
            FloodLSA(lsa);
            lsa->IncrementInstallTime();
        }
        if (lsAge == MAX_AGE) {
            OSPF::LSAKeyType lsaKey;

            lsaKey.linkStateID = lsa->getHeader().getLinkStateID();
            lsaKey.advertisingRouter = lsa->getHeader().getAdvertisingRouter().getInt();

            if (!IsOnAnyRetransmissionList(lsaKey) &&
                !HasAnyNeighborInStates(OSPF::Neighbor::ExchangeState | OSPF::Neighbor::LoadingState))
            {
                if (!selfOriginated || unreachable) {
                    networkLSAsByID.erase(lsa->getHeader().getLinkStateID());
                    delete lsa;
                    networkLSAs[i] = NULL;
                    rebuildRoutingTable = true;
                } else {
                    OSPF::NetworkLSA* newLSA              = OriginateNetworkLSA(localIntf);
                    long              sequenceNumber      = lsa->getHeader().getLsSequenceNumber();

                    if (newLSA != NULL) {
                        newLSA->getHeader().setLsSequenceNumber((sequenceNumber == MAX_SEQUENCE_NUMBER) ? INITIAL_SEQUENCE_NUMBER : sequenceNumber + 1);
                        newLSA->getHeader().setLsChecksum(0);    // TODO: calculate correct LS checksum
                        rebuildRoutingTable |= lsa->Update(newLSA);
                        delete newLSA;

                        FloodLSA(lsa);
                    } else {    // no neighbors on the network -> old NetworkLSA must be deleted
                        delete networkLSAs[i];
                    }
                }
            }
        }
    }

    std::vector<NetworkLSA*>::iterator networkIt = networkLSAs.begin();
    while (networkIt != networkLSAs.end()) {
        if ((*networkIt) == NULL) {
            networkIt = networkLSAs.erase(networkIt);
        } else {
            networkIt++;
        }
    }

    lsaCount = summaryLSAs.size();
    for (i = 0; i < lsaCount; i++) {
        unsigned short    lsAge          = summaryLSAs[i]->getHeader().getLsAge();
        bool              selfOriginated = (summaryLSAs[i]->getHeader().getAdvertisingRouter().getInt() == parentRouter->GetRouterID());
        bool              unreachable    = parentRouter->IsDestinationUnreachable(summaryLSAs[i]);
        OSPF::SummaryLSA* lsa            = summaryLSAs[i];

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
                FloodLSA(lsa);
                lsa->IncrementInstallTime();
            } else {
                long sequenceNumber = lsa->getHeader().getLsSequenceNumber();
                if (sequenceNumber == MAX_SEQUENCE_NUMBER) {
                    lsa->getHeader().setLsAge(MAX_AGE);
                    FloodLSA(lsa);
                    lsa->IncrementInstallTime();
                } else {
                    OSPF::SummaryLSA* newLSA = OriginateSummaryLSA(lsa);

                    if (newLSA != NULL) {
                        newLSA->getHeader().setLsSequenceNumber(sequenceNumber + 1);
                        newLSA->getHeader().setLsChecksum(0);    // TODO: calculate correct LS checksum
                        rebuildRoutingTable |= lsa->Update(newLSA);
                        delete newLSA;

                        FloodLSA(lsa);
                    } else {
                        lsa->getHeader().setLsAge(MAX_AGE);
                        FloodLSA(lsa);
                        lsa->IncrementInstallTime();
                    }
                }
            }
        }
        if (!selfOriginated && (lsAge == MAX_AGE - 1)) {
            lsa->getHeader().setLsAge(MAX_AGE);
            FloodLSA(lsa);
            lsa->IncrementInstallTime();
        }
        if (lsAge == MAX_AGE) {
            OSPF::LSAKeyType lsaKey;

            lsaKey.linkStateID = lsa->getHeader().getLinkStateID();
            lsaKey.advertisingRouter = lsa->getHeader().getAdvertisingRouter().getInt();

            if (!IsOnAnyRetransmissionList(lsaKey) &&
                !HasAnyNeighborInStates(OSPF::Neighbor::ExchangeState | OSPF::Neighbor::LoadingState))
            {
                if (!selfOriginated || unreachable) {
                    summaryLSAsByID.erase(lsaKey);
                    delete lsa;
                    summaryLSAs[i] = NULL;
                    rebuildRoutingTable = true;
                } else {
                    OSPF::SummaryLSA* newLSA = OriginateSummaryLSA(lsa);
                    if (newLSA != NULL) {
                        long sequenceNumber = lsa->getHeader().getLsSequenceNumber();

                        newLSA->getHeader().setLsSequenceNumber((sequenceNumber == MAX_SEQUENCE_NUMBER) ? INITIAL_SEQUENCE_NUMBER : sequenceNumber + 1);
                        newLSA->getHeader().setLsChecksum(0);    // TODO: calculate correct LS checksum
                        rebuildRoutingTable |= lsa->Update(newLSA);
                        delete newLSA;

                        FloodLSA(lsa);
                    } else {
                        summaryLSAsByID.erase(lsaKey);
                        delete lsa;
                        summaryLSAs[i] = NULL;
                        rebuildRoutingTable = true;
                    }
                }
            }
        }
    }

    std::vector<SummaryLSA*>::iterator summaryIt = summaryLSAs.begin();
    while (summaryIt != summaryLSAs.end()) {
        if ((*summaryIt) == NULL) {
            summaryIt = summaryLSAs.erase(summaryIt);
        } else {
            summaryIt++;
        }
    }

    long interfaceCount = associatedInterfaces.size();
    for (long m = 0; m < interfaceCount; m++) {
        associatedInterfaces[m]->AgeTransmittedLSALists();
    }

    if (rebuildRoutingTable) {
        parentRouter->RebuildRoutingTable();
    }
}

bool OSPF::Area::HasAnyNeighborInStates(int states) const
{
    long interfaceCount = associatedInterfaces.size();
    for (long i = 0; i < interfaceCount; i++) {
        if (associatedInterfaces[i]->HasAnyNeighborInStates(states)) {
            return true;
        }
    }
    return false;
}

void OSPF::Area::RemoveFromAllRetransmissionLists(OSPF::LSAKeyType lsaKey)
{
    long interfaceCount = associatedInterfaces.size();
    for (long i = 0; i < interfaceCount; i++) {
        associatedInterfaces[i]->RemoveFromAllRetransmissionLists(lsaKey);
    }
}

bool OSPF::Area::IsOnAnyRetransmissionList(OSPF::LSAKeyType lsaKey) const
{
    long interfaceCount = associatedInterfaces.size();
    for (long i = 0; i < interfaceCount; i++) {
        if (associatedInterfaces[i]->IsOnAnyRetransmissionList(lsaKey)) {
            return true;
        }
    }
    return false;
}

bool OSPF::Area::FloodLSA(OSPFLSA* lsa, OSPF::Interface* intf, OSPF::Neighbor* neighbor)
{
    bool floodedBackOut  = false;
    long interfaceCount = associatedInterfaces.size();

    for (long i = 0; i < interfaceCount; i++) {
        if (associatedInterfaces[i]->FloodLSA(lsa, intf, neighbor)) {
            floodedBackOut = true;
        }
    }

    return floodedBackOut;
}

bool OSPF::Area::IsLocalAddress(OSPF::IPv4Address address) const
{
    long interfaceCount = associatedInterfaces.size();
    for (long i = 0; i < interfaceCount; i++) {
        if (associatedInterfaces[i]->GetAddressRange().address == address) {
            return true;
        }
    }
    return false;
}

OSPF::RouterLSA* OSPF::Area::OriginateRouterLSA(void)
{
    OSPF::RouterLSA* routerLSA      = new OSPF::RouterLSA;
    OSPFLSAHeader&   lsaHeader      = routerLSA->getHeader();
    long             interfaceCount = associatedInterfaces.size();
    OSPFOptions      lsOptions;
    long             i;

    lsaHeader.setLsAge(0);
    memset(&lsOptions, 0, sizeof(OSPFOptions));
    lsOptions.E_ExternalRoutingCapability = externalRoutingCapability;
    lsaHeader.setLsOptions(lsOptions);
    lsaHeader.setLsType(RouterLSAType);
    lsaHeader.setLinkStateID(parentRouter->GetRouterID());
    lsaHeader.setAdvertisingRouter(parentRouter->GetRouterID());
    lsaHeader.setLsSequenceNumber(INITIAL_SEQUENCE_NUMBER);

    routerLSA->setB_AreaBorderRouter(parentRouter->GetAreaCount() > 1);
    routerLSA->setE_ASBoundaryRouter((externalRoutingCapability && parentRouter->GetASBoundaryRouter()) ? true : false);
    OSPF::Area* backbone = parentRouter->GetArea(OSPF::BackboneAreaID);
    routerLSA->setV_VirtualLinkEndpoint((backbone == NULL) ? false : backbone->HasVirtualLink(areaID));

    routerLSA->setNumberOfLinks(0);
    routerLSA->setLinksArraySize(0);
    for (i = 0; i < interfaceCount; i++) {
        OSPF::Interface* intf = associatedInterfaces[i];

        if (intf->GetState() == OSPF::Interface::DownState) {
            continue;
        }
        if ((intf->GetState() == OSPF::Interface::LoopbackState) &&
            ((intf->GetType() != OSPF::Interface::PointToPoint) ||
             (intf->GetAddressRange().address != OSPF::NullIPv4Address)))
        {
            Link stubLink;
            stubLink.setType(StubLink);
            stubLink.setLinkID(ULongFromIPv4Address(intf->GetAddressRange().address));
            stubLink.setLinkData(0xFFFFFFFF);
            stubLink.setLinkCost(0);
            stubLink.setNumberOfTOS(0);
            stubLink.setTosDataArraySize(0);

            unsigned short linkIndex = routerLSA->getLinksArraySize();
            routerLSA->setLinksArraySize(linkIndex + 1);
            routerLSA->setNumberOfLinks(linkIndex + 1);
            routerLSA->setLinks(linkIndex, stubLink);
        }
        if (intf->GetState() > OSPF::Interface::LoopbackState) {
            switch (intf->GetType()) {
                case OSPF::Interface::PointToPoint:
                    {
                        OSPF::Neighbor* neighbor = (intf->GetNeighborCount() > 0) ? intf->GetNeighbor(0) : NULL;
                        if (neighbor != NULL) {
                            if (neighbor->GetState() == OSPF::Neighbor::FullState) {
                                Link link;
                                link.setType(PointToPointLink);
                                link.setLinkID(neighbor->GetNeighborID());
                                if (intf->GetAddressRange().address != OSPF::NullIPv4Address) {
                                    link.setLinkData(ULongFromIPv4Address(intf->GetAddressRange().address));
                                } else {
                                    link.setLinkData(intf->GetIfIndex());
                                }
                                link.setLinkCost(intf->GetOutputCost());
                                link.setNumberOfTOS(0);
                                link.setTosDataArraySize(0);

                                unsigned short linkIndex = routerLSA->getLinksArraySize();
                                routerLSA->setLinksArraySize(linkIndex + 1);
                                routerLSA->setNumberOfLinks(linkIndex + 1);
                                routerLSA->setLinks(linkIndex, link);
                            }
                            if (intf->GetState() == OSPF::Interface::PointToPointState) {
                                if (neighbor->GetAddress() != OSPF::NullIPv4Address) {
                                    Link stubLink;
                                    stubLink.setType(StubLink);
                                    stubLink.setLinkID(ULongFromIPv4Address(neighbor->GetAddress()));
                                    stubLink.setLinkData(0xFFFFFFFF);
                                    stubLink.setLinkCost(intf->GetOutputCost());
                                    stubLink.setNumberOfTOS(0);
                                    stubLink.setTosDataArraySize(0);

                                    unsigned short linkIndex = routerLSA->getLinksArraySize();
                                    routerLSA->setLinksArraySize(linkIndex + 1);
                                    routerLSA->setNumberOfLinks(linkIndex + 1);
                                    routerLSA->setLinks(linkIndex, stubLink);
                                } else {
                                    if (ULongFromIPv4Address(intf->GetAddressRange().mask) != 0xFFFFFFFF) {
                                        Link stubLink;
                                        stubLink.setType(StubLink);
                                        stubLink.setLinkID(ULongFromIPv4Address(intf->GetAddressRange().address &
                                                                                  intf->GetAddressRange().mask));
                                        stubLink.setLinkData(ULongFromIPv4Address(intf->GetAddressRange().mask));
                                        stubLink.setLinkCost(intf->GetOutputCost());
                                        stubLink.setNumberOfTOS(0);
                                        stubLink.setTosDataArraySize(0);

                                        unsigned short linkIndex = routerLSA->getLinksArraySize();
                                        routerLSA->setLinksArraySize(linkIndex + 1);
                                        routerLSA->setNumberOfLinks(linkIndex + 1);
                                        routerLSA->setLinks(linkIndex, stubLink);
                                    }
                                }
                            }
                        }
                    }
                    break;
                case OSPF::Interface::Broadcast:
                case OSPF::Interface::NBMA:
                    {
                        if (intf->GetState() == OSPF::Interface::WaitingState) {
                            Link stubLink;
                            stubLink.setType(StubLink);
                            stubLink.setLinkID(ULongFromIPv4Address(intf->GetAddressRange().address &
                                                                      intf->GetAddressRange().mask));
                            stubLink.setLinkData(ULongFromIPv4Address(intf->GetAddressRange().mask));
                            stubLink.setLinkCost(intf->GetOutputCost());
                            stubLink.setNumberOfTOS(0);
                            stubLink.setTosDataArraySize(0);

                            unsigned short linkIndex = routerLSA->getLinksArraySize();
                            routerLSA->setLinksArraySize(linkIndex + 1);
                            routerLSA->setNumberOfLinks(linkIndex + 1);
                            routerLSA->setLinks(linkIndex, stubLink);
                        } else {
                            OSPF::Neighbor* dRouter = intf->GetNeighborByAddress(intf->GetDesignatedRouter().ipInterfaceAddress);
                            if (((dRouter != NULL) && (dRouter->GetState() == OSPF::Neighbor::FullState)) ||
                                ((intf->GetDesignatedRouter().routerID == parentRouter->GetRouterID()) &&
                                 (intf->HasAnyNeighborInStates(OSPF::Neighbor::FullState))))
                            {
                                Link link;
                                link.setType(TransitLink);
                                link.setLinkID(ULongFromIPv4Address(intf->GetDesignatedRouter().ipInterfaceAddress));
                                link.setLinkData(ULongFromIPv4Address(intf->GetAddressRange().address));
                                link.setLinkCost(intf->GetOutputCost());
                                link.setNumberOfTOS(0);
                                link.setTosDataArraySize(0);

                                unsigned short linkIndex = routerLSA->getLinksArraySize();
                                routerLSA->setLinksArraySize(linkIndex + 1);
                                routerLSA->setNumberOfLinks(linkIndex + 1);
                                routerLSA->setLinks(linkIndex, link);
                            } else {
                                Link stubLink;
                                stubLink.setType(StubLink);
                                stubLink.setLinkID(ULongFromIPv4Address(intf->GetAddressRange().address &
                                                                          intf->GetAddressRange().mask));
                                stubLink.setLinkData(ULongFromIPv4Address(intf->GetAddressRange().mask));
                                stubLink.setLinkCost(intf->GetOutputCost());
                                stubLink.setNumberOfTOS(0);
                                stubLink.setTosDataArraySize(0);

                                unsigned short linkIndex = routerLSA->getLinksArraySize();
                                routerLSA->setLinksArraySize(linkIndex + 1);
                                routerLSA->setNumberOfLinks(linkIndex + 1);
                                routerLSA->setLinks(linkIndex, stubLink);
                            }
                        }
                    }
                    break;
                case OSPF::Interface::Virtual:
                    {
                        OSPF::Neighbor* neighbor = (intf->GetNeighborCount() > 0) ? intf->GetNeighbor(0) : NULL;
                        if ((neighbor != NULL) && (neighbor->GetState() == OSPF::Neighbor::FullState)) {
                            Link link;
                            link.setType(VirtualLink);
                            link.setLinkID(neighbor->GetNeighborID());
                            link.setLinkData(ULongFromIPv4Address(intf->GetAddressRange().address));
                            link.setLinkCost(intf->GetOutputCost());
                            link.setNumberOfTOS(0);
                            link.setTosDataArraySize(0);

                            unsigned short linkIndex = routerLSA->getLinksArraySize();
                            routerLSA->setLinksArraySize(linkIndex + 1);
                            routerLSA->setNumberOfLinks(linkIndex + 1);
                            routerLSA->setLinks(linkIndex, link);
                        }
                    }
                    break;
                case OSPF::Interface::PointToMultiPoint:
                    {
                        Link stubLink;
                        stubLink.setType(StubLink);
                        stubLink.setLinkID(ULongFromIPv4Address(intf->GetAddressRange().address));
                        stubLink.setLinkData(0xFFFFFFFF);
                        stubLink.setLinkCost(0);
                        stubLink.setNumberOfTOS(0);
                        stubLink.setTosDataArraySize(0);

                        unsigned short linkIndex = routerLSA->getLinksArraySize();
                        routerLSA->setLinksArraySize(linkIndex + 1);
                        routerLSA->setNumberOfLinks(linkIndex + 1);
                        routerLSA->setLinks(linkIndex, stubLink);

                        long neighborCount = intf->GetNeighborCount();
                        for (long i = 0; i < neighborCount; i++) {
                            OSPF::Neighbor* neighbor = intf->GetNeighbor(i);
                            if (neighbor->GetState() == OSPF::Neighbor::FullState) {
                                Link link;
                                link.setType(PointToPointLink);
                                link.setLinkID(neighbor->GetNeighborID());
                                link.setLinkData(ULongFromIPv4Address(intf->GetAddressRange().address));
                                link.setLinkCost(intf->GetOutputCost());
                                link.setNumberOfTOS(0);
                                link.setTosDataArraySize(0);

                                unsigned short linkIndex = routerLSA->getLinksArraySize();
                                routerLSA->setLinksArraySize(linkIndex + 1);
                                routerLSA->setNumberOfLinks(linkIndex + 1);
                                routerLSA->setLinks(linkIndex, stubLink);
                            }
                        }
                    }
                    break;
                default: break;
            }
        }
    }

    long hostRouteCount = hostRoutes.size();
    for (i = 0; i < hostRouteCount; i++) {
        Link stubLink;
        stubLink.setType(StubLink);
        stubLink.setLinkID(ULongFromIPv4Address(hostRoutes[i].address));
        stubLink.setLinkData(0xFFFFFFFF);
        stubLink.setLinkCost(hostRoutes[i].linkCost);
        stubLink.setNumberOfTOS(0);
        stubLink.setTosDataArraySize(0);

        unsigned short linkIndex = routerLSA->getLinksArraySize();
        routerLSA->setLinksArraySize(linkIndex + 1);
        routerLSA->setNumberOfLinks(linkIndex + 1);
        routerLSA->setLinks(linkIndex, stubLink);
    }

    lsaHeader.setLsChecksum(0);    // TODO: calculate correct LS checksum

    routerLSA->SetSource(OSPF::LSATrackingInfo::Originated);

    return routerLSA;
}

OSPF::NetworkLSA* OSPF::Area::OriginateNetworkLSA(const OSPF::Interface* intf)
{
    if (intf->HasAnyNeighborInStates(OSPF::Neighbor::FullState)) {
        OSPF::NetworkLSA* networkLSA      = new OSPF::NetworkLSA;
        OSPFLSAHeader&   lsaHeader        = networkLSA->getHeader();
        long             neighborCount    = intf->GetNeighborCount();
        OSPFOptions      lsOptions;

        lsaHeader.setLsAge(0);
        memset(&lsOptions, 0, sizeof(OSPFOptions));
        lsOptions.E_ExternalRoutingCapability = externalRoutingCapability;
        lsaHeader.setLsOptions(lsOptions);
        lsaHeader.setLsType(NetworkLSAType);
        lsaHeader.setLinkStateID(ULongFromIPv4Address(intf->GetAddressRange().address));
        lsaHeader.setAdvertisingRouter(parentRouter->GetRouterID());
        lsaHeader.setLsSequenceNumber(INITIAL_SEQUENCE_NUMBER);

        networkLSA->setNetworkMask(ULongFromIPv4Address(intf->GetAddressRange().mask));

        for (long j = 0; j < neighborCount; j++) {
            const OSPF::Neighbor* neighbor = intf->GetNeighbor(j);
            if (neighbor->GetState() == OSPF::Neighbor::FullState) {
                unsigned short netIndex = networkLSA->getAttachedRoutersArraySize();
                networkLSA->setAttachedRoutersArraySize(netIndex + 1);
                networkLSA->setAttachedRouters(netIndex, neighbor->GetNeighborID());
            }
        }
        unsigned short netIndex = networkLSA->getAttachedRoutersArraySize();
        networkLSA->setAttachedRoutersArraySize(netIndex + 1);
        networkLSA->setAttachedRouters(netIndex, parentRouter->GetRouterID());

        lsaHeader.setLsChecksum(0);    // TODO: calculate correct LS checksum

        return networkLSA;
    } else {
        return NULL;
    }
}

/**
 * Returns a link state ID for the input destination.
 * If this router hasn't originated a Summary LSA for the input destination then
 * the function returs the destination address as link state ID. If it has originated
 * a Summary LSA for the input destination then the function checks which LSA would
 * contain the longer netmask. If the two masks are equal then this means thet we're
 * updating an LSA already in the database, so the function returns the destination
 * address as link state ID. If the input destination netmask is longer then the
 * one already in the database, then the returned link state ID is the input
 * destination address ORed together with the inverse of the input destination mask.
 * If the input destination netmask is shorter, then the Summary LSA already in the
 * database has to be replaced by the current destination. In this case the
 * lsaToReoriginate parameter is filled with a copy of the Summary LSA in the database
 * with it's mask replaced by the destination mask and the cost replaced by the input
 * destination cost; the returned link state ID is the input destination address ORed
 * together with the inverse of the mask stored in the Summary LSA in the database.
 * This means that if the lsaToReoriginate parameter is not NULL on return then another
 * lookup in the database is needed with the same LSAKey as used here(input
 * destination address and the router's own routerID) and the resulting Summary LSA's
 * link state ID should be changed to the one returned by this function.
 */
OSPF::LinkStateID OSPF::Area::GetUniqueLinkStateID(OSPF::IPv4AddressRange destination,
                                                    OSPF::Metric destinationCost,
                                                    OSPF::SummaryLSA*& lsaToReoriginate) const
{
    if (lsaToReoriginate != NULL) {
        delete lsaToReoriginate;
        lsaToReoriginate = NULL;
    }

    OSPF::LSAKeyType lsaKey;

    lsaKey.linkStateID = ULongFromIPv4Address(destination.address);
    lsaKey.advertisingRouter = parentRouter->GetRouterID();

    const OSPF::SummaryLSA* foundLSA = FindSummaryLSA(lsaKey);

    if (foundLSA == NULL) {
        return lsaKey.linkStateID;
    } else {
        OSPF::IPv4Address existingMask = IPv4AddressFromULong(foundLSA->getNetworkMask().getInt());

        if (destination.mask == existingMask) {
            return lsaKey.linkStateID;
        } else {
            if (destination.mask >= existingMask) {
                return (lsaKey.linkStateID | (~(ULongFromIPv4Address(destination.mask))));
            } else {
                OSPF::SummaryLSA* summaryLSA = new OSPF::SummaryLSA(*foundLSA);

                long sequenceNumber = summaryLSA->getHeader().getLsSequenceNumber();

                summaryLSA->getHeader().setLsAge(0);
                summaryLSA->getHeader().setLsSequenceNumber((sequenceNumber == MAX_SEQUENCE_NUMBER) ? INITIAL_SEQUENCE_NUMBER : sequenceNumber + 1);
                summaryLSA->setNetworkMask(ULongFromIPv4Address(destination.mask));
                summaryLSA->setRouteCost(destinationCost);
                summaryLSA->getHeader().setLsChecksum(0);    // TODO: calculate correct LS checksum

                lsaToReoriginate = summaryLSA;

                return (lsaKey.linkStateID | (~(ULongFromIPv4Address(existingMask))));
            }
        }
    }
}

OSPF::SummaryLSA* OSPF::Area::OriginateSummaryLSA(const OSPF::RoutingTableEntry* entry,
                                                   const std::map<OSPF::LSAKeyType, bool, OSPF::LSAKeyType_Less>& originatedLSAs,
                                                   OSPF::SummaryLSA*& lsaToReoriginate)
{
    if (((entry->GetDestinationType() & OSPF::RoutingTableEntry::AreaBorderRouterDestination) != 0) ||
        (entry->GetPathType() == OSPF::RoutingTableEntry::Type1External) ||
        (entry->GetPathType() == OSPF::RoutingTableEntry::Type2External) ||
        (entry->GetArea() == areaID))
    {
        return NULL;
    }

    bool         allNextHopsInThisArea = true;
    unsigned int nextHopCount          = entry->GetNextHopCount();

    for (unsigned int i = 0; i < nextHopCount; i++) {
        OSPF::Interface* nextHopInterface = parentRouter->GetNonVirtualInterface(entry->GetNextHop(i).ifIndex);
        if ((nextHopInterface != NULL) && (nextHopInterface->GetAreaID() != areaID)) {
            allNextHopsInThisArea = false;
            break;
        }
    }
    if ((allNextHopsInThisArea) || (entry->GetCost() >= LS_INFINITY)){
        return NULL;
    }

    if ((entry->GetDestinationType() & OSPF::RoutingTableEntry::ASBoundaryRouterDestination) != 0) {
        OSPF::RoutingTableEntry* preferredEntry = parentRouter->GetPreferredEntry(*(entry->GetLinkStateOrigin()), false);
        if ((preferredEntry != NULL) && (*preferredEntry == *entry) && (externalRoutingCapability)) {
            OSPF::SummaryLSA* summaryLSA    = new OSPF::SummaryLSA;
            OSPFLSAHeader&    lsaHeader     = summaryLSA->getHeader();
            OSPFOptions       lsOptions;

            lsaHeader.setLsAge(0);
            memset(&lsOptions, 0, sizeof(OSPFOptions));
            lsOptions.E_ExternalRoutingCapability = externalRoutingCapability;
            lsaHeader.setLsOptions(lsOptions);
            lsaHeader.setLsType(SummaryLSA_ASBoundaryRoutersType);
            lsaHeader.setLinkStateID(entry->GetDestinationID().getInt());
            lsaHeader.setAdvertisingRouter(parentRouter->GetRouterID());
            lsaHeader.setLsSequenceNumber(INITIAL_SEQUENCE_NUMBER);

            summaryLSA->setNetworkMask(entry->GetAddressMask());
            summaryLSA->setRouteCost(entry->GetCost());
            summaryLSA->setTosDataArraySize(0);

            lsaHeader.setLsChecksum(0);    // TODO: calculate correct LS checksum

            summaryLSA->SetSource(OSPF::LSATrackingInfo::Originated);

            return summaryLSA;
        }
    } else {    // entry->GetDestinationType() == OSPF::RoutingTableEntry::NetworkDestination
        if (entry->GetPathType() == OSPF::RoutingTableEntry::InterArea) {
            OSPF::IPv4AddressRange destinationRange;

            destinationRange.address = IPv4AddressFromULong(entry->GetDestinationID().getInt());
            destinationRange.mask = IPv4AddressFromULong(entry->GetAddressMask().getInt());

            OSPF::LinkStateID newLinkStateID = GetUniqueLinkStateID(destinationRange, entry->GetCost(), lsaToReoriginate);

            if (lsaToReoriginate != NULL) {
                OSPF::LSAKeyType lsaKey;

                lsaKey.linkStateID = entry->GetDestinationID().getInt();
                lsaKey.advertisingRouter = parentRouter->GetRouterID();

                std::map<OSPF::LSAKeyType, OSPF::SummaryLSA*, OSPF::LSAKeyType_Less>::iterator lsaIt = summaryLSAsByID.find(lsaKey);
                if (lsaIt == summaryLSAsByID.end()) {
                    delete(lsaToReoriginate);
                    lsaToReoriginate = NULL;
                    return NULL;
                } else {
                    OSPF::SummaryLSA* summaryLSA = new OSPF::SummaryLSA(*(lsaIt->second));
                    OSPFLSAHeader&    lsaHeader  = summaryLSA->getHeader();

                    lsaHeader.setLsAge(0);
                    lsaHeader.setLsSequenceNumber(INITIAL_SEQUENCE_NUMBER);
                    lsaHeader.setLinkStateID(newLinkStateID);
                    lsaHeader.setLsChecksum(0);    // TODO: calculate correct LS checksum

                    return summaryLSA;
                }
            } else {
                OSPF::SummaryLSA* summaryLSA    = new OSPF::SummaryLSA;
                OSPFLSAHeader&    lsaHeader     = summaryLSA->getHeader();
                OSPFOptions       lsOptions;

                lsaHeader.setLsAge(0);
                memset(&lsOptions, 0, sizeof(OSPFOptions));
                lsOptions.E_ExternalRoutingCapability = externalRoutingCapability;
                lsaHeader.setLsOptions(lsOptions);
                lsaHeader.setLsType(SummaryLSA_NetworksType);
                lsaHeader.setLinkStateID(newLinkStateID);
                lsaHeader.setAdvertisingRouter(parentRouter->GetRouterID());
                lsaHeader.setLsSequenceNumber(INITIAL_SEQUENCE_NUMBER);

                summaryLSA->setNetworkMask(entry->GetAddressMask());
                summaryLSA->setRouteCost(entry->GetCost());
                summaryLSA->setTosDataArraySize(0);

                lsaHeader.setLsChecksum(0);    // TODO: calculate correct LS checksum

                summaryLSA->SetSource(OSPF::LSATrackingInfo::Originated);

                return summaryLSA;
            }
        } else {    // entry->GetPathType() == OSPF::RoutingTableEntry::IntraArea
            OSPF::IPv4AddressRange destinationAddressRange;

            destinationAddressRange.address = IPv4AddressFromULong(entry->GetDestinationID().getInt());
            destinationAddressRange.mask = IPv4AddressFromULong(entry->GetAddressMask().getInt());

            bool doAdvertise = false;
            OSPF::IPv4AddressRange containingAddressRange = parentRouter->GetContainingAddressRange(destinationAddressRange, &doAdvertise);
            if (((entry->GetArea() == OSPF::BackboneAreaID) &&         // the backbone's configured ranges should be ignored
                 (transitCapability)) ||                                // when originating Summary LSAs into transit areas
                (containingAddressRange == OSPF::NullIPv4AddressRange))
            {
                OSPF::LinkStateID newLinkStateID = GetUniqueLinkStateID(destinationAddressRange, entry->GetCost(), lsaToReoriginate);

                if (lsaToReoriginate != NULL) {
                    OSPF::LSAKeyType lsaKey;

                    lsaKey.linkStateID = entry->GetDestinationID().getInt();
                    lsaKey.advertisingRouter = parentRouter->GetRouterID();

                    std::map<OSPF::LSAKeyType, OSPF::SummaryLSA*, OSPF::LSAKeyType_Less>::iterator lsaIt = summaryLSAsByID.find(lsaKey);
                    if (lsaIt == summaryLSAsByID.end()) {
                        delete(lsaToReoriginate);
                        lsaToReoriginate = NULL;
                        return NULL;
                    } else {
                        OSPF::SummaryLSA* summaryLSA = new OSPF::SummaryLSA(*(lsaIt->second));
                        OSPFLSAHeader&    lsaHeader  = summaryLSA->getHeader();

                        lsaHeader.setLsAge(0);
                        lsaHeader.setLsSequenceNumber(INITIAL_SEQUENCE_NUMBER);
                        lsaHeader.setLinkStateID(newLinkStateID);
                        lsaHeader.setLsChecksum(0);    // TODO: calculate correct LS checksum

                        return summaryLSA;
                    }
                } else {
                    OSPF::SummaryLSA* summaryLSA    = new OSPF::SummaryLSA;
                    OSPFLSAHeader&    lsaHeader     = summaryLSA->getHeader();
                    OSPFOptions       lsOptions;

                    lsaHeader.setLsAge(0);
                    memset(&lsOptions, 0, sizeof(OSPFOptions));
                    lsOptions.E_ExternalRoutingCapability = externalRoutingCapability;
                    lsaHeader.setLsOptions(lsOptions);
                    lsaHeader.setLsType(SummaryLSA_NetworksType);
                    lsaHeader.setLinkStateID(newLinkStateID);
                    lsaHeader.setAdvertisingRouter(parentRouter->GetRouterID());
                    lsaHeader.setLsSequenceNumber(INITIAL_SEQUENCE_NUMBER);

                    summaryLSA->setNetworkMask(entry->GetAddressMask());
                    summaryLSA->setRouteCost(entry->GetCost());
                    summaryLSA->setTosDataArraySize(0);

                    lsaHeader.setLsChecksum(0);    // TODO: calculate correct LS checksum

                    summaryLSA->SetSource(OSPF::LSATrackingInfo::Originated);

                    return summaryLSA;
                }
            } else {
                if (doAdvertise) {
                    Metric        maxRangeCost = 0;
                    unsigned long entryCount   = parentRouter->GetRoutingTableEntryCount();

                    for (unsigned long i = 0; i < entryCount; i++) {
                        const OSPF::RoutingTableEntry* routingEntry = parentRouter->GetRoutingTableEntry(i);

                        if ((routingEntry->GetDestinationType() == OSPF::RoutingTableEntry::NetworkDestination) &&
                            (routingEntry->GetPathType() == OSPF::RoutingTableEntry::IntraArea) &&
                            ((routingEntry->GetDestinationID().getInt() & routingEntry->GetAddressMask().getInt() & ULongFromIPv4Address(containingAddressRange.mask)) ==
                             ULongFromIPv4Address(containingAddressRange.address & containingAddressRange.mask)) &&
                            (routingEntry->GetCost() > maxRangeCost))
                        {
                            maxRangeCost = routingEntry->GetCost();
                        }
                    }

                    OSPF::LinkStateID newLinkStateID = GetUniqueLinkStateID(containingAddressRange, maxRangeCost, lsaToReoriginate);
                    OSPF::LSAKeyType  lsaKey;

                    if (lsaToReoriginate != NULL) {
                        lsaKey.linkStateID = lsaToReoriginate->getHeader().getLinkStateID();
                        lsaKey.advertisingRouter = parentRouter->GetRouterID();

                        std::map<OSPF::LSAKeyType, bool, OSPF::LSAKeyType_Less>::const_iterator originatedIt = originatedLSAs.find(lsaKey);
                        if (originatedIt != originatedLSAs.end()) {
                            delete(lsaToReoriginate);
                            lsaToReoriginate = NULL;
                            return NULL;
                        }

                        lsaKey.linkStateID = entry->GetDestinationID().getInt();
                        lsaKey.advertisingRouter = parentRouter->GetRouterID();

                        std::map<OSPF::LSAKeyType, OSPF::SummaryLSA*, OSPF::LSAKeyType_Less>::iterator lsaIt = summaryLSAsByID.find(lsaKey);
                        if (lsaIt == summaryLSAsByID.end()) {
                            delete(lsaToReoriginate);
                            lsaToReoriginate = NULL;
                            return NULL;
                        }

                        OSPF::SummaryLSA* summaryLSA = new OSPF::SummaryLSA(*(lsaIt->second));
                        OSPFLSAHeader&    lsaHeader  = summaryLSA->getHeader();

                        lsaHeader.setLsAge(0);
                        lsaHeader.setLsSequenceNumber(INITIAL_SEQUENCE_NUMBER);
                        lsaHeader.setLinkStateID(newLinkStateID);
                        lsaHeader.setLsChecksum(0);    // TODO: calculate correct LS checksum

                        return summaryLSA;
                    } else {
                        lsaKey.linkStateID = newLinkStateID;
                        lsaKey.advertisingRouter = parentRouter->GetRouterID();

                        std::map<OSPF::LSAKeyType, bool, OSPF::LSAKeyType_Less>::const_iterator originatedIt = originatedLSAs.find(lsaKey);
                        if (originatedIt != originatedLSAs.end()) {
                            return NULL;
                        }

                        OSPF::SummaryLSA* summaryLSA    = new OSPF::SummaryLSA;
                        OSPFLSAHeader&    lsaHeader     = summaryLSA->getHeader();
                        OSPFOptions       lsOptions;

                        lsaHeader.setLsAge(0);
                        memset(&lsOptions, 0, sizeof(OSPFOptions));
                        lsOptions.E_ExternalRoutingCapability = externalRoutingCapability;
                        lsaHeader.setLsOptions(lsOptions);
                        lsaHeader.setLsType(SummaryLSA_NetworksType);
                        lsaHeader.setLinkStateID(newLinkStateID);
                        lsaHeader.setAdvertisingRouter(parentRouter->GetRouterID());
                        lsaHeader.setLsSequenceNumber(INITIAL_SEQUENCE_NUMBER);

                        summaryLSA->setNetworkMask(entry->GetAddressMask());
                        summaryLSA->setRouteCost(entry->GetCost());
                        summaryLSA->setTosDataArraySize(0);

                        lsaHeader.setLsChecksum(0);    // TODO: calculate correct LS checksum

                        summaryLSA->SetSource(OSPF::LSATrackingInfo::Originated);

                        return summaryLSA;
                    }
                }
            }
        }
    }

    return NULL;
}

OSPF::SummaryLSA* OSPF::Area::OriginateSummaryLSA(const OSPF::SummaryLSA* summaryLSA)
{
    const std::map<OSPF::LSAKeyType, bool, OSPF::LSAKeyType_Less> emptyMap;
    OSPF::SummaryLSA*                                             dontReoriginate = NULL;

    const OSPFLSAHeader& lsaHeader   = summaryLSA->getHeader();
    unsigned long   entryCount = parentRouter->GetRoutingTableEntryCount();

    for (unsigned long i = 0; i < entryCount; i++) {
        const OSPF::RoutingTableEntry* entry = parentRouter->GetRoutingTableEntry(i);

        if ((lsaHeader.getLsType() == SummaryLSA_ASBoundaryRoutersType) &&
            ((((entry->GetDestinationType() & OSPF::RoutingTableEntry::AreaBorderRouterDestination) != 0) ||
              ((entry->GetDestinationType() & OSPF::RoutingTableEntry::ASBoundaryRouterDestination) != 0)) &&
             ((entry->GetDestinationID().getInt() == lsaHeader.getLinkStateID()) &&
              (entry->GetAddressMask() == summaryLSA->getNetworkMask()))))
        {
            OSPF::SummaryLSA* returnLSA = OriginateSummaryLSA(entry, emptyMap, dontReoriginate);
            if (dontReoriginate != NULL) {
                delete dontReoriginate;
            }
            return returnLSA;
        }

        unsigned long lsaMask = summaryLSA->getNetworkMask().getInt();

        if ((lsaHeader.getLsType() == SummaryLSA_NetworksType) &&
            (entry->GetDestinationType() == OSPF::RoutingTableEntry::NetworkDestination) &&
            (entry->GetAddressMask().getInt() == lsaMask) &&
            ((entry->GetDestinationID().getInt() & lsaMask) == (lsaHeader.getLinkStateID() & lsaMask)))
        {
            OSPF::SummaryLSA* returnLSA = OriginateSummaryLSA(entry, emptyMap, dontReoriginate);
            if (dontReoriginate != NULL) {
                delete dontReoriginate;
            }
            return returnLSA;
        }
    }

    return NULL;
}

void OSPF::Area::CalculateShortestPathTree(std::vector<OSPF::RoutingTableEntry*>& newRoutingTable)
{
    OSPF::RouterID          routerID = parentRouter->GetRouterID();
    bool                    finished = false;
    std::vector<OSPFLSA*>   treeVertices;
    OSPFLSA*                justAddedVertex;
    std::vector<OSPFLSA*>   candidateVertices;
    unsigned long            i, j, k;
    unsigned long            lsaCount;

    if (spfTreeRoot == NULL) {
        OSPF::RouterLSA* newLSA = OriginateRouterLSA();

        InstallRouterLSA(newLSA);

        OSPF::RouterLSA* routerLSA = FindRouterLSA(routerID);

        spfTreeRoot = routerLSA;
        FloodLSA(newLSA);
        delete newLSA;
    }
    if (spfTreeRoot == NULL) {
        return;
    }

    lsaCount = routerLSAs.size();
    for (i = 0; i < lsaCount; i++) {
        routerLSAs[i]->ClearNextHops();
    }
    lsaCount = networkLSAs.size();
    for (i = 0; i < lsaCount; i++) {
        networkLSAs[i]->ClearNextHops();
    }
    spfTreeRoot->SetDistance(0);
    treeVertices.push_back(spfTreeRoot);
    justAddedVertex = spfTreeRoot;          // (1)

    do {
        LSAType vertexType = static_cast<LSAType> (justAddedVertex->getHeader().getLsType());

        if ((vertexType == RouterLSAType)) {
            OSPF::RouterLSA* routerVertex = check_and_cast<OSPF::RouterLSA*> (justAddedVertex);
            if (routerVertex->getV_VirtualLinkEndpoint()) {    // (2)
                transitCapability = true;
            }

            unsigned int linkCount = routerVertex->getLinksArraySize();
            for (i = 0; i < linkCount; i++) {
                Link&    link     = routerVertex->getLinks(i);
                LinkType linkType = static_cast<LinkType> (link.getType());
                OSPFLSA* joiningVertex;
                LSAType  joiningVertexType;

                if (linkType == StubLink) {     // (2) (a)
                    continue;
                }

                if (linkType == TransitLink) {
                    joiningVertex     = FindNetworkLSA(link.getLinkID().getInt());
                    joiningVertexType = NetworkLSAType;
                } else {
                    joiningVertex     = FindRouterLSA(link.getLinkID().getInt());
                    joiningVertexType = RouterLSAType;
                }

                if ((joiningVertex == NULL) ||
                    (joiningVertex->getHeader().getLsAge() == MAX_AGE) ||
                    (!HasLink(joiningVertex, justAddedVertex)))  // (from, to)     (2) (b)
                {
                    continue;
                }

                unsigned int treeSize      = treeVertices.size();
                bool         alreadyOnTree = false;

                for (j = 0; j < treeSize; j++) {
                    if (treeVertices[j] == joiningVertex) {
                        alreadyOnTree = true;
                        break;
                    }
                }
                if (alreadyOnTree) {    // (2) (c)
                    continue;
                }

                unsigned long linkStateCost  = routerVertex->GetDistance() + link.getLinkCost();
                unsigned int  candidateCount = candidateVertices.size();
                OSPFLSA*      candidate      = NULL;

                for (j = 0; j < candidateCount; j++) {
                    if (candidateVertices[j] == joiningVertex) {
                        candidate = candidateVertices[j];
                    }
                }
                if (candidate != NULL) {    // (2) (d)
                    OSPF::RoutingInfo* routingInfo       = check_and_cast<OSPF::RoutingInfo*> (candidate);
                    unsigned long      candidateDistance = routingInfo->GetDistance();

                    if (linkStateCost > candidateDistance) {
                        continue;
                    }
                    if (linkStateCost < candidateDistance) {
                        routingInfo->SetDistance(linkStateCost);
                        routingInfo->ClearNextHops();
                    }
                    std::vector<OSPF::NextHop>* newNextHops = CalculateNextHops(joiningVertex, justAddedVertex); // (destination, parent)
                    unsigned int nextHopCount = newNextHops->size();
                    for (k = 0; k < nextHopCount; k++) {
                        routingInfo->AddNextHop((*newNextHops)[k]);
                    }
                    delete newNextHops;
                } else {
                    if (joiningVertexType == RouterLSAType) {
                        OSPF::RouterLSA* joiningRouterVertex = check_and_cast<OSPF::RouterLSA*> (joiningVertex);
                        joiningRouterVertex->SetDistance(linkStateCost);
                        std::vector<OSPF::NextHop>* newNextHops = CalculateNextHops(joiningVertex, justAddedVertex); // (destination, parent)
                        unsigned int nextHopCount = newNextHops->size();
                        for (k = 0; k < nextHopCount; k++) {
                            joiningRouterVertex->AddNextHop((*newNextHops)[k]);
                        }
                        delete newNextHops;
                        OSPF::RoutingInfo* vertexRoutingInfo = check_and_cast<OSPF::RoutingInfo*> (joiningRouterVertex);
                        vertexRoutingInfo->SetParent(justAddedVertex);

                        candidateVertices.push_back(joiningRouterVertex);
                    } else {
                        OSPF::NetworkLSA* joiningNetworkVertex = check_and_cast<OSPF::NetworkLSA*> (joiningVertex);
                        joiningNetworkVertex->SetDistance(linkStateCost);
                        std::vector<OSPF::NextHop>* newNextHops = CalculateNextHops(joiningVertex, justAddedVertex); // (destination, parent)
                        unsigned int nextHopCount = newNextHops->size();
                        for (k = 0; k < nextHopCount; k++) {
                            joiningNetworkVertex->AddNextHop((*newNextHops)[k]);
                        }
                        delete newNextHops;
                        OSPF::RoutingInfo* vertexRoutingInfo = check_and_cast<OSPF::RoutingInfo*> (joiningNetworkVertex);
                        vertexRoutingInfo->SetParent(justAddedVertex);

                        candidateVertices.push_back(joiningNetworkVertex);
                    }
                }
            }
        }

        if ((vertexType == NetworkLSAType)) {
            OSPF::NetworkLSA* networkVertex = check_and_cast<OSPF::NetworkLSA*> (justAddedVertex);
            unsigned int      routerCount   = networkVertex->getAttachedRoutersArraySize();

            for (i = 0; i < routerCount; i++) {     // (2)
                OSPF::RouterLSA* joiningVertex = FindRouterLSA(networkVertex->getAttachedRouters(i).getInt());
                if ((joiningVertex == NULL) ||
                    (joiningVertex->getHeader().getLsAge() == MAX_AGE) ||
                    (!HasLink(joiningVertex, justAddedVertex)))  // (from, to)     (2) (b)
                {
                    continue;
                }

                unsigned int treeSize      = treeVertices.size();
                bool         alreadyOnTree = false;

                for (j = 0; j < treeSize; j++) {
                    if (treeVertices[j] == joiningVertex) {
                        alreadyOnTree = true;
                        break;
                    }
                }
                if (alreadyOnTree) {    // (2) (c)
                    continue;
                }

                unsigned long linkStateCost  = networkVertex->GetDistance();   // link cost from network to router is always 0
                unsigned int  candidateCount = candidateVertices.size();
                OSPFLSA*      candidate      = NULL;

                for (j = 0; j < candidateCount; j++) {
                    if (candidateVertices[j] == joiningVertex) {
                        candidate = candidateVertices[j];
                    }
                }
                if (candidate != NULL) {    // (2) (d)
                    OSPF::RoutingInfo* routingInfo       = check_and_cast<OSPF::RoutingInfo*> (candidate);
                    unsigned long      candidateDistance = routingInfo->GetDistance();

                    if (linkStateCost > candidateDistance) {
                        continue;
                    }
                    if (linkStateCost < candidateDistance) {
                        routingInfo->SetDistance(linkStateCost);
                        routingInfo->ClearNextHops();
                    }
                    std::vector<OSPF::NextHop>* newNextHops = CalculateNextHops(joiningVertex, justAddedVertex); // (destination, parent)
                    unsigned int nextHopCount = newNextHops->size();
                    for (k = 0; k < nextHopCount; k++) {
                        routingInfo->AddNextHop((*newNextHops)[k]);
                    }
                    delete newNextHops;
                } else {
                    joiningVertex->SetDistance(linkStateCost);
                    std::vector<OSPF::NextHop>* newNextHops = CalculateNextHops(joiningVertex, justAddedVertex); // (destination, parent)
                    unsigned int nextHopCount = newNextHops->size();
                    for (k = 0; k < nextHopCount; k++) {
                        joiningVertex->AddNextHop((*newNextHops)[k]);
                    }
                    delete newNextHops;
                    OSPF::RoutingInfo* vertexRoutingInfo = check_and_cast<OSPF::RoutingInfo*> (joiningVertex);
                    vertexRoutingInfo->SetParent(justAddedVertex);

                    candidateVertices.push_back(joiningVertex);
                }
            }
        }

        if (candidateVertices.empty()) {  // (3)
            finished = true;
        } else {
            unsigned int  candidateCount = candidateVertices.size();
            unsigned long minDistance = LS_INFINITY;
            OSPFLSA*      closestVertex = candidateVertices[0];

            for (i = 0; i < candidateCount; i++) {
                OSPF::RoutingInfo* routingInfo     = check_and_cast<OSPF::RoutingInfo*> (candidateVertices[i]);
                unsigned long      currentDistance = routingInfo->GetDistance();

                if (currentDistance < minDistance) {
                    closestVertex = candidateVertices[i];
                    minDistance = currentDistance;
                } else {
                    if (currentDistance == minDistance) {
                        if ((closestVertex->getHeader().getLsType() == RouterLSAType) &&
                            (candidateVertices[i]->getHeader().getLsType() == NetworkLSAType))
                        {
                            closestVertex = candidateVertices[i];
                        }
                    }
                }
            }

            treeVertices.push_back(closestVertex);

            for (std::vector<OSPFLSA*>::iterator it = candidateVertices.begin(); it != candidateVertices.end(); it++) {
                if ((*it) == closestVertex) {
                    candidateVertices.erase(it);
                    break;
                }
            }

            if (closestVertex->getHeader().getLsType() == RouterLSAType) {
                OSPF::RouterLSA* routerLSA = check_and_cast<OSPF::RouterLSA*> (closestVertex);
                if (routerLSA->getB_AreaBorderRouter() || routerLSA->getE_ASBoundaryRouter()) {
                    OSPF::RoutingTableEntry*                        entry           = new OSPF::RoutingTableEntry;
                    OSPF::RouterID                                  destinationID   = routerLSA->getHeader().getLinkStateID();
                    unsigned int                                    nextHopCount    = routerLSA->GetNextHopCount();
                    OSPF::RoutingTableEntry::RoutingDestinationType destinationType = OSPF::RoutingTableEntry::NetworkDestination;

                    entry->SetDestinationID(destinationID);
                    entry->SetLinkStateOrigin(routerLSA);
                    entry->SetArea(areaID);
                    entry->SetPathType(OSPF::RoutingTableEntry::IntraArea);
                    entry->SetCost(routerLSA->GetDistance());
                    if (routerLSA->getB_AreaBorderRouter()) {
                        destinationType |= OSPF::RoutingTableEntry::AreaBorderRouterDestination;
                    }
                    if (routerLSA->getE_ASBoundaryRouter()) {
                        destinationType |= OSPF::RoutingTableEntry::ASBoundaryRouterDestination;
                    }
                    entry->SetDestinationType(destinationType);
                    entry->SetOptionalCapabilities(routerLSA->getHeader().getLsOptions());
                    for (i = 0; i < nextHopCount; i++) {
                        entry->AddNextHop(routerLSA->GetNextHop(i));
                    }

                    newRoutingTable.push_back(entry);

                    OSPF::Area* backbone;
                    if (areaID != OSPF::BackboneAreaID) {
                        backbone = parentRouter->GetArea(OSPF::BackboneAreaID);
                    } else {
                        backbone = this;
                    }
                    if (backbone != NULL) {
                        OSPF::Interface* virtualIntf = backbone->FindVirtualLink(destinationID);
                        if ((virtualIntf != NULL) && (virtualIntf->GetTransitAreaID() == areaID)) {
                            OSPF::IPv4AddressRange range;
                            range.address = GetInterface(routerLSA->GetNextHop(0).ifIndex)->GetAddressRange().address;
                            range.mask    = IPv4AddressFromULong(0xFFFFFFFF);
                            virtualIntf->SetAddressRange(range);
                            virtualIntf->SetIfIndex(routerLSA->GetNextHop(0).ifIndex);
                            virtualIntf->SetOutputCost(routerLSA->GetDistance());
                            OSPF::Neighbor* virtualNeighbor = virtualIntf->GetNeighbor(0);
                            if (virtualNeighbor != NULL) {
                                unsigned int     linkCount   = routerLSA->getLinksArraySize();
                                OSPF::RouterLSA* toRouterLSA = dynamic_cast<OSPF::RouterLSA*> (justAddedVertex);
                                if (toRouterLSA != NULL) {
                                    for (i = 0; i < linkCount; i++) {
                                        Link& link = routerLSA->getLinks(i);

                                        if ((link.getType() == PointToPointLink) &&
                                            (link.getLinkID() == toRouterLSA->getHeader().getLinkStateID()) &&
                                            (virtualIntf->GetState() < OSPF::Interface::WaitingState))
                                        {
                                            virtualNeighbor->SetAddress(IPv4AddressFromULong(link.getLinkData()));
                                            virtualIntf->ProcessEvent(OSPF::Interface::InterfaceUp);
                                            break;
                                        }
                                    }
                                } else {
                                    OSPF::NetworkLSA* toNetworkLSA = dynamic_cast<OSPF::NetworkLSA*> (justAddedVertex);
                                    if (toNetworkLSA != NULL) {
                                        for (i = 0; i < linkCount; i++) {
                                            Link& link = routerLSA->getLinks(i);

                                            if ((link.getType() == TransitLink) &&
                                                (link.getLinkID() == toNetworkLSA->getHeader().getLinkStateID()) &&
                                                (virtualIntf->GetState() < OSPF::Interface::WaitingState))
                                            {
                                                virtualNeighbor->SetAddress(IPv4AddressFromULong(link.getLinkData()));
                                                virtualIntf->ProcessEvent(OSPF::Interface::InterfaceUp);
                                                break;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

            if (closestVertex->getHeader().getLsType() == NetworkLSAType) {
                OSPF::NetworkLSA*        networkLSA    = check_and_cast<OSPF::NetworkLSA*> (closestVertex);
                unsigned long            destinationID = (networkLSA->getHeader().getLinkStateID() & networkLSA->getNetworkMask().getInt());
                unsigned int             nextHopCount  = networkLSA->GetNextHopCount();
                bool                     overWrite     = false;
                OSPF::RoutingTableEntry* entry         = NULL;
                unsigned long            routeCount    = newRoutingTable.size();
                unsigned long            longestMatch  = 0;

                for (i = 0; i < routeCount; i++) {
                    if (newRoutingTable[i]->GetDestinationType() == OSPF::RoutingTableEntry::NetworkDestination) {
                        OSPF::RoutingTableEntry* routingEntry = newRoutingTable[i];
                        unsigned long            entryAddress = routingEntry->GetDestinationID().getInt();
                        unsigned long            entryMask    = routingEntry->GetAddressMask().getInt();

                        if ((entryAddress & entryMask) == (destinationID & entryMask)) {
                            if ((destinationID & entryMask) > longestMatch) {
                                longestMatch = (destinationID & entryMask);
                                entry        = routingEntry;
                            }
                        }
                    }
                }
                if (entry != NULL) {
                    const OSPFLSA* entryOrigin = entry->GetLinkStateOrigin();
                    if ((entry->GetCost() != networkLSA->GetDistance()) ||
                        (entryOrigin->getHeader().getLinkStateID() >= networkLSA->getHeader().getLinkStateID()))
                    {
                        overWrite = true;
                    }
                }

                if ((entry == NULL) || (overWrite)) {
                    if (entry == NULL) {
                        entry = new OSPF::RoutingTableEntry;
                    }

                    entry->SetDestinationID(destinationID);
                    entry->SetAddressMask(networkLSA->getNetworkMask());
                    entry->SetLinkStateOrigin(networkLSA);
                    entry->SetArea(areaID);
                    entry->SetPathType(OSPF::RoutingTableEntry::IntraArea);
                    entry->SetCost(networkLSA->GetDistance());
                    entry->SetDestinationType(OSPF::RoutingTableEntry::NetworkDestination);
                    entry->SetOptionalCapabilities(networkLSA->getHeader().getLsOptions());
                    for (i = 0; i < nextHopCount; i++) {
                        entry->AddNextHop(networkLSA->GetNextHop(i));
                    }

                    if (!overWrite) {
                        newRoutingTable.push_back(entry);
                    }
                }
            }

            justAddedVertex = closestVertex;
        }
    } while (!finished);

    unsigned int treeSize      = treeVertices.size();
    for (i = 0; i < treeSize; i++) {
        OSPF::RouterLSA* routerVertex = dynamic_cast<OSPF::RouterLSA*> (treeVertices[i]);
        if (routerVertex == NULL) {
            continue;
        }

        unsigned int linkCount = routerVertex->getLinksArraySize();
        for (j = 0; j < linkCount; j++) {
            Link&    link     = routerVertex->getLinks(j);
            if (link.getType() != StubLink) {
                continue;
            }

            unsigned long            distance      = routerVertex->GetDistance() + link.getLinkCost();
            unsigned long            destinationID = (link.getLinkID().getInt() & link.getLinkData());
            OSPF::RoutingTableEntry* entry         = NULL;
            unsigned long            routeCount    = newRoutingTable.size();
            unsigned long            longestMatch  = 0;

            for (k = 0; k < routeCount; k++) {
                if (newRoutingTable[k]->GetDestinationType() == OSPF::RoutingTableEntry::NetworkDestination) {
                    OSPF::RoutingTableEntry* routingEntry = newRoutingTable[k];
                    unsigned long            entryAddress = routingEntry->GetDestinationID().getInt();
                    unsigned long            entryMask    = routingEntry->GetAddressMask().getInt();

                    if ((entryAddress & entryMask) == (destinationID & entryMask)) {
                        if ((destinationID & entryMask) > longestMatch) {
                            longestMatch = (destinationID & entryMask);
                            entry        = routingEntry;
                        }
                    }
                }
            }

            if (entry != NULL) {
                Metric entryCost = entry->GetCost();

                if (distance > entryCost) {
                    continue;
                }
                if (distance < entryCost) {
                    //FIXME remove
                    //if(parentRouter->GetRouterID() == 0xC0A80302) {
                    //    EV << "CHEAPER STUB LINK FOUND TO " << IPAddress(destinationID).str() << "\n";
                    //}
                    entry->SetCost(distance);
                    entry->ClearNextHops();
                    entry->SetLinkStateOrigin(routerVertex);
                }
                if (distance == entryCost) {
                    // no const version from check_and_cast
                    OSPF::RouterLSA* routerOrigin = check_and_cast<OSPF::RouterLSA*> (const_cast<OSPFLSA*> (entry->GetLinkStateOrigin()));
                    if (routerOrigin->getHeader().getLinkStateID() < routerVertex->getHeader().getLinkStateID()) {
                        entry->SetLinkStateOrigin(routerVertex);
                    }
                }
                std::vector<OSPF::NextHop>* newNextHops = CalculateNextHops(link, routerVertex); // (destination, parent)
                unsigned int nextHopCount = newNextHops->size();
                for (k = 0; k < nextHopCount; k++) {
                    entry->AddNextHop((*newNextHops)[k]);
                }
                delete newNextHops;
            } else {
                //FIXME remove
                //if(parentRouter->GetRouterID() == 0xC0A80302) {
                //    EV << "STUB LINK FOUND TO " << IPAddress(destinationID).str() << "\n";
                //}
                entry = new OSPF::RoutingTableEntry;

                entry->SetDestinationID(destinationID);
                entry->SetAddressMask(link.getLinkData());
                entry->SetLinkStateOrigin(routerVertex);
                entry->SetArea(areaID);
                entry->SetPathType(OSPF::RoutingTableEntry::IntraArea);
                entry->SetCost(distance);
                entry->SetDestinationType(OSPF::RoutingTableEntry::NetworkDestination);
                entry->SetOptionalCapabilities(routerVertex->getHeader().getLsOptions());
                std::vector<OSPF::NextHop>* newNextHops = CalculateNextHops(link, routerVertex); // (destination, parent)
                unsigned int nextHopCount = newNextHops->size();
                for (k = 0; k < nextHopCount; k++) {
                    entry->AddNextHop((*newNextHops)[k]);
                }
                delete newNextHops;

                newRoutingTable.push_back(entry);
            }
        }
    }
}

std::vector<OSPF::NextHop>* OSPF::Area::CalculateNextHops(OSPFLSA* destination, OSPFLSA* parent) const
{
    std::vector<OSPF::NextHop>* hops = new std::vector<OSPF::NextHop>;
    unsigned long               i, j;

    OSPF::RouterLSA* routerLSA = dynamic_cast<OSPF::RouterLSA*> (parent);
    if (routerLSA != NULL) {
        if (routerLSA != spfTreeRoot) {
            unsigned int nextHopCount = routerLSA->GetNextHopCount();
            for (i = 0; i < nextHopCount; i++) {
                hops->push_back(routerLSA->GetNextHop(i));
            }
            return hops;
        } else {
            OSPF::RouterLSA* destinationRouterLSA = dynamic_cast<OSPF::RouterLSA*> (destination);
            if (destinationRouterLSA != NULL) {
                unsigned long interfaceNum   = associatedInterfaces.size();
                for (i = 0; i < interfaceNum; i++) {
                    OSPF::Interface::OSPFInterfaceType intfType = associatedInterfaces[i]->GetType();
                    if ((intfType == OSPF::Interface::PointToPoint) ||
                        ((intfType == OSPF::Interface::Virtual) &&
                         (associatedInterfaces[i]->GetState() > OSPF::Interface::LoopbackState)))
                    {
                        OSPF::Neighbor* ptpNeighbor = associatedInterfaces[i]->GetNeighborCount() > 0 ? associatedInterfaces[i]->GetNeighbor(0) : NULL;
                        if (ptpNeighbor != NULL) {
                            if (ptpNeighbor->GetNeighborID() == destinationRouterLSA->getHeader().getLinkStateID()) {
                                NextHop nextHop;
                                nextHop.ifIndex           = associatedInterfaces[i]->GetIfIndex();
                                nextHop.hopAddress        = ptpNeighbor->GetAddress();
                                nextHop.advertisingRouter = destinationRouterLSA->getHeader().getAdvertisingRouter().getInt();
                                hops->push_back(nextHop);
                                break;
                            }
                        }
                    }
                    if (intfType == OSPF::Interface::PointToMultiPoint) {
                        OSPF::Neighbor* ptmpNeighbor = associatedInterfaces[i]->GetNeighborByID(destinationRouterLSA->getHeader().getLinkStateID());
                        if (ptmpNeighbor != NULL) {
                            unsigned int   linkCount = destinationRouterLSA->getLinksArraySize();
                            OSPF::RouterID rootID    = parentRouter->GetRouterID();
                            for (j = 0; j < linkCount; j++) {
                                Link& link = destinationRouterLSA->getLinks(j);
                                if (link.getLinkID() == rootID) {
                                    NextHop nextHop;
                                    nextHop.ifIndex           = associatedInterfaces[i]->GetIfIndex();
                                    nextHop.hopAddress        = IPv4AddressFromULong(link.getLinkData());
                                    nextHop.advertisingRouter = destinationRouterLSA->getHeader().getAdvertisingRouter().getInt();
                                    hops->push_back(nextHop);
                                }
                            }
                            break;
                        }
                    }
                }
            } else {
                OSPF::NetworkLSA* destinationNetworkLSA = dynamic_cast<OSPF::NetworkLSA*> (destination);
                if (destinationNetworkLSA != NULL) {
                    OSPF::IPv4Address networkDesignatedRouter = IPv4AddressFromULong(destinationNetworkLSA->getHeader().getLinkStateID());
                    unsigned long     interfaceNum            = associatedInterfaces.size();
                    for (i = 0; i < interfaceNum; i++) {
                        OSPF::Interface::OSPFInterfaceType intfType = associatedInterfaces[i]->GetType();
                        if (((intfType == OSPF::Interface::Broadcast) ||
                             (intfType == OSPF::Interface::NBMA)) &&
                            (associatedInterfaces[i]->GetDesignatedRouter().ipInterfaceAddress == networkDesignatedRouter))
                        {
                            OSPF::IPv4AddressRange range = associatedInterfaces[i]->GetAddressRange();
                            NextHop                nextHop;

                            nextHop.ifIndex           = associatedInterfaces[i]->GetIfIndex();
                            nextHop.hopAddress        = (range.address & range.mask);
                            nextHop.advertisingRouter = destinationNetworkLSA->getHeader().getAdvertisingRouter().getInt();
                            hops->push_back(nextHop);
                        }
                    }
                }
            }
        }
    } else {
        OSPF::NetworkLSA* networkLSA = dynamic_cast<OSPF::NetworkLSA*> (parent);
        if (networkLSA != NULL) {
            if (networkLSA->GetParent() != spfTreeRoot) {
                unsigned int nextHopCount = networkLSA->GetNextHopCount();
                for (i = 0; i < nextHopCount; i++) {
                    hops->push_back(networkLSA->GetNextHop(i));
                }
                return hops;
            } else {
                unsigned long parentLinkStateID = parent->getHeader().getLinkStateID();

                OSPF::RouterLSA* destinationRouterLSA = dynamic_cast<OSPF::RouterLSA*> (destination);
                if (destinationRouterLSA != NULL) {
                    OSPF::RouterID destinationRouterID = destinationRouterLSA->getHeader().getLinkStateID();
                    unsigned int   linkCount           = destinationRouterLSA->getLinksArraySize();
                    for (i = 0; i < linkCount; i++) {
                        Link&   link = destinationRouterLSA->getLinks(i);
                        NextHop nextHop;

                        if (((link.getType() == TransitLink) &&
                             (link.getLinkID().getInt() == parentLinkStateID)) ||
                            ((link.getType() == StubLink) &&
                             ((link.getLinkID().getInt() & link.getLinkData()) == (parentLinkStateID & networkLSA->getNetworkMask().getInt()))))
                        {
                            unsigned long interfaceNum   = associatedInterfaces.size();
                            for (j = 0; j < interfaceNum; j++) {
                                OSPF::Interface::OSPFInterfaceType intfType = associatedInterfaces[j]->GetType();
                                if (((intfType == OSPF::Interface::Broadcast) ||
                                     (intfType == OSPF::Interface::NBMA)) &&
                                    (associatedInterfaces[j]->GetDesignatedRouter().ipInterfaceAddress == IPv4AddressFromULong(parentLinkStateID)))
                                {
                                    OSPF::Neighbor* nextHopNeighbor = associatedInterfaces[j]->GetNeighborByID(destinationRouterID);
                                    if (nextHopNeighbor != NULL) {
                                        nextHop.ifIndex           = associatedInterfaces[j]->GetIfIndex();
                                        nextHop.hopAddress        = nextHopNeighbor->GetAddress();
                                        nextHop.advertisingRouter = destinationRouterLSA->getHeader().getAdvertisingRouter().getInt();
                                        hops->push_back(nextHop);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return hops;
}

std::vector<OSPF::NextHop>* OSPF::Area::CalculateNextHops(Link& destination, OSPFLSA* parent) const
{
    std::vector<OSPF::NextHop>* hops = new std::vector<OSPF::NextHop>;
    unsigned long                i;

    OSPF::RouterLSA* routerLSA = check_and_cast<OSPF::RouterLSA*> (parent);
    if (routerLSA != spfTreeRoot) {
        unsigned int nextHopCount = routerLSA->GetNextHopCount();
        for (i = 0; i < nextHopCount; i++) {
            hops->push_back(routerLSA->GetNextHop(i));
        }
        return hops;
    } else {
        unsigned long interfaceNum = associatedInterfaces.size();
        for (i = 0; i < interfaceNum; i++) {
            OSPF::Interface::OSPFInterfaceType intfType = associatedInterfaces[i]->GetType();

            if ((intfType == OSPF::Interface::PointToPoint) ||
                ((intfType == OSPF::Interface::Virtual) &&
                 (associatedInterfaces[i]->GetState() > OSPF::Interface::LoopbackState)))
            {
                OSPF::Neighbor* neighbor = (associatedInterfaces[i]->GetNeighborCount() > 0) ? associatedInterfaces[i]->GetNeighbor(0) : NULL;
                if (neighbor != NULL) {
                    OSPF::IPv4Address neighborAddress = neighbor->GetAddress();
                    if (((neighborAddress != OSPF::NullIPv4Address) &&
                         (ULongFromIPv4Address(neighborAddress) == destination.getLinkID().getInt())) ||
                        ((neighborAddress == OSPF::NullIPv4Address) &&
                         (ULongFromIPv4Address(associatedInterfaces[i]->GetAddressRange().address) == destination.getLinkID().getInt()) &&
                         (ULongFromIPv4Address(associatedInterfaces[i]->GetAddressRange().mask) == destination.getLinkData())))
                    {
                        NextHop nextHop;
                        nextHop.ifIndex           = associatedInterfaces[i]->GetIfIndex();
                        nextHop.hopAddress        = neighborAddress;
                        nextHop.advertisingRouter = parentRouter->GetRouterID();
                        hops->push_back(nextHop);
                        break;
                    }
                }
            }
            if ((intfType == OSPF::Interface::Broadcast) ||
                (intfType == OSPF::Interface::NBMA))
            {
                if ((destination.getLinkID().getInt() == ULongFromIPv4Address(associatedInterfaces[i]->GetAddressRange().address & associatedInterfaces[i]->GetAddressRange().mask)) &&
                    (destination.getLinkData() == ULongFromIPv4Address(associatedInterfaces[i]->GetAddressRange().mask)))
                {
                    NextHop nextHop;
                    nextHop.ifIndex           = associatedInterfaces[i]->GetIfIndex();
                    nextHop.hopAddress        = IPv4AddressFromULong(destination.getLinkID().getInt());
                    nextHop.advertisingRouter = parentRouter->GetRouterID();
                    hops->push_back(nextHop);
                    break;
                }
            }
            if (intfType == OSPF::Interface::PointToMultiPoint) {
                if (destination.getType() == StubLink) {
                    if (destination.getLinkID().getInt() == ULongFromIPv4Address(associatedInterfaces[i]->GetAddressRange().address)) {
                        // The link contains the router's own interface address and a full mask,
                        // so we insert a next hop pointing to the interface itself. Kind of pointless, but
                        // not much else we could do...
                        // TODO: check what other OSPF implementations do in this situation
                        NextHop nextHop;
                        nextHop.ifIndex           = associatedInterfaces[i]->GetIfIndex();
                        nextHop.hopAddress        = associatedInterfaces[i]->GetAddressRange().address;
                        nextHop.advertisingRouter = parentRouter->GetRouterID();
                        hops->push_back(nextHop);
                        break;
                    }
                }
                if (destination.getType() == PointToPointLink) {
                    OSPF::Neighbor* neighbor = associatedInterfaces[i]->GetNeighborByID(destination.getLinkID().getInt());
                    if (neighbor != NULL) {
                        NextHop nextHop;
                        nextHop.ifIndex           = associatedInterfaces[i]->GetIfIndex();
                        nextHop.hopAddress        = neighbor->GetAddress();
                        nextHop.advertisingRouter = parentRouter->GetRouterID();
                        hops->push_back(nextHop);
                        break;
                    }
                }
            }
            // next hops for virtual links are generated later, after examining transit areas' SummaryLSAs
        }

        if (hops->size() == 0) {
            unsigned long hostRouteCount = hostRoutes.size();
            for (i = 0; i < hostRouteCount; i++) {
                if ((destination.getLinkID().getInt() == ULongFromIPv4Address(hostRoutes[i].address)) &&
                    (destination.getLinkData() == 0xFFFFFFFF))
                {
                    NextHop nextHop;
                    nextHop.ifIndex           = hostRoutes[i].ifIndex;
                    nextHop.hopAddress        = hostRoutes[i].address;
                    nextHop.advertisingRouter = parentRouter->GetRouterID();
                    hops->push_back(nextHop);
                    break;
                }
            }
        }
    }

    return hops;
}

bool OSPF::Area::HasLink(OSPFLSA* fromLSA, OSPFLSA* toLSA) const
{
    unsigned int i;

    OSPF::RouterLSA* fromRouterLSA = dynamic_cast<OSPF::RouterLSA*> (fromLSA);
    if (fromRouterLSA != NULL) {
        unsigned int     linkCount   = fromRouterLSA->getLinksArraySize();
        OSPF::RouterLSA* toRouterLSA = dynamic_cast<OSPF::RouterLSA*> (toLSA);
        if (toRouterLSA != NULL) {
            for (i = 0; i < linkCount; i++) {
                Link&    link     = fromRouterLSA->getLinks(i);
                LinkType linkType = static_cast<LinkType> (link.getType());

                if (((linkType == PointToPointLink) ||
                     (linkType == VirtualLink)) &&
                    (link.getLinkID().getInt() == toRouterLSA->getHeader().getLinkStateID()))
                {
                    return true;
                }
            }
        } else {
            OSPF::NetworkLSA* toNetworkLSA = dynamic_cast<OSPF::NetworkLSA*> (toLSA);
            if (toNetworkLSA != NULL) {
                for (i = 0; i < linkCount; i++) {
                    Link&    link     = fromRouterLSA->getLinks(i);

                    if ((link.getType() == TransitLink) &&
                        (link.getLinkID().getInt() == toNetworkLSA->getHeader().getLinkStateID()))
                    {
                        return true;
                    }
                    if ((link.getType() == StubLink) &&
                        ((link.getLinkID().getInt() & link.getLinkData()) == (toNetworkLSA->getHeader().getLinkStateID() & toNetworkLSA->getNetworkMask().getInt())))
                    {
                        return true;
                    }
                }
            }
        }
    } else {
        OSPF::NetworkLSA* fromNetworkLSA = dynamic_cast<OSPF::NetworkLSA*> (fromLSA);
        if (fromNetworkLSA != NULL) {
            unsigned int     routerCount   = fromNetworkLSA->getAttachedRoutersArraySize();
            OSPF::RouterLSA* toRouterLSA = dynamic_cast<OSPF::RouterLSA*> (toLSA);
            if (toRouterLSA != NULL) {
                for (i = 0; i < routerCount; i++) {
                    if (fromNetworkLSA->getAttachedRouters(i).getInt() == toRouterLSA->getHeader().getLinkStateID()) {
                        return true;
                    }
                }
            }
        }
    }

    return false;
}

/**
 * Browse through the newRoutingTable looking for entries describing the same destination
 * as the currentLSA. If a cheaper route is found then skip this LSA(return true), else
 * note those which are of equal or worse cost than the currentCost.
 */
bool OSPF::Area::FindSameOrWorseCostRoute(const std::vector<OSPF::RoutingTableEntry*>& newRoutingTable,
                                           const OSPF::SummaryLSA&                      summaryLSA,
                                           unsigned short                               currentCost,
                                           bool&                                        destinationInRoutingTable,
                                           std::list<OSPF::RoutingTableEntry*>&         sameOrWorseCost) const
{
    destinationInRoutingTable = false;
    sameOrWorseCost.clear();

    long                   routeCount = newRoutingTable.size();
    OSPF::IPv4AddressRange destination;

    destination.address = IPv4AddressFromULong(summaryLSA.getHeader().getLinkStateID());
    destination.mask    = IPv4AddressFromULong(summaryLSA.getNetworkMask().getInt());

    for (long j = 0; j < routeCount; j++) {
        OSPF::RoutingTableEntry* routingEntry  = newRoutingTable[j];
        bool                     foundMatching = false;

        if (summaryLSA.getHeader().getLsType() == SummaryLSA_NetworksType) {
            if ((routingEntry->GetDestinationType() == OSPF::RoutingTableEntry::NetworkDestination) &&
                (ULongFromIPv4Address(destination.address & destination.mask) == routingEntry->GetDestinationID().getInt()))
            {
                foundMatching = true;
            }
        } else {
            if ((((routingEntry->GetDestinationType() & OSPF::RoutingTableEntry::AreaBorderRouterDestination) != 0) ||
                 ((routingEntry->GetDestinationType() & OSPF::RoutingTableEntry::ASBoundaryRouterDestination) != 0)) &&
                (ULongFromIPv4Address(destination.address) == routingEntry->GetDestinationID().getInt()))
            {
                foundMatching = true;
            }
        }

        if (foundMatching) {
            destinationInRoutingTable = true;

            /* If the matching entry is an IntraArea getRoute(intra-area paths are
                * always preferred to other paths of any cost), or it's a cheaper InterArea
                * route, then skip this LSA.
                */
            if ((routingEntry->GetPathType() == OSPF::RoutingTableEntry::IntraArea) ||
                ((routingEntry->GetPathType() == OSPF::RoutingTableEntry::InterArea) &&
                 (routingEntry->GetCost() < currentCost)))
            {
                return true;
            } else {
                // if it's an other InterArea path
                if ((routingEntry->GetPathType() == OSPF::RoutingTableEntry::InterArea) &&
                    (routingEntry->GetCost() >= currentCost))
                {
                    sameOrWorseCost.push_back(routingEntry);
                }   // else it's external -> same as if not in the table
            }
        }
    }
    return false;
}

/**
 * Returns a new RoutingTableEntry based on the input SummaryLSA, with the input cost
 * and the borderRouterEntry's next hops.
 */
OSPF::RoutingTableEntry* OSPF::Area::CreateRoutingTableEntryFromSummaryLSA(const OSPF::SummaryLSA&        summaryLSA,
                                                                            unsigned short                 entryCost,
                                                                            const OSPF::RoutingTableEntry& borderRouterEntry) const
{
    OSPF::IPv4AddressRange destination;

    destination.address = IPv4AddressFromULong(summaryLSA.getHeader().getLinkStateID());
    destination.mask    = IPv4AddressFromULong(summaryLSA.getNetworkMask().getInt());

    OSPF::RoutingTableEntry* newEntry = new OSPF::RoutingTableEntry;

    if (summaryLSA.getHeader().getLsType() == SummaryLSA_NetworksType) {
        newEntry->SetDestinationID(ULongFromIPv4Address(destination.address & destination.mask));
        newEntry->SetAddressMask(ULongFromIPv4Address(destination.mask));
        newEntry->SetDestinationType(OSPF::RoutingTableEntry::NetworkDestination);
    } else {
        newEntry->SetDestinationID(ULongFromIPv4Address(destination.address));
        newEntry->SetAddressMask(0xFFFFFFFF);
        newEntry->SetDestinationType(OSPF::RoutingTableEntry::ASBoundaryRouterDestination);
    }
    newEntry->SetArea(areaID);
    newEntry->SetPathType(OSPF::RoutingTableEntry::InterArea);
    newEntry->SetCost(entryCost);
    newEntry->SetOptionalCapabilities(summaryLSA.getHeader().getLsOptions());
    newEntry->SetLinkStateOrigin(&summaryLSA);

    unsigned int nextHopCount = borderRouterEntry.GetNextHopCount();
    for (unsigned int j = 0; j < nextHopCount; j++) {
        newEntry->AddNextHop(borderRouterEntry.GetNextHop(j));
    }

    return newEntry;
}

/**
 * @see RFC 2328 Section 16.2.
 * @todo This function does a lot of lookup in the input newRoutingTable.
 *       Restructuring the input vector into some kind of hash would quite
 *       probably speed up execution.
 */
void OSPF::Area::CalculateInterAreaRoutes(std::vector<OSPF::RoutingTableEntry*>& newRoutingTable)
{
    unsigned long i = 0;
    unsigned long j = 0;
    unsigned long lsaCount = summaryLSAs.size();

    for (i = 0; i < lsaCount; i++) {
        OSPF::SummaryLSA* currentLSA        = summaryLSAs[i];
        OSPFLSAHeader&    currentHeader     = currentLSA->getHeader();

        unsigned long     routeCost         = currentLSA->getRouteCost();
        unsigned short    lsAge             = currentHeader.getLsAge();
        RouterID          originatingRouter = currentHeader.getAdvertisingRouter().getInt();
        bool              selfOriginated    = (originatingRouter == parentRouter->GetRouterID());

        if ((routeCost == LS_INFINITY) || (lsAge == MAX_AGE) || (selfOriginated)) { // (1) and(2)
            continue;
        }

        char                   lsType     = currentHeader.getLsType();
        unsigned long          routeCount = newRoutingTable.size();
        OSPF::IPv4AddressRange destination;

        destination.address = IPv4AddressFromULong(currentHeader.getLinkStateID());
        destination.mask    = IPv4AddressFromULong(currentLSA->getNetworkMask().getInt());

        if ((lsType == SummaryLSA_NetworksType) && (parentRouter->HasAddressRange(destination))) { // (3)
            bool foundIntraAreaRoute = false;

            // look for an "Active" IntraArea route
            for (j = 0; j < routeCount; j++) {
                OSPF::RoutingTableEntry* routingEntry = newRoutingTable[j];

                if ((routingEntry->GetDestinationType() == OSPF::RoutingTableEntry::NetworkDestination) &&
                    (routingEntry->GetPathType() == OSPF::RoutingTableEntry::IntraArea) &&
                    ((routingEntry->GetDestinationID().getInt() &
                      routingEntry->GetAddressMask().getInt()   &
                      ULongFromIPv4Address(destination.mask)       ) == ULongFromIPv4Address(destination.address &
                                                                                               destination.mask)))
                {
                    foundIntraAreaRoute = true;
                    break;
                }
            }
            if (foundIntraAreaRoute) {
                continue;
            }
        }

        OSPF::RoutingTableEntry* borderRouterEntry = NULL;

        // The routingEntry describes a route to an other area -> look for the border router originating it
        for (j = 0; j < routeCount; j++) {     // (4) N == destination, BR == borderRouterEntry
            OSPF::RoutingTableEntry* routingEntry = newRoutingTable[j];

            if ((routingEntry->GetArea() == areaID) &&
                (((routingEntry->GetDestinationType() & OSPF::RoutingTableEntry::AreaBorderRouterDestination) != 0) ||
                 ((routingEntry->GetDestinationType() & OSPF::RoutingTableEntry::ASBoundaryRouterDestination) != 0)) &&
                (routingEntry->GetDestinationID().getInt() == originatingRouter))
            {
                borderRouterEntry = routingEntry;
                break;
            }
        }
        if (borderRouterEntry == NULL) {
            continue;
        } else {    // (5)
            /* "Else, this LSA describes an inter-area path to destination N,
             * whose cost is the distance to BR plus the cost specified in the LSA.
             * Call the cost of this inter-area path IAC."
             */
            bool                                destinationInRoutingTable = true;
            unsigned short                      currentCost               = routeCost + borderRouterEntry->GetCost();
            std::list<OSPF::RoutingTableEntry*> sameOrWorseCost;

            if (FindSameOrWorseCostRoute(newRoutingTable,
                                          *currentLSA,
                                          currentCost,
                                          destinationInRoutingTable,
                                          sameOrWorseCost))
            {
                continue;
            }

            if (destinationInRoutingTable && (sameOrWorseCost.size() > 0)) {
                OSPF::RoutingTableEntry* equalEntry = NULL;

                /* Look for an equal cost entry in the sameOrWorseCost list, and
                 * also clear the more expensive entries from the newRoutingTable.
                 */
                for (std::list<OSPF::RoutingTableEntry*>::iterator it = sameOrWorseCost.begin(); it != sameOrWorseCost.end(); it++) {
                    OSPF::RoutingTableEntry* checkedEntry = (*it);

                    if (checkedEntry->GetCost() > currentCost) {
                        for (std::vector<OSPF::RoutingTableEntry*>::iterator entryIt = newRoutingTable.begin(); entryIt != newRoutingTable.end(); entryIt++) {
                            if (checkedEntry == (*entryIt)) {
                                newRoutingTable.erase(entryIt);
                                break;
                            }
                        }
                    } else {    // EntryCost == currentCost
                        equalEntry = checkedEntry;  // should be only one - if there are more they are ignored
                    }
                }

                unsigned long nextHopCount = borderRouterEntry->GetNextHopCount();

                if (equalEntry != NULL) {
                    /* Add the next hops of the border router advertising this destination
                     * to the equal entry.
                     */
                    for (unsigned long j = 0; j < nextHopCount; j++) {
                        equalEntry->AddNextHop(borderRouterEntry->GetNextHop(j));
                    }
                } else {
                    OSPF::RoutingTableEntry* newEntry = CreateRoutingTableEntryFromSummaryLSA(*currentLSA, currentCost, *borderRouterEntry);
                    ASSERT(newEntry != NULL);
                    newRoutingTable.push_back(newEntry);
                }
            } else {
                OSPF::RoutingTableEntry* newEntry = CreateRoutingTableEntryFromSummaryLSA(*currentLSA, currentCost, *borderRouterEntry);
                ASSERT(newEntry != NULL);
                newRoutingTable.push_back(newEntry);
            }
        }
    }
}

void OSPF::Area::ReCheckSummaryLSAs(std::vector<OSPF::RoutingTableEntry*>& newRoutingTable)
{
    unsigned long i = 0;
    unsigned long j = 0;
    unsigned long lsaCount = summaryLSAs.size();

    for (i = 0; i < lsaCount; i++) {
        OSPF::SummaryLSA* currentLSA        = summaryLSAs[i];
        OSPFLSAHeader&    currentHeader     = currentLSA->getHeader();

        unsigned long     routeCost         = currentLSA->getRouteCost();
        unsigned short    lsAge             = currentHeader.getLsAge();
        RouterID          originatingRouter = currentHeader.getAdvertisingRouter().getInt();
        bool              selfOriginated    = (originatingRouter == parentRouter->GetRouterID());

        if ((routeCost == LS_INFINITY) || (lsAge == MAX_AGE) || (selfOriginated)) { // (1) and(2)
            continue;
        }

        unsigned long            routeCount       = newRoutingTable.size();
        char                     lsType           = currentHeader.getLsType();
        OSPF::RoutingTableEntry* destinationEntry = NULL;
        OSPF::IPv4AddressRange   destination;

        destination.address = IPv4AddressFromULong(currentHeader.getLinkStateID());
        destination.mask    = IPv4AddressFromULong(currentLSA->getNetworkMask().getInt());

        for (j = 0; j < routeCount; j++) {  // (3)
            OSPF::RoutingTableEntry* routingEntry  = newRoutingTable[j];
            bool                     foundMatching = false;

            if (lsType == SummaryLSA_NetworksType) {
                if ((routingEntry->GetDestinationType() == OSPF::RoutingTableEntry::NetworkDestination) &&
                    (ULongFromIPv4Address(destination.address & destination.mask) == routingEntry->GetDestinationID().getInt()))
                {
                    foundMatching = true;
                }
            } else {
                if ((((routingEntry->GetDestinationType() & OSPF::RoutingTableEntry::AreaBorderRouterDestination) != 0) ||
                     ((routingEntry->GetDestinationType() & OSPF::RoutingTableEntry::ASBoundaryRouterDestination) != 0)) &&
                    (ULongFromIPv4Address(destination.address) == routingEntry->GetDestinationID().getInt()))
                {
                    foundMatching = true;
                }
            }

            if (foundMatching) {
                OSPF::RoutingTableEntry::RoutingPathType pathType = routingEntry->GetPathType();

                if ((pathType == OSPF::RoutingTableEntry::Type1External) ||
                    (pathType == OSPF::RoutingTableEntry::Type2External) ||
                    (routingEntry->GetArea() != OSPF::BackboneAreaID))
                {
                    break;
                } else {
                    destinationEntry = routingEntry;
                    break;
                }
            }
        }
        if (destinationEntry == NULL) {
            continue;
        }

        OSPF::RoutingTableEntry* borderRouterEntry = NULL;
        unsigned short           currentCost       = routeCost;

        for (j = 0; j < routeCount; j++) {     // (4) BR == borderRouterEntry
            OSPF::RoutingTableEntry* routingEntry = newRoutingTable[j];

            if ((routingEntry->GetArea() == areaID) &&
                (((routingEntry->GetDestinationType() & OSPF::RoutingTableEntry::AreaBorderRouterDestination) != 0) ||
                 ((routingEntry->GetDestinationType() & OSPF::RoutingTableEntry::ASBoundaryRouterDestination) != 0)) &&
                (routingEntry->GetDestinationID().getInt() == originatingRouter))
            {
                borderRouterEntry = routingEntry;
                currentCost += borderRouterEntry->GetCost();
                break;
            }
        }
        if (borderRouterEntry == NULL) {
            continue;
        } else {    // (5)
            if (currentCost <= destinationEntry->GetCost()) {
                if (currentCost < destinationEntry->GetCost()) {
                    destinationEntry->ClearNextHops();
                }

                unsigned long nextHopCount = borderRouterEntry->GetNextHopCount();

                for (j = 0; j < nextHopCount; j++) {
                    destinationEntry->AddNextHop(borderRouterEntry->GetNextHop(j));
                }
            }
        }
    }
}
