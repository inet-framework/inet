//
// Copyright (C) 2004 OpenSim Ltd.
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"

#include <algorithm>
#include <sstream>

#include "inet/common/stlutils.h"
#include "inet/networklayer/contract/IInterfaceTable.h"

namespace inet {

Register_Abstract_Class(Ipv4MulticastGroupInfo);
Register_Abstract_Class(Ipv4MulticastGroupSourceInfo);

const Ipv4MulticastSourceList Ipv4MulticastSourceList::ALL_SOURCES(MCAST_EXCLUDE_SOURCES, Ipv4AddressVector());

bool Ipv4MulticastSourceList::contains(Ipv4Address source)
{
    return (filterMode == MCAST_INCLUDE_SOURCES && inet::contains(sources, source)) ||
           (filterMode == MCAST_EXCLUDE_SOURCES && !inet::contains(sources, source));
}

bool Ipv4MulticastSourceList::add(Ipv4Address source)
{
    size_t oldSize = sources.size();
    if (filterMode == MCAST_INCLUDE_SOURCES) {
        auto it = std::lower_bound(sources.begin(), sources.end(), source);
        if (it == sources.end() || *it != source)
            sources.insert(it, source);
    }
    else {
        sources.erase(std::remove(sources.begin(), sources.end(), source), sources.end());
    }
    return sources.size() != oldSize;
}

bool Ipv4MulticastSourceList::remove(Ipv4Address source)
{
    size_t oldSize = sources.size();
    if (filterMode == MCAST_INCLUDE_SOURCES) {
        sources.erase(std::remove(sources.begin(), sources.end(), source), sources.end());
    }
    else {
        auto it = lower_bound(sources.begin(), sources.end(), source);
        if (it == sources.end() || *it != source)
            sources.insert(it, source);
    }
    return sources.size() != oldSize;
}

std::string Ipv4MulticastSourceList::str() const
{
    std::stringstream out;
    out << (filterMode == MCAST_INCLUDE_SOURCES ? "I" : "E");
    for (auto& elem : sources)
        out << " " << elem;
    return out.str();
}

std::string Ipv4MulticastSourceList::detailedInfo() const
{
    std::stringstream out;
    out << (filterMode == MCAST_INCLUDE_SOURCES ? "INCLUDE(" : "EXCLUDE(");
    for (size_t i = 0; i < sources.size(); ++i)
        out << (i > 0 ? ", " : "") << sources[i];
    out << ")";
    return out.str();
}

Ipv4InterfaceData::HostMulticastData::~HostMulticastData()
{
    for (auto& elem : joinedMulticastGroups)
        delete elem;
    joinedMulticastGroups.clear();
}

std::string Ipv4InterfaceData::HostMulticastData::str()
{
    std::stringstream out;
    if (!joinedMulticastGroups.empty()) {
        out << " mcastgrps:";
        for (size_t i = 0; i < joinedMulticastGroups.size(); ++i) {
            out << (i > 0 ? "," : "") << joinedMulticastGroups[i]->multicastGroup;
            if (!joinedMulticastGroups[i]->sourceList.containsAll())
                out << " " << joinedMulticastGroups[i]->sourceList.str();
        }
    }
    return out.str();
}

std::string Ipv4InterfaceData::HostMulticastData::detailedInfo()
{
    std::stringstream out;
    out << "Joined Groups:";
    for (auto& elem : joinedMulticastGroups) {
        out << " " << elem->multicastGroup // << "(" << refCounts[i] << ")";
            << " " << elem->sourceList.detailedInfo();
    }
    out << "\n";
    return out.str();
}

Ipv4InterfaceData::RouterMulticastData::~RouterMulticastData()
{
    for (auto& elem : reportedMulticastGroups)
        delete elem;
    reportedMulticastGroups.clear();
}

std::string Ipv4InterfaceData::RouterMulticastData::str()
{
    std::stringstream out;
    if (reportedMulticastGroups.size() > 0) {
        out << " mcast_listeners:";
        for (size_t i = 0; i < reportedMulticastGroups.size(); ++i) {
            out << (i > 0 ? "," : "") << reportedMulticastGroups[i]->multicastGroup;
            if (!reportedMulticastGroups[i]->sourceList.containsAll())
                out << " " << reportedMulticastGroups[i]->sourceList.str();
        }
    }
    if (multicastTtlThreshold > 0)
        out << " ttl_threshold: " << multicastTtlThreshold;
    return out.str();
}

std::string Ipv4InterfaceData::RouterMulticastData::detailedInfo()
{
    std::stringstream out;
    out << "TTL Threshold: " << multicastTtlThreshold << "\n";
    out << "Multicast Listeners:";
    for (auto& elem : reportedMulticastGroups) {
        out << " " << elem->multicastGroup
            << " " << elem->sourceList.detailedInfo();
    }
    out << "\n";
    return out.str();
}

Ipv4InterfaceData::Ipv4InterfaceData()
    : InterfaceProtocolData(NetworkInterface::F_IPV4_DATA)
{
    netmask = Ipv4Address::ALLONES_ADDRESS;
    metric = 0;
    hostData = nullptr;
    routerData = nullptr;
}

Ipv4InterfaceData::~Ipv4InterfaceData()
{
    delete hostData;
    delete routerData;
}

std::string Ipv4InterfaceData::str() const
{
    std::stringstream out;
    out << "Ipv4:{inet_addr:" << getIPAddress() << "/" << getNetmask().getNetmaskLength();
    if (hostData)
        out << hostData->str();
    if (routerData)
        out << routerData->str();
    out << "}";
    return out.str();
}

std::string Ipv4InterfaceData::detailedInfo() const
{
    std::stringstream out;
    out << "inet addr:" << getIPAddress() << "\tMask: " << getNetmask() << "\n";
    out << "Metric: " << getMetric() << "\n";
    if (hostData)
        out << hostData->detailedInfo();
    if (routerData)
        out << routerData->detailedInfo();
    return out.str();
}

bool Ipv4InterfaceData::isMemberOfMulticastGroup(const Ipv4Address& multicastAddress) const
{
    HostMulticastGroupVector groups = getHostData()->joinedMulticastGroups;
    for (HostMulticastGroupVector::const_iterator it = groups.begin(); it != groups.end(); ++it)
        if ((*it)->multicastGroup == multicastAddress)
            return true;

    return false;
}

// TODO deprecated
void Ipv4InterfaceData::joinMulticastGroup(const Ipv4Address& multicastAddress)
{
    Ipv4AddressVector empty;
    changeMulticastGroupMembership(multicastAddress, MCAST_INCLUDE_SOURCES, empty, MCAST_EXCLUDE_SOURCES, empty);
}

// TODO deprecated
void Ipv4InterfaceData::leaveMulticastGroup(const Ipv4Address& multicastAddress)
{
    Ipv4AddressVector empty;
    changeMulticastGroupMembership(multicastAddress, MCAST_EXCLUDE_SOURCES, empty, MCAST_INCLUDE_SOURCES, empty);
}

/**
 * This method is called by sockets to register their multicast group membership changes in the interface.
 */
void Ipv4InterfaceData::changeMulticastGroupMembership(Ipv4Address multicastAddress,
        McastSourceFilterMode oldFilterMode, const Ipv4AddressVector& oldSourceList,
        McastSourceFilterMode newFilterMode, const Ipv4AddressVector& newSourceList)
{
    if (!ownerp->isMulticast())
        throw cRuntimeError("Ipv4InterfaceData::changeMulticastGroupMembership(): multicast interface expected, received %s.", ownerp->getInterfaceFullPath().c_str());
    if (!multicastAddress.isMulticast())
        throw cRuntimeError("Ipv4InterfaceData::changeMulticastGroupMembership(): multicast address expected, received %s.", multicastAddress.str().c_str());

    HostMulticastGroupData *entry = findHostGroupData(multicastAddress);
    if (!entry) {
        ASSERT(oldFilterMode == MCAST_INCLUDE_SOURCES && oldSourceList.empty());
        HostMulticastData *data = getHostData();
        data->joinedMulticastGroups.push_back(new HostMulticastGroupData(multicastAddress));
        entry = data->joinedMulticastGroups.back();
    }

    std::map<Ipv4Address, int> *counts = oldFilterMode == MCAST_INCLUDE_SOURCES ? &entry->includeCounts : &entry->excludeCounts;
    for (const auto& elem : oldSourceList) {
        auto count = counts->find(elem);
        if (count == counts->end())
            throw cRuntimeError("Inconsistent reference counts in Ipv4InterfaceData.");
        else if (count->second == 1)
            counts->erase(count);
        else
            count->second--;
    }

    counts = newFilterMode == MCAST_INCLUDE_SOURCES ? &entry->includeCounts : &entry->excludeCounts;
    for (const auto& elem : newSourceList) {
        auto count = counts->find(elem);
        if (count == counts->end())
            (*counts)[elem] = 1;
        else
            count->second++;
    }

    // update number of EXCLUDE mode sockets
    if (oldFilterMode == MCAST_INCLUDE_SOURCES && newFilterMode == MCAST_EXCLUDE_SOURCES)
        entry->numOfExcludeModeSockets++;
    else if (oldFilterMode == MCAST_EXCLUDE_SOURCES && newFilterMode == MCAST_INCLUDE_SOURCES)
        entry->numOfExcludeModeSockets--;

    // compute filterMode and sourceList
    bool changed = entry->updateSourceList();

    if (changed) {
        changed1(F_MULTICAST_ADDRESSES);

        cModule *m = ownerp ? dynamic_cast<cModule *>(ownerp->getInterfaceTable()) : nullptr;
        if (m) {
            Ipv4MulticastGroupSourceInfo info(ownerp, multicastAddress, entry->sourceList);
            m->emit(ipv4MulticastChangeSignal, &info);
        }

        // Legacy notifications
        if (oldFilterMode != newFilterMode && oldSourceList.empty() && newSourceList.empty()) {
            cModule *m = ownerp ? dynamic_cast<cModule *>(ownerp->getInterfaceTable()) : nullptr;
            if (m) {
                Ipv4MulticastGroupInfo info2(ownerp, multicastAddress);
                m->emit(newFilterMode == MCAST_EXCLUDE_SOURCES ? ipv4MulticastGroupJoinedSignal : ipv4MulticastGroupLeftSignal, &info2);
            }
        }

        // remove group data if it is INCLUDE(empty)
        if (entry->sourceList.isEmpty()) {
            removeHostGroupData(multicastAddress);
        }
    }
}

/**
 * Computes the filterMode and sourceList of the interface from the socket reference counts
 * according to RFC3376 3.2.
 * Returns true if filterMode or sourceList has been changed.
 */
bool Ipv4InterfaceData::HostMulticastGroupData::updateSourceList()
{
    // Filter mode is EXCLUDE if any of the sockets are in EXCLUDE mode, otherwise INCLUDE
    McastSourceFilterMode filterMode = numOfExcludeModeSockets == 0 ? MCAST_INCLUDE_SOURCES : MCAST_EXCLUDE_SOURCES;

    Ipv4AddressVector sourceList;
    if (numOfExcludeModeSockets == 0) {
        // If all socket is in INCLUDE mode, then the sourceList is the union of included sources
        for (auto& elem : includeCounts)
            sourceList.push_back(elem.first);
    }
    else {
        // If some socket is in EXCLUDE mode, then the sourceList contains the sources that are
        // excluded by all EXCLUDE mode sockets except if there is a socket including the source.
        for (auto& elem : excludeCounts)
            if (elem.second == numOfExcludeModeSockets && !containsKey(includeCounts, elem.first))
                sourceList.push_back(elem.first);
    }

    if (this->sourceList.filterMode != filterMode || this->sourceList.sources != sourceList) {
        this->sourceList.filterMode = filterMode;
        this->sourceList.sources = sourceList;
        return true;
    }
    else
        return false;
}

Ipv4InterfaceData::RouterMulticastGroupData *Ipv4InterfaceData::findRouterGroupData(Ipv4Address multicastAddress) const
{
    ASSERT(multicastAddress.isMulticast());
    const RouterMulticastGroupVector& entries = getRouterData()->reportedMulticastGroups;
    for (const auto& entrie : entries)
        if ((entrie)->multicastGroup == multicastAddress)
            return entrie;

    return nullptr;
}

bool Ipv4InterfaceData::removeRouterGroupData(Ipv4Address multicastAddress)
{
    ASSERT(multicastAddress.isMulticast());
    RouterMulticastGroupVector& entries = getRouterData()->reportedMulticastGroups;
    for (auto it = entries.begin(); it != entries.end(); ++it)
        if ((*it)->multicastGroup == multicastAddress) {
            delete *it;
            entries.erase(it);
            return true;
        }
    return false;
}

bool Ipv4InterfaceData::hasMulticastListener(Ipv4Address multicastAddress) const
{
    RouterMulticastGroupData *groupData = findRouterGroupData(multicastAddress);
    return groupData && !groupData->sourceList.isEmpty();
}

bool Ipv4InterfaceData::hasMulticastListener(Ipv4Address multicastAddress, Ipv4Address sourceAddress) const
{
    RouterMulticastGroupData *groupData = findRouterGroupData(multicastAddress);
    return groupData && !groupData->sourceList.contains(sourceAddress);
}

void Ipv4InterfaceData::addMulticastListener(const Ipv4Address& multicastAddress)
{
    if (!multicastAddress.isMulticast())
        throw cRuntimeError("Ipv4InterfaceData::addMulticastListener(): multicast address expected, received %s.", multicastAddress.str().c_str());

    RouterMulticastGroupData *groupData = findRouterGroupData(multicastAddress);
    if (!groupData) {
        groupData = new RouterMulticastGroupData(multicastAddress);
        getRouterData()->reportedMulticastGroups.push_back(groupData);

        Ipv4MulticastGroupInfo info(getNetworkInterface(), multicastAddress);
        check_and_cast<cModule *>(getNetworkInterface()->getInterfaceTable())->emit(ipv4MulticastGroupRegisteredSignal, &info);
    }

    if (!groupData->sourceList.containsAll()) {
        groupData->sourceList = Ipv4MulticastSourceList::ALL_SOURCES;
        changed1(F_MULTICAST_LISTENERS);
    }
}

void Ipv4InterfaceData::addMulticastListener(Ipv4Address multicastAddress, Ipv4Address sourceAddress)
{
    if (!multicastAddress.isMulticast())
        throw cRuntimeError("Ipv4InterfaceData::addMulticastListener(): multicast address expected, received %s.", multicastAddress.str().c_str());

    RouterMulticastGroupData *groupData = findRouterGroupData(multicastAddress);
    if (!groupData) {
        groupData = new RouterMulticastGroupData(multicastAddress);
        getRouterData()->reportedMulticastGroups.push_back(groupData);

        Ipv4MulticastGroupInfo info(getNetworkInterface(), multicastAddress);
        check_and_cast<cModule *>(getNetworkInterface()->getInterfaceTable())->emit(ipv4MulticastGroupRegisteredSignal, &info);
    }

    if (!groupData->sourceList.contains(sourceAddress)) {
        groupData->sourceList.add(sourceAddress);
        changed1(F_MULTICAST_LISTENERS);
    }
}

void Ipv4InterfaceData::removeMulticastListener(const Ipv4Address& multicastAddress)
{
    RouterMulticastGroupVector& multicastGroups = getRouterData()->reportedMulticastGroups;

    int n = multicastGroups.size();
    int i;
    for (i = 0; i < n; i++)
        if (multicastGroups[i]->multicastGroup == multicastAddress)
            break;

    if (i != n) {
        delete multicastGroups[i];
        multicastGroups.erase(multicastGroups.begin() + i);
        changed1(F_MULTICAST_LISTENERS);

        Ipv4MulticastGroupInfo info(getNetworkInterface(), multicastAddress);
        check_and_cast<cModule *>(getNetworkInterface()->getInterfaceTable())->emit(ipv4MulticastGroupUnregisteredSignal, &info);
    }
}

void Ipv4InterfaceData::removeMulticastListener(Ipv4Address multicastAddress, Ipv4Address sourceAddress)
{
    RouterMulticastGroupVector& multicastGroups = getRouterData()->reportedMulticastGroups;

    int n = multicastGroups.size();
    int i;
    for (i = 0; i < n; i++)
        if (multicastGroups[i]->multicastGroup == multicastAddress)
            break;

    if (i != n) {
        multicastGroups[i]->sourceList.remove(sourceAddress);
        if (multicastGroups[i]->sourceList.isEmpty()) {
            delete multicastGroups[i];
            multicastGroups.erase(multicastGroups.begin() + i);

            Ipv4MulticastGroupInfo info(getNetworkInterface(), multicastAddress);
            check_and_cast<cModule *>(getNetworkInterface()->getInterfaceTable())->emit(ipv4MulticastGroupUnregisteredSignal, &info);
        }
        changed1(F_MULTICAST_LISTENERS);
    }
}

void Ipv4InterfaceData::setMulticastListeners(Ipv4Address multicastAddress, McastSourceFilterMode filterMode, const Ipv4AddressVector& sourceList)
{
    if (!multicastAddress.isMulticast())
        throw cRuntimeError("Ipv4InterfaceData::setMulticastListeners(): multicast address expected, received %s.", multicastAddress.str().c_str());

    RouterMulticastGroupData *groupData = findRouterGroupData(multicastAddress);
    if (!groupData) {
        groupData = new RouterMulticastGroupData(multicastAddress);
        getRouterData()->reportedMulticastGroups.push_back(groupData);
    }

    if (filterMode != groupData->sourceList.filterMode || sourceList != groupData->sourceList.sources) {
        if (filterMode != MCAST_INCLUDE_SOURCES || !sourceList.empty()) {
            groupData->sourceList.filterMode = filterMode;
            groupData->sourceList.sources = sourceList;
        }
        else
            removeRouterGroupData(multicastAddress);

        changed1(F_MULTICAST_LISTENERS);
    }
}

Ipv4InterfaceData::HostMulticastGroupData *Ipv4InterfaceData::findHostGroupData(Ipv4Address multicastAddress)
{
    ASSERT(multicastAddress.isMulticast());
    HostMulticastGroupVector& entries = getHostData()->joinedMulticastGroups;
    for (auto& entrie : entries)
        if ((entrie)->multicastGroup == multicastAddress)
            return entrie;

    return nullptr;
}

bool Ipv4InterfaceData::removeHostGroupData(Ipv4Address multicastAddress)
{
    ASSERT(multicastAddress.isMulticast());
    HostMulticastGroupVector& entries = getHostData()->joinedMulticastGroups;
    for (auto it = entries.begin(); it != entries.end(); ++it)
        if ((*it)->multicastGroup == multicastAddress) {
            delete *it;
            entries.erase(it);
            return true;
        }
    return false;
}

} // namespace inet

