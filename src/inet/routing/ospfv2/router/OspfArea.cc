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

#include "inet/routing/ospfv2/router/OspfArea.h"
#include "inet/routing/ospfv2/router/OspfRouter.h"
#include <memory.h>

namespace inet {

namespace ospf {

Area::Area(IInterfaceTable *ift, AreaId id) :
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
    for (auto interface : associatedInterfaces)
        delete (interface);
    associatedInterfaces.clear();

    for (auto routerLSA : routerLSAs)
        delete routerLSA;
    routerLSAs.clear();

    for (auto networkLSA : networkLSAs)
        delete networkLSA;
    networkLSAs.clear();

    for (auto summaryLSA : summaryLSAs)
        delete summaryLSA;
    summaryLSAs.clear();
}

void Area::addWatches()
{
    WATCH_PTRVECTOR(routerLSAs);
    WATCH_PTRVECTOR(networkLSAs);
    WATCH_PTRVECTOR(summaryLSAs);
    WATCH_PTRVECTOR(associatedInterfaces);
}

void Area::addInterface(OspfInterface *intf)
{
    intf->setArea(this);
    associatedInterfaces.push_back(intf);
}

void Area::addAddressRange(Ipv4AddressRange addressRange, bool advertise)
{
    int addressRangeNum = areaAddressRanges.size();
    bool found = false;
    bool erased = false;

    for (int i = 0; i < addressRangeNum; i++) {
        Ipv4AddressRange curRange = areaAddressRanges[i];
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

std::string Area::str() const
{
    return areaID.str(false);
}

std::string Area::info() const
{
    std::stringstream out;

    out << "areaID: " << areaID.str(false) << ", ";
    out << "transitCapability: " << (transitCapability ? "true" : "false") << ", ";
    out << "externalRoutingCapability: " << (externalRoutingCapability ? "true" : "false") << ", ";
    out << "stubDefaultCost: " << stubDefaultCost << ", ";
    for (uint32_t i = 0; i < areaAddressRanges.size(); i++) {
        out << "addressRanges[" << i << "]: ";
        out << areaAddressRanges[i].address.str(false);
        out << "/" << areaAddressRanges[i].mask.str(false) << " ";
    }
    for (uint32_t i = 0; i < associatedInterfaces.size(); i++) {
        out << "interface[" << i << "]: ";
        out << associatedInterfaces[i]->getAddressRange().address.str(false);
        out << "/" << associatedInterfaces[i]->getAddressRange().mask.str(false) << " ";
    }

    return out.str();
}

std::string Area::detailedInfo() const
{
    std::stringstream out;

    out << info();

    out << "RouterLSAs:\n";
    for (auto &entry : routerLSAs)
        out << "        " << entry << "\n";
    out << "NetworkLSAs:\n";
    for (auto &entry : networkLSAs)
        out << "        " << entry << "\n";
    out << "SummaryLSAs:\n";
    for (auto &entry : summaryLSAs)
        out << "        " << entry << "\n";

    return out.str();
}

bool Area::containsAddress(Ipv4Address address) const
{
    int addressRangeNum = areaAddressRanges.size();
    for (int i = 0; i < addressRangeNum; i++) {
        if (areaAddressRanges[i].contains(address)) {
            return true;
        }
    }
    return false;
}

bool Area::hasAddressRange(Ipv4AddressRange addressRange) const
{
    int addressRangeNum = areaAddressRanges.size();
    for (int i = 0; i < addressRangeNum; i++) {
        if (areaAddressRanges[i] == addressRange) {
            return true;
        }
    }
    return false;
}

Ipv4AddressRange Area::getContainingAddressRange(Ipv4AddressRange addressRange, bool *advertise    /*= nullptr*/) const
{
    int addressRangeNum = areaAddressRanges.size();
    for (int i = 0; i < addressRangeNum; i++) {
        if (areaAddressRanges[i].contains(addressRange)) {
            if (advertise != nullptr) {
                std::map<Ipv4AddressRange, bool>::const_iterator rangeIt = advertiseAddressRanges.find(areaAddressRanges[i]);
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

OspfInterface *Area::getInterface(unsigned char ifIndex)
{
    int interfaceNum = associatedInterfaces.size();
    for (int i = 0; i < interfaceNum; i++) {
        if ((associatedInterfaces[i]->getType() != OspfInterface::VIRTUAL) &&
            (associatedInterfaces[i]->getIfIndex() == ifIndex))
        {
            return associatedInterfaces[i];
        }
    }
    return nullptr;
}

OspfInterface *Area::getInterface(Ipv4Address address)
{
    int interfaceNum = associatedInterfaces.size();
    for (int i = 0; i < interfaceNum; i++) {
        if ((associatedInterfaces[i]->getType() != OspfInterface::VIRTUAL) &&
            (associatedInterfaces[i]->getAddressRange().address == address))
        {
            return associatedInterfaces[i];
        }
    }
    return nullptr;
}

std::vector<int> Area::getInterfaceIndices()
{
    std::vector<int> indices;
    for(auto &intf : associatedInterfaces)
        indices.push_back(intf->getIfIndex());
    return indices;
}

bool Area::hasVirtualLink(AreaId withTransitArea) const
{
    if ((areaID != BACKBONE_AREAID) || (withTransitArea == BACKBONE_AREAID)) {
        return false;
    }

    int interfaceNum = associatedInterfaces.size();
    for (int i = 0; i < interfaceNum; i++) {
        if ((associatedInterfaces[i]->getType() == OspfInterface::VIRTUAL) &&
            (associatedInterfaces[i]->getTransitAreaId() == withTransitArea))
        {
            return true;
        }
    }
    return false;
}

OspfInterface *Area::findVirtualLink(RouterId routerID)
{
    int interfaceNum = associatedInterfaces.size();
    for (int i = 0; i < interfaceNum; i++) {
        if ((associatedInterfaces[i]->getType() == OspfInterface::VIRTUAL) &&
            (associatedInterfaces[i]->getNeighborById(routerID) != nullptr))
        {
            return associatedInterfaces[i];
        }
    }
    return nullptr;
}

bool Area::installRouterLSA(const OspfRouterLsa *lsa)
{
    LinkStateId linkStateID = lsa->getHeader().getLinkStateID();
    auto lsaIt = routerLSAsByID.find(linkStateID);
    if (lsaIt != routerLSAsByID.end()) {
        LsaKeyType lsaKey;

        lsaKey.linkStateID = lsa->getHeader().getLinkStateID();
        lsaKey.advertisingRouter = lsa->getHeader().getAdvertisingRouter();

        removeFromAllRetransmissionLists(lsaKey);
        return lsaIt->second->update(lsa);
    }
    else {
        RouterLsa *lsaCopy = new RouterLsa(*lsa);
        routerLSAsByID[linkStateID] = lsaCopy;
        routerLSAs.push_back(lsaCopy);
        return true;
    }
}

bool Area::installNetworkLSA(const OspfNetworkLsa *lsa)
{
    LinkStateId linkStateID = lsa->getHeader().getLinkStateID();
    auto lsaIt = networkLSAsByID.find(linkStateID);
    if (lsaIt != networkLSAsByID.end()) {
        LsaKeyType lsaKey;

        lsaKey.linkStateID = lsa->getHeader().getLinkStateID();
        lsaKey.advertisingRouter = lsa->getHeader().getAdvertisingRouter();

        removeFromAllRetransmissionLists(lsaKey);
        return lsaIt->second->update(lsa);
    }
    else {
        NetworkLsa *lsaCopy = new NetworkLsa(*lsa);
        networkLSAsByID[linkStateID] = lsaCopy;
        networkLSAs.push_back(lsaCopy);
        return true;
    }
}

bool Area::installSummaryLSA(const OspfSummaryLsa *lsa)
{
    LsaKeyType lsaKey;

    lsaKey.linkStateID = lsa->getHeader().getLinkStateID();
    lsaKey.advertisingRouter = lsa->getHeader().getAdvertisingRouter();

    auto lsaIt = summaryLSAsByID.find(lsaKey);
    if (lsaIt != summaryLSAsByID.end()) {
        LsaKeyType lsaKey;

        lsaKey.linkStateID = lsa->getHeader().getLinkStateID();
        lsaKey.advertisingRouter = lsa->getHeader().getAdvertisingRouter();

        removeFromAllRetransmissionLists(lsaKey);
        return lsaIt->second->update(lsa);
    }
    else {
        SummaryLsa *lsaCopy = new SummaryLsa(*lsa);
        summaryLSAsByID[lsaKey] = lsaCopy;
        summaryLSAs.push_back(lsaCopy);
        return true;
    }
}

RouterLsa *Area::findRouterLSA(LinkStateId linkStateID)
{
    auto lsaIt = routerLSAsByID.find(linkStateID);
    if (lsaIt != routerLSAsByID.end()) {
        return lsaIt->second;
    }
    else {
        return nullptr;
    }
}

const RouterLsa *Area::findRouterLSA(LinkStateId linkStateID) const
{
    auto lsaIt = routerLSAsByID.find(linkStateID);
    if (lsaIt != routerLSAsByID.end()) {
        return lsaIt->second;
    }
    else {
        return nullptr;
    }
}

NetworkLsa *Area::findNetworkLSA(LinkStateId linkStateID)
{
    auto lsaIt = networkLSAsByID.find(linkStateID);
    if (lsaIt != networkLSAsByID.end()) {
        return lsaIt->second;
    }
    else {
        return nullptr;
    }
}

const NetworkLsa *Area::findNetworkLSA(LinkStateId linkStateID) const
{
    auto lsaIt = networkLSAsByID.find(linkStateID);
    if (lsaIt != networkLSAsByID.end()) {
        return lsaIt->second;
    }
    else {
        return nullptr;
    }
}

SummaryLsa *Area::findSummaryLSA(LsaKeyType lsaKey)
{
    auto lsaIt = summaryLSAsByID.find(lsaKey);
    if (lsaIt != summaryLSAsByID.end()) {
        return lsaIt->second;
    }
    else {
        return nullptr;
    }
}

const SummaryLsa *Area::findSummaryLSA(LsaKeyType lsaKey) const
{
    auto lsaIt = summaryLSAsByID.find(lsaKey);
    if (lsaIt != summaryLSAsByID.end()) {
        return lsaIt->second;
    }
    else {
        return nullptr;
    }
}

void Area::ageDatabase()
{
    bool shouldRebuildRoutingTable = false;

    for (uint32_t i = 0; i < routerLSAs.size(); i++) {
        RouterLsa *lsa = routerLSAs[i];
        unsigned short lsAge = lsa->getHeader().getLsAge();
        bool selfOriginated = (lsa->getHeader().getAdvertisingRouter() == parentRouter->getRouterID());
        bool unreachable = parentRouter->isDestinationUnreachable(lsa);

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
                floodLSA(lsa);
                lsa->incrementInstallTime();
            }
            else {
                long sequenceNumber = lsa->getHeader().getLsSequenceNumber();
                if (sequenceNumber == MAX_SEQUENCE_NUMBER) {
                    lsa->getHeaderForUpdate().setLsAge(MAX_AGE);
                    floodLSA(lsa);
                    lsa->incrementInstallTime();
                }
                else {
                    RouterLsa *newLSA = originateRouterLSA();

                    newLSA->getHeaderForUpdate().setLsSequenceNumber(sequenceNumber + 1);
                    shouldRebuildRoutingTable |= lsa->update(newLSA);
                    delete newLSA;

                    floodLSA(lsa);
                }
            }
        }
        if (!selfOriginated && (lsAge == MAX_AGE - 1)) {
            lsa->getHeaderForUpdate().setLsAge(MAX_AGE);
            floodLSA(lsa);
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
                    routerLSAsByID.erase(lsa->getHeader().getLinkStateID());
                    delete lsa;
                    routerLSAs[i] = nullptr;
                    shouldRebuildRoutingTable = true;
                }
                else {
                    RouterLsa *newLSA = originateRouterLSA();
                    long sequenceNumber = lsa->getHeader().getLsSequenceNumber();

                    newLSA->getHeaderForUpdate().setLsSequenceNumber((sequenceNumber == MAX_SEQUENCE_NUMBER) ? INITIAL_SEQUENCE_NUMBER : sequenceNumber + 1);
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

    for (uint32_t i = 0; i < networkLSAs.size(); i++) {
        unsigned short lsAge = networkLSAs[i]->getHeader().getLsAge();
        bool unreachable = parentRouter->isDestinationUnreachable(networkLSAs[i]);
        NetworkLsa *lsa = networkLSAs[i];
        OspfInterface *localIntf = getInterface(lsa->getHeader().getLinkStateID());
        bool selfOriginated = false;

        if ((localIntf != nullptr) &&
            (localIntf->getState() == OspfInterface::DESIGNATED_ROUTER_STATE) &&
            (localIntf->getNeighborCount() > 0) &&
            (localIntf->hasAnyNeighborInStates(Neighbor::FULL_STATE)))
        {
            selfOriginated = true;
        }

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
                floodLSA(lsa);
                lsa->incrementInstallTime();
            }
            else {
                long sequenceNumber = lsa->getHeader().getLsSequenceNumber();
                if (sequenceNumber == MAX_SEQUENCE_NUMBER) {
                    lsa->getHeaderForUpdate().setLsAge(MAX_AGE);
                    floodLSA(lsa);
                    lsa->incrementInstallTime();
                }
                else {
                    NetworkLsa *newLSA = originateNetworkLSA(localIntf);

                    if (newLSA != nullptr) {
                        newLSA->getHeaderForUpdate().setLsSequenceNumber(sequenceNumber + 1);
                        shouldRebuildRoutingTable |= lsa->update(newLSA);
                        delete newLSA;
                    }
                    else {    // no neighbors on the network -> old NetworkLsa must be flushed
                        lsa->getHeaderForUpdate().setLsAge(MAX_AGE);
                        lsa->incrementInstallTime();
                    }

                    floodLSA(lsa);
                }
            }
        }
        if (!selfOriginated && (lsAge == MAX_AGE - 1)) {
            lsa->getHeaderForUpdate().setLsAge(MAX_AGE);
            floodLSA(lsa);
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
                    networkLSAsByID.erase(lsa->getHeader().getLinkStateID());
                    delete lsa;
                    networkLSAs[i] = nullptr;
                    shouldRebuildRoutingTable = true;
                }
                else {
                    NetworkLsa *newLSA = originateNetworkLSA(localIntf);
                    long sequenceNumber = lsa->getHeader().getLsSequenceNumber();

                    if (newLSA != nullptr) {
                        newLSA->getHeaderForUpdate().setLsSequenceNumber((sequenceNumber == MAX_SEQUENCE_NUMBER) ? INITIAL_SEQUENCE_NUMBER : sequenceNumber + 1);
                        shouldRebuildRoutingTable |= lsa->update(newLSA);
                        delete newLSA;

                        floodLSA(lsa);
                    }
                    else {    // no neighbors on the network -> old NetworkLsa must be deleted
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

    for (uint32_t i = 0; i < summaryLSAs.size(); i++) {
        unsigned short lsAge = summaryLSAs[i]->getHeader().getLsAge();
        bool selfOriginated = (summaryLSAs[i]->getHeader().getAdvertisingRouter() == parentRouter->getRouterID());
        bool unreachable = parentRouter->isDestinationUnreachable(summaryLSAs[i]);
        SummaryLsa *lsa = summaryLSAs[i];

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
                floodLSA(lsa);
                lsa->incrementInstallTime();
            }
            else {
                long sequenceNumber = lsa->getHeader().getLsSequenceNumber();
                if (sequenceNumber == MAX_SEQUENCE_NUMBER) {
                    lsa->getHeaderForUpdate().setLsAge(MAX_AGE);
                    floodLSA(lsa);
                    lsa->incrementInstallTime();
                }
                else {
                    SummaryLsa *newLSA = originateSummaryLSA(lsa);

                    if (newLSA != nullptr) {
                        newLSA->getHeaderForUpdate().setLsSequenceNumber(sequenceNumber + 1);
                        shouldRebuildRoutingTable |= lsa->update(newLSA);
                        delete newLSA;

                        floodLSA(lsa);
                    }
                    else {
                        lsa->getHeaderForUpdate().setLsAge(MAX_AGE);
                        floodLSA(lsa);
                        lsa->incrementInstallTime();
                    }
                }
            }
        }
        if (!selfOriginated && (lsAge == MAX_AGE - 1)) {
            lsa->getHeaderForUpdate().setLsAge(MAX_AGE);
            floodLSA(lsa);
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
                    summaryLSAsByID.erase(lsaKey);
                    delete lsa;
                    summaryLSAs[i] = nullptr;
                    shouldRebuildRoutingTable = true;
                }
                else {
                    SummaryLsa *newLSA = originateSummaryLSA(lsa);
                    if (newLSA != nullptr) {
                        long sequenceNumber = lsa->getHeader().getLsSequenceNumber();

                        newLSA->getHeaderForUpdate().setLsSequenceNumber((sequenceNumber == MAX_SEQUENCE_NUMBER) ? INITIAL_SEQUENCE_NUMBER : sequenceNumber + 1);
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

    for (uint32_t m = 0; m < associatedInterfaces.size(); m++)
        associatedInterfaces[m]->ageTransmittedLsaLists();

    if (shouldRebuildRoutingTable)
        parentRouter->rebuildRoutingTable();
}

bool Area::hasAnyNeighborInStates(int states) const
{
    for (uint32_t i = 0; i < associatedInterfaces.size(); i++) {
        if (associatedInterfaces[i]->hasAnyNeighborInStates(states))
            return true;
    }
    return false;
}

void Area::removeFromAllRetransmissionLists(LsaKeyType lsaKey)
{
    for (uint32_t i = 0; i <  associatedInterfaces.size(); i++)
        associatedInterfaces[i]->removeFromAllRetransmissionLists(lsaKey);
}

bool Area::isOnAnyRetransmissionList(LsaKeyType lsaKey) const
{
    for (uint32_t i = 0; i < associatedInterfaces.size(); i++) {
        if (associatedInterfaces[i]->isOnAnyRetransmissionList(lsaKey))
            return true;
    }
    return false;
}

bool Area::floodLSA(const OspfLsa *lsa, OspfInterface *intf, Neighbor *neighbor)
{
    bool floodedBackOut = false;
    for (uint32_t i = 0; i < associatedInterfaces.size(); i++) {
        if (associatedInterfaces[i]->floodLsa(lsa, intf, neighbor)) {
            floodedBackOut = true;
        }
    }
    return floodedBackOut;
}

bool Area::isLocalAddress(Ipv4Address address) const
{
    for (uint32_t i = 0; i < associatedInterfaces.size(); i++) {
        if (associatedInterfaces[i]->getAddressRange().address == address)
            return true;
    }
    return false;
}

RouterLsa *Area::originateRouterLSA()
{
    RouterLsa *routerLSA = new RouterLsa;
    OspfLsaHeader& lsaHeader = routerLSA->getHeaderForUpdate();
    OspfOptions lsOptions;

    lsaHeader.setLsAge(0);
    memset(&lsOptions, 0, sizeof(OspfOptions));
    lsOptions.E_ExternalRoutingCapability = externalRoutingCapability;
    lsaHeader.setLsOptions(lsOptions);
    lsaHeader.setLsType(ROUTERLSA_TYPE);
    lsaHeader.setLinkStateID(parentRouter->getRouterID());
    lsaHeader.setAdvertisingRouter(Ipv4Address(parentRouter->getRouterID()));
    lsaHeader.setLsSequenceNumber(INITIAL_SEQUENCE_NUMBER);

    routerLSA->setB_AreaBorderRouter(parentRouter->getAreaCount() > 1);
    routerLSA->setE_ASBoundaryRouter((externalRoutingCapability && parentRouter->getASBoundaryRouter()) ? true : false);
    Area *backbone = parentRouter->getAreaByID(BACKBONE_AREAID);
    routerLSA->setV_VirtualLinkEndpoint((backbone == nullptr) ? false : backbone->hasVirtualLink(areaID));

    routerLSA->setNumberOfLinks(0);
    routerLSA->setLinksArraySize(0);
    for (uint32_t i = 0; i < associatedInterfaces.size(); i++) {
        OspfInterface *intf = associatedInterfaces[i];

        if (intf->getState() == OspfInterface::DOWN_STATE) {
            continue;
        }

        if ((intf->getState() == OspfInterface::LOOPBACK_STATE) &&
            ((intf->getType() != OspfInterface::POINTTOPOINT) ||
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
        if (intf->getState() > OspfInterface::LOOPBACK_STATE) {
            switch (intf->getType()) {
                case OspfInterface::POINTTOPOINT: {
                    Neighbor *neighbor = (intf->getNeighborCount() > 0) ? intf->getNeighbor(0) : nullptr;
                    if (neighbor != nullptr) {
                        if (neighbor->getState() == Neighbor::FULL_STATE) {
                            Link link;
                            link.setType(POINTTOPOINT_LINK);
                            link.setLinkID(Ipv4Address(neighbor->getNeighborID()));
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
                        if (intf->getState() == OspfInterface::POINTTOPOINT_STATE) {
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

                case OspfInterface::BROADCAST:
                case OspfInterface::NBMA: {
                    if (intf->getState() == OspfInterface::WAITING_STATE) {
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

                case OspfInterface::VIRTUAL: {
                    Neighbor *neighbor = (intf->getNeighborCount() > 0) ? intf->getNeighbor(0) : nullptr;
                    if ((neighbor != nullptr) && (neighbor->getState() == Neighbor::FULL_STATE)) {
                        Link link;
                        link.setType(VIRTUAL_LINK);
                        link.setLinkID(Ipv4Address(neighbor->getNeighborID()));
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

                case OspfInterface::POINTTOMULTIPOINT: {
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
                            link.setLinkID(Ipv4Address(neighbor->getNeighborID()));
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

    for (uint32_t i = 0; i < hostRoutes.size(); i++) {
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

    // update the length field in the LSA header
    lsaHeader.setLsaLength(calculateLSASize(routerLSA).get());

    routerLSA->setSource(LsaTrackingInfo::ORIGINATED);

    return routerLSA;
}

NetworkLsa *Area::originateNetworkLSA(const OspfInterface *intf)
{
    if (intf->hasAnyNeighborInStates(Neighbor::FULL_STATE)) {
        NetworkLsa *networkLSA = new NetworkLsa;
        OspfLsaHeader& lsaHeader = networkLSA->getHeaderForUpdate();
        long neighborCount = intf->getNeighborCount();
        OspfOptions lsOptions;

        lsaHeader.setLsAge(0);
        memset(&lsOptions, 0, sizeof(OspfOptions));
        lsOptions.E_ExternalRoutingCapability = externalRoutingCapability;
        lsaHeader.setLsOptions(lsOptions);
        lsaHeader.setLsType(NETWORKLSA_TYPE);
        lsaHeader.setLinkStateID(intf->getAddressRange().address);
        lsaHeader.setAdvertisingRouter(Ipv4Address(parentRouter->getRouterID()));
        lsaHeader.setLsSequenceNumber(INITIAL_SEQUENCE_NUMBER);

        networkLSA->setNetworkMask(intf->getAddressRange().mask);

        for (long j = 0; j < neighborCount; j++) {
            const Neighbor *neighbor = intf->getNeighbor(j);
            if (neighbor->getState() == Neighbor::FULL_STATE) {
                unsigned short netIndex = networkLSA->getAttachedRoutersArraySize();
                networkLSA->setAttachedRoutersArraySize(netIndex + 1);
                networkLSA->setAttachedRouters(netIndex, Ipv4Address(neighbor->getNeighborID()));
            }
        }
        unsigned short netIndex = networkLSA->getAttachedRoutersArraySize();
        networkLSA->setAttachedRoutersArraySize(netIndex + 1);
        networkLSA->setAttachedRouters(netIndex, Ipv4Address(parentRouter->getRouterID()));

        // update the length field in the LSA header
        uint32_t totalSize = (OSPF_LSA_HEADER_LENGTH + OSPF_NETWORKLSA_MASK_LENGTH +
                B(networkLSA->getAttachedRoutersArraySize() * (OSPF_NETWORKLSA_ADDRESS_LENGTH).get()) ).get();
        lsaHeader.setLsaLength(totalSize);

        return networkLSA;
    }
    else {
        return nullptr;
    }
}

/**
 * Returns a link state ID for the input destination.
 * If this router hasn't originated a Summary LSA for the input destination then
 * the function returns the destination address as link state ID. If it has originated
 * a Summary LSA for the input destination then the function checks which LSA would
 * contain the longer netmask. If the two masks are equal then this means that we're
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
LinkStateId Area::getUniqueLinkStateID(Ipv4AddressRange destination,
        Metric destinationCost,
        SummaryLsa *& lsaToReoriginate) const
{
    if (lsaToReoriginate != nullptr) {
        delete lsaToReoriginate;
        lsaToReoriginate = nullptr;
    }

    LsaKeyType lsaKey;

    lsaKey.linkStateID = destination.address;
    lsaKey.advertisingRouter = parentRouter->getRouterID();

    const SummaryLsa *foundLSA = findSummaryLSA(lsaKey);

    if (foundLSA == nullptr) {
        return lsaKey.linkStateID;
    }
    else {
        Ipv4Address existingMask = foundLSA->getNetworkMask();

        if (destination.mask == existingMask) {
            return lsaKey.linkStateID;
        }
        else {
            if (destination.mask >= existingMask) {
                return lsaKey.linkStateID.makeBroadcastAddress(destination.mask);
            }
            else {
                SummaryLsa *summaryLSA = new SummaryLsa(*foundLSA);

                long sequenceNumber = summaryLSA->getHeader().getLsSequenceNumber();

                summaryLSA->getHeaderForUpdate().setLsAge(0);
                summaryLSA->getHeaderForUpdate().setLsSequenceNumber((sequenceNumber == MAX_SEQUENCE_NUMBER) ? INITIAL_SEQUENCE_NUMBER : sequenceNumber + 1);
                summaryLSA->setNetworkMask(destination.mask);
                summaryLSA->setRouteCost(destinationCost);

                lsaToReoriginate = summaryLSA;

                return lsaKey.linkStateID.makeBroadcastAddress(existingMask);
            }
        }
    }
}

SummaryLsa *Area::originateSummaryLSA(const OspfRoutingTableEntry *entry,
        const std::map<LsaKeyType, bool, LsaKeyType_Less>& originatedLSAs,
        SummaryLsa *& lsaToReoriginate)
{
    if (((entry->getDestinationType() & OspfRoutingTableEntry::AREA_BORDER_ROUTER_DESTINATION) != 0) ||
        (entry->getPathType() == OspfRoutingTableEntry::TYPE1_EXTERNAL) ||
        (entry->getPathType() == OspfRoutingTableEntry::TYPE2_EXTERNAL) ||
        (entry->getArea() == areaID))
    {
        return nullptr;
    }

    bool allNextHopsInThisArea = true;
    for (unsigned int i = 0; i < entry->getNextHopCount(); i++) {
        OspfInterface *nextHopInterface = parentRouter->getNonVirtualInterface(entry->getNextHop(i).ifIndex);
        if ((nextHopInterface != nullptr) && (nextHopInterface->getAreaId() != areaID)) {
            allNextHopsInThisArea = false;
            break;
        }
    }
    if ((allNextHopsInThisArea) || (entry->getCost() >= LS_INFINITY)) {
        return nullptr;
    }

    if ((entry->getDestinationType() & OspfRoutingTableEntry::AS_BOUNDARY_ROUTER_DESTINATION) != 0) {
        OspfRoutingTableEntry *preferredEntry = parentRouter->getPreferredEntry(*(entry->getLinkStateOrigin()), false);
        if ((preferredEntry != nullptr) && (*preferredEntry == *entry) && (externalRoutingCapability)) {
            SummaryLsa *summaryLSA = new SummaryLsa;
            OspfLsaHeader& lsaHeader = summaryLSA->getHeaderForUpdate();
            OspfOptions lsOptions;

            lsaHeader.setLsAge(0);
            memset(&lsOptions, 0, sizeof(OspfOptions));
            lsOptions.E_ExternalRoutingCapability = externalRoutingCapability;
            lsaHeader.setLsOptions(lsOptions);
            lsaHeader.setLsType(SUMMARYLSA_ASBOUNDARYROUTERS_TYPE);
            lsaHeader.setLinkStateID(entry->getDestination());
            lsaHeader.setAdvertisingRouter(Ipv4Address(parentRouter->getRouterID()));
            lsaHeader.setLsSequenceNumber(INITIAL_SEQUENCE_NUMBER);

            summaryLSA->setNetworkMask(entry->getNetmask());
            summaryLSA->setRouteCost(entry->getCost());
            summaryLSA->setTosDataArraySize(0);

            summaryLSA->setSource(LsaTrackingInfo::ORIGINATED);

            return summaryLSA;
        }
    }
    else {    // entry->getDestinationType() == OspfRoutingTableEntry::NETWORK_DESTINATION
        if (entry->getPathType() == OspfRoutingTableEntry::INTERAREA) {
            Ipv4AddressRange destinationRange;

            destinationRange.address = entry->getDestination();
            destinationRange.mask = entry->getNetmask();

            LinkStateId newLinkStateID = getUniqueLinkStateID(destinationRange, entry->getCost(), lsaToReoriginate);

            if (lsaToReoriginate != nullptr) {
                LsaKeyType lsaKey;

                lsaKey.linkStateID = entry->getDestination();
                lsaKey.advertisingRouter = parentRouter->getRouterID();

                auto lsaIt = summaryLSAsByID.find(lsaKey);
                if (lsaIt == summaryLSAsByID.end()) {
                    delete (lsaToReoriginate);
                    lsaToReoriginate = nullptr;
                    return nullptr;
                }
                else {
                    SummaryLsa *summaryLSA = new SummaryLsa(*(lsaIt->second));
                    OspfLsaHeader& lsaHeader = summaryLSA->getHeaderForUpdate();

                    lsaHeader.setLsAge(0);
                    lsaHeader.setLsSequenceNumber(INITIAL_SEQUENCE_NUMBER);
                    lsaHeader.setLinkStateID(newLinkStateID);

                    return summaryLSA;
                }
            }
            else {
                SummaryLsa *summaryLSA = new SummaryLsa;
                OspfLsaHeader& lsaHeader = summaryLSA->getHeaderForUpdate();
                OspfOptions lsOptions;

                lsaHeader.setLsAge(0);
                memset(&lsOptions, 0, sizeof(OspfOptions));
                lsOptions.E_ExternalRoutingCapability = externalRoutingCapability;
                lsaHeader.setLsOptions(lsOptions);
                lsaHeader.setLsType(SUMMARYLSA_NETWORKS_TYPE);
                lsaHeader.setLinkStateID(newLinkStateID);
                lsaHeader.setAdvertisingRouter(Ipv4Address(parentRouter->getRouterID()));
                lsaHeader.setLsSequenceNumber(INITIAL_SEQUENCE_NUMBER);

                summaryLSA->setNetworkMask(entry->getNetmask());
                summaryLSA->setRouteCost(entry->getCost());
                summaryLSA->setTosDataArraySize(0);

                summaryLSA->setSource(LsaTrackingInfo::ORIGINATED);

                return summaryLSA;
            }
        }
        else {    // entry->getPathType() == OspfRoutingTableEntry::INTRAAREA
            Ipv4AddressRange destinationAddressRange;

            destinationAddressRange.address = entry->getDestination();
            destinationAddressRange.mask = entry->getNetmask();

            bool doAdvertise = false;
            Ipv4AddressRange containingAddressRange = parentRouter->getContainingAddressRange(destinationAddressRange, &doAdvertise);
            if (((entry->getArea() == BACKBONE_AREAID) &&    // the backbone's configured ranges should be ignored
                 (transitCapability)) ||    // when originating Summary LSAs into transit areas
                (containingAddressRange == NULL_IPV4ADDRESSRANGE))
            {
                LinkStateId newLinkStateID = getUniqueLinkStateID(destinationAddressRange, entry->getCost(), lsaToReoriginate);

                if (lsaToReoriginate != nullptr) {
                    LsaKeyType lsaKey;

                    lsaKey.linkStateID = entry->getDestination();
                    lsaKey.advertisingRouter = parentRouter->getRouterID();

                    auto lsaIt = summaryLSAsByID.find(lsaKey);
                    if (lsaIt == summaryLSAsByID.end()) {
                        delete (lsaToReoriginate);
                        lsaToReoriginate = nullptr;
                        return nullptr;
                    }
                    else {
                        SummaryLsa *summaryLSA = new SummaryLsa(*(lsaIt->second));
                        OspfLsaHeader& lsaHeader = summaryLSA->getHeaderForUpdate();

                        lsaHeader.setLsAge(0);
                        lsaHeader.setLsSequenceNumber(INITIAL_SEQUENCE_NUMBER);
                        lsaHeader.setLinkStateID(newLinkStateID);

                        return summaryLSA;
                    }
                }
                else {
                    SummaryLsa *summaryLSA = new SummaryLsa;
                    OspfLsaHeader& lsaHeader = summaryLSA->getHeaderForUpdate();
                    OspfOptions lsOptions;

                    lsaHeader.setLsAge(0);
                    memset(&lsOptions, 0, sizeof(OspfOptions));
                    lsOptions.E_ExternalRoutingCapability = externalRoutingCapability;
                    lsaHeader.setLsOptions(lsOptions);
                    lsaHeader.setLsType(SUMMARYLSA_NETWORKS_TYPE);
                    lsaHeader.setLinkStateID(newLinkStateID);
                    lsaHeader.setAdvertisingRouter(Ipv4Address(parentRouter->getRouterID()));
                    lsaHeader.setLsSequenceNumber(INITIAL_SEQUENCE_NUMBER);

                    summaryLSA->setNetworkMask(entry->getNetmask());
                    summaryLSA->setRouteCost(entry->getCost());
                    summaryLSA->setTosDataArraySize(0);

                    summaryLSA->setSource(LsaTrackingInfo::ORIGINATED);

                    return summaryLSA;
                }
            }
            else {
                if (doAdvertise) {
                    Metric maxRangeCost = 0;
                    for (unsigned long i = 0; i < parentRouter->getRoutingTableEntryCount(); i++) {
                        const OspfRoutingTableEntry *routingEntry = parentRouter->getRoutingTableEntry(i);
                        if ((routingEntry->getDestinationType() == OspfRoutingTableEntry::NETWORK_DESTINATION) &&
                            (routingEntry->getPathType() == OspfRoutingTableEntry::INTRAAREA) &&
                            containingAddressRange.containsRange(routingEntry->getDestination(), routingEntry->getNetmask()) &&
                            (routingEntry->getCost() > maxRangeCost))
                        {
                            maxRangeCost = routingEntry->getCost();
                        }
                    }

                    LinkStateId newLinkStateID = getUniqueLinkStateID(containingAddressRange, maxRangeCost, lsaToReoriginate);
                    LsaKeyType lsaKey;

                    if (lsaToReoriginate != nullptr) {
                        lsaKey.linkStateID = lsaToReoriginate->getHeader().getLinkStateID();
                        lsaKey.advertisingRouter = parentRouter->getRouterID();

                        auto originatedIt = originatedLSAs.find(lsaKey);
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

                        SummaryLsa *summaryLSA = new SummaryLsa(*(lsaIt->second));
                        OspfLsaHeader& lsaHeader = summaryLSA->getHeaderForUpdate();

                        lsaHeader.setLsAge(0);
                        lsaHeader.setLsSequenceNumber(INITIAL_SEQUENCE_NUMBER);
                        lsaHeader.setLinkStateID(newLinkStateID);

                        return summaryLSA;
                    }
                    else {
                        lsaKey.linkStateID = newLinkStateID;
                        lsaKey.advertisingRouter = parentRouter->getRouterID();

                        auto originatedIt = originatedLSAs.find(lsaKey);
                        if (originatedIt != originatedLSAs.end()) {
                            return nullptr;
                        }

                        SummaryLsa *summaryLSA = new SummaryLsa;
                        OspfLsaHeader& lsaHeader = summaryLSA->getHeaderForUpdate();
                        OspfOptions lsOptions;

                        lsaHeader.setLsAge(0);
                        memset(&lsOptions, 0, sizeof(OspfOptions));
                        lsOptions.E_ExternalRoutingCapability = externalRoutingCapability;
                        lsaHeader.setLsOptions(lsOptions);
                        lsaHeader.setLsType(SUMMARYLSA_NETWORKS_TYPE);
                        lsaHeader.setLinkStateID(newLinkStateID);
                        lsaHeader.setAdvertisingRouter(Ipv4Address(parentRouter->getRouterID()));
                        lsaHeader.setLsSequenceNumber(INITIAL_SEQUENCE_NUMBER);

                        //summaryLSA->setNetworkMask(entry->getNetmask());
                        summaryLSA->setNetworkMask(containingAddressRange.mask);
                        summaryLSA->setRouteCost(entry->getCost());
                        summaryLSA->setTosDataArraySize(0);

                        summaryLSA->setSource(LsaTrackingInfo::ORIGINATED);

                        return summaryLSA;
                    }
                }
            }
        }
    }

    return nullptr;
}

SummaryLsa *Area::originateSummaryLSA_Stub()
{
    SummaryLsa *summaryLSA = new SummaryLsa;
    OspfLsaHeader& lsaHeader = summaryLSA->getHeaderForUpdate();
    OspfOptions lsOptions;

    lsaHeader.setLsAge(0);
    memset(&lsOptions, 0, sizeof(OspfOptions));
    lsOptions.E_ExternalRoutingCapability = externalRoutingCapability;
    lsaHeader.setLsOptions(lsOptions);
    lsaHeader.setLsType(SUMMARYLSA_NETWORKS_TYPE);
    lsaHeader.setLinkStateID(Ipv4Address("0.0.0.0"));
    lsaHeader.setAdvertisingRouter(Ipv4Address(parentRouter->getRouterID()));
    lsaHeader.setLsSequenceNumber(INITIAL_SEQUENCE_NUMBER);

    summaryLSA->setNetworkMask(Ipv4Address("0.0.0.0"));
    summaryLSA->setRouteCost(stubDefaultCost);
    summaryLSA->setTosDataArraySize(0);

    summaryLSA->setSource(LsaTrackingInfo::ORIGINATED);

    return summaryLSA;
}

SummaryLsa *Area::originateSummaryLSA(const SummaryLsa *summaryLSA)
{
    const std::map<LsaKeyType, bool, LsaKeyType_Less> emptyMap;
    SummaryLsa *dontReoriginate = nullptr;

    const OspfLsaHeader& lsaHeader = summaryLSA->getHeader();
    unsigned long entryCount = parentRouter->getRoutingTableEntryCount();

    for (unsigned long i = 0; i < entryCount; i++) {
        const OspfRoutingTableEntry *entry = parentRouter->getRoutingTableEntry(i);

        if ((lsaHeader.getLsType() == SUMMARYLSA_ASBOUNDARYROUTERS_TYPE) &&
            ((((entry->getDestinationType() & OspfRoutingTableEntry::AREA_BORDER_ROUTER_DESTINATION) != 0) ||
              ((entry->getDestinationType() & OspfRoutingTableEntry::AS_BOUNDARY_ROUTER_DESTINATION) != 0)) &&
             ((entry->getDestination() == lsaHeader.getLinkStateID()) &&    //FIXME Why not compare network addresses (addr masked with netmask)?
              (entry->getNetmask() == summaryLSA->getNetworkMask()))))
        {
            SummaryLsa *returnLSA = originateSummaryLSA(entry, emptyMap, dontReoriginate);
            if (dontReoriginate != nullptr) {
                delete dontReoriginate;
            }
            return returnLSA;
        }

        Ipv4Address lsaMask = summaryLSA->getNetworkMask();

        if ((lsaHeader.getLsType() == SUMMARYLSA_NETWORKS_TYPE) &&
            (entry->getDestinationType() == OspfRoutingTableEntry::NETWORK_DESTINATION) &&
            isSameNetwork(entry->getDestination(), entry->getNetmask(), lsaHeader.getLinkStateID(), lsaMask))
        {
            SummaryLsa *returnLSA = originateSummaryLSA(entry, emptyMap, dontReoriginate);
            if (dontReoriginate != nullptr) {
                delete dontReoriginate;
            }
            return returnLSA;
        }
    }

    return nullptr;
}

void Area::calculateShortestPathTree(std::vector<OspfRoutingTableEntry *>& newRoutingTable)
{
    bool finished = false;
    std::vector<OspfLsa *> treeVertices;
    OspfLsa *justAddedVertex;
    std::vector<OspfLsa *> candidateVertices;

    printLSDB();

    if (spfTreeRoot == nullptr) {
        RouterLsa *newLSA = originateRouterLSA();

        installRouterLSA(newLSA);

        RouterLsa *routerLSA = findRouterLSA(parentRouter->getRouterID());

        spfTreeRoot = routerLSA;
        floodLSA(newLSA);
        delete newLSA;
    }

    if (spfTreeRoot == nullptr)
        return;

    for (uint32_t i = 0; i < routerLSAs.size(); i++)
        routerLSAs[i]->clearNextHops();

    for (uint32_t i = 0; i < networkLSAs.size(); i++)
        networkLSAs[i]->clearNextHops();

    spfTreeRoot->setDistance(0);
    treeVertices.push_back(spfTreeRoot);
    justAddedVertex = spfTreeRoot;    // (1)

    do {
        LsaType vertexType = static_cast<LsaType>(justAddedVertex->getHeader().getLsType());

        if (vertexType == ROUTERLSA_TYPE) {
            RouterLsa *routerVertex = check_and_cast<RouterLsa *>(justAddedVertex);
            if (routerVertex->getV_VirtualLinkEndpoint()) {    // (2)
                transitCapability = true;
            }

            unsigned int linkCount = routerVertex->getLinksArraySize();
            for (uint32_t i = 0; i < linkCount; i++) {
                const Link& link = routerVertex->getLinks(i);
                LinkType linkType = static_cast<LinkType>(link.getType());
                OspfLsa *joiningVertex;
                LsaType joiningVertexType;

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

                for (uint32_t j = 0; j < treeSize; j++) {
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
                OspfLsa *candidate = nullptr;

                for (uint32_t j = 0; j < candidateCount; j++) {
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
                    for (auto it = newNextHops->begin(); it != newNextHops->end(); ++it)
                        routingInfo->addNextHop(*it);
                    delete newNextHops;
                }
                else {
                    if (joiningVertexType == ROUTERLSA_TYPE) {
                        RouterLsa *joiningRouterVertex = check_and_cast<RouterLsa *>(joiningVertex);
                        joiningRouterVertex->setDistance(linkStateCost);
                        std::vector<NextHop> *newNextHops = calculateNextHops(joiningVertex, justAddedVertex);    // (destination, parent)
                        for (auto it = newNextHops->begin(); it != newNextHops->end(); ++it)
                            joiningRouterVertex->addNextHop(*it);
                        delete newNextHops;
                        RoutingInfo *vertexRoutingInfo = check_and_cast<RoutingInfo *>(joiningRouterVertex);
                        vertexRoutingInfo->setParent(justAddedVertex);

                        candidateVertices.push_back(joiningRouterVertex);
                    }
                    else {
                        NetworkLsa *joiningNetworkVertex = check_and_cast<NetworkLsa *>(joiningVertex);
                        joiningNetworkVertex->setDistance(linkStateCost);
                        std::vector<NextHop> *newNextHops = calculateNextHops(joiningVertex, justAddedVertex);    // (destination, parent)
                        for (auto it = newNextHops->begin(); it != newNextHops->end(); ++it)
                            joiningNetworkVertex->addNextHop(*it);
                        delete newNextHops;
                        RoutingInfo *vertexRoutingInfo = check_and_cast<RoutingInfo *>(joiningNetworkVertex);
                        vertexRoutingInfo->setParent(justAddedVertex);

                        candidateVertices.push_back(joiningNetworkVertex);
                    }
                }
            }
        }

        if (vertexType == NETWORKLSA_TYPE) {
            NetworkLsa *networkVertex = check_and_cast<NetworkLsa *>(justAddedVertex);
            unsigned int routerCount = networkVertex->getAttachedRoutersArraySize();

            for (uint32_t i = 0; i < routerCount; i++) {    // (2)
                RouterLsa *joiningVertex = findRouterLSA(networkVertex->getAttachedRouters(i));
                if ((joiningVertex == nullptr) ||
                    (joiningVertex->getHeader().getLsAge() == MAX_AGE) ||
                    (!hasLink(joiningVertex, justAddedVertex)))    // (from, to)     (2) (b)
                {
                    continue;
                }

                unsigned int treeSize = treeVertices.size();
                bool alreadyOnTree = false;

                for (uint32_t j = 0; j < treeSize; j++) {
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
                OspfLsa *candidate = nullptr;

                for (uint32_t j = 0; j < candidateCount; j++) {
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
                    for (auto it = newNextHops->begin(); it != newNextHops->end(); ++it)
                        routingInfo->addNextHop(*it);
                    delete newNextHops;
                }
                else {
                    joiningVertex->setDistance(linkStateCost);
                    std::vector<NextHop> *newNextHops = calculateNextHops(joiningVertex, justAddedVertex);    // (destination, parent)
                    for (auto it = newNextHops->begin(); it != newNextHops->end(); ++it)
                        joiningVertex->addNextHop(*it);
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
            OspfLsa *closestVertex = candidateVertices[0];

            for (uint32_t i = 0; i < candidateCount; i++) {
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
                RouterLsa *routerLSA = check_and_cast<RouterLsa *>(closestVertex);
                if (routerLSA->getB_AreaBorderRouter() || routerLSA->getE_ASBoundaryRouter()) {
                    OspfRoutingTableEntry *entry = new OspfRoutingTableEntry(ift);
                    RouterId destinationID = routerLSA->getHeader().getLinkStateID();
                    unsigned int nextHopCount = routerLSA->getNextHopCount();
                    OspfRoutingTableEntry::RoutingDestinationType destinationType = OspfRoutingTableEntry::NETWORK_DESTINATION;

                    entry->setDestination(destinationID);
                    entry->setLinkStateOrigin(routerLSA);
                    entry->setArea(areaID);
                    entry->setPathType(OspfRoutingTableEntry::INTRAAREA);
                    entry->setCost(routerLSA->getDistance());
                    if (routerLSA->getB_AreaBorderRouter()) {
                        destinationType |= OspfRoutingTableEntry::AREA_BORDER_ROUTER_DESTINATION;
                    }
                    if (routerLSA->getE_ASBoundaryRouter()) {
                        destinationType |= OspfRoutingTableEntry::AS_BOUNDARY_ROUTER_DESTINATION;
                    }
                    entry->setDestinationType(destinationType);
                    entry->setOptionalCapabilities(routerLSA->getHeader().getLsOptions());
                    for (uint32_t i = 0; i < nextHopCount; i++) {
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
                        OspfInterface *virtualIntf = backbone->findVirtualLink(destinationID);
                        if ((virtualIntf != nullptr) && (virtualIntf->getTransitAreaId() == areaID)) {
                            Ipv4AddressRange range;
                            range.address = getInterface(routerLSA->getNextHop(0).ifIndex)->getAddressRange().address;
                            range.mask = Ipv4Address::ALLONES_ADDRESS;
                            virtualIntf->setAddressRange(range);
                            virtualIntf->setIfIndex(ift, routerLSA->getNextHop(0).ifIndex);
                            virtualIntf->setOutputCost(routerLSA->getDistance());
                            Neighbor *virtualNeighbor = virtualIntf->getNeighbor(0);
                            if (virtualNeighbor != nullptr) {
                                unsigned int linkCount = routerLSA->getLinksArraySize();
                                RouterLsa *toRouterLSA = dynamic_cast<RouterLsa *>(justAddedVertex);
                                if (toRouterLSA != nullptr) {
                                    for (uint32_t i = 0; i < linkCount; i++) {
                                        const Link& link = routerLSA->getLinks(i);

                                        if ((link.getType() == POINTTOPOINT_LINK) &&
                                            (link.getLinkID() == toRouterLSA->getHeader().getLinkStateID()) &&
                                            (virtualIntf->getState() < OspfInterface::WAITING_STATE))
                                        {
                                            virtualNeighbor->setAddress(Ipv4Address(link.getLinkData()));
                                            virtualIntf->processEvent(OspfInterface::INTERFACE_UP);
                                            break;
                                        }
                                    }
                                }
                                else {
                                    NetworkLsa *toNetworkLSA = dynamic_cast<NetworkLsa *>(justAddedVertex);
                                    if (toNetworkLSA != nullptr) {
                                        for (uint32_t i = 0; i < linkCount; i++) {
                                            const Link& link = routerLSA->getLinks(i);

                                            if ((link.getType() == TRANSIT_LINK) &&
                                                (link.getLinkID() == toNetworkLSA->getHeader().getLinkStateID()) &&
                                                (virtualIntf->getState() < OspfInterface::WAITING_STATE))
                                            {
                                                virtualNeighbor->setAddress(Ipv4Address(link.getLinkData()));
                                                virtualIntf->processEvent(OspfInterface::INTERFACE_UP);
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
                NetworkLsa *networkLSA = check_and_cast<NetworkLsa *>(closestVertex);
                Ipv4Address destinationID = (networkLSA->getHeader().getLinkStateID() & networkLSA->getNetworkMask());
                unsigned int nextHopCount = networkLSA->getNextHopCount();
                bool overWrite = false;
                OspfRoutingTableEntry *entry = nullptr;
                unsigned long routeCount = newRoutingTable.size();
                Ipv4Address longestMatch(0u);

                for (uint32_t i = 0; i < routeCount; i++) {
                    if (newRoutingTable[i]->getDestinationType() == OspfRoutingTableEntry::NETWORK_DESTINATION) {
                        OspfRoutingTableEntry *routingEntry = newRoutingTable[i];
                        Ipv4Address entryAddress = routingEntry->getDestination();
                        Ipv4Address entryMask = routingEntry->getNetmask();

                        if ((entryAddress & entryMask) == (destinationID & entryMask)) {
                            if ((destinationID & entryMask) > longestMatch) {
                                longestMatch = (destinationID & entryMask);
                                entry = routingEntry;
                            }
                        }
                    }
                }
                if (entry != nullptr) {
                    const OspfLsa *entryOrigin = entry->getLinkStateOrigin();
                    if ((entry->getCost() != networkLSA->getDistance()) ||
                        (entryOrigin->getHeader().getLinkStateID() >= networkLSA->getHeader().getLinkStateID()))
                    {
                        overWrite = true;
                    }
                }

                if ((entry == nullptr) || (overWrite)) {
                    if (entry == nullptr) {
                        entry = new OspfRoutingTableEntry(ift);
                    }

                    entry->setDestination(Ipv4Address(destinationID));
                    entry->setNetmask(networkLSA->getNetworkMask());
                    entry->setLinkStateOrigin(networkLSA);
                    entry->setArea(areaID);
                    entry->setPathType(OspfRoutingTableEntry::INTRAAREA);
                    entry->setCost(networkLSA->getDistance());
                    entry->setDestinationType(OspfRoutingTableEntry::NETWORK_DESTINATION);
                    entry->setOptionalCapabilities(networkLSA->getHeader().getLsOptions());
                    for (uint32_t i = 0; i < nextHopCount; i++) {
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

    // set parent to null for all router LSAs not on the SPF tree
    for (auto &routerLSA : routerLSAs) {
        bool onTree = false;
        for(auto &node : treeVertices) {
            if(node == routerLSA) {
                onTree = true;
                break;
            }
        }
        if (onTree)
            continue;
        else {
            routerLSA->setParent(nullptr);
        }
    }
    // set parent to null for all network LSAs not on the SPF tree
    for (auto &networkLSA : networkLSAs) {
        bool onTree = false;
        for(auto &node : treeVertices) {
            if(node == networkLSA) {
                onTree = true;
                break;
            }
        }
        if (onTree)
            continue;
        else {
            networkLSA->setParent(nullptr);
        }
    }

    for (uint32_t i = 0; i < treeVertices.size(); i++) {
        RouterLsa *routerVertex = dynamic_cast<RouterLsa *>(treeVertices[i]);
        if (routerVertex == nullptr)
            continue;

        for (uint32_t j = 0; j < routerVertex->getLinksArraySize(); j++) {
            const Link& link = routerVertex->getLinks(j);
            if (link.getType() != STUB_LINK)
                continue;

            unsigned long destinationID = (link.getLinkID().getInt() & link.getLinkData());
            OspfRoutingTableEntry *entry = nullptr;
            unsigned long longestMatch = 0;

            for (auto routingEntry : newRoutingTable) {
                if (routingEntry->getDestinationType() == OspfRoutingTableEntry::NETWORK_DESTINATION) {
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

            unsigned long distance = routerVertex->getDistance() + link.getLinkCost();

            if (entry != nullptr) {
                Metric entryCost = entry->getCost();

                if (distance > entryCost)
                    continue;
                else if (distance < entryCost) {
                    entry->setCost(distance);
                    entry->clearNextHops();
                    entry->setLinkStateOrigin(routerVertex);
                }
                else if (distance == entryCost) {
                    // no const version from check_and_cast
                    const OspfLsa *lsOrigin = entry->getLinkStateOrigin();
                    if (dynamic_cast<const RouterLsa *>(lsOrigin) || dynamic_cast<const NetworkLsa *>(lsOrigin)) {
                        if (lsOrigin->getHeader().getLinkStateID() < routerVertex->getHeader().getLinkStateID())
                            entry->setLinkStateOrigin(routerVertex);
                    }
                    else
                        throw cRuntimeError("Can not cast class '%s' to RouterLsa or NetworkLsa", lsOrigin->getClassName());
                }

                std::vector<NextHop> *newNextHops = calculateNextHops(link, routerVertex);    // (destination, parent)
                for (auto it = newNextHops->begin(); it != newNextHops->end(); ++it)
                    entry->addNextHop(*it);
                delete newNextHops;
            }
            else {
                entry = new OspfRoutingTableEntry(ift);

                entry->setDestination(Ipv4Address(destinationID));
                entry->setNetmask(Ipv4Address(link.getLinkData()));
                entry->setLinkStateOrigin(routerVertex);
                entry->setArea(areaID);
                entry->setPathType(OspfRoutingTableEntry::INTRAAREA);
                entry->setCost(distance);
                entry->setDestinationType(OspfRoutingTableEntry::NETWORK_DESTINATION);
                entry->setOptionalCapabilities(routerVertex->getHeader().getLsOptions());
                std::vector<NextHop> *newNextHops = calculateNextHops(link, routerVertex);    // (destination, parent)
                for (auto it = newNextHops->begin(); it != newNextHops->end(); ++it)
                    entry->addNextHop(*it);
                delete newNextHops;

                newRoutingTable.push_back(entry);
            }
        }
    }
}

std::vector<NextHop> *Area::calculateNextHops(OspfLsa *destination, OspfLsa *parent) const
{
    std::vector<NextHop> *hops = new std::vector<NextHop>;

    RouterLsa *routerLSA = dynamic_cast<RouterLsa *>(parent);
    if (routerLSA != nullptr) {
        if (routerLSA != spfTreeRoot) {
            for (uint32_t i = 0; i < routerLSA->getNextHopCount(); i++)
                hops->push_back(routerLSA->getNextHop(i));
            return hops;
        }
        else {
            RouterLsa *destinationRouterLSA = dynamic_cast<RouterLsa *>(destination);
            if (destinationRouterLSA != nullptr) {
                for (auto interface : associatedInterfaces) {
                    OspfInterface::OspfInterfaceType intfType = interface->getType();
                    if ((intfType == OspfInterface::POINTTOPOINT) ||
                        ((intfType == OspfInterface::VIRTUAL) &&
                         (interface->getState() > OspfInterface::LOOPBACK_STATE)))
                    {
                        Neighbor *ptpNeighbor = interface->getNeighborCount() > 0 ? interface->getNeighbor(0) : nullptr;
                        if (ptpNeighbor != nullptr) {
                            if (ptpNeighbor->getNeighborID() == destinationRouterLSA->getHeader().getLinkStateID()) {
                                NextHop nextHop;
                                nextHop.ifIndex = interface->getIfIndex();
                                nextHop.hopAddress = ptpNeighbor->getAddress();
                                nextHop.advertisingRouter = destinationRouterLSA->getHeader().getAdvertisingRouter();
                                hops->push_back(nextHop);
                                break;
                            }
                        }
                    }
                    if (intfType == OspfInterface::POINTTOMULTIPOINT) {
                        Neighbor *ptmpNeighbor = interface->getNeighborById(destinationRouterLSA->getHeader().getLinkStateID());
                        if (ptmpNeighbor != nullptr) {
                            unsigned int linkCount = destinationRouterLSA->getLinksArraySize();
                            Ipv4Address rootID = Ipv4Address(parentRouter->getRouterID());
                            for (uint32_t j = 0; j < linkCount; j++) {
                                const Link& link = destinationRouterLSA->getLinks(j);
                                if (link.getLinkID() == rootID) {
                                    NextHop nextHop;
                                    nextHop.ifIndex = interface->getIfIndex();
                                    nextHop.hopAddress = Ipv4Address(link.getLinkData());
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
                NetworkLsa *destinationNetworkLSA = dynamic_cast<NetworkLsa *>(destination);
                if (destinationNetworkLSA != nullptr) {
                    Ipv4Address networkDesignatedRouter = destinationNetworkLSA->getHeader().getLinkStateID();
                    for (auto interface : associatedInterfaces) {
                        OspfInterface::OspfInterfaceType intfType = interface->getType();
                        if (((intfType == OspfInterface::BROADCAST) ||
                             (intfType == OspfInterface::NBMA)) &&
                            (interface->getDesignatedRouter().ipInterfaceAddress == networkDesignatedRouter))
                        {
                            Ipv4AddressRange range = interface->getAddressRange();
                            NextHop nextHop;
                            nextHop.ifIndex = interface->getIfIndex();
                            if(interface->getNeighborCount() == 1) {
                                Neighbor *neighbor = interface->getNeighbor(0);
                                nextHop.hopAddress = neighbor->getAddress();
                            }
                            else
                                nextHop.hopAddress = Ipv4Address::UNSPECIFIED_ADDRESS;
                            nextHop.advertisingRouter = destinationNetworkLSA->getHeader().getAdvertisingRouter();
                            hops->push_back(nextHop);
                        }
                    }
                }
            }
        }
    }
    else {
        NetworkLsa *networkLSA = dynamic_cast<NetworkLsa *>(parent);
        if (networkLSA != nullptr) {
            if (networkLSA->getParent() != spfTreeRoot) {
                for (uint32_t i = 0; i < networkLSA->getNextHopCount(); i++)
                    hops->push_back(networkLSA->getNextHop(i));
                return hops;
            }
            else {
                Ipv4Address parentLinkStateID = parent->getHeader().getLinkStateID();

                RouterLsa *destinationRouterLSA = dynamic_cast<RouterLsa *>(destination);
                if (destinationRouterLSA != nullptr) {
                    RouterId destinationRouterID = destinationRouterLSA->getHeader().getLinkStateID();
                    for (uint32_t i = 0; i < destinationRouterLSA->getLinksArraySize(); i++) {
                        const Link& link = destinationRouterLSA->getLinks(i);
                        NextHop nextHop;

                        if (((link.getType() == TRANSIT_LINK) &&
                             (link.getLinkID() == parentLinkStateID)) ||
                            ((link.getType() == STUB_LINK) &&
                             ((link.getLinkID() & Ipv4Address(link.getLinkData())) == (parentLinkStateID & networkLSA->getNetworkMask()))))
                        {
                            for (auto interface : associatedInterfaces) {
                                OspfInterface::OspfInterfaceType intfType = interface->getType();
                                if (((intfType == OspfInterface::BROADCAST) ||
                                     (intfType == OspfInterface::NBMA)) &&
                                    (interface->getDesignatedRouter().ipInterfaceAddress == parentLinkStateID))
                                {
                                    Neighbor *nextHopNeighbor = interface->getNeighborById(destinationRouterID);
                                    if (nextHopNeighbor != nullptr) {
                                        nextHop.ifIndex = interface->getIfIndex();
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

std::vector<NextHop> *Area::calculateNextHops(const Link& destination, OspfLsa *parent) const
{
    std::vector<NextHop> *hops = new std::vector<NextHop>;

    RouterLsa *routerLSA = check_and_cast<RouterLsa *>(parent);
    if (routerLSA != spfTreeRoot) {
        for (uint32_t i = 0; i < routerLSA->getNextHopCount(); i++)
            hops->push_back(routerLSA->getNextHop(i));
        return hops;
    }
    else {
        for (auto interface : associatedInterfaces) {
            OspfInterface::OspfInterfaceType intfType = interface->getType();

            if ((intfType == OspfInterface::POINTTOPOINT) ||
                ((intfType == OspfInterface::VIRTUAL) &&
                 (interface->getState() > OspfInterface::LOOPBACK_STATE)))
            {
                Neighbor *neighbor = (interface->getNeighborCount() > 0) ? interface->getNeighbor(0) : nullptr;
                if (neighbor != nullptr) {
                    Ipv4Address neighborAddress = neighbor->getAddress();
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
            if ((intfType == OspfInterface::BROADCAST) ||
                (intfType == OspfInterface::NBMA))
            {
                if (isSameNetwork(destination.getLinkID(), Ipv4Address(destination.getLinkData()), interface->getAddressRange().address, interface->getAddressRange().mask)) {
                    NextHop nextHop;
                    nextHop.ifIndex = interface->getIfIndex();
                    if(interface->getNeighborCount() == 1) {
                        Neighbor *neighbor = interface->getNeighbor(0);
                        nextHop.hopAddress = neighbor->getAddress();
                    }
                    else
                        nextHop.hopAddress = Ipv4Address::UNSPECIFIED_ADDRESS;
                    nextHop.advertisingRouter = parentRouter->getRouterID();
                    hops->push_back(nextHop);
                    break;
                }
            }
            if (intfType == OspfInterface::POINTTOMULTIPOINT) {
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
                    Neighbor *neighbor = interface->getNeighborById(destination.getLinkID());
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
            for (uint32_t i = 0; i < hostRoutes.size(); i++) {
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

bool Area::hasLink(OspfLsa *fromLSA, OspfLsa *toLSA) const
{
    unsigned int i;

    RouterLsa *fromRouterLSA = dynamic_cast<RouterLsa *>(fromLSA);
    if (fromRouterLSA != nullptr) {
        unsigned int linkCount = fromRouterLSA->getLinksArraySize();
        RouterLsa *toRouterLSA = dynamic_cast<RouterLsa *>(toLSA);
        if (toRouterLSA != nullptr) {
            for (i = 0; i < linkCount; i++) {
                const Link& link = fromRouterLSA->getLinks(i);
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
            NetworkLsa *toNetworkLSA = dynamic_cast<NetworkLsa *>(toLSA);
            if (toNetworkLSA != nullptr) {
                for (i = 0; i < linkCount; i++) {
                    const Link& link = fromRouterLSA->getLinks(i);

                    if ((link.getType() == TRANSIT_LINK) &&
                        (link.getLinkID() == toNetworkLSA->getHeader().getLinkStateID()))
                    {
                        return true;
                    }
                    if ((link.getType() == STUB_LINK) &&
                        ((link.getLinkID() & Ipv4Address(link.getLinkData())) == (toNetworkLSA->getHeader().getLinkStateID() & toNetworkLSA->getNetworkMask())))    //FIXME should compare masks?
                    {
                        return true;
                    }
                }
            }
        }
    }
    else {
        NetworkLsa *fromNetworkLSA = dynamic_cast<NetworkLsa *>(fromLSA);
        if (fromNetworkLSA != nullptr) {
            unsigned int routerCount = fromNetworkLSA->getAttachedRoutersArraySize();
            RouterLsa *toRouterLSA = dynamic_cast<RouterLsa *>(toLSA);
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
bool Area::findSameOrWorseCostRoute(const std::vector<OspfRoutingTableEntry *>& newRoutingTable,
        const SummaryLsa& summaryLSA,
        unsigned short currentCost,
        bool& destinationInRoutingTable,
        std::list<OspfRoutingTableEntry *>& sameOrWorseCost) const
{
    destinationInRoutingTable = false;
    sameOrWorseCost.clear();

    Ipv4AddressRange destination;
    destination.address = summaryLSA.getHeader().getLinkStateID();
    destination.mask = summaryLSA.getNetworkMask();

    for (auto routingEntry : newRoutingTable) {
        bool foundMatching = false;

        if (summaryLSA.getHeader().getLsType() == SUMMARYLSA_NETWORKS_TYPE) {
            if ((routingEntry->getDestinationType() == OspfRoutingTableEntry::NETWORK_DESTINATION) &&
                    destination.containedByRange(routingEntry->getDestination(), routingEntry->getNetmask()))
            {
                if(isDefaultRoute(routingEntry) && !isAllZero(destination))
                    foundMatching = false;
                else
                    foundMatching = true;
            }
        }
        else {
            if ((((routingEntry->getDestinationType() & OspfRoutingTableEntry::AREA_BORDER_ROUTER_DESTINATION) != 0) ||
                 ((routingEntry->getDestinationType() & OspfRoutingTableEntry::AS_BOUNDARY_ROUTER_DESTINATION) != 0)) &&
                (destination.address == routingEntry->getDestination()) &&
                (destination.mask == routingEntry->getNetmask()))
            {
                foundMatching = true;
            }
        }

        if (foundMatching) {
            destinationInRoutingTable = true;

            /* If the matching entry is an INTRAAREA route (intra-area paths are
             * always preferred to other paths of any cost), or it's a cheaper INTERAREA
             * route, then skip this LSA.
             */
            if ((routingEntry->getPathType() == OspfRoutingTableEntry::INTRAAREA) ||
                ((routingEntry->getPathType() == OspfRoutingTableEntry::INTERAREA) &&
                 (routingEntry->getCost() < currentCost)))
            {
                return true;
            }
            else {
                // if it's an other INTERAREA path
                if ((routingEntry->getPathType() == OspfRoutingTableEntry::INTERAREA) &&
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
 * Returns a new OspfRoutingTableEntry based on the input SummaryLsa, with the input cost
 * and the borderRouterEntry's next hops.
 */
OspfRoutingTableEntry *Area::createRoutingTableEntryFromSummaryLSA(const SummaryLsa& summaryLSA,
        unsigned short entryCost,
        const OspfRoutingTableEntry& borderRouterEntry) const
{
    Ipv4AddressRange destination;

    destination.address = summaryLSA.getHeader().getLinkStateID();
    destination.mask = summaryLSA.getNetworkMask();

    OspfRoutingTableEntry *newEntry = new OspfRoutingTableEntry(ift);

    if (summaryLSA.getHeader().getLsType() == SUMMARYLSA_NETWORKS_TYPE) {
        newEntry->setDestination(destination.address & destination.mask);
        newEntry->setNetmask(destination.mask);
        newEntry->setDestinationType(OspfRoutingTableEntry::NETWORK_DESTINATION);
    }
    else {
        newEntry->setDestination(destination.address);
        newEntry->setNetmask(Ipv4Address::ALLONES_ADDRESS);
        newEntry->setDestinationType(OspfRoutingTableEntry::AS_BOUNDARY_ROUTER_DESTINATION);
    }
    newEntry->setArea(areaID);
    newEntry->setPathType(OspfRoutingTableEntry::INTERAREA);
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
void Area::calculateInterAreaRoutes(std::vector<OspfRoutingTableEntry *>& newRoutingTable)
{
    printSummaryLsa();

    for (uint32_t i = 0; i < summaryLSAs.size(); i++) {
        SummaryLsa *currentLSA = summaryLSAs[i];
        const OspfLsaHeader& currentHeader = currentLSA->getHeader();

        unsigned long routeCost = currentLSA->getRouteCost();
        unsigned short lsAge = currentHeader.getLsAge();
        RouterId originatingRouter = currentHeader.getAdvertisingRouter();
        bool selfOriginated = (originatingRouter == parentRouter->getRouterID());

        if ((routeCost == LS_INFINITY) || (lsAge == MAX_AGE) || (selfOriginated)) {    // (1) and(2)
            continue;
        }

        char lsType = currentHeader.getLsType();
        unsigned long routeCount = newRoutingTable.size();
        Ipv4AddressRange destination;

        destination.address = currentHeader.getLinkStateID();
        destination.mask = currentLSA->getNetworkMask();

        if ((lsType == SUMMARYLSA_NETWORKS_TYPE) && (parentRouter->hasAddressRange(destination))) {    // (3)
            // look for an "Active" INTRAAREA route
            bool foundIntraAreaRoute = false;
            for (uint32_t j = 0; j < routeCount; j++) {
                OspfRoutingTableEntry *routingEntry = newRoutingTable[j];
                if ((routingEntry->getDestinationType() == OspfRoutingTableEntry::NETWORK_DESTINATION) &&
                    (routingEntry->getPathType() == OspfRoutingTableEntry::INTRAAREA) &&
                    destination.containedByRange(routingEntry->getDestination(), routingEntry->getNetmask()))
                {
                    foundIntraAreaRoute = true;
                    break;
                }
            }
            if (foundIntraAreaRoute)
                continue;
        }

        OspfRoutingTableEntry *borderRouterEntry = nullptr;

        // The routingEntry describes a route to an other area -> look for the border router originating it
        for (uint32_t j = 0; j < routeCount; j++) {    // (4) N == destination, BR == borderRouterEntry
            OspfRoutingTableEntry *routingEntry = newRoutingTable[j];

            if ((routingEntry->getArea() == areaID) &&
                (((routingEntry->getDestinationType() & OspfRoutingTableEntry::AREA_BORDER_ROUTER_DESTINATION) != 0) ||
                 ((routingEntry->getDestinationType() & OspfRoutingTableEntry::AS_BOUNDARY_ROUTER_DESTINATION) != 0)) &&
                (routingEntry->getDestination() == originatingRouter))
            {
                borderRouterEntry = routingEntry;
                break;
            }
        }
        if (!borderRouterEntry)
            continue;

        /* (5) "this LSA describes an inter-area path to destination N,
         * whose cost is the distance to BR plus the cost specified in the LSA.
         * Call the cost of this inter-area path IAC."
         */
        bool destinationInRoutingTable = true;
        unsigned short currentCost = routeCost + borderRouterEntry->getCost();
        std::list<OspfRoutingTableEntry *> sameOrWorseCost;

        if (findSameOrWorseCostRoute(newRoutingTable,
                *currentLSA,
                currentCost,
                destinationInRoutingTable,
                sameOrWorseCost))
        {
            continue;
        }

        if (destinationInRoutingTable && (sameOrWorseCost.size() > 0)) {
            OspfRoutingTableEntry *equalEntry = nullptr;

            /* Look for an equal cost entry in the sameOrWorseCost list, and
             * also clear the more expensive entries from the newRoutingTable.
             */
            //FIXME The code does not work according to the comment
            for (auto checkedEntry : sameOrWorseCost) {
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
                OspfRoutingTableEntry *newEntry = createRoutingTableEntryFromSummaryLSA(*currentLSA, currentCost, *borderRouterEntry);
                ASSERT(newEntry != nullptr);
                newRoutingTable.push_back(newEntry);
            }
        }
        else {
            OspfRoutingTableEntry *newEntry = createRoutingTableEntryFromSummaryLSA(*currentLSA, currentCost, *borderRouterEntry);
            ASSERT(newEntry != nullptr);
            newRoutingTable.push_back(newEntry);
        }
    }
}

void Area::recheckSummaryLSAs(std::vector<OspfRoutingTableEntry *>& newRoutingTable)
{
    for (uint32_t i = 0; i < summaryLSAs.size(); i++) {
        SummaryLsa *currentLSA = summaryLSAs[i];
        const OspfLsaHeader& currentHeader = currentLSA->getHeader();

        unsigned long routeCost = currentLSA->getRouteCost();
        unsigned short lsAge = currentHeader.getLsAge();
        RouterId originatingRouter = currentHeader.getAdvertisingRouter();
        bool selfOriginated = (originatingRouter == parentRouter->getRouterID());

        if ((routeCost == LS_INFINITY) || (lsAge == MAX_AGE) || (selfOriginated)) {    // (1) and(2)
            continue;
        }

        unsigned long routeCount = newRoutingTable.size();
        char lsType = currentHeader.getLsType();
        OspfRoutingTableEntry *destinationEntry = nullptr;
        Ipv4AddressRange destination;

        destination.address = currentHeader.getLinkStateID();
        destination.mask = currentLSA->getNetworkMask();

        for (uint32_t j = 0; j < routeCount; j++) {    // (3)
            OspfRoutingTableEntry *routingEntry = newRoutingTable[j];
            bool foundMatching = false;

            if (lsType == SUMMARYLSA_NETWORKS_TYPE) {
                if ((routingEntry->getDestinationType() == OspfRoutingTableEntry::NETWORK_DESTINATION) &&
                    ((destination.address & destination.mask) == routingEntry->getDestination()))
                {
                    foundMatching = true;
                }
            }
            else {
                if ((((routingEntry->getDestinationType() & OspfRoutingTableEntry::AREA_BORDER_ROUTER_DESTINATION) != 0) ||
                     ((routingEntry->getDestinationType() & OspfRoutingTableEntry::AS_BOUNDARY_ROUTER_DESTINATION) != 0)) &&
                    (destination.address == routingEntry->getDestination()))
                {
                    foundMatching = true;
                }
            }

            if (foundMatching) {
                OspfRoutingTableEntry::RoutingPathType pathType = routingEntry->getPathType();

                if ((pathType == OspfRoutingTableEntry::TYPE1_EXTERNAL) ||
                    (pathType == OspfRoutingTableEntry::TYPE2_EXTERNAL) ||
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

        OspfRoutingTableEntry *borderRouterEntry = nullptr;
        unsigned short currentCost = routeCost;

        for (uint32_t j = 0; j < routeCount; j++) {    // (4) BR == borderRouterEntry
            OspfRoutingTableEntry *routingEntry = newRoutingTable[j];

            if ((routingEntry->getArea() == areaID) &&
                (((routingEntry->getDestinationType() & OspfRoutingTableEntry::AREA_BORDER_ROUTER_DESTINATION) != 0) ||
                 ((routingEntry->getDestinationType() & OspfRoutingTableEntry::AS_BOUNDARY_ROUTER_DESTINATION) != 0)) &&
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

                for (uint32_t j = 0; j < nextHopCount; j++) {
                    destinationEntry->addNextHop(borderRouterEntry->getNextHop(j));
                }
            }
        }
    }
}

void Area::printLSDB()
{
    // iterate over all routerLSA in all routers inside this area
    for (uint32_t i = 0; i < routerLSAs.size(); i++) {
        OspfRouterLsa *entry = check_and_cast<OspfRouterLsa *>(routerLSAs[i]);

        const OspfLsaHeader &head = entry->getHeader();
        std::string routerId = head.getAdvertisingRouter().str(false);
        EV_INFO << "Router LSA in Area " << areaID.str(false) << " in OSPF router with ID " << routerId << std::endl;

        // print header info
        EV_INFO << "    LS age: " << head.getLsAge() << std::endl;
        EV_INFO << "    LS type: " << head.getLsType() << std::endl;
        EV_INFO << "    Link state ID: " << head.getLinkStateID() << std::endl;
        EV_INFO << "    Advertising router: " << head.getAdvertisingRouter() << std::endl;
        EV_INFO << "    Seq number: " << head.getLsSequenceNumber() << std::endl;
        EV_INFO << "    Length: " << head.getLsaLength() << std::endl;

        EV_INFO << "    Number of links: " << entry->getLinksArraySize() << std::endl << std::endl;
        for(unsigned int j = 0; j < entry->getLinksArraySize(); j++) {
            const Link &lEntry = entry->getLinks(j);
            LinkType linkType = static_cast<LinkType>(lEntry.getType());
            if(linkType == POINTTOPOINT_LINK) {
                EV_INFO << "        Link connected to: another router (point-to-point)" << std::endl;
                EV_INFO << "        Neighboring router ID (link ID): " << lEntry.getLinkID() << std::endl;
                EV_INFO << "        Router interface address (link data): " << Ipv4Address(lEntry.getLinkData()).str(false) << std::endl;
                EV_INFO << "        Link cost: " << lEntry.getLinkCost() << std::endl;
            } else if(linkType == TRANSIT_LINK) {
                EV_INFO << "        Link connected to: a transit network" << std::endl;
                EV_INFO << "        DR address (link ID): " << lEntry.getLinkID() << std::endl;
                EV_INFO << "        Router interface address (link data): " << Ipv4Address(lEntry.getLinkData()).str(false) << std::endl;
                EV_INFO << "        Link cost: " << lEntry.getLinkCost() << std::endl;
            } else if(linkType == STUB_LINK) {
                EV_INFO << "        Link connected to: a stub network" << std::endl;
                EV_INFO << "        Network/subnet number (link ID): " << lEntry.getLinkID() << std::endl;
                EV_INFO << "        Network mask (link data): " << Ipv4Address(lEntry.getLinkData()).str(false) << std::endl;
                EV_INFO << "        Link cost: " << lEntry.getLinkCost() << std::endl;
            } else {
                EV_INFO << "        Link connected to: a virtual link" << std::endl;
            }
            EV_INFO << std::endl;
        }
    }

    // iterate over all networkLSA in all routers inside this area
    for (unsigned int i = 0; i < networkLSAs.size(); i++) {
        EV_INFO << "Network LSA in Area " << areaID.str(false) << std::endl;
        OspfNetworkLsa *entry = check_and_cast<OspfNetworkLsa *>(networkLSAs[i]);

        // print header info
        const OspfLsaHeader &head = entry->getHeader();
        EV_INFO << "    LS age: " << head.getLsAge() << std::endl;
        EV_INFO << "    LS type: " << head.getLsType() << std::endl;
        EV_INFO << "    Link state ID: " << head.getLinkStateID() << std::endl;
        EV_INFO << "    Advertising router: " << head.getAdvertisingRouter() << std::endl;
        EV_INFO << "    Seq number: " << head.getLsSequenceNumber() << std::endl;
        EV_INFO << "    Length: " << head.getLsaLength() << std::endl;

        EV_INFO << "    Number of attached routers: " << entry->getAttachedRoutersArraySize() << std::endl;
        for(unsigned int j = 0; j < entry->getAttachedRoutersArraySize(); j++)
            EV_INFO << "        Attached router: " << entry->getAttachedRouters(j) << std::endl;
        EV_INFO << std::endl;
    }
}

void Area::printSummaryLsa()
{
    for (uint32_t i = 0; i < summaryLSAs.size(); i++) {
        OspfSummaryLsa *entry = check_and_cast<OspfSummaryLsa *>(summaryLSAs[i]);

        const OspfLsaHeader& head = entry->getHeader();
        std::string routerId = head.getAdvertisingRouter().str(false);
        EV_INFO << "Summary LSA from OSPF router with ID " << routerId << std::endl;

        // print header info
        EV_INFO << "    LS age: " << head.getLsAge() << std::endl;
        EV_INFO << "    LS type: " << head.getLsType() << std::endl;
        EV_INFO << "    Link state ID (IP network): " << head.getLinkStateID().str(false) << std::endl;
        EV_INFO << "    Advertising router: " << head.getAdvertisingRouter() << std::endl;
        EV_INFO << "    Seq number: " << head.getLsSequenceNumber() << std::endl;
        EV_INFO << "    Length: " << head.getLsaLength() << std::endl;

        EV_INFO << "    Network Mask: " << entry->getNetworkMask().str(false) << std::endl;
        EV_INFO << "    Metric: " << entry->getRouteCost() << std::endl;
        EV_INFO << std::endl;
    }
}

bool Area::isDefaultRoute(OspfRoutingTableEntry *entry) const
{
    if(entry->getDestination().getInt() == 0 && entry->getNetmask().getInt() == 0)
        return true;
    return false;
}

bool Area::isAllZero(Ipv4AddressRange entry) const
{
    if(entry.address.getInt() == 0 && entry.mask.getInt() == 0)
        return true;
    return false;
}

} // namespace ospf

} // namespace inet