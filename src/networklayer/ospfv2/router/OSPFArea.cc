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

OSPF::Area::~Area()
{
    int interfaceNum = associatedInterfaces.size();
    for (int i = 0; i < interfaceNum; i++) {
        delete (associatedInterfaces[i]);
    }
    associatedInterfaces.clear();
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

void OSPF::Area::addInterface(OSPF::Interface* intf)
{
    intf->setArea(this);
    associatedInterfaces.push_back(intf);
}

void OSPF::Area::addAddressRange(IPv4AddressRange addressRange, bool advertise)
{
    int addressRangeNum = areaAddressRanges.size();
    bool found = false;
    bool erased = false;

    for (int i = 0; i < addressRangeNum; i++) {
        IPv4AddressRange curRange = areaAddressRanges[i];
        if (curRange.contains(addressRange)) {   // contains or same
            found = true;
            if (advertiseAddressRanges[curRange] != advertise) {
                throw cRuntimeError("Inconsistent advertise settings for %s and %s address ranges in area %s",
                        addressRange.str().c_str(), curRange.str().c_str(), areaID.str(false).c_str());
            }
        }
        else if (addressRange.contains(curRange)) {
            if (advertiseAddressRanges[curRange] != advertise) {
                throw cRuntimeError("Inconsistent advertise settings for %s and %s address ranges in area %s",
                        addressRange.str().c_str(), curRange.str().c_str(), areaID.str(false).c_str());
            }
            advertiseAddressRanges.erase(curRange);
            areaAddressRanges[i] = NULL_IPV4ADDRESSRANGE;
            erased = true;
        }
    }
    if (erased && found)  // the found entry contains new entry and new entry contains erased entry ==> the found entry also contains the erased entry
        throw cRuntimeError("Model error: bad contents in areaAddressRanges vector");
    if (erased) {
        std::vector<IPv4AddressRange>::iterator it = areaAddressRanges.begin();
        while (it != areaAddressRanges.end()) {
            if (*it == NULL_IPV4ADDRESSRANGE)
                it = areaAddressRanges.erase(it);
            else
                it++;
        }
    }
    if (!found) {
        areaAddressRanges.push_back(addressRange);
        advertiseAddressRanges[addressRange] = advertise;
    }
}

std::string OSPF::Area::info() const
{
    std::stringstream out;
    out << "areaID: " << areaID.str(false);
    return out.str();
}

std::string OSPF::Area::detailedInfo() const
{
    std::stringstream out;
    int i;
    out << "\n    areaID: " << areaID.str(false) << ", ";
    out << "transitCapability: " << (transitCapability ? "true" : "false") << ", ";
    out << "externalRoutingCapability: " << (externalRoutingCapability ? "true" : "false") << ", ";
    out << "stubDefaultCost: " << stubDefaultCost << "\n";
    int addressRangeNum = areaAddressRanges.size();
    for (i = 0; i < addressRangeNum; i++) {
        out << "    addressRanges[" << i << "]: ";
        out << areaAddressRanges[i].address.str(false);
        out << "/" << areaAddressRanges[i].mask.str(false) << "\n";
    }
    int interfaceNum = associatedInterfaces.size();
    for (i = 0; i < interfaceNum; i++) {
        out << "    interface[" << i << "]: address: ";
        out << associatedInterfaces[i]->getAddressRange().address.str(false);
        out << "/" << associatedInterfaces[i]->getAddressRange().mask.str(false) << "\n";
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

bool OSPF::Area::containsAddress(IPv4Address address) const
{
    int addressRangeNum = areaAddressRanges.size();
    for (int i = 0; i < addressRangeNum; i++) {
        if (areaAddressRanges[i].contains(address)) {
            return true;
        }
    }
    return false;
}

bool OSPF::Area::hasAddressRange(OSPF::IPv4AddressRange addressRange) const
{
    int addressRangeNum = areaAddressRanges.size();
    for (int i = 0; i < addressRangeNum; i++) {
        if (areaAddressRanges[i] == addressRange) {
            return true;
        }
    }
    return false;
}

OSPF::IPv4AddressRange OSPF::Area::getContainingAddressRange(OSPF::IPv4AddressRange addressRange, bool* advertise /*= NULL*/) const
{
    int addressRangeNum = areaAddressRanges.size();
    for (int i = 0; i < addressRangeNum; i++) {
        if (areaAddressRanges[i].contains(addressRange)) {
            if (advertise != NULL) {
                std::map<OSPF::IPv4AddressRange, bool>::const_iterator rangeIt = advertiseAddressRanges.find(areaAddressRanges[i]);
                if (rangeIt != advertiseAddressRanges.end()) {
                    *advertise = rangeIt->second;
                } else {
                    throw cRuntimeError("Model error: inconsistent contents in areaAddressRanges and advertiseAddressRanges variables");
                }
            }
            return areaAddressRanges[i];
        }
    }
    if (advertise != NULL) {
        *advertise = false;
    }
    return NULL_IPV4ADDRESSRANGE;
}

OSPF::Interface*  OSPF::Area::getInterface(unsigned char ifIndex)
{
    int interfaceNum = associatedInterfaces.size();
    for (int i = 0; i < interfaceNum; i++) {
        if ((associatedInterfaces[i]->getType() != OSPF::Interface::VIRTUAL) &&
            (associatedInterfaces[i]->getIfIndex() == ifIndex))
        {
            return associatedInterfaces[i];
        }
    }
    return NULL;
}

OSPF::Interface*  OSPF::Area::getInterface(IPv4Address address)
{
    int interfaceNum = associatedInterfaces.size();
    for (int i = 0; i < interfaceNum; i++) {
        if ((associatedInterfaces[i]->getType() != OSPF::Interface::VIRTUAL) &&
            (associatedInterfaces[i]->getAddressRange().address == address))
        {
            return associatedInterfaces[i];
        }
    }
    return NULL;
}

bool OSPF::Area::hasVirtualLink(OSPF::AreaID withTransitArea) const
{
    if ((areaID != OSPF::BACKBONE_AREAID) || (withTransitArea == OSPF::BACKBONE_AREAID)) {
        return false;
    }

    int interfaceNum = associatedInterfaces.size();
    for (int i = 0; i < interfaceNum; i++) {
        if ((associatedInterfaces[i]->getType() == OSPF::Interface::VIRTUAL) &&
            (associatedInterfaces[i]->getTransitAreaID() == withTransitArea))
        {
            return true;
        }
    }
    return false;
}


OSPF::Interface*  OSPF::Area::findVirtualLink(OSPF::RouterID routerID)
{
    int interfaceNum = associatedInterfaces.size();
    for (int i = 0; i < interfaceNum; i++) {
        if ((associatedInterfaces[i]->getType() == OSPF::Interface::VIRTUAL) &&
            (associatedInterfaces[i]->getNeighborByID(routerID) != NULL))
        {
            return associatedInterfaces[i];
        }
    }
    return NULL;
}

bool OSPF::Area::installRouterLSA(OSPFRouterLSA* lsa)
{
    OSPF::LinkStateID linkStateID = lsa->getHeader().getLinkStateID();
    std::map<OSPF::LinkStateID, OSPF::RouterLSA*>::iterator lsaIt = routerLSAsByID.find(linkStateID);
    if (lsaIt != routerLSAsByID.end()) {
        OSPF::LSAKeyType lsaKey;

        lsaKey.linkStateID = lsa->getHeader().getLinkStateID();
        lsaKey.advertisingRouter = lsa->getHeader().getAdvertisingRouter();

        removeFromAllRetransmissionLists(lsaKey);
        return lsaIt->second->update(lsa);
    } else {
        OSPF::RouterLSA* lsaCopy = new OSPF::RouterLSA(*lsa);
        routerLSAsByID[linkStateID] = lsaCopy;
        routerLSAs.push_back(lsaCopy);
        return true;
    }
}

bool OSPF::Area::installNetworkLSA(OSPFNetworkLSA* lsa)
{
    OSPF::LinkStateID linkStateID = lsa->getHeader().getLinkStateID();
    std::map<OSPF::LinkStateID, OSPF::NetworkLSA*>::iterator lsaIt = networkLSAsByID.find(linkStateID);
    if (lsaIt != networkLSAsByID.end()) {
        OSPF::LSAKeyType lsaKey;

        lsaKey.linkStateID = lsa->getHeader().getLinkStateID();
        lsaKey.advertisingRouter = lsa->getHeader().getAdvertisingRouter();

        removeFromAllRetransmissionLists(lsaKey);
        return lsaIt->second->update(lsa);
    } else {
        OSPF::NetworkLSA* lsaCopy = new OSPF::NetworkLSA(*lsa);
        networkLSAsByID[linkStateID] = lsaCopy;
        networkLSAs.push_back(lsaCopy);
        return true;
    }
}

bool OSPF::Area::installSummaryLSA(OSPFSummaryLSA* lsa)
{
    OSPF::LSAKeyType lsaKey;

    lsaKey.linkStateID = lsa->getHeader().getLinkStateID();
    lsaKey.advertisingRouter = lsa->getHeader().getAdvertisingRouter();

    std::map<OSPF::LSAKeyType, OSPF::SummaryLSA*, OSPF::LSAKeyType_Less>::iterator lsaIt = summaryLSAsByID.find(lsaKey);
    if (lsaIt != summaryLSAsByID.end()) {
        OSPF::LSAKeyType lsaKey;

        lsaKey.linkStateID = lsa->getHeader().getLinkStateID();
        lsaKey.advertisingRouter = lsa->getHeader().getAdvertisingRouter();

        removeFromAllRetransmissionLists(lsaKey);
        return lsaIt->second->update(lsa);
    } else {
        OSPF::SummaryLSA* lsaCopy = new OSPF::SummaryLSA(*lsa);
        summaryLSAsByID[lsaKey] = lsaCopy;
        summaryLSAs.push_back(lsaCopy);
        return true;
    }
}

OSPF::RouterLSA* OSPF::Area::findRouterLSA(OSPF::LinkStateID linkStateID)
{
    std::map<OSPF::LinkStateID, OSPF::RouterLSA*>::iterator lsaIt = routerLSAsByID.find(linkStateID);
    if (lsaIt != routerLSAsByID.end()) {
        return lsaIt->second;
    } else {
        return NULL;
    }
}

const OSPF::RouterLSA* OSPF::Area::findRouterLSA(OSPF::LinkStateID linkStateID) const
{
    std::map<OSPF::LinkStateID, OSPF::RouterLSA*>::const_iterator lsaIt = routerLSAsByID.find(linkStateID);
    if (lsaIt != routerLSAsByID.end()) {
        return lsaIt->second;
    } else {
        return NULL;
    }
}

OSPF::NetworkLSA* OSPF::Area::findNetworkLSA(OSPF::LinkStateID linkStateID)
{
    std::map<OSPF::LinkStateID, OSPF::NetworkLSA*>::iterator lsaIt = networkLSAsByID.find(linkStateID);
    if (lsaIt != networkLSAsByID.end()) {
        return lsaIt->second;
    } else {
        return NULL;
    }
}

const OSPF::NetworkLSA* OSPF::Area::findNetworkLSA(OSPF::LinkStateID linkStateID) const
{
    std::map<OSPF::LinkStateID, OSPF::NetworkLSA*>::const_iterator lsaIt = networkLSAsByID.find(linkStateID);
    if (lsaIt != networkLSAsByID.end()) {
        return lsaIt->second;
    } else {
        return NULL;
    }
}

OSPF::SummaryLSA* OSPF::Area::findSummaryLSA(OSPF::LSAKeyType lsaKey)
{
    std::map<OSPF::LSAKeyType, OSPF::SummaryLSA*, OSPF::LSAKeyType_Less>::iterator lsaIt = summaryLSAsByID.find(lsaKey);
    if (lsaIt != summaryLSAsByID.end()) {
        return lsaIt->second;
    } else {
        return NULL;
    }
}

const OSPF::SummaryLSA* OSPF::Area::findSummaryLSA(OSPF::LSAKeyType lsaKey) const
{
    std::map<OSPF::LSAKeyType, OSPF::SummaryLSA*, OSPF::LSAKeyType_Less>::const_iterator lsaIt = summaryLSAsByID.find(lsaKey);
    if (lsaIt != summaryLSAsByID.end()) {
        return lsaIt->second;
    } else {
        return NULL;
    }
}

void OSPF::Area::ageDatabase()
{
    long lsaCount = routerLSAs.size();
    bool shouldRebuildRoutingTable = false;
    long i;

    for (i = 0; i < lsaCount; i++) {
        OSPF::RouterLSA* lsa = routerLSAs[i];
        unsigned short lsAge = lsa->getHeader().getLsAge();
        bool selfOriginated = (lsa->getHeader().getAdvertisingRouter() == parentRouter->getRouterID());
        bool unreachable = parentRouter->isDestinationUnreachable(lsa);

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
                floodLSA(lsa);
                lsa->incrementInstallTime();
            } else {
                long sequenceNumber = lsa->getHeader().getLsSequenceNumber();
                if (sequenceNumber == MAX_SEQUENCE_NUMBER) {
                    lsa->getHeader().setLsAge(MAX_AGE);
                    floodLSA(lsa);
                    lsa->incrementInstallTime();
                } else {
                    OSPF::RouterLSA* newLSA = originateRouterLSA();

                    newLSA->getHeader().setLsSequenceNumber(sequenceNumber + 1);
                    shouldRebuildRoutingTable |= lsa->update(newLSA);
                    delete newLSA;

                    floodLSA(lsa);
                }
            }
        }
        if (!selfOriginated && (lsAge == MAX_AGE - 1)) {
            lsa->getHeader().setLsAge(MAX_AGE);
            floodLSA(lsa);
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
                    routerLSAsByID.erase(lsa->getHeader().getLinkStateID());
                    delete lsa;
                    routerLSAs[i] = NULL;
                    shouldRebuildRoutingTable = true;
                } else {
                    OSPF::RouterLSA* newLSA = originateRouterLSA();
                    long sequenceNumber = lsa->getHeader().getLsSequenceNumber();

                    newLSA->getHeader().setLsSequenceNumber((sequenceNumber == MAX_SEQUENCE_NUMBER) ? INITIAL_SEQUENCE_NUMBER : sequenceNumber + 1);
                    shouldRebuildRoutingTable |= lsa->update(newLSA);
                    delete newLSA;

                    floodLSA(lsa);
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
        unsigned short lsAge = networkLSAs[i]->getHeader().getLsAge();
        bool unreachable = parentRouter->isDestinationUnreachable(networkLSAs[i]);
        OSPF::NetworkLSA* lsa = networkLSAs[i];
        OSPF::Interface* localIntf = getInterface(lsa->getHeader().getLinkStateID());
        bool selfOriginated = false;

        if ((localIntf != NULL) &&
            (localIntf->getState() == OSPF::Interface::DESIGNATED_ROUTER_STATE) &&
            (localIntf->getNeighborCount() > 0) &&
            (localIntf->hasAnyNeighborInStates(OSPF::Neighbor::FULL_STATE)))
        {
            selfOriginated = true;
        }

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
                floodLSA(lsa);
                lsa->incrementInstallTime();
            } else {
                long sequenceNumber = lsa->getHeader().getLsSequenceNumber();
                if (sequenceNumber == MAX_SEQUENCE_NUMBER) {
                    lsa->getHeader().setLsAge(MAX_AGE);
                    floodLSA(lsa);
                    lsa->incrementInstallTime();
                } else {
                    OSPF::NetworkLSA* newLSA = originateNetworkLSA(localIntf);

                    if (newLSA != NULL) {
                        newLSA->getHeader().setLsSequenceNumber(sequenceNumber + 1);
                        shouldRebuildRoutingTable |= lsa->update(newLSA);
                        delete newLSA;
                    } else {    // no neighbors on the network -> old NetworkLSA must be flushed
                        lsa->getHeader().setLsAge(MAX_AGE);
                        lsa->incrementInstallTime();
                    }

                    floodLSA(lsa);
                }
            }
        }
        if (!selfOriginated && (lsAge == MAX_AGE - 1)) {
            lsa->getHeader().setLsAge(MAX_AGE);
            floodLSA(lsa);
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
                    networkLSAsByID.erase(lsa->getHeader().getLinkStateID());
                    delete lsa;
                    networkLSAs[i] = NULL;
                    shouldRebuildRoutingTable = true;
                } else {
                    OSPF::NetworkLSA* newLSA = originateNetworkLSA(localIntf);
                    long sequenceNumber = lsa->getHeader().getLsSequenceNumber();

                    if (newLSA != NULL) {
                        newLSA->getHeader().setLsSequenceNumber((sequenceNumber == MAX_SEQUENCE_NUMBER) ? INITIAL_SEQUENCE_NUMBER : sequenceNumber + 1);
                        shouldRebuildRoutingTable |= lsa->update(newLSA);
                        delete newLSA;

                        floodLSA(lsa);
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
        unsigned short lsAge = summaryLSAs[i]->getHeader().getLsAge();
        bool selfOriginated = (summaryLSAs[i]->getHeader().getAdvertisingRouter() == parentRouter->getRouterID());
        bool unreachable = parentRouter->isDestinationUnreachable(summaryLSAs[i]);
        OSPF::SummaryLSA* lsa = summaryLSAs[i];

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
                floodLSA(lsa);
                lsa->incrementInstallTime();
            } else {
                long sequenceNumber = lsa->getHeader().getLsSequenceNumber();
                if (sequenceNumber == MAX_SEQUENCE_NUMBER) {
                    lsa->getHeader().setLsAge(MAX_AGE);
                    floodLSA(lsa);
                    lsa->incrementInstallTime();
                } else {
                    OSPF::SummaryLSA* newLSA = originateSummaryLSA(lsa);

                    if (newLSA != NULL) {
                        newLSA->getHeader().setLsSequenceNumber(sequenceNumber + 1);
                        shouldRebuildRoutingTable |= lsa->update(newLSA);
                        delete newLSA;

                        floodLSA(lsa);
                    } else {
                        lsa->getHeader().setLsAge(MAX_AGE);
                        floodLSA(lsa);
                        lsa->incrementInstallTime();
                    }
                }
            }
        }
        if (!selfOriginated && (lsAge == MAX_AGE - 1)) {
            lsa->getHeader().setLsAge(MAX_AGE);
            floodLSA(lsa);
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
                    summaryLSAsByID.erase(lsaKey);
                    delete lsa;
                    summaryLSAs[i] = NULL;
                    shouldRebuildRoutingTable = true;
                } else {
                    OSPF::SummaryLSA* newLSA = originateSummaryLSA(lsa);
                    if (newLSA != NULL) {
                        long sequenceNumber = lsa->getHeader().getLsSequenceNumber();

                        newLSA->getHeader().setLsSequenceNumber((sequenceNumber == MAX_SEQUENCE_NUMBER) ? INITIAL_SEQUENCE_NUMBER : sequenceNumber + 1);
                        shouldRebuildRoutingTable |= lsa->update(newLSA);
                        delete newLSA;

                        floodLSA(lsa);
                    } else {
                        summaryLSAsByID.erase(lsaKey);
                        delete lsa;
                        summaryLSAs[i] = NULL;
                        shouldRebuildRoutingTable = true;
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
        associatedInterfaces[m]->ageTransmittedLSALists();
    }

    if (shouldRebuildRoutingTable) {
        parentRouter->rebuildRoutingTable();
    }
}

bool OSPF::Area::hasAnyNeighborInStates(int states) const
{
    long interfaceCount = associatedInterfaces.size();
    for (long i = 0; i < interfaceCount; i++) {
        if (associatedInterfaces[i]->hasAnyNeighborInStates(states)) {
            return true;
        }
    }
    return false;
}

void OSPF::Area::removeFromAllRetransmissionLists(OSPF::LSAKeyType lsaKey)
{
    long interfaceCount = associatedInterfaces.size();
    for (long i = 0; i < interfaceCount; i++) {
        associatedInterfaces[i]->removeFromAllRetransmissionLists(lsaKey);
    }
}

bool OSPF::Area::isOnAnyRetransmissionList(OSPF::LSAKeyType lsaKey) const
{
    long interfaceCount = associatedInterfaces.size();
    for (long i = 0; i < interfaceCount; i++) {
        if (associatedInterfaces[i]->isOnAnyRetransmissionList(lsaKey)) {
            return true;
        }
    }
    return false;
}

bool OSPF::Area::floodLSA(OSPFLSA* lsa, OSPF::Interface* intf, OSPF::Neighbor* neighbor)
{
    bool floodedBackOut = false;
    long interfaceCount = associatedInterfaces.size();

    for (long i = 0; i < interfaceCount; i++) {
        if (associatedInterfaces[i]->floodLSA(lsa, intf, neighbor)) {
            floodedBackOut = true;
        }
    }

    return floodedBackOut;
}

bool OSPF::Area::isLocalAddress(IPv4Address address) const
{
    long interfaceCount = associatedInterfaces.size();
    for (long i = 0; i < interfaceCount; i++) {
        if (associatedInterfaces[i]->getAddressRange().address == address) {
            return true;
        }
    }
    return false;
}

OSPF::RouterLSA* OSPF::Area::originateRouterLSA()
{
    OSPF::RouterLSA* routerLSA = new OSPF::RouterLSA;
    OSPFLSAHeader& lsaHeader = routerLSA->getHeader();
    long interfaceCount = associatedInterfaces.size();
    OSPFOptions lsOptions;
    long i;

    lsaHeader.setLsAge(0);
    memset(&lsOptions, 0, sizeof(OSPFOptions));
    lsOptions.E_ExternalRoutingCapability = externalRoutingCapability;
    lsaHeader.setLsOptions(lsOptions);
    lsaHeader.setLsType(ROUTERLSA_TYPE);
    lsaHeader.setLinkStateID(parentRouter->getRouterID());
    lsaHeader.setAdvertisingRouter(IPv4Address(parentRouter->getRouterID()));
    lsaHeader.setLsSequenceNumber(INITIAL_SEQUENCE_NUMBER);

    routerLSA->setB_AreaBorderRouter(parentRouter->getAreaCount() > 1);
    routerLSA->setE_ASBoundaryRouter((externalRoutingCapability && parentRouter->getASBoundaryRouter()) ? true : false);
    OSPF::Area* backbone = parentRouter->getAreaByID(OSPF::BACKBONE_AREAID);
    routerLSA->setV_VirtualLinkEndpoint((backbone == NULL) ? false : backbone->hasVirtualLink(areaID));

    routerLSA->setNumberOfLinks(0);
    routerLSA->setLinksArraySize(0);
    for (i = 0; i < interfaceCount; i++) {
        OSPF::Interface* intf = associatedInterfaces[i];

        if (intf->getState() == OSPF::Interface::DOWN_STATE) {
            continue;
        }
        if ((intf->getState() == OSPF::Interface::LOOPBACK_STATE) &&
            ((intf->getType() != OSPF::Interface::POINTTOPOINT) ||
             (intf->getAddressRange().address != OSPF::NULL_IPV4ADDRESS)))
        {
            Link stubLink;
            stubLink.setType(STUB_LINK);
            stubLink.setLinkID(intf->getAddressRange().address);
            stubLink.setLinkData(0xFFFFFFFF);
            stubLink.setLinkCost(0);
            stubLink.setNumberOfTOS(0);
            stubLink.setTosDataArraySize(0);

            unsigned short linkIndex = routerLSA->getLinksArraySize();
            routerLSA->setLinksArraySize(linkIndex + 1);
            routerLSA->setNumberOfLinks(linkIndex + 1);
            routerLSA->setLinks(linkIndex, stubLink);
        }
        if (intf->getState() > OSPF::Interface::LOOPBACK_STATE) {
            switch (intf->getType()) {
                case OSPF::Interface::POINTTOPOINT:
                    {
                        OSPF::Neighbor* neighbor = (intf->getNeighborCount() > 0) ? intf->getNeighbor(0) : NULL;
                        if (neighbor != NULL) {
                            if (neighbor->getState() == OSPF::Neighbor::FULL_STATE) {
                                Link link;
                                link.setType(POINTTOPOINT_LINK);
                                link.setLinkID(IPv4Address(neighbor->getNeighborID()));
                                if (intf->getAddressRange().address != OSPF::NULL_IPV4ADDRESS) {
                                    link.setLinkData(intf->getAddressRange().address.getInt());
                                } else {
                                    link.setLinkData(intf->getIfIndex());
                                }
                                link.setLinkCost(intf->getOutputCost());
                                link.setNumberOfTOS(0);
                                link.setTosDataArraySize(0);

                                unsigned short linkIndex = routerLSA->getLinksArraySize();
                                routerLSA->setLinksArraySize(linkIndex + 1);
                                routerLSA->setNumberOfLinks(linkIndex + 1);
                                routerLSA->setLinks(linkIndex, link);
                            }
                            if (intf->getState() == OSPF::Interface::POINTTOPOINT_STATE) {
                                if (neighbor->getAddress() != OSPF::NULL_IPV4ADDRESS) {
                                    Link stubLink;
                                    stubLink.setType(STUB_LINK);
                                    stubLink.setLinkID(neighbor->getAddress());
                                    stubLink.setLinkData(0xFFFFFFFF);
                                    stubLink.setLinkCost(intf->getOutputCost());
                                    stubLink.setNumberOfTOS(0);
                                    stubLink.setTosDataArraySize(0);

                                    unsigned short linkIndex = routerLSA->getLinksArraySize();
                                    routerLSA->setLinksArraySize(linkIndex + 1);
                                    routerLSA->setNumberOfLinks(linkIndex + 1);
                                    routerLSA->setLinks(linkIndex, stubLink);
                                } else {
                                    if (intf->getAddressRange().mask.getInt() != 0xFFFFFFFF) {
                                        Link stubLink;
                                        stubLink.setType(STUB_LINK);
                                        stubLink.setLinkID(intf->getAddressRange().address &
                                                                                  intf->getAddressRange().mask);
                                        stubLink.setLinkData(intf->getAddressRange().mask.getInt());
                                        stubLink.setLinkCost(intf->getOutputCost());
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
                case OSPF::Interface::BROADCAST:
                case OSPF::Interface::NBMA:
                    {
                        if (intf->getState() == OSPF::Interface::WAITING_STATE) {
                            Link stubLink;
                            stubLink.setType(STUB_LINK);
                            stubLink.setLinkID(intf->getAddressRange().address &
                                                                      intf->getAddressRange().mask);
                            stubLink.setLinkData(intf->getAddressRange().mask.getInt());
                            stubLink.setLinkCost(intf->getOutputCost());
                            stubLink.setNumberOfTOS(0);
                            stubLink.setTosDataArraySize(0);

                            unsigned short linkIndex = routerLSA->getLinksArraySize();
                            routerLSA->setLinksArraySize(linkIndex + 1);
                            routerLSA->setNumberOfLinks(linkIndex + 1);
                            routerLSA->setLinks(linkIndex, stubLink);
                        } else {
                            OSPF::Neighbor* dRouter = intf->getNeighborByAddress(intf->getDesignatedRouter().ipInterfaceAddress);
                            if (((dRouter != NULL) && (dRouter->getState() == OSPF::Neighbor::FULL_STATE)) ||
                                ((intf->getDesignatedRouter().routerID == parentRouter->getRouterID()) &&
                                 (intf->hasAnyNeighborInStates(OSPF::Neighbor::FULL_STATE))))
                            {
                                Link link;
                                link.setType(TRANSIT_LINK);
                                link.setLinkID(intf->getDesignatedRouter().ipInterfaceAddress);
                                link.setLinkData(intf->getAddressRange().address.getInt());
                                link.setLinkCost(intf->getOutputCost());
                                link.setNumberOfTOS(0);
                                link.setTosDataArraySize(0);

                                unsigned short linkIndex = routerLSA->getLinksArraySize();
                                routerLSA->setLinksArraySize(linkIndex + 1);
                                routerLSA->setNumberOfLinks(linkIndex + 1);
                                routerLSA->setLinks(linkIndex, link);
                            } else {
                                Link stubLink;
                                stubLink.setType(STUB_LINK);
                                stubLink.setLinkID(intf->getAddressRange().address &
                                                                          intf->getAddressRange().mask);
                                stubLink.setLinkData(intf->getAddressRange().mask.getInt());
                                stubLink.setLinkCost(intf->getOutputCost());
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
                case OSPF::Interface::VIRTUAL:
                    {
                        OSPF::Neighbor* neighbor = (intf->getNeighborCount() > 0) ? intf->getNeighbor(0) : NULL;
                        if ((neighbor != NULL) && (neighbor->getState() == OSPF::Neighbor::FULL_STATE)) {
                            Link link;
                            link.setType(VIRTUAL_LINK);
                            link.setLinkID(IPv4Address(neighbor->getNeighborID()));
                            link.setLinkData(intf->getAddressRange().address.getInt());
                            link.setLinkCost(intf->getOutputCost());
                            link.setNumberOfTOS(0);
                            link.setTosDataArraySize(0);

                            unsigned short linkIndex = routerLSA->getLinksArraySize();
                            routerLSA->setLinksArraySize(linkIndex + 1);
                            routerLSA->setNumberOfLinks(linkIndex + 1);
                            routerLSA->setLinks(linkIndex, link);
                        }
                    }
                    break;
                case OSPF::Interface::POINTTOMULTIPOINT:
                    {
                        Link stubLink;
                        stubLink.setType(STUB_LINK);
                        stubLink.setLinkID(intf->getAddressRange().address);
                        stubLink.setLinkData(0xFFFFFFFF);
                        stubLink.setLinkCost(0);
                        stubLink.setNumberOfTOS(0);
                        stubLink.setTosDataArraySize(0);

                        unsigned short linkIndex = routerLSA->getLinksArraySize();
                        routerLSA->setLinksArraySize(linkIndex + 1);
                        routerLSA->setNumberOfLinks(linkIndex + 1);
                        routerLSA->setLinks(linkIndex, stubLink);

                        long neighborCount = intf->getNeighborCount();
                        for (long i = 0; i < neighborCount; i++) {
                            OSPF::Neighbor* neighbor = intf->getNeighbor(i);
                            if (neighbor->getState() == OSPF::Neighbor::FULL_STATE) {
                                Link link;
                                link.setType(POINTTOPOINT_LINK);
                                link.setLinkID(IPv4Address(neighbor->getNeighborID()));
                                link.setLinkData(intf->getAddressRange().address.getInt());
                                link.setLinkCost(intf->getOutputCost());
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
        stubLink.setType(STUB_LINK);
        stubLink.setLinkID(hostRoutes[i].address);
        stubLink.setLinkData(0xFFFFFFFF);
        stubLink.setLinkCost(hostRoutes[i].linkCost);
        stubLink.setNumberOfTOS(0);
        stubLink.setTosDataArraySize(0);

        unsigned short linkIndex = routerLSA->getLinksArraySize();
        routerLSA->setLinksArraySize(linkIndex + 1);
        routerLSA->setNumberOfLinks(linkIndex + 1);
        routerLSA->setLinks(linkIndex, stubLink);
    }

    routerLSA->setSource(OSPF::LSATrackingInfo::ORIGINATED);

    return routerLSA;
}

OSPF::NetworkLSA* OSPF::Area::originateNetworkLSA(const OSPF::Interface* intf)
{
    if (intf->hasAnyNeighborInStates(OSPF::Neighbor::FULL_STATE)) {
        OSPF::NetworkLSA* networkLSA = new OSPF::NetworkLSA;
        OSPFLSAHeader& lsaHeader = networkLSA->getHeader();
        long neighborCount = intf->getNeighborCount();
        OSPFOptions lsOptions;

        lsaHeader.setLsAge(0);
        memset(&lsOptions, 0, sizeof(OSPFOptions));
        lsOptions.E_ExternalRoutingCapability = externalRoutingCapability;
        lsaHeader.setLsOptions(lsOptions);
        lsaHeader.setLsType(NETWORKLSA_TYPE);
        lsaHeader.setLinkStateID(intf->getAddressRange().address);
        lsaHeader.setAdvertisingRouter(IPv4Address(parentRouter->getRouterID()));
        lsaHeader.setLsSequenceNumber(INITIAL_SEQUENCE_NUMBER);

        networkLSA->setNetworkMask(intf->getAddressRange().mask);

        for (long j = 0; j < neighborCount; j++) {
            const OSPF::Neighbor* neighbor = intf->getNeighbor(j);
            if (neighbor->getState() == OSPF::Neighbor::FULL_STATE) {
                unsigned short netIndex = networkLSA->getAttachedRoutersArraySize();
                networkLSA->setAttachedRoutersArraySize(netIndex + 1);
                networkLSA->setAttachedRouters(netIndex, IPv4Address(neighbor->getNeighborID()));
            }
        }
        unsigned short netIndex = networkLSA->getAttachedRoutersArraySize();
        networkLSA->setAttachedRoutersArraySize(netIndex + 1);
        networkLSA->setAttachedRouters(netIndex, IPv4Address(parentRouter->getRouterID()));

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
OSPF::LinkStateID OSPF::Area::getUniqueLinkStateID(OSPF::IPv4AddressRange destination,
                                                    OSPF::Metric destinationCost,
                                                    OSPF::SummaryLSA*& lsaToReoriginate) const
{
    if (lsaToReoriginate != NULL) {
        delete lsaToReoriginate;
        lsaToReoriginate = NULL;
    }

    OSPF::LSAKeyType lsaKey;

    lsaKey.linkStateID = destination.address;
    lsaKey.advertisingRouter = parentRouter->getRouterID();

    const OSPF::SummaryLSA* foundLSA = findSummaryLSA(lsaKey);

    if (foundLSA == NULL) {
        return lsaKey.linkStateID;
    } else {
        IPv4Address existingMask = foundLSA->getNetworkMask();

        if (destination.mask == existingMask) {
            return lsaKey.linkStateID;
        } else {
            if (destination.mask >= existingMask) {
                return lsaKey.linkStateID.getBroadcastAddress(destination.mask);
            } else {
                OSPF::SummaryLSA* summaryLSA = new OSPF::SummaryLSA(*foundLSA);

                long sequenceNumber = summaryLSA->getHeader().getLsSequenceNumber();

                summaryLSA->getHeader().setLsAge(0);
                summaryLSA->getHeader().setLsSequenceNumber((sequenceNumber == MAX_SEQUENCE_NUMBER) ? INITIAL_SEQUENCE_NUMBER : sequenceNumber + 1);
                summaryLSA->setNetworkMask(destination.mask);
                summaryLSA->setRouteCost(destinationCost);

                lsaToReoriginate = summaryLSA;

                return lsaKey.linkStateID.getBroadcastAddress(existingMask);
            }
        }
    }
}

OSPF::SummaryLSA* OSPF::Area::originateSummaryLSA(const OSPF::RoutingTableEntry* entry,
                                                   const std::map<OSPF::LSAKeyType, bool, OSPF::LSAKeyType_Less>& originatedLSAs,
                                                   OSPF::SummaryLSA*& lsaToReoriginate)
{
    if (((entry->getDestinationType() & OSPF::RoutingTableEntry::AREA_BORDER_ROUTER_DESTINATION) != 0) ||
        (entry->getPathType() == OSPF::RoutingTableEntry::TYPE1_EXTERNAL) ||
        (entry->getPathType() == OSPF::RoutingTableEntry::TYPE2_EXTERNAL) ||
        (entry->getArea() == areaID))
    {
        return NULL;
    }

    bool allNextHopsInThisArea = true;
    unsigned int nextHopCount = entry->getNextHopCount();

    for (unsigned int i = 0; i < nextHopCount; i++) {
        OSPF::Interface* nextHopInterface = parentRouter->getNonVirtualInterface(entry->getNextHop(i).ifIndex);
        if ((nextHopInterface != NULL) && (nextHopInterface->getAreaID() != areaID)) {
            allNextHopsInThisArea = false;
            break;
        }
    }
    if ((allNextHopsInThisArea) || (entry->getCost() >= LS_INFINITY)){
        return NULL;
    }

    if ((entry->getDestinationType() & OSPF::RoutingTableEntry::AS_BOUNDARY_ROUTER_DESTINATION) != 0) {
        OSPF::RoutingTableEntry* preferredEntry = parentRouter->getPreferredEntry(*(entry->getLinkStateOrigin()), false);
        if ((preferredEntry != NULL) && (*preferredEntry == *entry) && (externalRoutingCapability)) {
            OSPF::SummaryLSA* summaryLSA = new OSPF::SummaryLSA;
            OSPFLSAHeader& lsaHeader = summaryLSA->getHeader();
            OSPFOptions lsOptions;

            lsaHeader.setLsAge(0);
            memset(&lsOptions, 0, sizeof(OSPFOptions));
            lsOptions.E_ExternalRoutingCapability = externalRoutingCapability;
            lsaHeader.setLsOptions(lsOptions);
            lsaHeader.setLsType(SUMMARYLSA_ASBOUNDARYROUTERS_TYPE);
            lsaHeader.setLinkStateID(entry->getDestination());
            lsaHeader.setAdvertisingRouter(IPv4Address(parentRouter->getRouterID()));
            lsaHeader.setLsSequenceNumber(INITIAL_SEQUENCE_NUMBER);

            summaryLSA->setNetworkMask(entry->getNetmask());
            summaryLSA->setRouteCost(entry->getCost());
            summaryLSA->setTosDataArraySize(0);

            summaryLSA->setSource(OSPF::LSATrackingInfo::ORIGINATED);

            return summaryLSA;
        }
    } else {    // entry->getDestinationType() == OSPF::RoutingTableEntry::NETWORK_DESTINATION
        if (entry->getPathType() == OSPF::RoutingTableEntry::INTERAREA) {
            OSPF::IPv4AddressRange destinationRange;

            destinationRange.address = entry->getDestination();
            destinationRange.mask = entry->getNetmask();

            OSPF::LinkStateID newLinkStateID = getUniqueLinkStateID(destinationRange, entry->getCost(), lsaToReoriginate);

            if (lsaToReoriginate != NULL) {
                OSPF::LSAKeyType lsaKey;

                lsaKey.linkStateID = entry->getDestination();
                lsaKey.advertisingRouter = parentRouter->getRouterID();

                std::map<OSPF::LSAKeyType, OSPF::SummaryLSA*, OSPF::LSAKeyType_Less>::iterator lsaIt = summaryLSAsByID.find(lsaKey);
                if (lsaIt == summaryLSAsByID.end()) {
                    delete (lsaToReoriginate);
                    lsaToReoriginate = NULL;
                    return NULL;
                } else {
                    OSPF::SummaryLSA* summaryLSA = new OSPF::SummaryLSA(*(lsaIt->second));
                    OSPFLSAHeader& lsaHeader = summaryLSA->getHeader();

                    lsaHeader.setLsAge(0);
                    lsaHeader.setLsSequenceNumber(INITIAL_SEQUENCE_NUMBER);
                    lsaHeader.setLinkStateID(newLinkStateID);

                    return summaryLSA;
                }
            } else {
                OSPF::SummaryLSA* summaryLSA = new OSPF::SummaryLSA;
                OSPFLSAHeader& lsaHeader = summaryLSA->getHeader();
                OSPFOptions lsOptions;

                lsaHeader.setLsAge(0);
                memset(&lsOptions, 0, sizeof(OSPFOptions));
                lsOptions.E_ExternalRoutingCapability = externalRoutingCapability;
                lsaHeader.setLsOptions(lsOptions);
                lsaHeader.setLsType(SUMMARYLSA_NETWORKS_TYPE);
                lsaHeader.setLinkStateID(newLinkStateID);
                lsaHeader.setAdvertisingRouter(IPv4Address(parentRouter->getRouterID()));
                lsaHeader.setLsSequenceNumber(INITIAL_SEQUENCE_NUMBER);

                summaryLSA->setNetworkMask(entry->getNetmask());
                summaryLSA->setRouteCost(entry->getCost());
                summaryLSA->setTosDataArraySize(0);

                summaryLSA->setSource(OSPF::LSATrackingInfo::ORIGINATED);

                return summaryLSA;
            }
        } else {    // entry->getPathType() == OSPF::RoutingTableEntry::INTRAAREA
            OSPF::IPv4AddressRange destinationAddressRange;

            destinationAddressRange.address = entry->getDestination();
            destinationAddressRange.mask = entry->getNetmask();

            bool doAdvertise = false;
            OSPF::IPv4AddressRange containingAddressRange = parentRouter->getContainingAddressRange(destinationAddressRange, &doAdvertise);
            if (((entry->getArea() == OSPF::BACKBONE_AREAID) && // the backbone's configured ranges should be ignored
                 (transitCapability)) || // when originating Summary LSAs into transit areas
                (containingAddressRange == OSPF::NULL_IPV4ADDRESSRANGE))
            {
                OSPF::LinkStateID newLinkStateID = getUniqueLinkStateID(destinationAddressRange, entry->getCost(), lsaToReoriginate);

                if (lsaToReoriginate != NULL) {
                    OSPF::LSAKeyType lsaKey;

                    lsaKey.linkStateID = entry->getDestination();
                    lsaKey.advertisingRouter = parentRouter->getRouterID();

                    std::map<OSPF::LSAKeyType, OSPF::SummaryLSA*, OSPF::LSAKeyType_Less>::iterator lsaIt = summaryLSAsByID.find(lsaKey);
                    if (lsaIt == summaryLSAsByID.end()) {
                        delete (lsaToReoriginate);
                        lsaToReoriginate = NULL;
                        return NULL;
                    } else {
                        OSPF::SummaryLSA* summaryLSA = new OSPF::SummaryLSA(*(lsaIt->second));
                        OSPFLSAHeader& lsaHeader = summaryLSA->getHeader();

                        lsaHeader.setLsAge(0);
                        lsaHeader.setLsSequenceNumber(INITIAL_SEQUENCE_NUMBER);
                        lsaHeader.setLinkStateID(newLinkStateID);

                        return summaryLSA;
                    }
                } else {
                    OSPF::SummaryLSA* summaryLSA = new OSPF::SummaryLSA;
                    OSPFLSAHeader& lsaHeader = summaryLSA->getHeader();
                    OSPFOptions lsOptions;

                    lsaHeader.setLsAge(0);
                    memset(&lsOptions, 0, sizeof(OSPFOptions));
                    lsOptions.E_ExternalRoutingCapability = externalRoutingCapability;
                    lsaHeader.setLsOptions(lsOptions);
                    lsaHeader.setLsType(SUMMARYLSA_NETWORKS_TYPE);
                    lsaHeader.setLinkStateID(newLinkStateID);
                    lsaHeader.setAdvertisingRouter(IPv4Address(parentRouter->getRouterID()));
                    lsaHeader.setLsSequenceNumber(INITIAL_SEQUENCE_NUMBER);

                    summaryLSA->setNetworkMask(entry->getNetmask());
                    summaryLSA->setRouteCost(entry->getCost());
                    summaryLSA->setTosDataArraySize(0);

                    summaryLSA->setSource(OSPF::LSATrackingInfo::ORIGINATED);

                    return summaryLSA;
                }
            } else {
                if (doAdvertise) {
                    Metric maxRangeCost = 0;
                    unsigned long entryCount = parentRouter->getRoutingTableEntryCount();

                    for (unsigned long i = 0; i < entryCount; i++) {
                        const OSPF::RoutingTableEntry* routingEntry = parentRouter->getRoutingTableEntry(i);

                        if ((routingEntry->getDestinationType() == OSPF::RoutingTableEntry::NETWORK_DESTINATION) &&
                                (routingEntry->getPathType() == OSPF::RoutingTableEntry::INTRAAREA) &&
                                containingAddressRange.containsRange(routingEntry->getDestination(), routingEntry->getNetmask()) &&
                                (routingEntry->getCost() > maxRangeCost))
                        {
                            maxRangeCost = routingEntry->getCost();
                        }
                    }

                    OSPF::LinkStateID newLinkStateID = getUniqueLinkStateID(containingAddressRange, maxRangeCost, lsaToReoriginate);
                    OSPF::LSAKeyType lsaKey;

                    if (lsaToReoriginate != NULL) {
                        lsaKey.linkStateID = lsaToReoriginate->getHeader().getLinkStateID();
                        lsaKey.advertisingRouter = parentRouter->getRouterID();

                        std::map<OSPF::LSAKeyType, bool, OSPF::LSAKeyType_Less>::const_iterator originatedIt = originatedLSAs.find(lsaKey);
                        if (originatedIt != originatedLSAs.end()) {
                            delete (lsaToReoriginate);
                            lsaToReoriginate = NULL;
                            return NULL;
                        }

                        lsaKey.linkStateID = entry->getDestination();
                        lsaKey.advertisingRouter = parentRouter->getRouterID();

                        std::map<OSPF::LSAKeyType, OSPF::SummaryLSA*, OSPF::LSAKeyType_Less>::iterator lsaIt = summaryLSAsByID.find(lsaKey);
                        if (lsaIt == summaryLSAsByID.end()) {
                            delete (lsaToReoriginate);
                            lsaToReoriginate = NULL;
                            return NULL;
                        }

                        OSPF::SummaryLSA* summaryLSA = new OSPF::SummaryLSA(*(lsaIt->second));
                        OSPFLSAHeader& lsaHeader = summaryLSA->getHeader();

                        lsaHeader.setLsAge(0);
                        lsaHeader.setLsSequenceNumber(INITIAL_SEQUENCE_NUMBER);
                        lsaHeader.setLinkStateID(newLinkStateID);

                        return summaryLSA;
                    } else {
                        lsaKey.linkStateID = newLinkStateID;
                        lsaKey.advertisingRouter = parentRouter->getRouterID();

                        std::map<OSPF::LSAKeyType, bool, OSPF::LSAKeyType_Less>::const_iterator originatedIt = originatedLSAs.find(lsaKey);
                        if (originatedIt != originatedLSAs.end()) {
                            return NULL;
                        }

                        OSPF::SummaryLSA* summaryLSA = new OSPF::SummaryLSA;
                        OSPFLSAHeader& lsaHeader = summaryLSA->getHeader();
                        OSPFOptions lsOptions;

                        lsaHeader.setLsAge(0);
                        memset(&lsOptions, 0, sizeof(OSPFOptions));
                        lsOptions.E_ExternalRoutingCapability = externalRoutingCapability;
                        lsaHeader.setLsOptions(lsOptions);
                        lsaHeader.setLsType(SUMMARYLSA_NETWORKS_TYPE);
                        lsaHeader.setLinkStateID(newLinkStateID);
                        lsaHeader.setAdvertisingRouter(IPv4Address(parentRouter->getRouterID()));
                        lsaHeader.setLsSequenceNumber(INITIAL_SEQUENCE_NUMBER);

                        summaryLSA->setNetworkMask(entry->getNetmask());
                        summaryLSA->setRouteCost(entry->getCost());
                        summaryLSA->setTosDataArraySize(0);

                        summaryLSA->setSource(OSPF::LSATrackingInfo::ORIGINATED);

                        return summaryLSA;
                    }
                }
            }
        }
    }

    return NULL;
}

OSPF::SummaryLSA* OSPF::Area::originateSummaryLSA(const OSPF::SummaryLSA* summaryLSA)
{
    const std::map<OSPF::LSAKeyType, bool, OSPF::LSAKeyType_Less> emptyMap;
    OSPF::SummaryLSA* dontReoriginate = NULL;

    const OSPFLSAHeader& lsaHeader = summaryLSA->getHeader();
    unsigned long entryCount = parentRouter->getRoutingTableEntryCount();

    for (unsigned long i = 0; i < entryCount; i++) {
        const OSPF::RoutingTableEntry* entry = parentRouter->getRoutingTableEntry(i);

        if ((lsaHeader.getLsType() == SUMMARYLSA_ASBOUNDARYROUTERS_TYPE) &&
            ((((entry->getDestinationType() & OSPF::RoutingTableEntry::AREA_BORDER_ROUTER_DESTINATION) != 0) ||
              ((entry->getDestinationType() & OSPF::RoutingTableEntry::AS_BOUNDARY_ROUTER_DESTINATION) != 0)) &&
             ((entry->getDestination() == lsaHeader.getLinkStateID()) &&    //FIXME Why not compare network addresses (addr masked with netmask)?
              (entry->getNetmask() == summaryLSA->getNetworkMask()))))
        {
            OSPF::SummaryLSA* returnLSA = originateSummaryLSA(entry, emptyMap, dontReoriginate);
            if (dontReoriginate != NULL) {
                delete dontReoriginate;
            }
            return returnLSA;
        }

        IPv4Address lsaMask = summaryLSA->getNetworkMask();

        if ((lsaHeader.getLsType() == SUMMARYLSA_NETWORKS_TYPE) &&
            (entry->getDestinationType() == OSPF::RoutingTableEntry::NETWORK_DESTINATION) &&
            isSameNetwork(entry->getDestination(), entry->getNetmask(), lsaHeader.getLinkStateID(), lsaMask))
        {
            OSPF::SummaryLSA* returnLSA = originateSummaryLSA(entry, emptyMap, dontReoriginate);
            if (dontReoriginate != NULL) {
                delete dontReoriginate;
            }
            return returnLSA;
        }
    }

    return NULL;
}

void OSPF::Area::calculateShortestPathTree(std::vector<OSPF::RoutingTableEntry*>& newRoutingTable)
{
    OSPF::RouterID routerID = parentRouter->getRouterID();
    bool finished = false;
    std::vector<OSPFLSA*> treeVertices;
    OSPFLSA* justAddedVertex;
    std::vector<OSPFLSA*> candidateVertices;
    unsigned long            i, j, k;
    unsigned long lsaCount;

    if (spfTreeRoot == NULL) {
        OSPF::RouterLSA* newLSA = originateRouterLSA();

        installRouterLSA(newLSA);

        OSPF::RouterLSA* routerLSA = findRouterLSA(routerID);

        spfTreeRoot = routerLSA;
        floodLSA(newLSA);
        delete newLSA;
    }
    if (spfTreeRoot == NULL) {
        return;
    }

    lsaCount = routerLSAs.size();
    for (i = 0; i < lsaCount; i++) {
        routerLSAs[i]->clearNextHops();
    }
    lsaCount = networkLSAs.size();
    for (i = 0; i < lsaCount; i++) {
        networkLSAs[i]->clearNextHops();
    }
    spfTreeRoot->setDistance(0);
    treeVertices.push_back(spfTreeRoot);
    justAddedVertex = spfTreeRoot;          // (1)

    do {
        LSAType vertexType = static_cast<LSAType> (justAddedVertex->getHeader().getLsType());

        if ((vertexType == ROUTERLSA_TYPE)) {
            OSPF::RouterLSA* routerVertex = check_and_cast<OSPF::RouterLSA*> (justAddedVertex);
            if (routerVertex->getV_VirtualLinkEndpoint()) {    // (2)
                transitCapability = true;
            }

            unsigned int linkCount = routerVertex->getLinksArraySize();
            for (i = 0; i < linkCount; i++) {
                Link& link = routerVertex->getLinks(i);
                LinkType linkType = static_cast<LinkType> (link.getType());
                OSPFLSA* joiningVertex;
                LSAType joiningVertexType;

                if (linkType == STUB_LINK) {     // (2) (a)
                    continue;
                }

                if (linkType == TRANSIT_LINK) {
                    joiningVertex = findNetworkLSA(link.getLinkID());
                    joiningVertexType = NETWORKLSA_TYPE;
                } else {
                    joiningVertex = findRouterLSA(link.getLinkID());
                    joiningVertexType = ROUTERLSA_TYPE;
                }

                if ((joiningVertex == NULL) ||
                    (joiningVertex->getHeader().getLsAge() == MAX_AGE) ||
                    (!hasLink(joiningVertex, justAddedVertex)))  // (from, to)     (2) (b)
                {
                    continue;
                }

                unsigned int treeSize = treeVertices.size();
                bool alreadyOnTree = false;

                for (j = 0; j < treeSize; j++) {
                    if (treeVertices[j] == joiningVertex) {
                        alreadyOnTree = true;
                        break;
                    }
                }
                if (alreadyOnTree) {    // (2) (c)
                    continue;
                }

                unsigned long linkStateCost = routerVertex->getDistance() + link.getLinkCost();
                unsigned int candidateCount = candidateVertices.size();
                OSPFLSA* candidate = NULL;

                for (j = 0; j < candidateCount; j++) {
                    if (candidateVertices[j] == joiningVertex) {
                        candidate = candidateVertices[j];
                    }
                }
                if (candidate != NULL) {    // (2) (d)
                    OSPF::RoutingInfo* routingInfo = check_and_cast<OSPF::RoutingInfo*> (candidate);
                    unsigned long candidateDistance = routingInfo->getDistance();

                    if (linkStateCost > candidateDistance) {
                        continue;
                    }
                    if (linkStateCost < candidateDistance) {
                        routingInfo->setDistance(linkStateCost);
                        routingInfo->clearNextHops();
                    }
                    std::vector<OSPF::NextHop>* newNextHops = calculateNextHops(joiningVertex, justAddedVertex); // (destination, parent)
                    unsigned int nextHopCount = newNextHops->size();
                    for (k = 0; k < nextHopCount; k++) {
                        routingInfo->addNextHop((*newNextHops)[k]);
                    }
                    delete newNextHops;
                } else {
                    if (joiningVertexType == ROUTERLSA_TYPE) {
                        OSPF::RouterLSA* joiningRouterVertex = check_and_cast<OSPF::RouterLSA*> (joiningVertex);
                        joiningRouterVertex->setDistance(linkStateCost);
                        std::vector<OSPF::NextHop>* newNextHops = calculateNextHops(joiningVertex, justAddedVertex); // (destination, parent)
                        unsigned int nextHopCount = newNextHops->size();
                        for (k = 0; k < nextHopCount; k++) {
                            joiningRouterVertex->addNextHop((*newNextHops)[k]);
                        }
                        delete newNextHops;
                        OSPF::RoutingInfo* vertexRoutingInfo = check_and_cast<OSPF::RoutingInfo*> (joiningRouterVertex);
                        vertexRoutingInfo->setParent(justAddedVertex);

                        candidateVertices.push_back(joiningRouterVertex);
                    } else {
                        OSPF::NetworkLSA* joiningNetworkVertex = check_and_cast<OSPF::NetworkLSA*> (joiningVertex);
                        joiningNetworkVertex->setDistance(linkStateCost);
                        std::vector<OSPF::NextHop>* newNextHops = calculateNextHops(joiningVertex, justAddedVertex); // (destination, parent)
                        unsigned int nextHopCount = newNextHops->size();
                        for (k = 0; k < nextHopCount; k++) {
                            joiningNetworkVertex->addNextHop((*newNextHops)[k]);
                        }
                        delete newNextHops;
                        OSPF::RoutingInfo* vertexRoutingInfo = check_and_cast<OSPF::RoutingInfo*> (joiningNetworkVertex);
                        vertexRoutingInfo->setParent(justAddedVertex);

                        candidateVertices.push_back(joiningNetworkVertex);
                    }
                }
            }
        }

        if ((vertexType == NETWORKLSA_TYPE)) {
            OSPF::NetworkLSA* networkVertex = check_and_cast<OSPF::NetworkLSA*> (justAddedVertex);
            unsigned int routerCount = networkVertex->getAttachedRoutersArraySize();

            for (i = 0; i < routerCount; i++) {     // (2)
                OSPF::RouterLSA* joiningVertex = findRouterLSA(networkVertex->getAttachedRouters(i));
                if ((joiningVertex == NULL) ||
                    (joiningVertex->getHeader().getLsAge() == MAX_AGE) ||
                    (!hasLink(joiningVertex, justAddedVertex)))  // (from, to)     (2) (b)
                {
                    continue;
                }

                unsigned int treeSize = treeVertices.size();
                bool alreadyOnTree = false;

                for (j = 0; j < treeSize; j++) {
                    if (treeVertices[j] == joiningVertex) {
                        alreadyOnTree = true;
                        break;
                    }
                }
                if (alreadyOnTree) {    // (2) (c)
                    continue;
                }

                unsigned long linkStateCost = networkVertex->getDistance();   // link cost from network to router is always 0
                unsigned int candidateCount = candidateVertices.size();
                OSPFLSA* candidate = NULL;

                for (j = 0; j < candidateCount; j++) {
                    if (candidateVertices[j] == joiningVertex) {
                        candidate = candidateVertices[j];
                    }
                }
                if (candidate != NULL) {    // (2) (d)
                    OSPF::RoutingInfo* routingInfo = check_and_cast<OSPF::RoutingInfo*> (candidate);
                    unsigned long candidateDistance = routingInfo->getDistance();

                    if (linkStateCost > candidateDistance) {
                        continue;
                    }
                    if (linkStateCost < candidateDistance) {
                        routingInfo->setDistance(linkStateCost);
                        routingInfo->clearNextHops();
                    }
                    std::vector<OSPF::NextHop>* newNextHops = calculateNextHops(joiningVertex, justAddedVertex); // (destination, parent)
                    unsigned int nextHopCount = newNextHops->size();
                    for (k = 0; k < nextHopCount; k++) {
                        routingInfo->addNextHop((*newNextHops)[k]);
                    }
                    delete newNextHops;
                } else {
                    joiningVertex->setDistance(linkStateCost);
                    std::vector<OSPF::NextHop>* newNextHops = calculateNextHops(joiningVertex, justAddedVertex); // (destination, parent)
                    unsigned int nextHopCount = newNextHops->size();
                    for (k = 0; k < nextHopCount; k++) {
                        joiningVertex->addNextHop((*newNextHops)[k]);
                    }
                    delete newNextHops;
                    OSPF::RoutingInfo* vertexRoutingInfo = check_and_cast<OSPF::RoutingInfo*> (joiningVertex);
                    vertexRoutingInfo->setParent(justAddedVertex);

                    candidateVertices.push_back(joiningVertex);
                }
            }
        }

        if (candidateVertices.empty()) {  // (3)
            finished = true;
        } else {
            unsigned int candidateCount = candidateVertices.size();
            unsigned long minDistance = LS_INFINITY;
            OSPFLSA* closestVertex = candidateVertices[0];

            for (i = 0; i < candidateCount; i++) {
                OSPF::RoutingInfo* routingInfo = check_and_cast<OSPF::RoutingInfo*> (candidateVertices[i]);
                unsigned long currentDistance = routingInfo->getDistance();

                if (currentDistance < minDistance) {
                    closestVertex = candidateVertices[i];
                    minDistance = currentDistance;
                } else {
                    if (currentDistance == minDistance) {
                        if ((closestVertex->getHeader().getLsType() == ROUTERLSA_TYPE) &&
                            (candidateVertices[i]->getHeader().getLsType() == NETWORKLSA_TYPE))
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

            if (closestVertex->getHeader().getLsType() == ROUTERLSA_TYPE) {
                OSPF::RouterLSA* routerLSA = check_and_cast<OSPF::RouterLSA*> (closestVertex);
                if (routerLSA->getB_AreaBorderRouter() || routerLSA->getE_ASBoundaryRouter()) {
                    OSPF::RoutingTableEntry* entry = new OSPF::RoutingTableEntry;
                    OSPF::RouterID destinationID = routerLSA->getHeader().getLinkStateID();
                    unsigned int nextHopCount = routerLSA->getNextHopCount();
                    OSPF::RoutingTableEntry::RoutingDestinationType destinationType = OSPF::RoutingTableEntry::NETWORK_DESTINATION;

                    entry->setDestination(destinationID);
                    entry->setLinkStateOrigin(routerLSA);
                    entry->setArea(areaID);
                    entry->setPathType(OSPF::RoutingTableEntry::INTRAAREA);
                    entry->setCost(routerLSA->getDistance());
                    if (routerLSA->getB_AreaBorderRouter()) {
                        destinationType |= OSPF::RoutingTableEntry::AREA_BORDER_ROUTER_DESTINATION;
                    }
                    if (routerLSA->getE_ASBoundaryRouter()) {
                        destinationType |= OSPF::RoutingTableEntry::AS_BOUNDARY_ROUTER_DESTINATION;
                    }
                    entry->setDestinationType(destinationType);
                    entry->setOptionalCapabilities(routerLSA->getHeader().getLsOptions());
                    for (i = 0; i < nextHopCount; i++) {
                        entry->addNextHop(routerLSA->getNextHop(i));
                    }

                    newRoutingTable.push_back(entry);

                    OSPF::Area* backbone;
                    if (areaID != OSPF::BACKBONE_AREAID) {
                        backbone = parentRouter->getAreaByID(OSPF::BACKBONE_AREAID);
                    } else {
                        backbone = this;
                    }
                    if (backbone != NULL) {
                        OSPF::Interface* virtualIntf = backbone->findVirtualLink(destinationID);
                        if ((virtualIntf != NULL) && (virtualIntf->getTransitAreaID() == areaID)) {
                            OSPF::IPv4AddressRange range;
                            range.address = getInterface(routerLSA->getNextHop(0).ifIndex)->getAddressRange().address;
                            range.mask = IPv4Address::ALLONES_ADDRESS;
                            virtualIntf->setAddressRange(range);
                            virtualIntf->setIfIndex(routerLSA->getNextHop(0).ifIndex);
                            virtualIntf->setOutputCost(routerLSA->getDistance());
                            OSPF::Neighbor* virtualNeighbor = virtualIntf->getNeighbor(0);
                            if (virtualNeighbor != NULL) {
                                unsigned int linkCount = routerLSA->getLinksArraySize();
                                OSPF::RouterLSA* toRouterLSA = dynamic_cast<OSPF::RouterLSA*> (justAddedVertex);
                                if (toRouterLSA != NULL) {
                                    for (i = 0; i < linkCount; i++) {
                                        Link& link = routerLSA->getLinks(i);

                                        if ((link.getType() == POINTTOPOINT_LINK) &&
                                            (link.getLinkID() == toRouterLSA->getHeader().getLinkStateID()) &&
                                            (virtualIntf->getState() < OSPF::Interface::WAITING_STATE))
                                        {
                                            virtualNeighbor->setAddress(IPv4Address(link.getLinkData()));
                                            virtualIntf->processEvent(OSPF::Interface::INTERFACE_UP);
                                            break;
                                        }
                                    }
                                } else {
                                    OSPF::NetworkLSA* toNetworkLSA = dynamic_cast<OSPF::NetworkLSA*> (justAddedVertex);
                                    if (toNetworkLSA != NULL) {
                                        for (i = 0; i < linkCount; i++) {
                                            Link& link = routerLSA->getLinks(i);

                                            if ((link.getType() == TRANSIT_LINK) &&
                                                (link.getLinkID() == toNetworkLSA->getHeader().getLinkStateID()) &&
                                                (virtualIntf->getState() < OSPF::Interface::WAITING_STATE))
                                            {
                                                virtualNeighbor->setAddress(IPv4Address(link.getLinkData()));
                                                virtualIntf->processEvent(OSPF::Interface::INTERFACE_UP);
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

            if (closestVertex->getHeader().getLsType() == NETWORKLSA_TYPE) {
                OSPF::NetworkLSA* networkLSA = check_and_cast<OSPF::NetworkLSA*> (closestVertex);
                IPv4Address destinationID = (networkLSA->getHeader().getLinkStateID() & networkLSA->getNetworkMask());
                unsigned int nextHopCount = networkLSA->getNextHopCount();
                bool overWrite = false;
                OSPF::RoutingTableEntry* entry = NULL;
                unsigned long routeCount = newRoutingTable.size();
                IPv4Address longestMatch(0u);

                for (i = 0; i < routeCount; i++) {
                    if (newRoutingTable[i]->getDestinationType() == OSPF::RoutingTableEntry::NETWORK_DESTINATION) {
                        OSPF::RoutingTableEntry* routingEntry = newRoutingTable[i];
                        IPv4Address entryAddress = routingEntry->getDestination();
                        IPv4Address entryMask = routingEntry->getNetmask();

                        if ((entryAddress & entryMask) == (destinationID & entryMask)) {
                            if ((destinationID & entryMask) > longestMatch) {
                                longestMatch = (destinationID & entryMask);
                                entry = routingEntry;
                            }
                        }
                    }
                }
                if (entry != NULL) {
                    const OSPFLSA* entryOrigin = entry->getLinkStateOrigin();
                    if ((entry->getCost() != networkLSA->getDistance()) ||
                        (entryOrigin->getHeader().getLinkStateID() >= networkLSA->getHeader().getLinkStateID()))
                    {
                        overWrite = true;
                    }
                }

                if ((entry == NULL) || (overWrite)) {
                    if (entry == NULL) {
                        entry = new OSPF::RoutingTableEntry;
                    }

                    entry->setDestination(IPv4Address(destinationID));
                    entry->setNetmask(networkLSA->getNetworkMask());
                    entry->setLinkStateOrigin(networkLSA);
                    entry->setArea(areaID);
                    entry->setPathType(OSPF::RoutingTableEntry::INTRAAREA);
                    entry->setCost(networkLSA->getDistance());
                    entry->setDestinationType(OSPF::RoutingTableEntry::NETWORK_DESTINATION);
                    entry->setOptionalCapabilities(networkLSA->getHeader().getLsOptions());
                    for (i = 0; i < nextHopCount; i++) {
                        entry->addNextHop(networkLSA->getNextHop(i));
                    }

                    if (!overWrite) {
                        newRoutingTable.push_back(entry);
                    }
                }
            }

            justAddedVertex = closestVertex;
        }
    } while (!finished);

    unsigned int treeSize = treeVertices.size();
    for (i = 0; i < treeSize; i++) {
        OSPF::RouterLSA* routerVertex = dynamic_cast<OSPF::RouterLSA*> (treeVertices[i]);
        if (routerVertex == NULL) {
            continue;
        }

        unsigned int linkCount = routerVertex->getLinksArraySize();
        for (j = 0; j < linkCount; j++) {
            Link& link = routerVertex->getLinks(j);
            if (link.getType() != STUB_LINK) {
                continue;
            }

            unsigned long distance = routerVertex->getDistance() + link.getLinkCost();
            unsigned long destinationID = (link.getLinkID().getInt() & link.getLinkData());
            OSPF::RoutingTableEntry* entry = NULL;
            unsigned long routeCount = newRoutingTable.size();
            unsigned long longestMatch = 0;

            for (k = 0; k < routeCount; k++) {
                if (newRoutingTable[k]->getDestinationType() == OSPF::RoutingTableEntry::NETWORK_DESTINATION) {
                    OSPF::RoutingTableEntry* routingEntry = newRoutingTable[k];
                    unsigned long entryAddress = routingEntry->getDestination().getInt();
                    unsigned long entryMask = routingEntry->getNetmask().getInt();

                    if ((entryAddress & entryMask) == (destinationID & entryMask)) {
                        if ((destinationID & entryMask) > longestMatch) {
                            longestMatch = (destinationID & entryMask);
                            entry = routingEntry;
                        }
                    }
                }
            }

            if (entry != NULL) {
                Metric entryCost = entry->getCost();

                if (distance > entryCost) {
                    continue;
                }
                if (distance < entryCost) {
                    //FIXME remove
                    //if(parentRouter->getRouterID() == 0xC0A80302) {
                    //    EV << "CHEAPER STUB LINK FOUND TO " << IPv4Address(destinationID).str() << "\n";
                    //}
                    entry->setCost(distance);
                    entry->clearNextHops();
                    entry->setLinkStateOrigin(routerVertex);
                }
                if (distance == entryCost) {
                    // no const version from check_and_cast
                    const OSPFLSA *lsOrigin = entry->getLinkStateOrigin();
                    if (dynamic_cast<const OSPF::RouterLSA*> (lsOrigin)  || dynamic_cast<const OSPF::NetworkLSA*> (lsOrigin)) {
                        if (lsOrigin->getHeader().getLinkStateID() < routerVertex->getHeader().getLinkStateID()) {
                            entry->setLinkStateOrigin(routerVertex);
                        }
                    } else {
                        throw cRuntimeError("Can not cast class '%s' to OSPF::RouterLSA or OSPF::NetworkLSA", lsOrigin->getClassName());
                    }
                }
                std::vector<OSPF::NextHop>* newNextHops = calculateNextHops(link, routerVertex); // (destination, parent)
                unsigned int nextHopCount = newNextHops->size();
                for (k = 0; k < nextHopCount; k++) {
                    entry->addNextHop((*newNextHops)[k]);
                }
                delete newNextHops;
            } else {
                //FIXME remove
                //if(parentRouter->getRouterID() == 0xC0A80302) {
                //    EV << "STUB LINK FOUND TO " << IPv4Address(destinationID).str() << "\n";
                //}
                entry = new OSPF::RoutingTableEntry;

                entry->setDestination(IPv4Address(destinationID));
                entry->setNetmask(IPv4Address(link.getLinkData()));
                entry->setLinkStateOrigin(routerVertex);
                entry->setArea(areaID);
                entry->setPathType(OSPF::RoutingTableEntry::INTRAAREA);
                entry->setCost(distance);
                entry->setDestinationType(OSPF::RoutingTableEntry::NETWORK_DESTINATION);
                entry->setOptionalCapabilities(routerVertex->getHeader().getLsOptions());
                std::vector<OSPF::NextHop>* newNextHops = calculateNextHops(link, routerVertex); // (destination, parent)
                unsigned int nextHopCount = newNextHops->size();
                for (k = 0; k < nextHopCount; k++) {
                    entry->addNextHop((*newNextHops)[k]);
                }
                delete newNextHops;

                newRoutingTable.push_back(entry);
            }
        }
    }
}

std::vector<OSPF::NextHop>* OSPF::Area::calculateNextHops(OSPFLSA* destination, OSPFLSA* parent) const
{
    std::vector<OSPF::NextHop>* hops = new std::vector<OSPF::NextHop>;
    unsigned long               i, j;

    OSPF::RouterLSA* routerLSA = dynamic_cast<OSPF::RouterLSA*> (parent);
    if (routerLSA != NULL) {
        if (routerLSA != spfTreeRoot) {
            unsigned int nextHopCount = routerLSA->getNextHopCount();
            for (i = 0; i < nextHopCount; i++) {
                hops->push_back(routerLSA->getNextHop(i));
            }
            return hops;
        } else {
            OSPF::RouterLSA* destinationRouterLSA = dynamic_cast<OSPF::RouterLSA*> (destination);
            if (destinationRouterLSA != NULL) {
                unsigned long interfaceNum = associatedInterfaces.size();
                for (i = 0; i < interfaceNum; i++) {
                    OSPF::Interface::OSPFInterfaceType intfType = associatedInterfaces[i]->getType();
                    if ((intfType == OSPF::Interface::POINTTOPOINT) ||
                        ((intfType == OSPF::Interface::VIRTUAL) &&
                         (associatedInterfaces[i]->getState() > OSPF::Interface::LOOPBACK_STATE)))
                    {
                        OSPF::Neighbor* ptpNeighbor = associatedInterfaces[i]->getNeighborCount() > 0 ? associatedInterfaces[i]->getNeighbor(0) : NULL;
                        if (ptpNeighbor != NULL) {
                            if (ptpNeighbor->getNeighborID() == destinationRouterLSA->getHeader().getLinkStateID()) {
                                NextHop nextHop;
                                nextHop.ifIndex = associatedInterfaces[i]->getIfIndex();
                                nextHop.hopAddress = ptpNeighbor->getAddress();
                                nextHop.advertisingRouter = destinationRouterLSA->getHeader().getAdvertisingRouter();
                                hops->push_back(nextHop);
                                break;
                            }
                        }
                    }
                    if (intfType == OSPF::Interface::POINTTOMULTIPOINT) {
                        OSPF::Neighbor* ptmpNeighbor = associatedInterfaces[i]->getNeighborByID(destinationRouterLSA->getHeader().getLinkStateID());
                        if (ptmpNeighbor != NULL) {
                            unsigned int linkCount = destinationRouterLSA->getLinksArraySize();
                            IPv4Address rootID = IPv4Address(parentRouter->getRouterID());
                            for (j = 0; j < linkCount; j++) {
                                Link& link = destinationRouterLSA->getLinks(j);
                                if (link.getLinkID() == rootID) {
                                    NextHop nextHop;
                                    nextHop.ifIndex = associatedInterfaces[i]->getIfIndex();
                                    nextHop.hopAddress = IPv4Address(link.getLinkData());
                                    nextHop.advertisingRouter = destinationRouterLSA->getHeader().getAdvertisingRouter();
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
                    IPv4Address networkDesignatedRouter = destinationNetworkLSA->getHeader().getLinkStateID();
                    unsigned long interfaceNum = associatedInterfaces.size();
                    for (i = 0; i < interfaceNum; i++) {
                        OSPF::Interface::OSPFInterfaceType intfType = associatedInterfaces[i]->getType();
                        if (((intfType == OSPF::Interface::BROADCAST) ||
                             (intfType == OSPF::Interface::NBMA)) &&
                            (associatedInterfaces[i]->getDesignatedRouter().ipInterfaceAddress == networkDesignatedRouter))
                        {
                            OSPF::IPv4AddressRange range = associatedInterfaces[i]->getAddressRange();
                            NextHop nextHop;

                            nextHop.ifIndex = associatedInterfaces[i]->getIfIndex();
                            //nextHop.hopAddress = (range.address & range.mask); //TODO revise it!
                            nextHop.hopAddress = IPv4Address::UNSPECIFIED_ADDRESS; //TODO revise it!
                            nextHop.advertisingRouter = destinationNetworkLSA->getHeader().getAdvertisingRouter();
                            hops->push_back(nextHop);
                        }
                    }
                }
            }
        }
    } else {
        OSPF::NetworkLSA* networkLSA = dynamic_cast<OSPF::NetworkLSA*> (parent);
        if (networkLSA != NULL) {
            if (networkLSA->getParent() != spfTreeRoot) {
                unsigned int nextHopCount = networkLSA->getNextHopCount();
                for (i = 0; i < nextHopCount; i++) {
                    hops->push_back(networkLSA->getNextHop(i));
                }
                return hops;
            } else {
                IPv4Address parentLinkStateID = parent->getHeader().getLinkStateID();

                OSPF::RouterLSA* destinationRouterLSA = dynamic_cast<OSPF::RouterLSA*> (destination);
                if (destinationRouterLSA != NULL) {
                    OSPF::RouterID destinationRouterID = destinationRouterLSA->getHeader().getLinkStateID();
                    unsigned int linkCount = destinationRouterLSA->getLinksArraySize();
                    for (i = 0; i < linkCount; i++) {
                        Link& link = destinationRouterLSA->getLinks(i);
                        NextHop nextHop;

                        if (((link.getType() == TRANSIT_LINK) &&
                             (link.getLinkID() == parentLinkStateID)) ||
                            ((link.getType() == STUB_LINK) &&
                             ((link.getLinkID() & IPv4Address(link.getLinkData())) == (parentLinkStateID & networkLSA->getNetworkMask()))))
                        {
                            unsigned long interfaceNum = associatedInterfaces.size();
                            for (j = 0; j < interfaceNum; j++) {
                                OSPF::Interface::OSPFInterfaceType intfType = associatedInterfaces[j]->getType();
                                if (((intfType == OSPF::Interface::BROADCAST) ||
                                     (intfType == OSPF::Interface::NBMA)) &&
                                    (associatedInterfaces[j]->getDesignatedRouter().ipInterfaceAddress == parentLinkStateID))
                                {
                                    OSPF::Neighbor* nextHopNeighbor = associatedInterfaces[j]->getNeighborByID(destinationRouterID);
                                    if (nextHopNeighbor != NULL) {
                                        nextHop.ifIndex = associatedInterfaces[j]->getIfIndex();
                                        nextHop.hopAddress = nextHopNeighbor->getAddress();
                                        nextHop.advertisingRouter = destinationRouterLSA->getHeader().getAdvertisingRouter();
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

std::vector<OSPF::NextHop>* OSPF::Area::calculateNextHops(Link& destination, OSPFLSA* parent) const
{
    std::vector<OSPF::NextHop>* hops = new std::vector<OSPF::NextHop>;
    unsigned long i;

    OSPF::RouterLSA* routerLSA = check_and_cast<OSPF::RouterLSA*> (parent);
    if (routerLSA != spfTreeRoot) {
        unsigned int nextHopCount = routerLSA->getNextHopCount();
        for (i = 0; i < nextHopCount; i++) {
            hops->push_back(routerLSA->getNextHop(i));
        }
        return hops;
    } else {
        unsigned long interfaceNum = associatedInterfaces.size();
        for (i = 0; i < interfaceNum; i++) {
            OSPF::Interface *interface = associatedInterfaces[i];
            OSPF::Interface::OSPFInterfaceType intfType = interface->getType();

            if ((intfType == OSPF::Interface::POINTTOPOINT) ||
                ((intfType == OSPF::Interface::VIRTUAL) &&
                 (interface->getState() > OSPF::Interface::LOOPBACK_STATE)))
            {
                OSPF::Neighbor* neighbor = (interface->getNeighborCount() > 0) ? interface->getNeighbor(0) : NULL;
                if (neighbor != NULL) {
                    IPv4Address neighborAddress = neighbor->getAddress();
                    if (((neighborAddress != OSPF::NULL_IPV4ADDRESS) &&
                         (neighborAddress == destination.getLinkID())) ||
                        ((neighborAddress == OSPF::NULL_IPV4ADDRESS) &&
                         (interface->getAddressRange().address == destination.getLinkID()) &&
                         (interface->getAddressRange().mask.getInt() == destination.getLinkData())))
                    {
                        NextHop nextHop;
                        nextHop.ifIndex = interface->getIfIndex();
                        nextHop.hopAddress = neighborAddress;
                        nextHop.advertisingRouter = parentRouter->getRouterID();
                        hops->push_back(nextHop);
                        break;
                    }
                }
            }
            if ((intfType == OSPF::Interface::BROADCAST) ||
                (intfType == OSPF::Interface::NBMA))
            {
                if (isSameNetwork(destination.getLinkID(), IPv4Address(destination.getLinkData()), interface->getAddressRange().address, interface->getAddressRange().mask))
                {
                    NextHop nextHop;
                    nextHop.ifIndex = interface->getIfIndex();
                    // TODO: this has been commented because the linkID is not a real IP address in this case and we don't know the next hop address here, verify
                    // nextHop.hopAddress = destination.getLinkID();
                    nextHop.advertisingRouter = parentRouter->getRouterID();
                    hops->push_back(nextHop);
                    break;
                }
            }
            if (intfType == OSPF::Interface::POINTTOMULTIPOINT) {
                if (destination.getType() == STUB_LINK) {
                    if (destination.getLinkID() == interface->getAddressRange().address) {
                        // The link contains the router's own interface address and a full mask,
                        // so we insert a next hop pointing to the interface itself. Kind of pointless, but
                        // not much else we could do...
                        // TODO: check what other OSPF implementations do in this situation
                        NextHop nextHop;
                        nextHop.ifIndex = interface->getIfIndex();
                        nextHop.hopAddress = interface->getAddressRange().address;
                        nextHop.advertisingRouter = parentRouter->getRouterID();
                        hops->push_back(nextHop);
                        break;
                    }
                }
                if (destination.getType() == POINTTOPOINT_LINK) {
                    OSPF::Neighbor* neighbor = interface->getNeighborByID(destination.getLinkID());
                    if (neighbor != NULL) {
                        NextHop nextHop;
                        nextHop.ifIndex = interface->getIfIndex();
                        nextHop.hopAddress = neighbor->getAddress();
                        nextHop.advertisingRouter = parentRouter->getRouterID();
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
                if ((destination.getLinkID() == hostRoutes[i].address) &&
                    (destination.getLinkData() == 0xFFFFFFFF))
                {
                    NextHop nextHop;
                    nextHop.ifIndex = hostRoutes[i].ifIndex;
                    nextHop.hopAddress = hostRoutes[i].address;
                    nextHop.advertisingRouter = parentRouter->getRouterID();
                    hops->push_back(nextHop);
                    break;
                }
            }
        }
    }

    return hops;
}

bool OSPF::Area::hasLink(OSPFLSA* fromLSA, OSPFLSA* toLSA) const
{
    unsigned int i;

    OSPF::RouterLSA* fromRouterLSA = dynamic_cast<OSPF::RouterLSA*> (fromLSA);
    if (fromRouterLSA != NULL) {
        unsigned int linkCount = fromRouterLSA->getLinksArraySize();
        OSPF::RouterLSA* toRouterLSA = dynamic_cast<OSPF::RouterLSA*> (toLSA);
        if (toRouterLSA != NULL) {
            for (i = 0; i < linkCount; i++) {
                Link& link = fromRouterLSA->getLinks(i);
                LinkType linkType = static_cast<LinkType> (link.getType());

                if (((linkType == POINTTOPOINT_LINK) ||
                     (linkType == VIRTUAL_LINK)) &&
                    (link.getLinkID() == toRouterLSA->getHeader().getLinkStateID()))
                {
                    return true;
                }
            }
        } else {
            OSPF::NetworkLSA* toNetworkLSA = dynamic_cast<OSPF::NetworkLSA*> (toLSA);
            if (toNetworkLSA != NULL) {
                for (i = 0; i < linkCount; i++) {
                    Link& link = fromRouterLSA->getLinks(i);

                    if ((link.getType() == TRANSIT_LINK) &&
                        (link.getLinkID() == toNetworkLSA->getHeader().getLinkStateID()))
                    {
                        return true;
                    }
                    if ((link.getType() == STUB_LINK) &&
                        ((link.getLinkID() & IPv4Address(link.getLinkData())) == (toNetworkLSA->getHeader().getLinkStateID() & toNetworkLSA->getNetworkMask())))   //FIXME should compare masks?
                    {
                        return true;
                    }
                }
            }
        }
    } else {
        OSPF::NetworkLSA* fromNetworkLSA = dynamic_cast<OSPF::NetworkLSA*> (fromLSA);
        if (fromNetworkLSA != NULL) {
            unsigned int routerCount = fromNetworkLSA->getAttachedRoutersArraySize();
            OSPF::RouterLSA* toRouterLSA = dynamic_cast<OSPF::RouterLSA*> (toLSA);
            if (toRouterLSA != NULL) {
                for (i = 0; i < routerCount; i++) {
                    if (fromNetworkLSA->getAttachedRouters(i) == toRouterLSA->getHeader().getLinkStateID()) {
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
bool OSPF::Area::findSameOrWorseCostRoute(const std::vector<OSPF::RoutingTableEntry*>& newRoutingTable,
                                           const OSPF::SummaryLSA&                      summaryLSA,
                                           unsigned short                               currentCost,
                                           bool&                                        destinationInRoutingTable,
                                           std::list<OSPF::RoutingTableEntry*>&         sameOrWorseCost) const
{
    destinationInRoutingTable = false;
    sameOrWorseCost.clear();

    long routeCount = newRoutingTable.size();
    OSPF::IPv4AddressRange destination;

    destination.address = summaryLSA.getHeader().getLinkStateID();
    destination.mask = summaryLSA.getNetworkMask();

    for (long j = 0; j < routeCount; j++) {
        OSPF::RoutingTableEntry* routingEntry = newRoutingTable[j];
        bool foundMatching = false;

        if (summaryLSA.getHeader().getLsType() == SUMMARYLSA_NETWORKS_TYPE) {
            if ((routingEntry->getDestinationType() == OSPF::RoutingTableEntry::NETWORK_DESTINATION) &&
                isSameNetwork(destination.address, destination.mask, routingEntry->getDestination(), routingEntry->getNetmask()))           //TODO  or use containing ?
            {
                foundMatching = true;
            }
        } else {
            if ((((routingEntry->getDestinationType() & OSPF::RoutingTableEntry::AREA_BORDER_ROUTER_DESTINATION) != 0) ||
                 ((routingEntry->getDestinationType() & OSPF::RoutingTableEntry::AS_BOUNDARY_ROUTER_DESTINATION) != 0)) &&
                (destination.address == routingEntry->getDestination()) &&
                (destination.mask == routingEntry->getNetmask()))
            {
                foundMatching = true;
            }
        }

        if (foundMatching) {
            destinationInRoutingTable = true;

            /* If the matching entry is an INTRAAREA getRoute(intra-area paths are
                * always preferred to other paths of any cost), or it's a cheaper INTERAREA
                * route, then skip this LSA.
                */
            if ((routingEntry->getPathType() == OSPF::RoutingTableEntry::INTRAAREA) ||
                ((routingEntry->getPathType() == OSPF::RoutingTableEntry::INTERAREA) &&
                 (routingEntry->getCost() < currentCost)))
            {
                return true;
            } else {
                // if it's an other INTERAREA path
                if ((routingEntry->getPathType() == OSPF::RoutingTableEntry::INTERAREA) &&
                    (routingEntry->getCost() >= currentCost))
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
OSPF::RoutingTableEntry* OSPF::Area::createRoutingTableEntryFromSummaryLSA(const OSPF::SummaryLSA&        summaryLSA,
                                                                            unsigned short                 entryCost,
                                                                            const OSPF::RoutingTableEntry& borderRouterEntry) const
{
    OSPF::IPv4AddressRange destination;

    destination.address = summaryLSA.getHeader().getLinkStateID();
    destination.mask = summaryLSA.getNetworkMask();

    OSPF::RoutingTableEntry* newEntry = new OSPF::RoutingTableEntry;

    if (summaryLSA.getHeader().getLsType() == SUMMARYLSA_NETWORKS_TYPE) {
        newEntry->setDestination(destination.address & destination.mask);
        newEntry->setNetmask(destination.mask);
        newEntry->setDestinationType(OSPF::RoutingTableEntry::NETWORK_DESTINATION);
    } else {
        newEntry->setDestination(destination.address);
        newEntry->setNetmask(IPv4Address::ALLONES_ADDRESS);
        newEntry->setDestinationType(OSPF::RoutingTableEntry::AS_BOUNDARY_ROUTER_DESTINATION);
    }
    newEntry->setArea(areaID);
    newEntry->setPathType(OSPF::RoutingTableEntry::INTERAREA);
    newEntry->setCost(entryCost);
    newEntry->setOptionalCapabilities(summaryLSA.getHeader().getLsOptions());
    newEntry->setLinkStateOrigin(&summaryLSA);

    unsigned int nextHopCount = borderRouterEntry.getNextHopCount();
    for (unsigned int j = 0; j < nextHopCount; j++) {
        newEntry->addNextHop(borderRouterEntry.getNextHop(j));
    }

    return newEntry;
}

/**
 * @see RFC 2328 Section 16.2.
 * @todo This function does a lot of lookup in the input newRoutingTable.
 *       Restructuring the input vector into some kind of hash would quite
 *       probably speed up execution.
 */
void OSPF::Area::calculateInterAreaRoutes(std::vector<OSPF::RoutingTableEntry*>& newRoutingTable)
{
    unsigned long i = 0;
    unsigned long j = 0;
    unsigned long lsaCount = summaryLSAs.size();

    for (i = 0; i < lsaCount; i++) {
        OSPF::SummaryLSA* currentLSA = summaryLSAs[i];
        OSPFLSAHeader& currentHeader = currentLSA->getHeader();

        unsigned long routeCost = currentLSA->getRouteCost();
        unsigned short lsAge = currentHeader.getLsAge();
        RouterID originatingRouter = currentHeader.getAdvertisingRouter();
        bool selfOriginated = (originatingRouter == parentRouter->getRouterID());

        if ((routeCost == LS_INFINITY) || (lsAge == MAX_AGE) || (selfOriginated)) { // (1) and(2)
            continue;
        }

        char lsType = currentHeader.getLsType();
        unsigned long routeCount = newRoutingTable.size();
        OSPF::IPv4AddressRange destination;

        destination.address = currentHeader.getLinkStateID();
        destination.mask = currentLSA->getNetworkMask();

        if ((lsType == SUMMARYLSA_NETWORKS_TYPE) && (parentRouter->hasAddressRange(destination))) { // (3)
            bool foundIntraAreaRoute = false;

            // look for an "Active" INTRAAREA route
            for (j = 0; j < routeCount; j++) {
                OSPF::RoutingTableEntry* routingEntry = newRoutingTable[j];

                if ((routingEntry->getDestinationType() == OSPF::RoutingTableEntry::NETWORK_DESTINATION) &&
                    (routingEntry->getPathType() == OSPF::RoutingTableEntry::INTRAAREA) &&
                    destination.containedByRange(routingEntry->getDestination(), routingEntry->getNetmask()))
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

            if ((routingEntry->getArea() == areaID) &&
                (((routingEntry->getDestinationType() & OSPF::RoutingTableEntry::AREA_BORDER_ROUTER_DESTINATION) != 0) ||
                 ((routingEntry->getDestinationType() & OSPF::RoutingTableEntry::AS_BOUNDARY_ROUTER_DESTINATION) != 0)) &&
                (routingEntry->getDestination() == originatingRouter))
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
            bool destinationInRoutingTable = true;
            unsigned short currentCost = routeCost + borderRouterEntry->getCost();
            std::list<OSPF::RoutingTableEntry*> sameOrWorseCost;

            if (findSameOrWorseCostRoute(newRoutingTable,
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

                    if (checkedEntry->getCost() > currentCost) {
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

                unsigned long nextHopCount = borderRouterEntry->getNextHopCount();

                if (equalEntry != NULL) {
                    /* Add the next hops of the border router advertising this destination
                     * to the equal entry.
                     */
                    for (unsigned long j = 0; j < nextHopCount; j++) {
                        equalEntry->addNextHop(borderRouterEntry->getNextHop(j));
                    }
                } else {
                    OSPF::RoutingTableEntry* newEntry = createRoutingTableEntryFromSummaryLSA(*currentLSA, currentCost, *borderRouterEntry);
                    ASSERT(newEntry != NULL);
                    newRoutingTable.push_back(newEntry);
                }
            } else {
                OSPF::RoutingTableEntry* newEntry = createRoutingTableEntryFromSummaryLSA(*currentLSA, currentCost, *borderRouterEntry);
                ASSERT(newEntry != NULL);
                newRoutingTable.push_back(newEntry);
            }
        }
    }
}

void OSPF::Area::recheckSummaryLSAs(std::vector<OSPF::RoutingTableEntry*>& newRoutingTable)
{
    unsigned long i = 0;
    unsigned long j = 0;
    unsigned long lsaCount = summaryLSAs.size();

    for (i = 0; i < lsaCount; i++) {
        OSPF::SummaryLSA* currentLSA = summaryLSAs[i];
        OSPFLSAHeader& currentHeader = currentLSA->getHeader();

        unsigned long routeCost = currentLSA->getRouteCost();
        unsigned short lsAge = currentHeader.getLsAge();
        RouterID originatingRouter = currentHeader.getAdvertisingRouter();
        bool selfOriginated = (originatingRouter == parentRouter->getRouterID());

        if ((routeCost == LS_INFINITY) || (lsAge == MAX_AGE) || (selfOriginated)) { // (1) and(2)
            continue;
        }

        unsigned long routeCount = newRoutingTable.size();
        char lsType = currentHeader.getLsType();
        OSPF::RoutingTableEntry* destinationEntry = NULL;
        OSPF::IPv4AddressRange destination;

        destination.address = currentHeader.getLinkStateID();
        destination.mask = currentLSA->getNetworkMask();

        for (j = 0; j < routeCount; j++) {  // (3)
            OSPF::RoutingTableEntry* routingEntry = newRoutingTable[j];
            bool foundMatching = false;

            if (lsType == SUMMARYLSA_NETWORKS_TYPE) {
                if ((routingEntry->getDestinationType() == OSPF::RoutingTableEntry::NETWORK_DESTINATION) &&
                    ((destination.address & destination.mask) == routingEntry->getDestination()))
                {
                    foundMatching = true;
                }
            } else {
                if ((((routingEntry->getDestinationType() & OSPF::RoutingTableEntry::AREA_BORDER_ROUTER_DESTINATION) != 0) ||
                     ((routingEntry->getDestinationType() & OSPF::RoutingTableEntry::AS_BOUNDARY_ROUTER_DESTINATION) != 0)) &&
                    (destination.address == routingEntry->getDestination()))
                {
                    foundMatching = true;
                }
            }

            if (foundMatching) {
                OSPF::RoutingTableEntry::RoutingPathType pathType = routingEntry->getPathType();

                if ((pathType == OSPF::RoutingTableEntry::TYPE1_EXTERNAL) ||
                    (pathType == OSPF::RoutingTableEntry::TYPE2_EXTERNAL) ||
                    (routingEntry->getArea() != OSPF::BACKBONE_AREAID))
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
        unsigned short currentCost = routeCost;

        for (j = 0; j < routeCount; j++) {     // (4) BR == borderRouterEntry
            OSPF::RoutingTableEntry* routingEntry = newRoutingTable[j];

            if ((routingEntry->getArea() == areaID) &&
                (((routingEntry->getDestinationType() & OSPF::RoutingTableEntry::AREA_BORDER_ROUTER_DESTINATION) != 0) ||
                 ((routingEntry->getDestinationType() & OSPF::RoutingTableEntry::AS_BOUNDARY_ROUTER_DESTINATION) != 0)) &&
                (routingEntry->getDestination() == originatingRouter))
            {
                borderRouterEntry = routingEntry;
                currentCost += borderRouterEntry->getCost();
                break;
            }
        }
        if (borderRouterEntry == NULL) {
            continue;
        } else {    // (5)
            if (currentCost <= destinationEntry->getCost()) {
                if (currentCost < destinationEntry->getCost()) {
                    destinationEntry->clearNextHops();
                }

                unsigned long nextHopCount = borderRouterEntry->getNextHopCount();

                for (j = 0; j < nextHopCount; j++) {
                    destinationEntry->addNextHop(borderRouterEntry->getNextHop(j));
                }
            }
        }
    }
}
