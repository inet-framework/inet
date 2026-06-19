//
// Copyright (C) 2005 OpenSim Ltd.
// Copyright (C) 2005 Wei Yang, Ng
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/networklayer/ipv6/Ipv6InterfaceData.h"

#include <algorithm>
#include <sstream>

#include "inet/common/stlutils.h"
#include "inet/networklayer/contract/IInterfaceTable.h"

namespace inet {

Register_Abstract_Class(Ipv6MulticastGroupInfo);
Register_Abstract_Class(Ipv6MulticastGroupSourceInfo);
Register_Abstract_Class(Ipv6AddressInfo);

// FIXME invoked changed() from state-changing methods, to trigger notification...

const Ipv6MulticastSourceList Ipv6MulticastSourceList::ALL_SOURCES(MCAST_EXCLUDE_SOURCES, Ipv6MulticastSourceList::Ipv6AddressVector());

bool Ipv6MulticastSourceList::contains(Ipv6Address source)
{
    return (filterMode == MCAST_INCLUDE_SOURCES && inet::contains(sources, source)) ||
           (filterMode == MCAST_EXCLUDE_SOURCES && !inet::contains(sources, source));
}

bool Ipv6MulticastSourceList::add(Ipv6Address source)
{
    size_t oldSize = sources.size();
    if (filterMode == MCAST_INCLUDE_SOURCES) {
        auto it = std::lower_bound(sources.begin(), sources.end(), source);
        if (it == sources.end() || *it != source)
            sources.insert(it, source);
    }
    else
        sources.erase(std::remove(sources.begin(), sources.end(), source), sources.end());
    return sources.size() != oldSize;
}

bool Ipv6MulticastSourceList::remove(Ipv6Address source)
{
    size_t oldSize = sources.size();
    if (filterMode == MCAST_INCLUDE_SOURCES)
        sources.erase(std::remove(sources.begin(), sources.end(), source), sources.end());
    else {
        auto it = std::lower_bound(sources.begin(), sources.end(), source);
        if (it == sources.end() || *it != source)
            sources.insert(it, source);
    }
    return sources.size() != oldSize;
}

std::string Ipv6MulticastSourceList::str() const
{
    std::stringstream out;
    out << (filterMode == MCAST_INCLUDE_SOURCES ? "I" : "E");
    for (auto& elem : sources)
        out << " " << elem;
    return out.str();
}

std::string Ipv6MulticastSourceList::detailedInfo() const
{
    std::stringstream out;
    out << "Source list: " << str();
    return out.str();
}

bool Ipv6InterfaceData::HostMulticastGroupData::updateSourceList()
{
    // Filter mode is EXCLUDE if any of the sockets are in EXCLUDE mode, otherwise INCLUDE
    McastSourceFilterMode filterMode = numOfExcludeModeSockets == 0 ? MCAST_INCLUDE_SOURCES : MCAST_EXCLUDE_SOURCES;

    Ipv6MulticastSourceList::Ipv6AddressVector sourceList;
    if (numOfExcludeModeSockets == 0) {
        // all sockets INCLUDE: the source list is the union of the included sources
        for (auto& elem : includeCounts)
            sourceList.push_back(elem.first);
    }
    else {
        // some socket EXCLUDE: the source list contains the sources excluded by every
        // EXCLUDE-mode socket, unless some socket includes them
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

Ipv6InterfaceData::HostMulticastData::~HostMulticastData()
{
    for (auto& elem : joinedMulticastGroups)
        delete elem;
    joinedMulticastGroups.clear();
}

std::string Ipv6InterfaceData::HostMulticastData::str()
{
    std::stringstream out;
    if (!joinedMulticastGroups.empty()) {
        out << "\tmcastgrps:";
        bool addComma = false;
        for (auto& group : joinedMulticastGroups) {
            if (group->multicastGroup != Ipv6Address::ALL_NODES_1) {
                out << (addComma ? "," : "") << group->multicastGroup;
                if (!group->sourceList.containsAll())
                    out << " " << group->sourceList.str();
                addComma = true;
            }
        }
    }
    return out.str();
}

std::string Ipv6InterfaceData::HostMulticastData::detailedInfo()
{
    std::stringstream out;
    out << "Joined Groups:";
    for (auto& group : joinedMulticastGroups) {
        out << " " << group->multicastGroup;
        if (!group->sourceList.containsAll())
            out << "(" << group->sourceList.str() << ")";
    }
    out << "\n";
    return out.str();
}

Ipv6InterfaceData::RouterMulticastData::~RouterMulticastData()
{
    for (auto& elem : reportedMulticastGroups)
        delete elem;
    reportedMulticastGroups.clear();
}

std::string Ipv6InterfaceData::RouterMulticastData::str()
{
    std::stringstream out;
    if (reportedMulticastGroups.size() > 0) {
        out << "\tmcast_listeners:";
        for (size_t i = 0; i < reportedMulticastGroups.size(); ++i) {
            out << (i > 0 ? "," : "") << reportedMulticastGroups[i]->multicastGroup;
            if (!reportedMulticastGroups[i]->sourceList.containsAll())
                out << " " << reportedMulticastGroups[i]->sourceList.str();
        }
    }
    return out.str();
}

std::string Ipv6InterfaceData::RouterMulticastData::detailedInfo()
{
    std::stringstream out;
    out << "Multicast Listeners:";
    for (auto& group : reportedMulticastGroups) {
        out << " " << group->multicastGroup;
        if (!group->sourceList.containsAll())
            out << "(" << group->sourceList.str() << ")";
    }
    out << "\n";
    return out.str();
}

Ipv6InterfaceData::Ipv6InterfaceData()
    : InterfaceProtocolData(NetworkInterface::F_IPV6_DATA)
{
    /*******************Setting host/node/router Protocol Constants************/
    routerConstants.maxInitialRtrAdvertInterval = IPv6_MAX_INITIAL_RTR_ADVERT_INTERVAL;
    routerConstants.maxInitialRtrAdvertisements = IPv6_MAX_INITIAL_RTR_ADVERTISEMENTS;
    routerConstants.maxFinalRtrAdvertisements = IPv6_MAX_FINAL_RTR_ADVERTISEMENTS;
    routerConstants.minDelayBetweenRAs = IPv6_MIN_DELAY_BETWEEN_RAS;
    routerConstants.maxRADelayTime = IPv6_MAX_RA_DELAY_TIME;

    hostConstants.maxRtrSolicitationDelay = IPv6_MAX_RTR_SOLICITATION_DELAY;
    hostConstants.rtrSolicitationInterval = IPv6_RTR_SOLICITATION_INTERVAL;
    hostConstants.maxRtrSolicitations = IPv6_MAX_RTR_SOLICITATIONS;

    nodeConstants.maxMulticastSolicit = IPv6_MAX_MULTICAST_SOLICIT;
    nodeConstants.maxUnicastSolicit = IPv6_MAX_UNICAST_SOLICIT;
    nodeConstants.maxAnycastDelayTime = IPv6_MAX_ANYCAST_DELAY_TIME;
    nodeConstants.maxNeighbourAdvertisement = IPv6_MAX_NEIGHBOUR_ADVERTISEMENT;
    nodeConstants.reachableTime = IPv6_REACHABLE_TIME;
    nodeConstants.retransTimer = IPv6_RETRANS_TIMER;
    nodeConstants.delayFirstProbeTime = IPv6_DELAY_FIRST_PROBE_TIME;
    nodeConstants.minRandomFactor = IPv6_MIN_RANDOM_FACTOR;
    nodeConstants.maxRandomFactor = IPv6_MAX_RANDOM_FACTOR;

    /*******************Setting host/node/router variables*********************/
    nodeVars.dupAddrDetectTransmits = IPv6_DEFAULT_DUPADDRDETECTTRANSMITS;

    hostVars.linkMTU = IPv6_MIN_MTU;
    hostVars.curHopLimit = IPv6_DEFAULT_ADVCURHOPLIMIT; // value specified in RFC 1700-can't find it
    hostVars.baseReachableTime = IPv6_REACHABLE_TIME;
    hostVars.reachableTime = generateReachableTime(_getMinRandomFactor(),
                _getMaxRandomFactor(), getBaseReachableTime());
    hostVars.retransTimer = IPv6_RETRANS_TIMER;

    // rtrVars.advSendAdvertisements is set in Ipv6RoutingTable.cc:line 143
    rtrVars.maxRtrAdvInterval = IPv6_DEFAULT_MAX_RTR_ADV_INT;
    rtrVars.minRtrAdvInterval = IPv6_DEFAULT_MIN_TO_MAX_RTR_ADV_RATIO * rtrVars.maxRtrAdvInterval;
    rtrVars.advManagedFlag = false;
    rtrVars.advOtherConfigFlag = false;

    rtrVars.advLinkMTU = IPv6_MIN_MTU;
    rtrVars.advReachableTime = IPv6_DEFAULT_ADV_REACHABLE_TIME;
    rtrVars.advRetransTimer = IPv6_DEFAULT_ADV_RETRANS_TIMER;
    rtrVars.advCurHopLimit = IPv6_DEFAULT_ADVCURHOPLIMIT;
    rtrVars.advDefaultLifetime = 3 * rtrVars.maxRtrAdvInterval;

    rtrVars.advHomeAgentFlag = false;

#if USE_MOBILITY
    if (rtrVars.advDefaultLifetime < 1)
        rtrVars.advDefaultLifetime = 1;
#endif // if USE_MOBILITY
}

Ipv6InterfaceData::~Ipv6InterfaceData()
{
    delete hostMcastData;
    delete routerMcastData;
}

std::string Ipv6InterfaceData::str() const
{
    // FIXME FIXME FIXME FIXME info() should never print a newline
    std::ostringstream os;
    os << "Ipv6:{" << endl;
    for (int i = 0; i < getNumAddresses(); i++) {
        os << (i ? "\t            , " : "\tAddrs:") << getAddress(i)
           << "(" << Ipv6Address::scopeName(getAddress(i).getScope())
           << (isTentativeAddress(i) ? " tent" : "") << ") "

           << " expiryTime: " << (addresses[i].expiryTime == SIMTIME_ZERO ? "inf" : SIMTIME_STR(addresses[i].expiryTime))
           << " prefExpiryTime: " << (addresses[i].prefExpiryTime == SIMTIME_ZERO ? "inf" : SIMTIME_STR(addresses[i].prefExpiryTime))
           << endl;
    }

    if (hostMcastData)
        os << hostMcastData->str();

    for (int i = 0; i < getNumAdvPrefixes(); i++) {
        const AdvPrefix& a = getAdvPrefix(i);
        os << (i ? ", " : "\tAdvPrefixes: ") << a.prefix << "/" << a.prefixLength << "("
           << (a.advOnLinkFlag ? "" : "off-link ")
           << (a.advAutonomousFlag ? "" : "non-auto ");

        os << "R-Flag = " << (a.advRtrAddr ? "1 " : "0 ");

        if (a.advValidLifetime == SIMTIME_ZERO)
            os << "lifetime:inf";
        else if (a.advValidLifetime > SIMTIME_ZERO)
            os << "expires:" << a.advValidLifetime;
        else
            os << "lifetime:+" << (-1 * a.advValidLifetime);
        os << ")" << endl;
    }
    os << " ";

    if (routerMcastData)
        os << routerMcastData->str();

    // uncomment the following as needed!
    os << "\tNode:";
    os << " dupAddrDetectTrans=" << nodeVars.dupAddrDetectTransmits;
//    os << " curHopLimit=" << hostVars.curHopLimit;
//    os << " retransTimer=" << hostVars.retransTimer;
//    os << " baseReachableTime=" << hostVars.baseReachableTime;
    os << " reachableTime=" << hostVars.reachableTime << endl;

    if (rtrVars.advSendAdvertisements) {
        os << "\tRouter:";
        os << " maxRtrAdvInt=" << rtrVars.maxRtrAdvInterval;
        os << " minRtrAdvInt=" << rtrVars.minRtrAdvInterval << endl;
//        os << " advManagedFlag=" << rtrVars.advManagedFlag;
//        os << " advOtherFlag=" << rtrVars.advOtherFlag;
//        os << " advLinkMTU=" << rtrVars.advLinkMTU;
//        os << " advReachableTime=" << rtrVars.advReachableTime;
//        os << " advRetransTimer=" << rtrVars.advRetransTimer;
//        os << " advCurHopLimit=" << rtrVars.advCurHopLimit;
//        os << " advDefaultLifetime=" << rtrVars.advDefaultLifetime;
    }

    os << "   }" << endl;
    return os.str();
}

std::string Ipv6InterfaceData::detailedInfo() const
{
    return str(); // TODO this could be improved: multi-line text, etc
}

void Ipv6InterfaceData::assignAddress(const Ipv6Address& addr, bool tentative,
        simtime_t expiryTime, simtime_t prefExpiryTime, bool hFlag)
{
    addresses.push_back(AddressData());
    AddressData& a = addresses.back();
    a.address = addr;
    a.tentative = tentative;
    a.expiryTime = expiryTime;
    a.prefExpiryTime = prefExpiryTime;

    if (addr.isGlobal()) {
        a.addrType = hFlag ? HoA : CoA;
    }

    choosePreferredAddress();
    changed1(F_IP_ADDRESS);

    cModule *m = ownerp ? dynamic_cast<cModule *>(ownerp->getInterfaceTable()) : nullptr;
    if (m) {
        Ipv6AddressInfo info(ownerp, addr);
        m->emit(ipv6AddressAssignedSignal, &info);
    }
}

void Ipv6InterfaceData::updateMatchingAddressExpiryTimes(const Ipv6Address& prefix, int length,
        simtime_t expiryTime, simtime_t prefExpiryTime)
{
    for (auto& elem : addresses) {
        if (elem.address.matches(prefix, length)) {
            elem.expiryTime = expiryTime;
            elem.prefExpiryTime = prefExpiryTime;
        }
    }
    choosePreferredAddress();
}

int Ipv6InterfaceData::findAddress(const Ipv6Address& addr) const
{
    for (AddressDataVector::const_iterator it = addresses.begin(); it != addresses.end(); it++)
        if (it->address == addr)
            return it - addresses.begin();

    return -1;
}

const Ipv6Address& Ipv6InterfaceData::getAddress(int i) const
{
    ASSERT(i >= 0 && i < (int)addresses.size());
    return addresses[i].address;
}

const Ipv6Address& Ipv6InterfaceData::getGlblAddress() const
{
    for (const auto& elem : addresses)
        if (elem.address.isGlobal())
            return elem.address;

    return Ipv6Address::UNSPECIFIED_ADDRESS;
}

bool Ipv6InterfaceData::isTentativeAddress(int i) const
{
    ASSERT(i >= 0 && i < (int)addresses.size());
    return addresses[i].tentative;
}

Ipv6InterfaceData::AddressType Ipv6InterfaceData::getAddressType(int i) const
{
    ASSERT(i >= 0 && i < (int)addresses.size());
    return addresses[i].addrType;
}

Ipv6InterfaceData::AddressType Ipv6InterfaceData::getAddressType(const Ipv6Address& addr) const
{
    return getAddressType(findAddress(addr));
}

bool Ipv6InterfaceData::hasAddress(const Ipv6Address& addr) const
{
    return findAddress(addr) != -1;
}

bool Ipv6InterfaceData::matchesSolicitedNodeMulticastAddress(const Ipv6Address& solNodeAddr) const
{
    for (const auto& elem : addresses)
        if (elem.address.formSolicitedNodeMulticastAddress() == solNodeAddr)
            return true;

    return false;
}

bool Ipv6InterfaceData::isTentativeAddress(const Ipv6Address& addr) const
{
    int k = findAddress(addr);
    return k != -1 && addresses[k].tentative;
}

void Ipv6InterfaceData::permanentlyAssign(const Ipv6Address& addr)
{
    int k = findAddress(addr);
    ASSERT(k != -1);
    addresses[k].tentative = false;
    choosePreferredAddress();
}

void Ipv6InterfaceData::tentativelyAssign(int i)
{
    ASSERT(i >= 0 && i < (int)addresses.size());
    addresses[i].tentative = true;
    choosePreferredAddress();
}


const Ipv6Address& Ipv6InterfaceData::getLinkLocalAddress() const
{
    for (const auto& elem : addresses)
        if (elem.address.isLinkLocal() && (elem.expiryTime == SIMTIME_ZERO || elem.expiryTime > simTime()))
            return elem.address;

    return Ipv6Address::UNSPECIFIED_ADDRESS;
}

void Ipv6InterfaceData::removeAddress(const Ipv6Address& address)
{
    int k = findAddress(address);
    ASSERT(k != -1);
    addresses.erase(addresses.begin() + k);
    choosePreferredAddress();
    changed1(F_IP_ADDRESS);

    cModule *m = ownerp ? dynamic_cast<cModule *>(ownerp->getInterfaceTable()) : nullptr;
    if (m) {
        Ipv6AddressInfo info(ownerp, address);
        m->emit(ipv6AddressRemovedSignal, &info);
    }
}

bool Ipv6InterfaceData::addrLess(const AddressData& a, const AddressData& b)
{
    // This method is used in choosePreferredAddress().
    // sort() produces increasing order, so "better" addresses should
    // compare as "less", to make them appear first in the array
    if (a.tentative != b.tentative)
        return !a.tentative; // tentative=false is better
    if (a.address.getScope() != b.address.getScope())
        return a.address.getScope() > b.address.getScope(); // bigger scope is better

    if (a.address.isGlobal() && b.address.isGlobal() && a.addrType != b.addrType)
        return a.addrType == CoA; // HoA is better than CoA

    return (a.expiryTime == SIMTIME_ZERO && b.expiryTime != SIMTIME_ZERO) || a.expiryTime > b.expiryTime; // longer expiry time is better
}

void Ipv6InterfaceData::choosePreferredAddress()
{
    // remove expired addresses (expiryTime == 0 means infinite lifetime)
    simtime_t now = simTime();
    bool changed = false;
    for (auto it = addresses.begin(); it != addresses.end(); ) {
        if (it->expiryTime != SIMTIME_ZERO && it->expiryTime <= now) {
            EV_INFO << "Address " << it->address << " has expired, removing\n";
            it = addresses.erase(it);
            changed = true;
        }
        else
            ++it;
    }

    if (addresses.empty()) {
        preferredAddr = Ipv6Address();
        return;
    }

    // sort addresses by scope and expiry time
    std::stable_sort(addresses.begin(), addresses.end(), addrLess);
    // Choose the preferred (default) source address. Prefer a routable
    // (non-link-local) unicast address: this is the source used for off-link
    // destinations. A tentative global address is still preferred over a
    // link-local one -- the sender (Ipv6) then either defers the datagram until
    // DAD completes (RFC 4862) or, under Optimistic DAD (RFC 4429), uses it
    // right away. Falling back to a link-local source for an off-link
    // destination would be non-routable (RFC 4291 Section 2.5.6), so link-local
    // is used only when no routable address exists at all.
    Ipv6Address linkLocalCandidate = Ipv6Address::UNSPECIFIED_ADDRESS;
    simtime_t linkLocalExpiry = SIMTIME_ZERO;
    for (auto& elem : addresses) {
        if (!elem.address.isUnicast())
            continue;
        if (!elem.address.isLinkLocal()) {
            preferredAddr = elem.address;
            preferredAddrExpiryTime = elem.expiryTime;
            if (changed)
                changed1(F_IP_ADDRESS);
            return;
        }
        if (linkLocalCandidate.isUnspecified()) {
            linkLocalCandidate = elem.address;
            linkLocalExpiry = elem.expiryTime;
        }
    }
    preferredAddr = linkLocalCandidate;
    preferredAddrExpiryTime = linkLocalExpiry;
    if (changed)
        changed1(F_IP_ADDRESS);
}

void Ipv6InterfaceData::addAdvPrefix(const AdvPrefix& advPrefix)
{
    rtrVars.advPrefixList.push_back(advPrefix);
}

const Ipv6InterfaceData::AdvPrefix& Ipv6InterfaceData::getAdvPrefix(int i) const
{
    ASSERT(i >= 0 && i < (int)rtrVars.advPrefixList.size());
    return rtrVars.advPrefixList[i];
}

void Ipv6InterfaceData::setAdvPrefix(int i, const AdvPrefix& advPrefix)
{
    ASSERT(i >= 0 && i < (int)rtrVars.advPrefixList.size());
    ASSERT(rtrVars.advPrefixList[i].prefix == advPrefix.prefix);
    ASSERT(rtrVars.advPrefixList[i].prefixLength == advPrefix.prefixLength);
    rtrVars.advPrefixList[i] = advPrefix;
}

void Ipv6InterfaceData::removeAdvPrefix(int i)
{
    ASSERT(i >= 0 && i < (int)rtrVars.advPrefixList.size());
    rtrVars.advPrefixList.erase(rtrVars.advPrefixList.begin() + i);
}

simtime_t Ipv6InterfaceData::generateReachableTime(double MIN_RANDOM_FACTOR,
        double MAX_RANDOM_FACTOR, uint baseReachableTime)
{
    return RNGCONTEXT uniform(MIN_RANDOM_FACTOR, MAX_RANDOM_FACTOR) * baseReachableTime;
}

simtime_t Ipv6InterfaceData::generateReachableTime()
{
    return RNGCONTEXT uniform(_getMinRandomFactor(), _getMaxRandomFactor()) * getBaseReachableTime();
}

bool Ipv6InterfaceData::isMemberOfMulticastGroup(const Ipv6Address& multicastAddress) const
{
    const HostMulticastGroupVector& groups = getHostData()->joinedMulticastGroups;
    for (auto& group : groups)
        if (group->multicastGroup == multicastAddress)
            return true;
    return false;
}

void Ipv6InterfaceData::joinMulticastGroup(const Ipv6Address& multicastAddress)
{
    Ipv6AddressVector empty;
    changeMulticastGroupMembership(multicastAddress, MCAST_INCLUDE_SOURCES, empty, MCAST_EXCLUDE_SOURCES, empty);
}

void Ipv6InterfaceData::leaveMulticastGroup(const Ipv6Address& multicastAddress)
{
    Ipv6AddressVector empty;
    changeMulticastGroupMembership(multicastAddress, MCAST_EXCLUDE_SOURCES, empty, MCAST_INCLUDE_SOURCES, empty);
}

void Ipv6InterfaceData::changeMulticastGroupMembership(Ipv6Address multicastAddress,
        McastSourceFilterMode oldFilterMode, const Ipv6AddressVector& oldSourceList,
        McastSourceFilterMode newFilterMode, const Ipv6AddressVector& newSourceList)
{
    if (ownerp && !ownerp->isMulticast())
        throw cRuntimeError("Ipv6InterfaceData::changeMulticastGroupMembership(): multicast interface expected, received %s.", ownerp->getInterfaceFullPath().c_str());
    if (!multicastAddress.isMulticast())
        throw cRuntimeError("Ipv6InterfaceData::changeMulticastGroupMembership(): multicast address expected, received %s.", multicastAddress.str().c_str());

    HostMulticastGroupData *entry = findHostGroupData(multicastAddress);
    if (!entry) {
        ASSERT(oldFilterMode == MCAST_INCLUDE_SOURCES && oldSourceList.empty());
        HostMulticastData *data = getHostData();
        data->joinedMulticastGroups.push_back(new HostMulticastGroupData(multicastAddress));
        entry = data->joinedMulticastGroups.back();
    }

    std::map<Ipv6Address, int> *counts = oldFilterMode == MCAST_INCLUDE_SOURCES ? &entry->includeCounts : &entry->excludeCounts;
    for (const auto& elem : oldSourceList) {
        auto count = counts->find(elem);
        if (count == counts->end())
            throw cRuntimeError("Inconsistent reference counts in Ipv6InterfaceData.");
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

    // recompute the merged interface-level filterMode + sourceList
    bool changed = entry->updateSourceList();

    if (changed) {
        changed1(F_MULTICAST_ADDRESSES);

        cModule *m = ownerp ? dynamic_cast<cModule *>(ownerp->getInterfaceTable()) : nullptr;
        if (m) {
            Ipv6MulticastGroupSourceInfo info(ownerp, multicastAddress, entry->sourceList);
            m->emit(ipv6MulticastChangeSignal, &info);

            // legacy any-source join/leave notifications (these drive MLDv1)
            if (oldFilterMode != newFilterMode && oldSourceList.empty() && newSourceList.empty()) {
                Ipv6MulticastGroupInfo info2(ownerp, multicastAddress);
                m->emit(newFilterMode == MCAST_EXCLUDE_SOURCES ? ipv6MulticastGroupJoinedSignal : ipv6MulticastGroupLeftSignal, &info2);
            }
        }

        // drop the group entry if it has become INCLUDE(empty)
        if (entry->sourceList.isEmpty())
            removeHostGroupData(multicastAddress);
    }
}

Ipv6InterfaceData::HostMulticastGroupData *Ipv6InterfaceData::findHostGroupData(Ipv6Address multicastAddress)
{
    ASSERT(multicastAddress.isMulticast());
    HostMulticastGroupVector& entries = getHostData()->joinedMulticastGroups;
    for (auto& entry : entries)
        if (entry->multicastGroup == multicastAddress)
            return entry;
    return nullptr;
}

bool Ipv6InterfaceData::removeHostGroupData(Ipv6Address multicastAddress)
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

Ipv6InterfaceData::RouterMulticastGroupData *Ipv6InterfaceData::findRouterGroupData(Ipv6Address multicastAddress) const
{
    ASSERT(multicastAddress.isMulticast());
    const RouterMulticastGroupVector& entries = getRouterData()->reportedMulticastGroups;
    for (auto& entry : entries)
        if (entry->multicastGroup == multicastAddress)
            return entry;
    return nullptr;
}

bool Ipv6InterfaceData::removeRouterGroupData(Ipv6Address multicastAddress)
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

bool Ipv6InterfaceData::hasMulticastListener(const Ipv6Address& multicastAddress) const
{
    RouterMulticastGroupData *groupData = findRouterGroupData(multicastAddress);
    return groupData && !groupData->sourceList.isEmpty();
}

bool Ipv6InterfaceData::hasMulticastListener(const Ipv6Address& multicastAddress, const Ipv6Address& sourceAddress) const
{
    RouterMulticastGroupData *groupData = findRouterGroupData(multicastAddress);
    return groupData && groupData->sourceList.contains(sourceAddress);
}

void Ipv6InterfaceData::addMulticastListener(const Ipv6Address& multicastAddress)
{
    if (!multicastAddress.isMulticast())
        throw cRuntimeError("Ipv6InterfaceData::addMulticastListener(): multicast address expected, received %s.", multicastAddress.str().c_str());

    RouterMulticastGroupData *groupData = findRouterGroupData(multicastAddress);
    if (!groupData) {
        groupData = new RouterMulticastGroupData(multicastAddress);
        getRouterData()->reportedMulticastGroups.push_back(groupData);

        Ipv6MulticastGroupInfo info(getNetworkInterface(), multicastAddress);
        check_and_cast<cModule *>(getNetworkInterface()->getInterfaceTable())->emit(ipv6MulticastGroupRegisteredSignal, &info);
    }

    if (!groupData->sourceList.containsAll()) {
        groupData->sourceList = Ipv6MulticastSourceList::ALL_SOURCES;
        changed1(F_MULTICAST_LISTENERS);
    }
}

void Ipv6InterfaceData::addMulticastListener(const Ipv6Address& multicastAddress, const Ipv6Address& sourceAddress)
{
    if (!multicastAddress.isMulticast())
        throw cRuntimeError("Ipv6InterfaceData::addMulticastListener(): multicast address expected, received %s.", multicastAddress.str().c_str());

    RouterMulticastGroupData *groupData = findRouterGroupData(multicastAddress);
    if (!groupData) {
        groupData = new RouterMulticastGroupData(multicastAddress);
        getRouterData()->reportedMulticastGroups.push_back(groupData);

        Ipv6MulticastGroupInfo info(getNetworkInterface(), multicastAddress);
        check_and_cast<cModule *>(getNetworkInterface()->getInterfaceTable())->emit(ipv6MulticastGroupRegisteredSignal, &info);
    }

    if (!groupData->sourceList.contains(sourceAddress)) {
        groupData->sourceList.add(sourceAddress);
        changed1(F_MULTICAST_LISTENERS);
    }
}

void Ipv6InterfaceData::removeMulticastListener(const Ipv6Address& multicastAddress)
{
    if (removeRouterGroupData(multicastAddress)) {
        changed1(F_MULTICAST_LISTENERS);

        Ipv6MulticastGroupInfo info(getNetworkInterface(), multicastAddress);
        check_and_cast<cModule *>(getNetworkInterface()->getInterfaceTable())->emit(ipv6MulticastGroupUnregisteredSignal, &info);
    }
}

void Ipv6InterfaceData::removeMulticastListener(const Ipv6Address& multicastAddress, const Ipv6Address& sourceAddress)
{
    RouterMulticastGroupData *groupData = findRouterGroupData(multicastAddress);
    if (groupData) {
        groupData->sourceList.remove(sourceAddress);
        if (groupData->sourceList.isEmpty()) {
            removeRouterGroupData(multicastAddress);

            Ipv6MulticastGroupInfo info(getNetworkInterface(), multicastAddress);
            check_and_cast<cModule *>(getNetworkInterface()->getInterfaceTable())->emit(ipv6MulticastGroupUnregisteredSignal, &info);
        }
        changed1(F_MULTICAST_LISTENERS);
    }
}

void Ipv6InterfaceData::setMulticastListeners(Ipv6Address multicastAddress, McastSourceFilterMode filterMode, const Ipv6AddressVector& sourceList)
{
    if (!multicastAddress.isMulticast())
        throw cRuntimeError("Ipv6InterfaceData::setMulticastListeners(): multicast address expected, received %s.", multicastAddress.str().c_str());

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

        // notify source-filter-aware listeners (e.g. PIM-SM SSM) that the set of
        // source-specific listeners on this interface changed
        Ipv6MulticastGroupSourceInfo info(getNetworkInterface(), multicastAddress, Ipv6MulticastSourceList(filterMode, sourceList));
        check_and_cast<cModule *>(getNetworkInterface()->getInterfaceTable())->emit(ipv6MulticastListenerChangeSignal, &info);
    }
}

const Ipv6Address& Ipv6InterfaceData::getGlobalAddress(AddressType type) const
{
    for (const auto& elem : addresses)
        if (elem.address.isGlobal() && elem.addrType == type) // FIXME and valid, 25.9.07 - CB
            return elem.address;

    return Ipv6Address::UNSPECIFIED_ADDRESS;
}

// =======Zarrar Yousaf @ CNI UNI Dortmund 08.07.07; ==============================

const Ipv6Address Ipv6InterfaceData::autoConfRouterGlobalScopeAddress(int i) // removed return-by-reference - CB
{
    AdvPrefix& p = rtrVars.advPrefixList[i];
    Ipv6Address prefix = p.prefix;
    short length = p.prefixLength;
    Ipv6Address linkLocalAddr = getLinkLocalAddress();
    Ipv6Address globalAddress = linkLocalAddr.setPrefix(prefix, length); // the global address gets autoconfigured, given its prefix, which during initialisation is supplied by the FlatnetworkConfigurator6
    p.rtrAddress = globalAddress; // the newly formed global address from the respective adv prefix is stored in the AdvPrefix list, which will be used later by the RA prefix info option
    return globalAddress;
}

void Ipv6InterfaceData::autoConfRouterGlobalScopeAddress(AdvPrefix& p)
{
    Ipv6Address prefix = p.prefix;
    short length = p.prefixLength;
    Ipv6Address linkLocalAddr = getLinkLocalAddress();
    Ipv6Address globalAddress = linkLocalAddr.setPrefix(prefix, length); // the global address gets autoconfigured, given its prefix, which during initialisation is supplied by the FlatnetworkConfigurator6
    p.rtrAddress = globalAddress; // the newly formed global address from the respective adv prefix is stored in the AdvPrefix list, which will be used later by the RA prefix info option
}

void Ipv6InterfaceData::deduceAdvPrefix()
{
    for (int i = 0; i < getNumAdvPrefixes(); i++) {
        Ipv6InterfaceData::AdvPrefix& p = rtrVars.advPrefixList[i];
        /*Ipv6Address globalAddr = */
        autoConfRouterGlobalScopeAddress(p);
        assignAddress(p.rtrAddress, false, SIMTIME_ZERO, SIMTIME_ZERO);
    }
}

/**
 * This method traverses the address list and searches for a specific address.
 * The element is removed and returned.
 */
Ipv6Address Ipv6InterfaceData::removeAddress(Ipv6InterfaceData::AddressType type)
{
    Ipv6Address addr;

    for (auto it = addresses.begin(); it != addresses.end(); ++it) { // 24.9.07 - CB
        if ((*it).addrType == type) {
            addr = it->address;
            addresses.erase(it);
            break; // it is assumed that we do not have more than one CoA
        }
    }

    // pick new address as we've removed the old one
    choosePreferredAddress();
    changed1(F_IP_ADDRESS);

    if (!addr.isUnspecified()) {
        cModule *m = ownerp ? dynamic_cast<cModule *>(ownerp->getInterfaceTable()) : nullptr;
        if (m) {
            Ipv6AddressInfo info(ownerp, addr);
            m->emit(ipv6AddressRemovedSignal, &info);
        }
    }

    return addr;
}

} // namespace inet

