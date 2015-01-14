//
// Copyright (C) 2004 Andras Varga
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
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

//  Author: Andras Varga, 2004

#include <algorithm>
#include <sstream>

#include "inet/networklayer/ipv4/IPv4InterfaceData.h"
#include "inet/networklayer/contract/IInterfaceTable.h"

namespace inet {

Register_Abstract_Class(IPv4MulticastGroupInfo);
Register_Abstract_Class(IPv4MulticastGroupSourceInfo);

const IPv4MulticastSourceList IPv4MulticastSourceList::ALL_SOURCES(MCAST_EXCLUDE_SOURCES, IPv4AddressVector());

bool IPv4MulticastSourceList::contains(IPv4Address source)
{
    return (filterMode == MCAST_INCLUDE_SOURCES && find(sources.begin(), sources.end(), source) != sources.end()) ||
           (filterMode == MCAST_EXCLUDE_SOURCES && find(sources.begin(), sources.end(), source) == sources.end());
}

bool IPv4MulticastSourceList::add(IPv4Address source)
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

bool IPv4MulticastSourceList::remove(IPv4Address source)
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

std::string IPv4MulticastSourceList::info() const
{
    std::stringstream out;
    out << (filterMode == MCAST_INCLUDE_SOURCES ? "I" : "E");
    for (auto & elem : sources)
        out << " " << elem;
    return out.str();
}

std::string IPv4MulticastSourceList::detailedInfo() const
{
    std::stringstream out;
    out << (filterMode == MCAST_INCLUDE_SOURCES ? "INCLUDE(" : "EXCLUDE(");
    for (int i = 0; i < (int)sources.size(); ++i)
        out << (i > 0 ? ", " : "") << sources[i];
    out << ")";
    return out.str();
}

IPv4InterfaceData::HostMulticastData::~HostMulticastData()
{
    for (auto & elem : joinedMulticastGroups)
        delete (elem);
    joinedMulticastGroups.clear();
}

std::string IPv4InterfaceData::HostMulticastData::info()
{
    std::stringstream out;
    if (!joinedMulticastGroups.empty()) {
        out << " mcastgrps:";
        for (int i = 0; i < (int)joinedMulticastGroups.size(); ++i) {
            out << (i > 0 ? "," : "") << joinedMulticastGroups[i]->multicastGroup;
            if (!joinedMulticastGroups[i]->sourceList.containsAll())
                out << " " << joinedMulticastGroups[i]->sourceList.info();
        }
    }
    return out.str();
}

std::string IPv4InterfaceData::HostMulticastData::detailedInfo()
{
    std::stringstream out;
    out << "Joined Groups:";
    for (auto & elem : joinedMulticastGroups) {
        out << " " << elem->multicastGroup    // << "(" << refCounts[i] << ")";
            << " " << elem->sourceList.detailedInfo();
    }
    out << "\n";
    return out.str();
}

IPv4InterfaceData::RouterMulticastData::~RouterMulticastData()
{
    for (auto & elem : reportedMulticastGroups)
        delete elem;
    reportedMulticastGroups.clear();
}

std::string IPv4InterfaceData::RouterMulticastData::info()
{
    std::stringstream out;
    if (reportedMulticastGroups.size() > 0) {
        out << " mcast_listeners:";
        for (int i = 0; i < (int)reportedMulticastGroups.size(); ++i) {
            out << (i > 0 ? "," : "") << reportedMulticastGroups[i]->multicastGroup;
            if (!reportedMulticastGroups[i]->sourceList.containsAll())
                out << " " << reportedMulticastGroups[i]->sourceList.info();
        }
    }
    if (multicastTtlThreshold > 0)
        out << " ttl_threshold: " << multicastTtlThreshold;
    return out.str();
}

std::string IPv4InterfaceData::RouterMulticastData::detailedInfo()
{
    std::stringstream out;
    out << "TTL Threshold: " << multicastTtlThreshold << "\n";
    out << "Multicast Listeners:";
    for (auto & elem : reportedMulticastGroups) {
        out << " " << elem->multicastGroup
            << " " << elem->sourceList.detailedInfo();
    }
    out << "\n";
    return out.str();
}

IPv4InterfaceData::IPv4InterfaceData()
{
    netmask = IPv4Address::ALLONES_ADDRESS;
    metric = 0;
    hostData = nullptr;
    routerData = nullptr;
}

IPv4InterfaceData::~IPv4InterfaceData()
{
    delete hostData;
    delete routerData;
}

std::string IPv4InterfaceData::info() const
{
    std::stringstream out;
    out << "IPv4:{inet_addr:" << getIPAddress() << "/" << getNetmask().getNetmaskLength();
    if (hostData)
        out << hostData->info();
    if (routerData)
        out << routerData->info();
    out << "}";
    return out.str();
}

std::string IPv4InterfaceData::detailedInfo() const
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

bool IPv4InterfaceData::isMemberOfMulticastGroup(const IPv4Address& multicastAddress) const
{
    HostMulticastGroupVector groups = getHostData()->joinedMulticastGroups;
    for (HostMulticastGroupVector::const_iterator it = groups.begin(); it != groups.end(); ++it)
        if ((*it)->multicastGroup == multicastAddress)
            return true;

    return false;
}

// XXX deprecated
void IPv4InterfaceData::joinMulticastGroup(const IPv4Address& multicastAddress)
{
    IPv4AddressVector empty;
    changeMulticastGroupMembership(multicastAddress, MCAST_INCLUDE_SOURCES, empty, MCAST_EXCLUDE_SOURCES, empty);
}

// XXX deprecated
void IPv4InterfaceData::leaveMulticastGroup(const IPv4Address& multicastAddress)
{
    IPv4AddressVector empty;
    changeMulticastGroupMembership(multicastAddress, MCAST_EXCLUDE_SOURCES, empty, MCAST_INCLUDE_SOURCES, empty);
}

/**
 * This method is called by sockets to register their multicast group membership changes in the interface.
 */
void IPv4InterfaceData::changeMulticastGroupMembership(IPv4Address multicastAddress,
        McastSourceFilterMode oldFilterMode, const IPv4AddressVector& oldSourceList,
        McastSourceFilterMode newFilterMode, const IPv4AddressVector& newSourceList)
{
    if (!multicastAddress.isMulticast())
        throw cRuntimeError("IPv4InterfaceData::changeMulticastGroupMembership(): multicast address expected, received %s.", multicastAddress.str().c_str());

    HostMulticastGroupData *entry = findHostGroupData(multicastAddress);
    if (!entry) {
        ASSERT(oldFilterMode == MCAST_INCLUDE_SOURCES && oldSourceList.empty());
        HostMulticastData *data = getHostData();
        data->joinedMulticastGroups.push_back(new HostMulticastGroupData(multicastAddress));
        entry = data->joinedMulticastGroups.back();
    }

    std::map<IPv4Address, int> *counts = oldFilterMode == MCAST_INCLUDE_SOURCES ? &entry->includeCounts : &entry->excludeCounts;
    for (const auto & elem : oldSourceList) {
        auto count = counts->find(elem);
        if (count == counts->end())
            throw cRuntimeError("Inconsistent reference counts in IPv4InterfaceData.");
        else if (count->second == 1)
            counts->erase(count);
        else
            count->second--;
    }

    counts = newFilterMode == MCAST_INCLUDE_SOURCES ? &entry->includeCounts : &entry->excludeCounts;
    for (const auto & elem : newSourceList) {
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
            IPv4MulticastGroupSourceInfo info(ownerp, multicastAddress, entry->sourceList);
            m->emit(NF_IPv4_MCAST_CHANGE, &info);
        }

        // Legacy notifications
        if (oldFilterMode != newFilterMode && oldSourceList.empty() && newSourceList.empty()) {
            cModule *m = ownerp ? dynamic_cast<cModule *>(ownerp->getInterfaceTable()) : nullptr;
            if (m) {
                IPv4MulticastGroupInfo info2(ownerp, multicastAddress);
                m->emit(newFilterMode == MCAST_EXCLUDE_SOURCES ? NF_IPv4_MCAST_JOIN : NF_IPv4_MCAST_LEAVE, &info2);
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
bool IPv4InterfaceData::HostMulticastGroupData::updateSourceList()
{
    // Filter mode is EXCLUDE if any of the sockets are in EXCLUDE mode, otherwise INCLUDE
    McastSourceFilterMode filterMode = numOfExcludeModeSockets == 0 ? MCAST_INCLUDE_SOURCES : MCAST_EXCLUDE_SOURCES;

    IPv4AddressVector sourceList;
    if (numOfExcludeModeSockets == 0) {
        // If all socket is in INCLUDE mode, then the sourceList is the union of included sources
        for (auto & elem : includeCounts)
            sourceList.push_back(elem.first);
    }
    else {
        // If some socket is in EXCLUDE mode, then the sourceList contains the sources that are
        // excluded by all EXCLUDE mode sockets except if there is a socket including the source.
        for (auto & elem : excludeCounts)
            if (elem.second == numOfExcludeModeSockets && includeCounts.find(elem.first) == includeCounts.end())
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

IPv4InterfaceData::RouterMulticastGroupData *IPv4InterfaceData::findRouterGroupData(IPv4Address multicastAddress) const
{
    ASSERT(multicastAddress.isMulticast());
    const RouterMulticastGroupVector& entries = getRouterData()->reportedMulticastGroups;
    for (const auto & entrie : entries)
        if ((entrie)->multicastGroup == multicastAddress)
            return entrie;

    return nullptr;
}

bool IPv4InterfaceData::removeRouterGroupData(IPv4Address multicastAddress)
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

bool IPv4InterfaceData::hasMulticastListener(IPv4Address multicastAddress) const
{
    RouterMulticastGroupData *groupData = findRouterGroupData(multicastAddress);
    return groupData && !groupData->sourceList.isEmpty();
}

bool IPv4InterfaceData::hasMulticastListener(IPv4Address multicastAddress, IPv4Address sourceAddress) const
{
    RouterMulticastGroupData *groupData = findRouterGroupData(multicastAddress);
    return groupData && !groupData->sourceList.contains(sourceAddress);
}

void IPv4InterfaceData::addMulticastListener(const IPv4Address& multicastAddress)
{
    if (!multicastAddress.isMulticast())
        throw cRuntimeError("IPv4InterfaceData::addMulticastListener(): multicast address expected, received %s.", multicastAddress.str().c_str());

    RouterMulticastGroupData *groupData = findRouterGroupData(multicastAddress);
    if (!groupData) {
        groupData = new RouterMulticastGroupData(multicastAddress);
        getRouterData()->reportedMulticastGroups.push_back(groupData);

        IPv4MulticastGroupInfo info(getInterfaceEntry(), multicastAddress);
        check_and_cast<cModule *>(getInterfaceEntry()->getInterfaceTable())->emit(NF_IPv4_MCAST_REGISTERED, &info);
    }

    if (!groupData->sourceList.containsAll()) {
        groupData->sourceList = IPv4MulticastSourceList::ALL_SOURCES;
        changed1(F_MULTICAST_LISTENERS);
    }
}

void IPv4InterfaceData::addMulticastListener(IPv4Address multicastAddress, IPv4Address sourceAddress)
{
    if (!multicastAddress.isMulticast())
        throw cRuntimeError("IPv4InterfaceData::addMulticastListener(): multicast address expected, received %s.", multicastAddress.str().c_str());

    RouterMulticastGroupData *groupData = findRouterGroupData(multicastAddress);
    if (!groupData) {
        groupData = new RouterMulticastGroupData(multicastAddress);
        getRouterData()->reportedMulticastGroups.push_back(groupData);

        IPv4MulticastGroupInfo info(getInterfaceEntry(), multicastAddress);
        check_and_cast<cModule *>(getInterfaceEntry()->getInterfaceTable())->emit(NF_IPv4_MCAST_REGISTERED, &info);
    }

    if (!groupData->sourceList.contains(sourceAddress)) {
        groupData->sourceList.add(sourceAddress);
        changed1(F_MULTICAST_LISTENERS);
    }
}

void IPv4InterfaceData::removeMulticastListener(const IPv4Address& multicastAddress)
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

        IPv4MulticastGroupInfo info(getInterfaceEntry(), multicastAddress);
        check_and_cast<cModule *>(getInterfaceEntry()->getInterfaceTable())->emit(NF_IPv4_MCAST_UNREGISTERED, &info);
    }
}

void IPv4InterfaceData::removeMulticastListener(IPv4Address multicastAddress, IPv4Address sourceAddress)
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

            IPv4MulticastGroupInfo info(getInterfaceEntry(), multicastAddress);
            check_and_cast<cModule *>(getInterfaceEntry()->getInterfaceTable())->emit(NF_IPv4_MCAST_UNREGISTERED, &info);
        }
        changed1(F_MULTICAST_LISTENERS);
    }
}

void IPv4InterfaceData::setMulticastListeners(IPv4Address multicastAddress, McastSourceFilterMode filterMode, const IPv4AddressVector& sourceList)
{
    if (!multicastAddress.isMulticast())
        throw cRuntimeError("IPv4InterfaceData::setMulticastListeners(): multicast address expected, received %s.", multicastAddress.str().c_str());

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

IPv4InterfaceData::HostMulticastGroupData *IPv4InterfaceData::findHostGroupData(IPv4Address multicastAddress)
{
    ASSERT(multicastAddress.isMulticast());
    HostMulticastGroupVector& entries = getHostData()->joinedMulticastGroups;
    for (auto & entrie : entries)
        if ((entrie)->multicastGroup == multicastAddress)
            return entrie;

    return nullptr;
}

bool IPv4InterfaceData::removeHostGroupData(IPv4Address multicastAddress)
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

