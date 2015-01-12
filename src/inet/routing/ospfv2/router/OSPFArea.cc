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

#include "inet/routing/ospfv2/router/OSPFArea.h"
#include "inet/routing/ospfv2/router/OSPFRouter.h"
#include <memory.h>

namespace inet {

namespace ospf {

Area::Area(IInterfaceTable *ift, AreaID id) :
    ift(ift),
    areaID(id),
    transitCapability(false),
    externalRoutingCapability(true),
    stubDefaultCost(1),
    spfTreeRoot(nullptr),
    parentRouter(nullptr)
{
}

Area::~Area()
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

void Area::addInterface(Interface *intf)
{
    intf->setArea(this);
    associatedInterfaces.push_back(intf);
}

void Area::addAddressRange(IPv4AddressRange addressRange, bool advertise)
{
    int addressRangeNum = areaAddressRanges.size();
    bool found = false;
    bool erased = false;

    for (int i = 0; i < addressRangeNum; i++) {
        IPv4AddressRange curRange = areaAddressRanges[i];
        if (curRange.contains(addressRange)) {    // contains or same
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
    if (erased && found) // the found entry contains new entry and new entry contains erased entry ==> the found entry also contains the erased entry
        throw cRuntimeError("Model error: bad contents in areaAddressRanges vector");
    if (erased) {
        auto it = areaAddressRanges.begin();
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

std::string Area::info() const
{
    std::stringstream out;
    out << "areaID: " << areaID.str(false);
    return out.str();
}

std::string Area::detailedInfo() const
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

bool Area::containsAddress(IPv4Address address) const
{
    int addressRangeNum = areaAddressRanges.size();
    for (int i = 0; i < addressRangeNum; i++) {
        if (areaAddressRanges[i].contains(address)) {
            return true;
        }
    }
    return false;
}

bool Area::hasAddressRange(IPv4AddressRange addressRange) const
{
    int addressRangeNum = areaAddressRanges.size();
    for (int i = 0; i < addressRangeNum; i++) {
        if (areaAddressRanges[i] == addressRange) {
            return true;
        }
    }
    return false;
}

IPv4AddressRange Area::getContainingAddressRange(IPv4AddressRange addressRange, bool *advertise    /*= nullptr*/) const
{
    int addressRangeNum = areaAddressRanges.size();
    for (int i = 0; i < addressRangeNum; i++) {
        if (areaAddressRanges[i].contains(addressRange)) {
            if (advertise != nullptr) {
                std::map<IPv4AddressRange, bool>::const_iterator rangeIt = advertiseAddressRanges.find(areaAddressRanges[i]);
                if (rangeIt != advertiseAddressRanges.end()) {
                    *advertise = rangeIt->second;
                }
                else {
                    throw cRuntimeError("Model error: inconsistent contents in areaAddressRanges and advertiseAddressRanges variables");
                }
            }
            return areaAddressRanges[i];
        }
    }
    if (advertise != nullptr) {
        *advertise = false;
    }
    return NULL_IPV4ADDRESSRANGE;
}

Interface *Area::getInterface(unsigned char ifIndex)
{
    int interfaceNum = associatedInterfaces.size();
    for (int i = 0; i < interfaceNum; i++) {
        if ((associatedInterfaces[i]->getType() != Interface::VIRTUAL) &&
            (associatedInterfaces[i]->getIfIndex() == ifIndex))
        {
            return associatedInterfaces[i];
        }
    }
    return nullptr;
}

Interface *Area::getInterface(IPv4Address address)
{
    int interfaceNum = associatedInterfaces.size();
    for (int i = 0; i < interfaceNum; i++) {
        if ((associatedInterfaces[i]->getType() != Interface::VIRTUAL) &&
            (associatedInterfaces[i]->getAddressRange().address == address))
        {
            return associatedInterfaces[i];
        }
    }
    return nullptr;
}

bool Area::hasVirtualLink(AreaID withTransitArea) const
{
    if ((areaID != BACKBONE_AREAID) || (withTransitArea == BACKBONE_AREAID)) {
        return false;
    }

    int interfaceNum = associatedInterfaces.size();
    for (int i = 0; i < interfaceNum; i++) {
        if ((associatedInterfaces[i]->getType() == Interface::VIRTUAL) &&
            (associatedInterfaces[i]->getTransitAreaID() == withTransitArea))
        {
            return true;
        }
    }
    return false;
}

Interface *Area::findVirtualLink(RouterID routerID)
{
    int interfaceNum = associatedInterfaces.size();
    for (int i = 0; i < interfaceNum; i++) {
        if ((associatedInterfaces[i]->getType() == Interface::VIRTUAL) &&
            (associatedInterfaces[i]->getNeighborByID(routerID) != nullptr))
        {
            return associatedInterfaces[i];
        }
    }
    return nullptr;
}

bool Area::installRouterLSA(OSPFRouterLSA *lsa)
{
    LinkStateID linkStateID = lsa->getHeader().getLinkStateID();
    auto lsaIt = routerLSAsByID.find(linkStateID);
    if (lsaIt != routerLSAsByID.end()) {
        LSAKeyType lsaKey;

        lsaKey.linkStateID = lsa->getHeader().getLinkStateID();
        lsaKey.advertisingRouter = lsa->getHeader().getAdvertisingRouter();

        removeFromAllRetransmissionLists(lsaKey);
        return lsaIt->second->update(lsa);
    }
    else {
        RouterLSA *lsaCopy = new RouterLSA(*lsa);
        routerLSAsByID[linkStateID] = lsaCopy;
        routerLSAs.push_back(lsaCopy);
        return true;
    }
}

bool Area::installNetworkLSA(OSPFNetworkLSA *lsa)
{
    LinkStateID linkStateID = lsa->getHeader().getLinkStateID();
    auto lsaIt = networkLSAsByID.find(linkStateID);
    if (lsaIt != networkLSAsByID.end()) {
        LSAKeyType lsaKey;

        lsaKey.linkStateID = lsa->getHeader().getLinkStateID();
        lsaKey.advertisingRouter = lsa->getHeader().getAdvertisingRouter();

        removeFromAllRetransmissionLists(lsaKey);
        return lsaIt->second->update(lsa);
    }
    else {
        NetworkLSA *lsaCopy = new NetworkLSA(*lsa);
        networkLSAsByID[linkStateID] = lsaCopy;
        networkLSAs.push_back(lsaCopy);
        return true;
    }
}

bool Area::installSummaryLSA(OSPFSummaryLSA *lsa)
{
    LSAKeyType lsaKey;

    lsaKey.linkStateID = lsa->getHeader().getLinkStateID();
    lsaKey.advertisingRouter = lsa->getHeader().getAdvertisingRouter();

    auto lsaIt = summaryLSAsByID.find(lsaKey);
    if (lsaIt != summaryLSAsByID.end()) {
        LSAKeyType lsaKey;

        lsaKey.linkStateID = lsa->getHeader().getLinkStateID();
        lsaKey.advertisingRouter = lsa->getHeader().getAdvertisingRouter();

        removeFromAllRetransmissionLists(lsaKey);
        return lsaIt->second->update(lsa);
    }
    else {
        SummaryLSA *lsaCopy = new SummaryLSA(*lsa);
        summaryLSAsByID[lsaKey] = lsaCopy;
        summaryLSAs.push_back(lsaCopy);
        return true;
    }
}

RouterLSA *Area::findRouterLSA(LinkStateID linkStateID)
{
    auto lsaIt = routerLSAsByID.find(linkStateID);
    if (lsaIt != routerLSAsByID.end()) {
        return lsaIt->second;
    }
    else {
        return nullptr;
    }
}

const RouterLSA *Area::findRouterLSA(LinkStateID linkStateID) const
{
    std::map<LinkStateID, RouterLSA *>::const_iterator lsaIt = routerLSAsByID.find(linkStateID);
    if (lsaIt != routerLSAsByID.end()) {
        return lsaIt->second;
    }
    else {
        return nullptr;
    }
}

NetworkLSA *Area::findNetworkLSA(LinkStateID linkStateID)
{
    auto lsaIt = networkLSAsByID.find(linkStateID);
    if (lsaIt != networkLSAsByID.end()) {
        return lsaIt->second;
    }
    else {
        return nullptr;
    }
}

const NetworkLSA *Area::findNetworkLSA(LinkStateID linkStateID) const
{
    std::map<LinkStateID, NetworkLSA *>::const_iterator lsaIt = networkLSAsByID.find(linkStateID);
    if (lsaIt != networkLSAsByID.end()) {
        return lsaIt->second;
    }
    else {
        return nullptr;
    }
}

SummaryLSA *Area::findSummaryLSA(LSAKeyType lsaKey)
{
    auto lsaIt = summaryLSAsByID.find(lsaKey);
    if (lsaIt != summaryLSAsByID.end()) {
        return lsaIt->second;
    }
    else {
        return nullptr;
    }
}

const SummaryLSA *Area::findSummaryLSA(LSAKeyType lsaKey) const
{
    std::map<LSAKeyType, SummaryLSA *, LSAKeyType_Less>::const_iterator lsaIt = summaryLSAsByID.find(lsaKey);
    if (lsaIt != summaryLSAsByID.end()) {
        return lsaIt->second;
    }
    else {
        return nullptr;
    }
}

void Area::ageDatabase()
{
    long lsaCount = routerLSAs.size();
    bool shouldRebuildRoutingTable = false;
    long i;

    for (i = 0; i < lsaCount; i++) {
        RouterLSA *lsa = routerLSAs[i];
        unsigned short lsAge = lsa->getHeader().getLsAge();
        bool selfOriginated = (lsa->getHeader().getAdvertisingRouter() == parentRouter->getRouterID());
        bool unreachable = parentRouter->isDestinationUnreachable(lsa);

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
                floodLSA(lsa);
                lsa->incrementInstallTime();
            }
            else {
                long sequenceNumber = lsa->getHeader().getLsSequenceNumber();
                if (sequenceNumber == MAX_SEQUENCE_NUMBER) {
                    lsa->getHeader().setLsAge(MAX_AGE);
                    floodLSA(lsa);
                    lsa->incrementInstallTime();
                }
                else {
                    RouterLSA *newLSA = originateRouterLSA();

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
            LSAKeyType lsaKey;

            lsaKey.linkStateID = lsa->getHeader().getLinkStateID();
            lsaKey.advertisingRouter = lsa->getHeader().getAdvertisingRouter();

            if (!isOnAnyRetransmissionList(lsaKey) &&
                !hasAnyNeighborInStates(Neighbor::EXCHANGE_STATE | Neighbor::LOADING_STATE))
            {
                if (!selfOriginated || unreachable) {
                    routerLSAsByID.erase(lsa->getHeader().getLinkStateID());
                    delete lsa;
                    routerLSAs[i] = nullptr;
                    shouldRebuildRoutingTable = true;
                }
                else {
                    RouterLSA *newLSA = originateRouterLSA();
                    long sequenceNumber = lsa->getHeader().getLsSequenceNumber();

                    newLSA->getHeader().setLsSequenceNumber((sequenceNumber == MAX_SEQUENCE_NUMBER) ? INITIAL_SEQUENCE_NUMBER : sequenceNumber + 1);
                    shouldRebuildRoutingTable |= lsa->update(newLSA);
                    delete newLSA;

                    floodLSA(lsa);
                }
            }
        }
    }

    auto routerIt = routerLSAs.begin();
    while (routerIt != routerLSAs.end()) {
        if ((*routerIt) == nullptr) {
            routerIt = routerLSAs.erase(routerIt);
        }
        else {
            routerIt++;
        }
    }

    lsaCount = networkLSAs.size();
    for (i = 0; i < lsaCount; i++) {
        unsigned short lsAge = networkLSAs[i]->getHeader().getLsAge();
        bool unreachable = parentRouter->isDestinationUnreachable(networkLSAs[i]);
        NetworkLSA *lsa = networkLSAs[i];
        Interface *localIntf = getInterface(lsa->getHeader().getLinkStateID());
        bool selfOriginated = false;

        if ((localIntf != nullptr) &&
            (localIntf->getState() == Interface::DESIGNATED_ROUTER_STATE) &&
            (localIntf->getNeighborCount() > 0) &&
            (localIntf->hasAnyNeighborInStates(Neighbor::FULL_STATE)))
        {
            selfOriginated = true;
        }

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
                floodLSA(lsa);
                lsa->incrementInstallTime();
            }
            else {
                long sequenceNumber = lsa->getHeader().getLsSequenceNumber();
                if (sequenceNumber == MAX_SEQUENCE_NUMBER) {
                    lsa->getHeader().setLsAge(MAX_AGE);
                    floodLSA(lsa);
                    lsa->incrementInstallTime();
                }
                else {
                    NetworkLSA *newLSA = originateNetworkLSA(localIntf);

                    if (newLSA != nullptr) {
                        newLSA->getHeader().setLsSequenceNumber(sequenceNumber + 1);
                        shouldRebuildRoutingTable |= lsa->update(newLSA);
                        delete newLSA;
                    }
                    else {    // no neighbors on the network -> old NetworkLSA must be flushed
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
            LSAKeyType lsaKey;

            lsaKey.linkStateID = lsa->getHeader().getLinkStateID();
            lsaKey.advertisingRouter = lsa->getHeader().getAdvertisingRouter();

            if (!isOnAnyRetransmissionList(lsaKey) &&
                !hasAnyNeighborInStates(Neighbor::EXCHANGE_STATE | Neighbor::LOADING_STATE))
            {
                if (!selfOriginated || unreachable) {
                    networkLSAsByID.erase(lsa->getHeader().getLinkStateID());
                    delete lsa;
                    networkLSAs[i] = nullptr;
                    shouldRebuildRoutingTable = true;
                }
                else {
                    NetworkLSA *newLSA = originateNetworkLSA(localIntf);
                    long sequenceNumber = lsa->getHeader().getLsSequenceNumber();

                    if (newLSA != nullptr) {
                        newLSA->getHeader().setLsSequenceNumber((sequenceNumber == MAX_SEQUENCE_NUMBER) ? INITIAL_SEQUENCE_NUMBER : sequenceNumber + 1);
                        shouldRebuildRoutingTable |= lsa->update(newLSA);
                        delete newLSA;

                        floodLSA(lsa);
                    }
                    else {    // no neighbors on the network -> old NetworkLSA must be deleted
                        delete networkLSAs[i];
                    }
                }
            }
        }
    }

    auto networkIt = networkLSAs.begin();
    while (networkIt != networkLSAs.end()) {
        if ((*networkIt) == nullptr) {
            networkIt = networkLSAs.erase(networkIt);
        }
        else {
            networkIt++;
        }
    }

    lsaCount = summaryLSAs.size();
    for (i = 0; i < lsaCount; i++) {
        unsigned short lsAge = summaryLSAs[i]->getHeader().getLsAge();
        bool selfOriginated = (summaryLSAs[i]->getHeader().getAdvertisingRouter() == parentRouter->getRouterID());
        bool unreachable = parentRouter->isDestinationUnreachable(summaryLSAs[i]);
        SummaryLSA *lsa = summaryLSAs[i];

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
                floodLSA(lsa);
                lsa->incrementInstallTime();
            }
            else {
                long sequenceNumber = lsa->getHeader().getLsSequenceNumber();
                if (sequenceNumber == MAX_SEQUENCE_NUMBER) {
                    lsa->getHeader().setLsAge(MAX_AGE);
                    floodLSA(lsa);
                    lsa->incrementInstallTime();
                }
                else {
                    SummaryLSA *newLSA = originateSummaryLSA(lsa);

                    if (newLSA != nullptr) {
                        newLSA->getHeader().setLsSequenceNumber(sequenceNumber + 1);
                        shouldRebuildRoutingTable |= lsa->update(newLSA);
                        delete newLSA;

                        floodLSA(lsa);
                    }
                    else {
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
            LSAKeyType lsaKey;

            lsaKey.linkStateID = lsa->getHeader().getLinkStateID();
            lsaKey.advertisingRouter = lsa->getHeader().getAdvertisingRouter();

            if (!isOnAnyRetransmissionList(lsaKey) &&
                !hasAnyNeighborInStates(Neighbor::EXCHANGE_STATE | Neighbor::LOADING_STATE))
            {
                if (!selfOriginated || unreachable) {
                    summaryLSAsByID.erase(lsaKey);
                    delete lsa;
                    summaryLSAs[i] = nullptr;
                    shouldRebuildRoutingTable = true;
                }
                else {
                    SummaryLSA *newLSA = originateSummaryLSA(lsa);
                    if (newLSA != nullptr) {
                        long sequenceNumber = lsa->getHeader().getLsSequenceNumber();

                        newLSA->getHeader().setLsSequenceNumber((sequenceNumber == MAX_SEQUENCE_NUMBER) ? INITIAL_SEQUENCE_NUMBER : sequenceNumber + 1);
                        shouldRebuildRoutingTable |= lsa->update(newLSA);
                        delete newLSA;

                        floodLSA(lsa);
                    }
                    else {
                        summaryLSAsByID.erase(lsaKey);
                        delete lsa;
                        summaryLSAs[i] = nullptr;
                        shouldRebuildRoutingTable = true;
                    }
                }
            }
        }
    }

    auto summaryIt = summaryLSAs.begin();
    while (summaryIt != summaryLSAs.end()) {
        if ((*summaryIt) == nullptr) {
            summaryIt = summaryLSAs.erase(summaryIt);
        }
        else {
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

bool Area::hasAnyNeighborInStates(int states) const
{
    long interfaceCount = associatedInterfaces.size();
    for (long i = 0; i < interfaceCount; i++) {
        if (associatedInterfaces[i]->hasAnyNeighborInStates(states)) {
            return true;
        }
    }
    return false;
}

void Area::removeFromAllRetransmissionLists(LSAKeyType lsaKey)
{
    long interfaceCount = associatedInterfaces.size();
    for (long i = 0; i < interfaceCount; i++) {
        associatedInterfaces[i]->removeFromAllRetransmissionLists(lsaKey);
    }
}

bool Area::isOnAnyRetransmissionList(LSAKeyType lsaKey) const
{
    long interfaceCount = associatedInterfaces.size();
    for (long i = 0; i < interfaceCount; i++) {
        if (associatedInterfaces[i]->isOnAnyRetransmissionList(lsaKey)) {
            return true;
        }
    }
    return false;
}

bool Area::floodLSA(OSPFLSA *lsa, Interface *intf, Neighbor *neighbor)
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

bool Area::isLocalAddress(IPv4Address address) const
{
    long interfaceCount = associatedInterfaces.size();
    for (long i = 0; i < interfaceCount; i++) {
        if (associatedInterfaces[i]->getAddressRange().address == address) {
            return true;
        }
    }
    return false;
}

RouterLSA *Area::originateRouterLSA()
{
    RouterLSA *routerLSA = new RouterLSA;
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
    Area *backbone = parentRouter->getAreaByID(BACKBONE_AREAID);
    routerLSA->setV_VirtualLinkEndpoint((backbone == nullptr) ? false : backbone->hasVirtualLink(areaID));

    routerLSA->setNumberOfLinks(0);
    routerLSA->setLinksArraySize(0);
    for (i = 0; i < interfaceCount; i++) {
        Interface *intf = associatedInterfaces[i];

        if (intf->getState() == Interface::DOWN_STATE) {
            continue;
        }
        if ((intf->getState() == Interface::LOOPBACK_STATE) &&
            ((intf->getType() != Interface::POINTTOPOINT) ||
             (intf->getAddressRange().address != NULL_IPV4ADDRESS)))
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
        if (intf->getState() > Interface::LOOPBACK_STATE) {
            switch (intf->getType()) {
                case Interface::POINTTOPOINT: {
                    Neighbor *neighbor = (intf->getNeighborCount() > 0) ? intf->getNeighbor(0) : nullptr;
                    if (neighbor != nullptr) {
                        if (neighbor->getState() == Neighbor::FULL_STATE) {
                            Link link;
                            link.setType(POINTTOPOINT_LINK);
                            link.setLinkID(IPv4Address(neighbor->getNeighborID()));
                            if (intf->getAddressRange().address != NULL_IPV4ADDRESS) {
                                link.setLinkData(intf->getAddressRange().address.getInt());
                            }
                            else {
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
                        if (intf->getState() == Interface::POINTTOPOINT_STATE) {
                            if (neighbor->getAddress() != NULL_IPV4ADDRESS) {
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
                            }
                            else {
                                if (intf->getAddressRange().mask.getInt() != 0xFFFFFFFF) {
                                    Link stubLink;
                                    stubLink.setType(STUB_LINK);
                                    stubLink.setLinkID(intf->getAddressRange().address
                                            & intf->getAddressRange().mask);
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

                case Interface::BROADCAST:
                case Interface::NBMA: {
                    if (intf->getState() == Interface::WAITING_STATE) {
                        Link stubLink;
                        stubLink.setType(STUB_LINK);
                        stubLink.setLinkID(intf->getAddressRange().address
                                & intf->getAddressRange().mask);
                        stubLink.setLinkData(intf->getAddressRange().mask.getInt());
                        stubLink.setLinkCost(intf->getOutputCost());
                        stubLink.setNumberOfTOS(0);
                        stubLink.setTosDataArraySize(0);

                        unsigned short linkIndex = routerLSA->getLinksArraySize();
                        routerLSA->setLinksArraySize(linkIndex + 1);
                        routerLSA->setNumberOfLinks(linkIndex + 1);
                        routerLSA->setLinks(linkIndex, stubLink);
                    }
                    else {
                        Neighbor *dRouter = intf->getNeighborByAddress(intf->getDesignatedRouter().ipInterfaceAddress);
                        if (((dRouter != nullptr) && (dRouter->getState() == Neighbor::FULL_STATE)) ||
                            ((intf->getDesignatedRouter().routerID == parentRouter->getRouterID()) &&
                             (intf->hasAnyNeighborInStates(Neighbor::FULL_STATE))))
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
                        }
                        else {
                            Link stubLink;
                            stubLink.setType(STUB_LINK);
                            stubLink.setLinkID(intf->getAddressRange().address
                                    & intf->getAddressRange().mask);
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

                case Interface::VIRTUAL: {
                    Neighbor *neighbor = (intf->getNeighborCount() > 0) ? intf->getNeighbor(0) : nullptr;
                    if ((neighbor != nullptr) && (neighbor->getState() == Neighbor::FULL_STATE)) {
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

                case Interface::POINTTOMULTIPOINT: {
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
                        Neighbor *neighbor = intf->getNeighbor(i);
                        if (neighbor->getState() == Neighbor::FULL_STATE) {
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

                default:
                    break;
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

    routerLSA->setSource(LSATrackingInfo::ORIGINATED);

    return routerLSA;
}

NetworkLSA *Area::originateNetworkLSA(const Interface *intf)
{
    if (intf->hasAnyNeighborInStates(Neighbor::FULL_STATE)) {
        NetworkLSA *networkLSA = new NetworkLSA;
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
            const Neighbor *neighbor = intf->getNeighbor(j);
            if (neighbor->getState() == Neighbor::FULL_STATE) {
                unsigned short netIndex = networkLSA->getAttachedRoutersArraySize();
                networkLSA->setAttachedRoutersArraySize(netIndex + 1);
                networkLSA->setAttachedRouters(netIndex, IPv4Address(neighbor->getNeighborID()));
            }
        }
        unsigned short netIndex = networkLSA->getAttachedRoutersArraySize();
        networkLSA->setAttachedRoutersArraySize(netIndex + 1);
        networkLSA->setAttachedRouters(netIndex, IPv4Address(parentRouter->getRouterID()));

        return networkLSA;
    }
    else {
        return nullptr;
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
 * This means that if the lsaToReoriginate parameter is not nullptr on return then another
 * lookup in the database is needed with the same LSAKey as used here(input
 * destination address and the router's own routerID) and the resulting Summary LSA's
 * link state ID should be changed to the one returned by this function.
 */
LinkStateID Area::getUniqueLinkStateID(IPv4AddressRange destination,
        Metric destinationCost,
        SummaryLSA *& lsaToReoriginate) const
{
    if (lsaToReoriginate != nullptr) {
        delete lsaToReoriginate;
        lsaToReoriginate = nullptr;
    }

    LSAKeyType lsaKey;

    lsaKey.linkStateID = destination.address;
    lsaKey.advertisingRouter = parentRouter->getRouterID();

    const SummaryLSA *foundLSA = findSummaryLSA(lsaKey);

    if (foundLSA == nullptr) {
        return lsaKey.linkStateID;
    }
    else {
        IPv4Address existingMask = foundLSA->getNetworkMask();

        if (destination.mask == existingMask) {
            return lsaKey.linkStateID;
        }
        else {
            if (destination.mask >= existingMask) {
                return lsaKey.linkStateID.makeBroadcastAddress(destination.mask);
            }
            else {
                SummaryLSA *summaryLSA = new SummaryLSA(*foundLSA);

                long sequenceNumber = summaryLSA->getHeader().getLsSequenceNumber();

                summaryLSA->getHeader().setLsAge(0);
                summaryLSA->getHeader().setLsSequenceNumber((sequenceNumber == MAX_SEQUENCE_NUMBER) ? INITIAL_SEQUENCE_NUMBER : sequenceNumber + 1);
                summaryLSA->setNetworkMask(destination.mask);
                summaryLSA->setRouteCost(destinationCost);

                lsaToReoriginate = summaryLSA;

                return lsaKey.linkStateID.makeBroadcastAddress(existingMask);
            }
        }
    }
}

SummaryLSA *Area::originateSummaryLSA(const RoutingTableEntry *entry,
        const std::map<LSAKeyType, bool, LSAKeyType_Less>& originatedLSAs,
        SummaryLSA *& lsaToReoriginate)
{
    if (((entry->getDestinationType() & RoutingTableEntry::AREA_BORDER_ROUTER_DESTINATION) != 0) ||
        (entry->getPathType() == RoutingTableEntry::TYPE1_EXTERNAL) ||
        (entry->getPathType() == RoutingTableEntry::TYPE2_EXTERNAL) ||
        (entry->getArea() == areaID))
    {
        return nullptr;
    }

    bool allNextHopsInThisArea = true;
    unsigned int nextHopCount = entry->getNextHopCount();

    for (unsigned int i = 0; i < nextHopCount; i++) {
        Interface *nextHopInterface = parentRouter->getNonVirtualInterface(entry->getNextHop(i).ifIndex);
        if ((nextHopInterface != nullptr) && (nextHopInterface->getAreaID() != areaID)) {
            allNextHopsInThisArea = false;
            break;
        }
    }
    if ((allNextHopsInThisArea) || (entry->getCost() >= LS_INFINITY)) {
        return nullptr;
    }

    if ((entry->getDestinationType() & RoutingTableEntry::AS_BOUNDARY_ROUTER_DESTINATION) != 0) {
        RoutingTableEntry *preferredEntry = parentRouter->getPreferredEntry(*(entry->getLinkStateOrigin()), false);
        if ((preferredEntry != nullptr) && (*preferredEntry == *entry) && (externalRoutingCapability)) {
            SummaryLSA *summaryLSA = new SummaryLSA;
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

            summaryLSA->setSource(LSATrackingInfo::ORIGINATED);

            return summaryLSA;
        }
    }
    else {    // entry->getDestinationType() == RoutingTableEntry::NETWORK_DESTINATION
        if (entry->getPathType() == RoutingTableEntry::INTERAREA) {
            IPv4AddressRange destinationRange;

            destinationRange.address = entry->getDestination();
            destinationRange.mask = entry->getNetmask();

            LinkStateID newLinkStateID = getUniqueLinkStateID(destinationRange, entry->getCost(), lsaToReoriginate);

            if (lsaToReoriginate != nullptr) {
                LSAKeyType lsaKey;

                lsaKey.linkStateID = entry->getDestination();
                lsaKey.advertisingRouter = parentRouter->getRouterID();

                auto lsaIt = summaryLSAsByID.find(lsaKey);
                if (lsaIt == summaryLSAsByID.end()) {
                    delete (lsaToReoriginate);
                    lsaToReoriginate = nullptr;
                    return nullptr;
                }
                else {
                    SummaryLSA *summaryLSA = new SummaryLSA(*(lsaIt->second));
                    OSPFLSAHeader& lsaHeader = summaryLSA->getHeader();

                    lsaHeader.setLsAge(0);
                    lsaHeader.setLsSequenceNumber(INITIAL_SEQUENCE_NUMBER);
                    lsaHeader.setLinkStateID(newLinkStateID);

                    return summaryLSA;
                }
            }
            else {
                SummaryLSA *summaryLSA = new SummaryLSA;
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

                summaryLSA->setSource(LSATrackingInfo::ORIGINATED);

                return summaryLSA;
            }
        }
        else {    // entry->getPathType() == RoutingTableEntry::INTRAAREA
            IPv4AddressRange destinationAddressRange;

            destinationAddressRange.address = entry->getDestination();
            destinationAddressRange.mask = entry->getNetmask();

            bool doAdvertise = false;
            IPv4AddressRange containingAddressRange = parentRouter->getContainingAddressRange(destinationAddressRange, &doAdvertise);
            if (((entry->getArea() == BACKBONE_AREAID) &&    // the backbone's configured ranges should be ignored
                 (transitCapability)) ||    // when originating Summary LSAs into transit areas
                (containingAddressRange == NULL_IPV4ADDRESSRANGE))
            {
                LinkStateID newLinkStateID = getUniqueLinkStateID(destinationAddressRange, entry->getCost(), lsaToReoriginate);

                if (lsaToReoriginate != nullptr) {
                    LSAKeyType lsaKey;

                    lsaKey.linkStateID = entry->getDestination();
                    lsaKey.advertisingRouter = parentRouter->getRouterID();

                    auto lsaIt = summaryLSAsByID.find(lsaKey);
                    if (lsaIt == summaryLSAsByID.end()) {
                        delete (lsaToReoriginate);
                        lsaToReoriginate = nullptr;
                        return nullptr;
                    }
                    else {
                        SummaryLSA *summaryLSA = new SummaryLSA(*(lsaIt->second));
                        OSPFLSAHeader& lsaHeader = summaryLSA->getHeader();

                        lsaHeader.setLsAge(0);
                        lsaHeader.setLsSequenceNumber(INITIAL_SEQUENCE_NUMBER);
                        lsaHeader.setLinkStateID(newLinkStateID);

                        return summaryLSA;
                    }
                }
                else {
                    SummaryLSA *summaryLSA = new SummaryLSA;
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

                    summaryLSA->setSource(LSATrackingInfo::ORIGINATED);

                    return summaryLSA;
                }
            }
            else {
                if (doAdvertise) {
                    Metric maxRangeCost = 0;
                    unsigned long entryCount = parentRouter->getRoutingTableEntryCount();

                    for (unsigned long i = 0; i < entryCount; i++) {
                        const RoutingTableEntry *routingEntry = parentRouter->getRoutingTableEntry(i);

                        if ((routingEntry->getDestinationType() == RoutingTableEntry::NETWORK_DESTINATION) &&
                            (routingEntry->getPathType() == RoutingTableEntry::INTRAAREA) &&
                            containingAddressRange.containsRange(routingEntry->getDestination(), routingEntry->getNetmask()) &&
                            (routingEntry->getCost() > maxRangeCost))
                        {
                            maxRangeCost = routingEntry->getCost();
                        }
                    }

                    LinkStateID newLinkStateID = getUniqueLinkStateID(containingAddressRange, maxRangeCost, lsaToReoriginate);
                    LSAKeyType lsaKey;

                    if (lsaToReoriginate != nullptr) {
                        lsaKey.linkStateID = lsaToReoriginate->getHeader().getLinkStateID();
                        lsaKey.advertisingRouter = parentRouter->getRouterID();

                        std::map<LSAKeyType, bool, LSAKeyType_Less>::const_iterator originatedIt = originatedLSAs.find(lsaKey);
                        if (originatedIt != originatedLSAs.end()) {
                            delete (lsaToReoriginate);
                            lsaToReoriginate = nullptr;
                            return nullptr;
                        }

                        lsaKey.linkStateID = entry->getDestination();
                        lsaKey.advertisingRouter = parentRouter->getRouterID();

                        auto lsaIt = summaryLSAsByID.find(lsaKey);
                        if (lsaIt == summaryLSAsByID.end()) {
                            delete (lsaToReoriginate);
                            lsaToReoriginate = nullptr;
                            return nullptr;
                        }

                        SummaryLSA *summaryLSA = new SummaryLSA(*(lsaIt->second));
                        OSPFLSAHeader& lsaHeader = summaryLSA->getHeader();

                        lsaHeader.setLsAge(0);
                        lsaHeader.setLsSequenceNumber(INITIAL_SEQUENCE_NUMBER);
                        lsaHeader.setLinkStateID(newLinkStateID);

                        return summaryLSA;
                    }
                    else {
                        lsaKey.linkStateID = newLinkStateID;
                        lsaKey.advertisingRouter = parentRouter->getRouterID();

                        std::map<LSAKeyType, bool, LSAKeyType_Less>::const_iterator originatedIt = originatedLSAs.find(lsaKey);
                        if (originatedIt != originatedLSAs.end()) {
                            return nullptr;
                        }

                        SummaryLSA *summaryLSA = new SummaryLSA;
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

                        summaryLSA->setSource(LSATrackingInfo::ORIGINATED);

                        return summaryLSA;
                    }
                }
            }
        }
    }

    return nullptr;
}

SummaryLSA *Area::originateSummaryLSA(const SummaryLSA *summaryLSA)
{
    const std::map<LSAKeyType, bool, LSAKeyType_Less> emptyMap;
    SummaryLSA *dontReoriginate = nullptr;

    const OSPFLSAHeader& lsaHeader = summaryLSA->getHeader();
    unsigned long entryCount = parentRouter->getRoutingTableEntryCount();

    for (unsigned long i = 0; i < entryCount; i++) {
        const RoutingTableEntry *entry = parentRouter->getRoutingTableEntry(i);

        if ((lsaHeader.getLsType() == SUMMARYLSA_ASBOUNDARYROUTERS_TYPE) &&
            ((((entry->getDestinationType() & RoutingTableEntry::AREA_BORDER_ROUTER_DESTINATION) != 0) ||
              ((entry->getDestinationType() & RoutingTableEntry::AS_BOUNDARY_ROUTER_DESTINATION) != 0)) &&
             ((entry->getDestination() == lsaHeader.getLinkStateID()) &&    //FIXME Why not compare network addresses (addr masked with netmask)?
              (entry->getNetmask() == summaryLSA->getNetworkMask()))))
        {
            SummaryLSA *returnLSA = originateSummaryLSA(entry, emptyMap, dontReoriginate);
            if (dontReoriginate != nullptr) {
                delete dontReoriginate;
            }
            return returnLSA;
        }

        IPv4Address lsaMask = summaryLSA->getNetworkMask();

        if ((lsaHeader.getLsType() == SUMMARYLSA_NETWORKS_TYPE) &&
            (entry->getDestinationType() == RoutingTableEntry::NETWORK_DESTINATION) &&
            isSameNetwork(entry->getDestination(), entry->getNetmask(), lsaHeader.getLinkStateID(), lsaMask))
        {
            SummaryLSA *returnLSA = originateSummaryLSA(entry, emptyMap, dontReoriginate);
            if (dontReoriginate != nullptr) {
                delete dontReoriginate;
            }
            return returnLSA;
        }
    }

    return nullptr;
}

void Area::calculateShortestPathTree(std::vector<RoutingTableEntry *>& newRoutingTable)
{
    RouterID routerID = parentRouter->getRouterID();
    bool finished = false;
    std::vector<OSPFLSA *> treeVertices;
    OSPFLSA *justAddedVertex;
    std::vector<OSPFLSA *> candidateVertices;
    unsigned long i, j, k;
    unsigned long lsaCount;

    if (spfTreeRoot == nullptr) {
        RouterLSA *newLSA = originateRouterLSA();

        installRouterLSA(newLSA);

        RouterLSA *routerLSA = findRouterLSA(routerID);

        spfTreeRoot = routerLSA;
        floodLSA(newLSA);
        delete newLSA;
    }
    if (spfTreeRoot == nullptr) {
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
    justAddedVertex = spfTreeRoot;    // (1)

    do {
        LSAType vertexType = static_cast<LSAType>(justAddedVertex->getHeader().getLsType());

        if (vertexType == ROUTERLSA_TYPE) {
            RouterLSA *routerVertex = check_and_cast<RouterLSA *>(justAddedVertex);
            if (routerVertex->getV_VirtualLinkEndpoint()) {    // (2)
                transitCapability = true;
            }

            unsigned int linkCount = routerVertex->getLinksArraySize();
            for (i = 0; i < linkCount; i++) {
                Link& link = routerVertex->getLinks(i);
                LinkType linkType = static_cast<LinkType>(link.getType());
                OSPFLSA *joiningVertex;
                LSAType joiningVertexType;

                if (linkType == STUB_LINK) {    // (2) (a)
                    continue;
                }

                if (linkType == TRANSIT_LINK) {
                    joiningVertex = findNetworkLSA(link.getLinkID());
                    joiningVertexType = NETWORKLSA_TYPE;
                }
                else {
                    joiningVertex = findRouterLSA(link.getLinkID());
                    joiningVertexType = ROUTERLSA_TYPE;
                }

                if ((joiningVertex == nullptr) ||
                    (joiningVertex->getHeader().getLsAge() == MAX_AGE) ||
                    (!hasLink(joiningVertex, justAddedVertex)))    // (from, to)     (2) (b)
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
                OSPFLSA *candidate = nullptr;

                for (j = 0; j < candidateCount; j++) {
                    if (candidateVertices[j] == joiningVertex) {
                        candidate = candidateVertices[j];
                    }
                }
                if (candidate != nullptr) {    // (2) (d)
                    RoutingInfo *routingInfo = check_and_cast<RoutingInfo *>(candidate);
                    unsigned long candidateDistance = routingInfo->getDistance();

                    if (linkStateCost > candidateDistance) {
                        continue;
                    }
                    if (linkStateCost < candidateDistance) {
                        routingInfo->setDistance(linkStateCost);
                        routingInfo->clearNextHops();
                    }
                    std::vector<NextHop> *newNextHops = calculateNextHops(joiningVertex, justAddedVertex);    // (destination, parent)
                    unsigned int nextHopCount = newNextHops->size();
                    for (k = 0; k < nextHopCount; k++) {
                        routingInfo->addNextHop((*newNextHops)[k]);
                    }
                    delete newNextHops;
                }
                else {
                    if (joiningVertexType == ROUTERLSA_TYPE) {
                        RouterLSA *joiningRouterVertex = check_and_cast<RouterLSA *>(joiningVertex);
                        joiningRouterVertex->setDistance(linkStateCost);
                        std::vector<NextHop> *newNextHops = calculateNextHops(joiningVertex, justAddedVertex);    // (destination, parent)
                        unsigned int nextHopCount = newNextHops->size();
                        for (k = 0; k < nextHopCount; k++) {
                            joiningRouterVertex->addNextHop((*newNextHops)[k]);
                        }
                        delete newNextHops;
                        RoutingInfo *vertexRoutingInfo = check_and_cast<RoutingInfo *>(joiningRouterVertex);
                        vertexRoutingInfo->setParent(justAddedVertex);

                        candidateVertices.push_back(joiningRouterVertex);
                    }
                    else {
                        NetworkLSA *joiningNetworkVertex = check_and_cast<NetworkLSA *>(joiningVertex);
                        joiningNetworkVertex->setDistance(linkStateCost);
                        std::vector<NextHop> *newNextHops = calculateNextHops(joiningVertex, justAddedVertex);    // (destination, parent)
                        unsigned int nextHopCount = newNextHops->size();
                        for (k = 0; k < nextHopCount; k++) {
                            joiningNetworkVertex->addNextHop((*newNextHops)[k]);
                        }
                        delete newNextHops;
                        RoutingInfo *vertexRoutingInfo = check_and_cast<RoutingInfo *>(joiningNetworkVertex);
                        vertexRoutingInfo->setParent(justAddedVertex);

                        candidateVertices.push_back(joiningNetworkVertex);
                    }
                }
            }
        }

        if (vertexType == NETWORKLSA_TYPE) {
            NetworkLSA *networkVertex = check_and_cast<NetworkLSA *>(justAddedVertex);
            unsigned int routerCount = networkVertex->getAttachedRoutersArraySize();

            for (i = 0; i < routerCount; i++) {    // (2)
                RouterLSA *joiningVertex = findRouterLSA(networkVertex->getAttachedRouters(i));
                if ((joiningVertex == nullptr) ||
                    (joiningVertex->getHeader().getLsAge() == MAX_AGE) ||
                    (!hasLink(joiningVertex, justAddedVertex)))    // (from, to)     (2) (b)
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

                unsigned long linkStateCost = networkVertex->getDistance();    // link cost from network to router is always 0
                unsigned int candidateCount = candidateVertices.size();
                OSPFLSA *candidate = nullptr;

                for (j = 0; j < candidateCount; j++) {
                    if (candidateVertices[j] == joiningVertex) {
                        candidate = candidateVertices[j];
                    }
                }
                if (candidate != nullptr) {    // (2) (d)
                    RoutingInfo *routingInfo = check_and_cast<RoutingInfo *>(candidate);
                    unsigned long candidateDistance = routingInfo->getDistance();

                    if (linkStateCost > candidateDistance) {
                        continue;
                    }
                    if (linkStateCost < candidateDistance) {
                        routingInfo->setDistance(linkStateCost);
                        routingInfo->clearNextHops();
                    }
                    std::vector<NextHop> *newNextHops = calculateNextHops(joiningVertex, justAddedVertex);    // (destination, parent)
                    unsigned int nextHopCount = newNextHops->size();
                    for (k = 0; k < nextHopCount; k++) {
                        routingInfo->addNextHop((*newNextHops)[k]);
                    }
                    delete newNextHops;
                }
                else {
                    joiningVertex->setDistance(linkStateCost);
                    std::vector<NextHop> *newNextHops = calculateNextHops(joiningVertex, justAddedVertex);    // (destination, parent)
                    unsigned int nextHopCount = newNextHops->size();
                    for (k = 0; k < nextHopCount; k++) {
                        joiningVertex->addNextHop((*newNextHops)[k]);
                    }
                    delete newNextHops;
                    RoutingInfo *vertexRoutingInfo = check_and_cast<RoutingInfo *>(joiningVertex);
                    vertexRoutingInfo->setParent(justAddedVertex);

                    candidateVertices.push_back(joiningVertex);
                }
            }
        }

        if (candidateVertices.empty()) {    // (3)
            finished = true;
        }
        else {
            unsigned int candidateCount = candidateVertices.size();
            unsigned long minDistance = LS_INFINITY;
            OSPFLSA *closestVertex = candidateVertices[0];

            for (i = 0; i < candidateCount; i++) {
                RoutingInfo *routingInfo = check_and_cast<RoutingInfo *>(candidateVertices[i]);
                unsigned long currentDistance = routingInfo->getDistance();

                if (currentDistance < minDistance) {
                    closestVertex = candidateVertices[i];
                    minDistance = currentDistance;
                }
                else {
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

            for (auto it = candidateVertices.begin(); it != candidateVertices.end(); it++) {
                if ((*it) == closestVertex) {
                    candidateVertices.erase(it);
                    break;
                }
            }

            if (closestVertex->getHeader().getLsType() == ROUTERLSA_TYPE) {
                RouterLSA *routerLSA = check_and_cast<RouterLSA *>(closestVertex);
                if (routerLSA->getB_AreaBorderRouter() || routerLSA->getE_ASBoundaryRouter()) {
                    RoutingTableEntry *entry = new RoutingTableEntry(ift);
                    RouterID destinationID = routerLSA->getHeader().getLinkStateID();
                    unsigned int nextHopCount = routerLSA->getNextHopCount();
                    RoutingTableEntry::RoutingDestinationType destinationType = RoutingTableEntry::NETWORK_DESTINATION;

                    entry->setDestination(destinationID);
                    entry->setLinkStateOrigin(routerLSA);
                    entry->setArea(areaID);
                    entry->setPathType(RoutingTableEntry::INTRAAREA);
                    entry->setCost(routerLSA->getDistance());
                    if (routerLSA->getB_AreaBorderRouter()) {
                        destinationType |= RoutingTableEntry::AREA_BORDER_ROUTER_DESTINATION;
                    }
                    if (routerLSA->getE_ASBoundaryRouter()) {
                        destinationType |= RoutingTableEntry::AS_BOUNDARY_ROUTER_DESTINATION;
                    }
                    entry->setDestinationType(destinationType);
                    entry->setOptionalCapabilities(routerLSA->getHeader().getLsOptions());
                    for (i = 0; i < nextHopCount; i++) {
                        entry->addNextHop(routerLSA->getNextHop(i));
                    }

                    newRoutingTable.push_back(entry);

                    Area *backbone;
                    if (areaID != BACKBONE_AREAID) {
                        backbone = parentRouter->getAreaByID(BACKBONE_AREAID);
                    }
                    else {
                        backbone = this;
                    }
                    if (backbone != nullptr) {
                        Interface *virtualIntf = backbone->findVirtualLink(destinationID);
                        if ((virtualIntf != nullptr) && (virtualIntf->getTransitAreaID() == areaID)) {
                            IPv4AddressRange range;
                            range.address = getInterface(routerLSA->getNextHop(0).ifIndex)->getAddressRange().address;
                            range.mask = IPv4Address::ALLONES_ADDRESS;
                            virtualIntf->setAddressRange(range);
                            virtualIntf->setIfIndex(ift, routerLSA->getNextHop(0).ifIndex);
                            virtualIntf->setOutputCost(routerLSA->getDistance());
                            Neighbor *virtualNeighbor = virtualIntf->getNeighbor(0);
                            if (virtualNeighbor != nullptr) {
                                unsigned int linkCount = routerLSA->getLinksArraySize();
                                RouterLSA *toRouterLSA = dynamic_cast<RouterLSA *>(justAddedVertex);
                                if (toRouterLSA != nullptr) {
                                    for (i = 0; i < linkCount; i++) {
                                        Link& link = routerLSA->getLinks(i);

                                        if ((link.getType() == POINTTOPOINT_LINK) &&
                                            (link.getLinkID() == toRouterLSA->getHeader().getLinkStateID()) &&
                                            (virtualIntf->getState() < Interface::WAITING_STATE))
                                        {
                                            virtualNeighbor->setAddress(IPv4Address(link.getLinkData()));
                                            virtualIntf->processEvent(Interface::INTERFACE_UP);
                                            break;
                                        }
                                    }
                                }
                                else {
                                    NetworkLSA *toNetworkLSA = dynamic_cast<NetworkLSA *>(justAddedVertex);
                                    if (toNetworkLSA != nullptr) {
                                        for (i = 0; i < linkCount; i++) {
                                            Link& link = routerLSA->getLinks(i);

                                            if ((link.getType() == TRANSIT_LINK) &&
                                                (link.getLinkID() == toNetworkLSA->getHeader().getLinkStateID()) &&
                                                (virtualIntf->getState() < Interface::WAITING_STATE))
                                            {
                                                virtualNeighbor->setAddress(IPv4Address(link.getLinkData()));
                                                virtualIntf->processEvent(Interface::INTERFACE_UP);
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
                NetworkLSA *networkLSA = check_and_cast<NetworkLSA *>(closestVertex);
                IPv4Address destinationID = (networkLSA->getHeader().getLinkStateID() & networkLSA->getNetworkMask());
                unsigned int nextHopCount = networkLSA->getNextHopCount();
                bool overWrite = false;
                RoutingTableEntry *entry = nullptr;
                unsigned long routeCount = newRoutingTable.size();
                IPv4Address longestMatch(0u);

                for (i = 0; i < routeCount; i++) {
                    if (newRoutingTable[i]->getDestinationType() == RoutingTableEntry::NETWORK_DESTINATION) {
                        RoutingTableEntry *routingEntry = newRoutingTable[i];
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
                if (entry != nullptr) {
                    const OSPFLSA *entryOrigin = entry->getLinkStateOrigin();
                    if ((entry->getCost() != networkLSA->getDistance()) ||
                        (entryOrigin->getHeader().getLinkStateID() >= networkLSA->getHeader().getLinkStateID()))
                    {
                        overWrite = true;
                    }
                }

                if ((entry == nullptr) || (overWrite)) {
                    if (entry == nullptr) {
                        entry = new RoutingTableEntry(ift);
                    }

                    entry->setDestination(IPv4Address(destinationID));
                    entry->setNetmask(networkLSA->getNetworkMask());
                    entry->setLinkStateOrigin(networkLSA);
                    entry->setArea(areaID);
                    entry->setPathType(RoutingTableEntry::INTRAAREA);
                    entry->setCost(networkLSA->getDistance());
                    entry->setDestinationType(RoutingTableEntry::NETWORK_DESTINATION);
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
        RouterLSA *routerVertex = dynamic_cast<RouterLSA *>(treeVertices[i]);
        if (routerVertex == nullptr) {
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
            RoutingTableEntry *entry = nullptr;
            unsigned long routeCount = newRoutingTable.size();
            unsigned long longestMatch = 0;

            for (k = 0; k < routeCount; k++) {
                if (newRoutingTable[k]->getDestinationType() == RoutingTableEntry::NETWORK_DESTINATION) {
                    RoutingTableEntry *routingEntry = newRoutingTable[k];
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

            if (entry != nullptr) {
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
                    if (dynamic_cast<const RouterLSA *>(lsOrigin) || dynamic_cast<const NetworkLSA *>(lsOrigin)) {
                        if (lsOrigin->getHeader().getLinkStateID() < routerVertex->getHeader().getLinkStateID()) {
                            entry->setLinkStateOrigin(routerVertex);
                        }
                    }
                    else {
                        throw cRuntimeError("Can not cast class '%s' to RouterLSA or NetworkLSA", lsOrigin->getClassName());
                    }
                }
                std::vector<NextHop> *newNextHops = calculateNextHops(link, routerVertex);    // (destination, parent)
                unsigned int nextHopCount = newNextHops->size();
                for (k = 0; k < nextHopCount; k++) {
                    entry->addNextHop((*newNextHops)[k]);
                }
                delete newNextHops;
            }
            else {
                //FIXME remove
                //if(parentRouter->getRouterID() == 0xC0A80302) {
                //    EV << "STUB LINK FOUND TO " << IPv4Address(destinationID).str() << "\n";
                //}
                entry = new RoutingTableEntry(ift);

                entry->setDestination(IPv4Address(destinationID));
                entry->setNetmask(IPv4Address(link.getLinkData()));
                entry->setLinkStateOrigin(routerVertex);
                entry->setArea(areaID);
                entry->setPathType(RoutingTableEntry::INTRAAREA);
                entry->setCost(distance);
                entry->setDestinationType(RoutingTableEntry::NETWORK_DESTINATION);
                entry->setOptionalCapabilities(routerVertex->getHeader().getLsOptions());
                std::vector<NextHop> *newNextHops = calculateNextHops(link, routerVertex);    // (destination, parent)
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

std::vector<NextHop> *Area::calculateNextHops(OSPFLSA *destination, OSPFLSA *parent) const
{
    std::vector<NextHop> *hops = new std::vector<NextHop>;
    unsigned long i, j;

    RouterLSA *routerLSA = dynamic_cast<RouterLSA *>(parent);
    if (routerLSA != nullptr) {
        if (routerLSA != spfTreeRoot) {
            unsigned int nextHopCount = routerLSA->getNextHopCount();
            for (i = 0; i < nextHopCount; i++) {
                hops->push_back(routerLSA->getNextHop(i));
            }
            return hops;
        }
        else {
            RouterLSA *destinationRouterLSA = dynamic_cast<RouterLSA *>(destination);
            if (destinationRouterLSA != nullptr) {
                unsigned long interfaceNum = associatedInterfaces.size();
                for (i = 0; i < interfaceNum; i++) {
                    Interface::OSPFInterfaceType intfType = associatedInterfaces[i]->getType();
                    if ((intfType == Interface::POINTTOPOINT) ||
                        ((intfType == Interface::VIRTUAL) &&
                         (associatedInterfaces[i]->getState() > Interface::LOOPBACK_STATE)))
                    {
                        Neighbor *ptpNeighbor = associatedInterfaces[i]->getNeighborCount() > 0 ? associatedInterfaces[i]->getNeighbor(0) : nullptr;
                        if (ptpNeighbor != nullptr) {
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
                    if (intfType == Interface::POINTTOMULTIPOINT) {
                        Neighbor *ptmpNeighbor = associatedInterfaces[i]->getNeighborByID(destinationRouterLSA->getHeader().getLinkStateID());
                        if (ptmpNeighbor != nullptr) {
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
            }
            else {
                NetworkLSA *destinationNetworkLSA = dynamic_cast<NetworkLSA *>(destination);
                if (destinationNetworkLSA != nullptr) {
                    IPv4Address networkDesignatedRouter = destinationNetworkLSA->getHeader().getLinkStateID();
                    unsigned long interfaceNum = associatedInterfaces.size();
                    for (i = 0; i < interfaceNum; i++) {
                        Interface::OSPFInterfaceType intfType = associatedInterfaces[i]->getType();
                        if (((intfType == Interface::BROADCAST) ||
                             (intfType == Interface::NBMA)) &&
                            (associatedInterfaces[i]->getDesignatedRouter().ipInterfaceAddress == networkDesignatedRouter))
                        {
                            IPv4AddressRange range = associatedInterfaces[i]->getAddressRange();
                            NextHop nextHop;

                            nextHop.ifIndex = associatedInterfaces[i]->getIfIndex();
                            //nextHop.hopAddress = (range.address & range.mask); //TODO revise it!
                            nextHop.hopAddress = IPv4Address::UNSPECIFIED_ADDRESS;    //TODO revise it!
                            nextHop.advertisingRouter = destinationNetworkLSA->getHeader().getAdvertisingRouter();
                            hops->push_back(nextHop);
                        }
                    }
                }
            }
        }
    }
    else {
        NetworkLSA *networkLSA = dynamic_cast<NetworkLSA *>(parent);
        if (networkLSA != nullptr) {
            if (networkLSA->getParent() != spfTreeRoot) {
                unsigned int nextHopCount = networkLSA->getNextHopCount();
                for (i = 0; i < nextHopCount; i++) {
                    hops->push_back(networkLSA->getNextHop(i));
                }
                return hops;
            }
            else {
                IPv4Address parentLinkStateID = parent->getHeader().getLinkStateID();

                RouterLSA *destinationRouterLSA = dynamic_cast<RouterLSA *>(destination);
                if (destinationRouterLSA != nullptr) {
                    RouterID destinationRouterID = destinationRouterLSA->getHeader().getLinkStateID();
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
                                Interface::OSPFInterfaceType intfType = associatedInterfaces[j]->getType();
                                if (((intfType == Interface::BROADCAST) ||
                                     (intfType == Interface::NBMA)) &&
                                    (associatedInterfaces[j]->getDesignatedRouter().ipInterfaceAddress == parentLinkStateID))
                                {
                                    Neighbor *nextHopNeighbor = associatedInterfaces[j]->getNeighborByID(destinationRouterID);
                                    if (nextHopNeighbor != nullptr) {
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

std::vector<NextHop> *Area::calculateNextHops(Link& destination, OSPFLSA *parent) const
{
    std::vector<NextHop> *hops = new std::vector<NextHop>;
    unsigned long i;

    RouterLSA *routerLSA = check_and_cast<RouterLSA *>(parent);
    if (routerLSA != spfTreeRoot) {
        unsigned int nextHopCount = routerLSA->getNextHopCount();
        for (i = 0; i < nextHopCount; i++) {
            hops->push_back(routerLSA->getNextHop(i));
        }
        return hops;
    }
    else {
        unsigned long interfaceNum = associatedInterfaces.size();
        for (i = 0; i < interfaceNum; i++) {
            Interface *interface = associatedInterfaces[i];
            Interface::OSPFInterfaceType intfType = interface->getType();

            if ((intfType == Interface::POINTTOPOINT) ||
                ((intfType == Interface::VIRTUAL) &&
                 (interface->getState() > Interface::LOOPBACK_STATE)))
            {
                Neighbor *neighbor = (interface->getNeighborCount() > 0) ? interface->getNeighbor(0) : nullptr;
                if (neighbor != nullptr) {
                    IPv4Address neighborAddress = neighbor->getAddress();
                    if (((neighborAddress != NULL_IPV4ADDRESS) &&
                         (neighborAddress == destination.getLinkID())) ||
                        ((neighborAddress == NULL_IPV4ADDRESS) &&
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
            if ((intfType == Interface::BROADCAST) ||
                (intfType == Interface::NBMA))
            {
                if (isSameNetwork(destination.getLinkID(), IPv4Address(destination.getLinkData()), interface->getAddressRange().address, interface->getAddressRange().mask)) {
                    NextHop nextHop;
                    nextHop.ifIndex = interface->getIfIndex();
                    // TODO: this has been commented because the linkID is not a real IP address in this case and we don't know the next hop address here, verify
                    // nextHop.hopAddress = destination.getLinkID();
                    nextHop.advertisingRouter = parentRouter->getRouterID();
                    hops->push_back(nextHop);
                    break;
                }
            }
            if (intfType == Interface::POINTTOMULTIPOINT) {
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
                    Neighbor *neighbor = interface->getNeighborByID(destination.getLinkID());
                    if (neighbor != nullptr) {
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

bool Area::hasLink(OSPFLSA *fromLSA, OSPFLSA *toLSA) const
{
    unsigned int i;

    RouterLSA *fromRouterLSA = dynamic_cast<RouterLSA *>(fromLSA);
    if (fromRouterLSA != nullptr) {
        unsigned int linkCount = fromRouterLSA->getLinksArraySize();
        RouterLSA *toRouterLSA = dynamic_cast<RouterLSA *>(toLSA);
        if (toRouterLSA != nullptr) {
            for (i = 0; i < linkCount; i++) {
                Link& link = fromRouterLSA->getLinks(i);
                LinkType linkType = static_cast<LinkType>(link.getType());

                if (((linkType == POINTTOPOINT_LINK) ||
                     (linkType == VIRTUAL_LINK)) &&
                    (link.getLinkID() == toRouterLSA->getHeader().getLinkStateID()))
                {
                    return true;
                }
            }
        }
        else {
            NetworkLSA *toNetworkLSA = dynamic_cast<NetworkLSA *>(toLSA);
            if (toNetworkLSA != nullptr) {
                for (i = 0; i < linkCount; i++) {
                    Link& link = fromRouterLSA->getLinks(i);

                    if ((link.getType() == TRANSIT_LINK) &&
                        (link.getLinkID() == toNetworkLSA->getHeader().getLinkStateID()))
                    {
                        return true;
                    }
                    if ((link.getType() == STUB_LINK) &&
                        ((link.getLinkID() & IPv4Address(link.getLinkData())) == (toNetworkLSA->getHeader().getLinkStateID() & toNetworkLSA->getNetworkMask())))    //FIXME should compare masks?
                    {
                        return true;
                    }
                }
            }
        }
    }
    else {
        NetworkLSA *fromNetworkLSA = dynamic_cast<NetworkLSA *>(fromLSA);
        if (fromNetworkLSA != nullptr) {
            unsigned int routerCount = fromNetworkLSA->getAttachedRoutersArraySize();
            RouterLSA *toRouterLSA = dynamic_cast<RouterLSA *>(toLSA);
            if (toRouterLSA != nullptr) {
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
bool Area::findSameOrWorseCostRoute(const std::vector<RoutingTableEntry *>& newRoutingTable,
        const SummaryLSA& summaryLSA,
        unsigned short currentCost,
        bool& destinationInRoutingTable,
        std::list<RoutingTableEntry *>& sameOrWorseCost) const
{
    destinationInRoutingTable = false;
    sameOrWorseCost.clear();

    long routeCount = newRoutingTable.size();
    IPv4AddressRange destination;

    destination.address = summaryLSA.getHeader().getLinkStateID();
    destination.mask = summaryLSA.getNetworkMask();

    for (long j = 0; j < routeCount; j++) {
        RoutingTableEntry *routingEntry = newRoutingTable[j];
        bool foundMatching = false;

        if (summaryLSA.getHeader().getLsType() == SUMMARYLSA_NETWORKS_TYPE) {
            if ((routingEntry->getDestinationType() == RoutingTableEntry::NETWORK_DESTINATION) &&
                isSameNetwork(destination.address, destination.mask, routingEntry->getDestination(), routingEntry->getNetmask()))    //TODO  or use containing ?
            {
                foundMatching = true;
            }
        }
        else {
            if ((((routingEntry->getDestinationType() & RoutingTableEntry::AREA_BORDER_ROUTER_DESTINATION) != 0) ||
                 ((routingEntry->getDestinationType() & RoutingTableEntry::AS_BOUNDARY_ROUTER_DESTINATION) != 0)) &&
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
            if ((routingEntry->getPathType() == RoutingTableEntry::INTRAAREA) ||
                ((routingEntry->getPathType() == RoutingTableEntry::INTERAREA) &&
                 (routingEntry->getCost() < currentCost)))
            {
                return true;
            }
            else {
                // if it's an other INTERAREA path
                if ((routingEntry->getPathType() == RoutingTableEntry::INTERAREA) &&
                    (routingEntry->getCost() >= currentCost))
                {
                    sameOrWorseCost.push_back(routingEntry);
                }    // else it's external -> same as if not in the table
            }
        }
    }
    return false;
}

/**
 * Returns a new RoutingTableEntry based on the input SummaryLSA, with the input cost
 * and the borderRouterEntry's next hops.
 */
RoutingTableEntry *Area::createRoutingTableEntryFromSummaryLSA(const SummaryLSA& summaryLSA,
        unsigned short entryCost,
        const RoutingTableEntry& borderRouterEntry) const
{
    IPv4AddressRange destination;

    destination.address = summaryLSA.getHeader().getLinkStateID();
    destination.mask = summaryLSA.getNetworkMask();

    RoutingTableEntry *newEntry = new RoutingTableEntry(ift);

    if (summaryLSA.getHeader().getLsType() == SUMMARYLSA_NETWORKS_TYPE) {
        newEntry->setDestination(destination.address & destination.mask);
        newEntry->setNetmask(destination.mask);
        newEntry->setDestinationType(RoutingTableEntry::NETWORK_DESTINATION);
    }
    else {
        newEntry->setDestination(destination.address);
        newEntry->setNetmask(IPv4Address::ALLONES_ADDRESS);
        newEntry->setDestinationType(RoutingTableEntry::AS_BOUNDARY_ROUTER_DESTINATION);
    }
    newEntry->setArea(areaID);
    newEntry->setPathType(RoutingTableEntry::INTERAREA);
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
void Area::calculateInterAreaRoutes(std::vector<RoutingTableEntry *>& newRoutingTable)
{
    unsigned long i = 0;
    unsigned long j = 0;
    unsigned long lsaCount = summaryLSAs.size();

    for (i = 0; i < lsaCount; i++) {
        SummaryLSA *currentLSA = summaryLSAs[i];
        OSPFLSAHeader& currentHeader = currentLSA->getHeader();

        unsigned long routeCost = currentLSA->getRouteCost();
        unsigned short lsAge = currentHeader.getLsAge();
        RouterID originatingRouter = currentHeader.getAdvertisingRouter();
        bool selfOriginated = (originatingRouter == parentRouter->getRouterID());

        if ((routeCost == LS_INFINITY) || (lsAge == MAX_AGE) || (selfOriginated)) {    // (1) and(2)
            continue;
        }

        char lsType = currentHeader.getLsType();
        unsigned long routeCount = newRoutingTable.size();
        IPv4AddressRange destination;

        destination.address = currentHeader.getLinkStateID();
        destination.mask = currentLSA->getNetworkMask();

        if ((lsType == SUMMARYLSA_NETWORKS_TYPE) && (parentRouter->hasAddressRange(destination))) {    // (3)
            bool foundIntraAreaRoute = false;

            // look for an "Active" INTRAAREA route
            for (j = 0; j < routeCount; j++) {
                RoutingTableEntry *routingEntry = newRoutingTable[j];

                if ((routingEntry->getDestinationType() == RoutingTableEntry::NETWORK_DESTINATION) &&
                    (routingEntry->getPathType() == RoutingTableEntry::INTRAAREA) &&
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

        RoutingTableEntry *borderRouterEntry = nullptr;

        // The routingEntry describes a route to an other area -> look for the border router originating it
        for (j = 0; j < routeCount; j++) {    // (4) N == destination, BR == borderRouterEntry
            RoutingTableEntry *routingEntry = newRoutingTable[j];

            if ((routingEntry->getArea() == areaID) &&
                (((routingEntry->getDestinationType() & RoutingTableEntry::AREA_BORDER_ROUTER_DESTINATION) != 0) ||
                 ((routingEntry->getDestinationType() & RoutingTableEntry::AS_BOUNDARY_ROUTER_DESTINATION) != 0)) &&
                (routingEntry->getDestination() == originatingRouter))
            {
                borderRouterEntry = routingEntry;
                break;
            }
        }
        if (borderRouterEntry == nullptr) {
            continue;
        }
        else {    // (5)
            /* "Else, this LSA describes an inter-area path to destination N,
             * whose cost is the distance to BR plus the cost specified in the LSA.
             * Call the cost of this inter-area path IAC."
             */
            bool destinationInRoutingTable = true;
            unsigned short currentCost = routeCost + borderRouterEntry->getCost();
            std::list<RoutingTableEntry *> sameOrWorseCost;

            if (findSameOrWorseCostRoute(newRoutingTable,
                        *currentLSA,
                        currentCost,
                        destinationInRoutingTable,
                        sameOrWorseCost))
            {
                continue;
            }

            if (destinationInRoutingTable && (sameOrWorseCost.size() > 0)) {
                RoutingTableEntry *equalEntry = nullptr;

                /* Look for an equal cost entry in the sameOrWorseCost list, and
                 * also clear the more expensive entries from the newRoutingTable.
                 */
                for (auto it = sameOrWorseCost.begin(); it != sameOrWorseCost.end(); it++) {
                    RoutingTableEntry *checkedEntry = (*it);

                    if (checkedEntry->getCost() > currentCost) {
                        for (auto entryIt = newRoutingTable.begin(); entryIt != newRoutingTable.end(); entryIt++) {
                            if (checkedEntry == (*entryIt)) {
                                newRoutingTable.erase(entryIt);
                                break;
                            }
                        }
                    }
                    else {    // EntryCost == currentCost
                        equalEntry = checkedEntry;    // should be only one - if there are more they are ignored
                    }
                }

                unsigned long nextHopCount = borderRouterEntry->getNextHopCount();

                if (equalEntry != nullptr) {
                    /* Add the next hops of the border router advertising this destination
                     * to the equal entry.
                     */
                    for (unsigned long j = 0; j < nextHopCount; j++) {
                        equalEntry->addNextHop(borderRouterEntry->getNextHop(j));
                    }
                }
                else {
                    RoutingTableEntry *newEntry = createRoutingTableEntryFromSummaryLSA(*currentLSA, currentCost, *borderRouterEntry);
                    ASSERT(newEntry != nullptr);
                    newRoutingTable.push_back(newEntry);
                }
            }
            else {
                RoutingTableEntry *newEntry = createRoutingTableEntryFromSummaryLSA(*currentLSA, currentCost, *borderRouterEntry);
                ASSERT(newEntry != nullptr);
                newRoutingTable.push_back(newEntry);
            }
        }
    }
}

void Area::recheckSummaryLSAs(std::vector<RoutingTableEntry *>& newRoutingTable)
{
    unsigned long i = 0;
    unsigned long j = 0;
    unsigned long lsaCount = summaryLSAs.size();

    for (i = 0; i < lsaCount; i++) {
        SummaryLSA *currentLSA = summaryLSAs[i];
        OSPFLSAHeader& currentHeader = currentLSA->getHeader();

        unsigned long routeCost = currentLSA->getRouteCost();
        unsigned short lsAge = currentHeader.getLsAge();
        RouterID originatingRouter = currentHeader.getAdvertisingRouter();
        bool selfOriginated = (originatingRouter == parentRouter->getRouterID());

        if ((routeCost == LS_INFINITY) || (lsAge == MAX_AGE) || (selfOriginated)) {    // (1) and(2)
            continue;
        }

        unsigned long routeCount = newRoutingTable.size();
        char lsType = currentHeader.getLsType();
        RoutingTableEntry *destinationEntry = nullptr;
        IPv4AddressRange destination;

        destination.address = currentHeader.getLinkStateID();
        destination.mask = currentLSA->getNetworkMask();

        for (j = 0; j < routeCount; j++) {    // (3)
            RoutingTableEntry *routingEntry = newRoutingTable[j];
            bool foundMatching = false;

            if (lsType == SUMMARYLSA_NETWORKS_TYPE) {
                if ((routingEntry->getDestinationType() == RoutingTableEntry::NETWORK_DESTINATION) &&
                    ((destination.address & destination.mask) == routingEntry->getDestination()))
                {
                    foundMatching = true;
                }
            }
            else {
                if ((((routingEntry->getDestinationType() & RoutingTableEntry::AREA_BORDER_ROUTER_DESTINATION) != 0) ||
                     ((routingEntry->getDestinationType() & RoutingTableEntry::AS_BOUNDARY_ROUTER_DESTINATION) != 0)) &&
                    (destination.address == routingEntry->getDestination()))
                {
                    foundMatching = true;
                }
            }

            if (foundMatching) {
                RoutingTableEntry::RoutingPathType pathType = routingEntry->getPathType();

                if ((pathType == RoutingTableEntry::TYPE1_EXTERNAL) ||
                    (pathType == RoutingTableEntry::TYPE2_EXTERNAL) ||
                    (routingEntry->getArea() != BACKBONE_AREAID))
                {
                    break;
                }
                else {
                    destinationEntry = routingEntry;
                    break;
                }
            }
        }
        if (destinationEntry == nullptr) {
            continue;
        }

        RoutingTableEntry *borderRouterEntry = nullptr;
        unsigned short currentCost = routeCost;

        for (j = 0; j < routeCount; j++) {    // (4) BR == borderRouterEntry
            RoutingTableEntry *routingEntry = newRoutingTable[j];

            if ((routingEntry->getArea() == areaID) &&
                (((routingEntry->getDestinationType() & RoutingTableEntry::AREA_BORDER_ROUTER_DESTINATION) != 0) ||
                 ((routingEntry->getDestinationType() & RoutingTableEntry::AS_BOUNDARY_ROUTER_DESTINATION) != 0)) &&
                (routingEntry->getDestination() == originatingRouter))
            {
                borderRouterEntry = routingEntry;
                currentCost += borderRouterEntry->getCost();
                break;
            }
        }
        if (borderRouterEntry == nullptr) {
            continue;
        }
        else {    // (5)
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

} // namespace ospf

} // namespace inet

