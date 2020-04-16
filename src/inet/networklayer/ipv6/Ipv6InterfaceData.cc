//
// Copyright (C) 2005 Andras Varga
// Copyright (C) 2005 Wei Yang, Ng
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include <algorithm>
#include <sstream>

#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/ipv6/Ipv6InterfaceData.h"

namespace inet {

Register_Abstract_Class(Ipv6MulticastGroupInfo);

//FIXME invoked changed() from state-changing methods, to trigger notification...

std::string Ipv6InterfaceData::HostMulticastData::str()
{
    std::stringstream out;
    if (!joinedMulticastGroups.empty() &&
        (joinedMulticastGroups[0] != Ipv6Address::ALL_NODES_1 || joinedMulticastGroups.size() > 1))
    {
        out << "\tmcastgrps:";
        bool addComma = false;
        for (auto & elem : joinedMulticastGroups) {
            if (elem != Ipv6Address::ALL_NODES_1) {
                out << (addComma ? "," : "") << elem;
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
    for (size_t i = 0; i < joinedMulticastGroups.size(); ++i)
        out << " " << joinedMulticastGroups[i] << "(" << refCounts[i] << ")";
    out << "\n";
    return out.str();
}

std::string Ipv6InterfaceData::RouterMulticastData::str()
{
    std::stringstream out;
    if (reportedMulticastGroups.size() > 0) {
        out << "\tmcast_listeners:";
        for (size_t i = 0; i < reportedMulticastGroups.size(); ++i)
            out << (i > 0 ? "," : "") << reportedMulticastGroups[i];
    }
    return out.str();
}

std::string Ipv6InterfaceData::RouterMulticastData::detailedInfo()
{
    std::stringstream out;
    out << "Multicast Listeners:";
    for (auto & elem : reportedMulticastGroups)
        out << " " << elem;
    out << "\n";
    return out.str();
}

Ipv6InterfaceData::Ipv6InterfaceData()
    : InterfaceProtocolData(InterfaceEntry::F_IPV6_DATA)
{
#ifdef WITH_xMIPv6
    // rt6 = IPv6RoutingTableAccess().get();
#endif /* WITH_xMIPv6 */
    /*******************Setting host/node/router Protocol Constants************/
    routerConstants.maxInitialRtrAdvertInterval = IPv6_MAX_INITIAL_RTR_ADVERT_INTERVAL;
    routerConstants.maxInitialRtrAdvertisements = IPv6_MAX_INITIAL_RTR_ADVERTISEMENTS;
    routerConstants.maxFinalRtrAdvertisements = IPv6_MAX_FINAL_RTR_ADVERTISEMENTS;
    routerConstants.minDelayBetweenRAs = IPv6_MIN_DELAY_BETWEEN_RAS;
    routerConstants.maxRADelayTime = IPv6_MAX_RA_DELAY_TIME;

    hostConstants.maxRtrSolicitationDelay = IPv6_MAX_RTR_SOLICITATION_DELAY;
    hostConstants.rtrSolicitationInterval = IPv6_RTR_SOLICITATION_INTERVAL;
    hostConstants.maxRtrSolicitations = IPv6_MAX_RTR_SOLICITATIONS;

#ifdef WITH_xMIPv6
    hostConstants.initialBindAckTimeout = MIPv6_INITIAL_BINDACK_TIMEOUT;    //MIPv6: added by Zarrar Yousaf @ CNI UniDo 17.06.07
    hostConstants.maxBindAckTimeout = MIPv6_MAX_BINDACK_TIMEOUT;    //MIPv6: added by Zarrar Yousaf @ CNI UniDo 17.06.07
    hostConstants.initialBindAckTimeoutFirst = MIPv6_INITIAL_BINDACK_TIMEOUT_FIRST;    //MIPv6: 12.9.07 - CB
    hostConstants.maxRRBindingLifeTime = MIPv6_MAX_RR_BINDING_LIFETIME;    // 14.9.07 - CB
    hostConstants.maxHABindingLifeTime = MIPv6_MAX_HA_BINDING_LIFETIME;    // 14.9.07 - CB
    hostConstants.maxTokenLifeTime = MIPv6_MAX_TOKEN_LIFETIME;    // 14.9.07 - CB
#endif /* WITH_xMIPv6 */

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
    hostVars.curHopLimit = IPv6_DEFAULT_ADVCURHOPLIMIT;    //value specified in RFC 1700-can't find it
    hostVars.baseReachableTime = IPv6_REACHABLE_TIME;
    hostVars.reachableTime = generateReachableTime(_getMinRandomFactor(),
                _getMaxRandomFactor(), getBaseReachableTime());
    hostVars.retransTimer = IPv6_RETRANS_TIMER;

    //rtrVars.advSendAdvertisements is set in Ipv6RoutingTable.cc:line 143
    rtrVars.maxRtrAdvInterval = IPv6_DEFAULT_MAX_RTR_ADV_INT;
    rtrVars.minRtrAdvInterval = 0.33 * rtrVars.maxRtrAdvInterval;
    rtrVars.advManagedFlag = false;
    rtrVars.advOtherConfigFlag = false;

#ifdef WITH_xMIPv6
    rtrVars.advHomeAgentFlag = false;    //Zarrar Yousaf Feb-March 2007
#endif /* WITH_xMIPv6 */

    rtrVars.advLinkMTU = IPv6_MIN_MTU;
    rtrVars.advReachableTime = IPv6_DEFAULT_ADV_REACHABLE_TIME;
    rtrVars.advRetransTimer = IPv6_DEFAULT_ADV_RETRANS_TIMER;
    rtrVars.advCurHopLimit = IPv6_DEFAULT_ADVCURHOPLIMIT;
    rtrVars.advDefaultLifetime = 3 * rtrVars.maxRtrAdvInterval;

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

            #ifdef WITH_xMIPv6
// TODO: revise, routing table is not that simple to access
//           << ((rt6->isMobileNode() && getAddress(i).isGlobal())
//               ? (addresses[i].addrType==HoA ? "HoA" : "CoA") : "")
            #endif /* WITH_xMIPv6 */

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

#ifdef WITH_xMIPv6
        os << "R-Flag = " << (a.advRtrAddr ? "1 " : "0 ");
#endif /* WITH_xMIPv6 */

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
    //os << " curHopLimit=" << hostVars.curHopLimit;
    //os << " retransTimer=" << hostVars.retransTimer;
    //os << " baseReachableTime=" << hostVars.baseReachableTime;
    os << " reachableTime=" << hostVars.reachableTime << endl;

#ifdef WITH_xMIPv6
    // the following is for MIPv6 support
    // 4.9.07 - Zarrar, CB
// TODO: revise, routing table is not that simple to access
//    if ( rt6->isMobileNode() )
//        os << "\tHome Network Info: " << " HoA="<< homeInfo.HoA << ", HA=" << homeInfo.homeAgentAddr
//           << ", home prefix=" << homeInfo.prefix/*.prefix()*/ << "\n";
#endif /* WITH_xMIPv6 */

    if (rtrVars.advSendAdvertisements) {
        os << "\tRouter:";
        os << " maxRtrAdvInt=" << rtrVars.maxRtrAdvInterval;
        os << " minRtrAdvInt=" << rtrVars.minRtrAdvInterval << endl;
        //os << " advManagedFlag=" << rtrVars.advManagedFlag;
        //os << " advOtherFlag=" << rtrVars.advOtherFlag;
        //os << " advLinkMTU=" << rtrVars.advLinkMTU;
        //os << " advReachableTime=" << rtrVars.advReachableTime;
        //os << " advRetransTimer=" << rtrVars.advRetransTimer;
        //os << " advCurHopLimit=" << rtrVars.advCurHopLimit;
        //os << " advDefaultLifetime=" << rtrVars.advDefaultLifetime;
    }

    os << "   }" << endl;
    return os.str();
}

std::string Ipv6InterfaceData::detailedInfo() const
{
    return str();    // TBD this could be improved: multi-line text, etc
}

#ifndef WITH_xMIPv6
void Ipv6InterfaceData::assignAddress(const Ipv6Address& addr, bool tentative,
        simtime_t expiryTime, simtime_t prefExpiryTime)
#else /* WITH_xMIPv6 */
void Ipv6InterfaceData::assignAddress(const Ipv6Address& addr, bool tentative,
        simtime_t expiryTime, simtime_t prefExpiryTime, bool hFlag)
#endif /* WITH_xMIPv6 */
{
    addresses.push_back(AddressData());
    AddressData& a = addresses.back();
    a.address = addr;
    a.tentative = tentative;
    a.expiryTime = expiryTime;
    a.prefExpiryTime = prefExpiryTime;

#ifdef WITH_xMIPv6
    if (addr.isGlobal()) {    //only tag a global scope address as HoA or CoA, depending on the status of the H-Flag
        if (hFlag == true)
            a.addrType = HoA; //if H-Flag is set then the auto-conf address is the Home address -.....
        else
            a.addrType = CoA; // else it is a care of address (CoA)
    }
    // FIXME else a.addrType = ???;
#endif /* WITH_xMIPv6 */

    choosePreferredAddress();
    changed1(F_IP_ADDRESS);
}

void Ipv6InterfaceData::updateMatchingAddressExpiryTimes(const Ipv6Address& prefix, int length,
        simtime_t expiryTime, simtime_t prefExpiryTime)
{
    for (auto & elem : addresses) {
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
    for (const auto & elem : addresses)
        if (elem.address.isGlobal())
            return elem.address;

    return Ipv6Address::UNSPECIFIED_ADDRESS;
}

bool Ipv6InterfaceData::isTentativeAddress(int i) const
{
    ASSERT(i >= 0 && i < (int)addresses.size());
    return addresses[i].tentative;
}

#ifdef WITH_xMIPv6
Ipv6InterfaceData::AddressType Ipv6InterfaceData::getAddressType(int i) const
{
    ASSERT(i >= 0 && i < (int)addresses.size());
    return addresses[i].addrType;
}

Ipv6InterfaceData::AddressType Ipv6InterfaceData::getAddressType(const Ipv6Address& addr) const
{
    return getAddressType(findAddress(addr));
}

#endif /* WITH_xMIPv6 */
bool Ipv6InterfaceData::hasAddress(const Ipv6Address& addr) const
{
    return findAddress(addr) != -1;
}

bool Ipv6InterfaceData::matchesSolicitedNodeMulticastAddress(const Ipv6Address& solNodeAddr) const
{
    for (const auto & elem : addresses)
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

#ifdef WITH_xMIPv6
void Ipv6InterfaceData::tentativelyAssign(int i)
{
    ASSERT(i >= 0 && i < (int)addresses.size());
    addresses[i].tentative = true;
    choosePreferredAddress();
}

#endif /* WITH_xMIPv6 */

const Ipv6Address& Ipv6InterfaceData::getLinkLocalAddress() const
{
    for (const auto & elem : addresses)
        if (elem.address.isLinkLocal()) // FIXME and valid
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

#ifdef WITH_xMIPv6
    // FIXME  check a.address.isGlobal() != b.address.isGlobal()

    if (a.address.isGlobal() && b.address.isGlobal() && a.addrType != b.addrType)
        return a.addrType == CoA; // HoA is better than CoA, 24.9.07 - CB
#endif /* WITH_xMIPv6 */

    return (a.expiryTime == SIMTIME_ZERO && b.expiryTime != SIMTIME_ZERO) || a.expiryTime > b.expiryTime;    // longer expiry time is better
}

void Ipv6InterfaceData::choosePreferredAddress()
{
    // do we have addresses?
    if (addresses.size() == 0) {
        preferredAddr = Ipv6Address();
        return;
    }

    // FIXME shouldn't we stick to the current preferredAddress if its prefLifetime
    // hasn't expired yet?

    // FIXME TBD throw out expired addresses! 0 should be treated as infinity

    // sort addresses by scope and expiry time, then pick the first one
    std::stable_sort(addresses.begin(), addresses.end(), addrLess);
    // choose first unicast address
    for (auto & elem : addresses) {
        if (elem.address.isUnicast()) {
            preferredAddr = elem.address;
            preferredAddrExpiryTime = elem.expiryTime;
            return;
        }
    }
    preferredAddr = Ipv6Address::UNSPECIFIED_ADDRESS;
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
    const Ipv6AddressVector& multicastGroups = getJoinedMulticastGroups();
    return find(multicastGroups.begin(), multicastGroups.end(), multicastAddress) != multicastGroups.end();
}

void Ipv6InterfaceData::joinMulticastGroup(const Ipv6Address& multicastAddress)
{
    if (!multicastAddress.isMulticast())
        throw cRuntimeError("Ipv6InterfaceData::joinMulticastGroup(): multicast address expected, received %s.", multicastAddress.str().c_str());

    Ipv6AddressVector& multicastGroups = getHostData()->joinedMulticastGroups;

    std::vector<int>& refCounts = getHostData()->refCounts;
    for (int i = 0; i < (int)multicastGroups.size(); ++i) {
        if (multicastGroups[i] == multicastAddress) {
            refCounts[i]++;
            return;
        }
    }

    multicastGroups.push_back(multicastAddress);
    refCounts.push_back(1);

    changed1(F_MULTICAST_ADDRESSES);

    cModule *m = ownerp ? dynamic_cast<cModule *>(ownerp->getInterfaceTable()) : nullptr;
    if (m) {
        Ipv6MulticastGroupInfo info(ownerp, multicastAddress);
        m->emit(ipv6MulticastGroupJoinedSignal, &info);
    }
}

void Ipv6InterfaceData::leaveMulticastGroup(const Ipv6Address& multicastAddress)
{
    if (!multicastAddress.isMulticast())
        throw cRuntimeError("Ipv6InterfaceData::leaveMulticastGroup(): multicast address expected, received %s.", multicastAddress.str().c_str());

    Ipv6AddressVector& multicastGroups = getHostData()->joinedMulticastGroups;
    std::vector<int>& refCounts = getHostData()->refCounts;
    for (int i = 0; i < (int)multicastGroups.size(); ++i) {
        if (multicastGroups[i] == multicastAddress) {
            if (--refCounts[i] == 0) {
                multicastGroups.erase(multicastGroups.begin() + i);
                refCounts.erase(refCounts.begin() + i);

                changed1(F_MULTICAST_ADDRESSES);

                cModule *m = ownerp ? dynamic_cast<cModule *>(ownerp->getInterfaceTable()) : nullptr;
                if (m) {
                    Ipv6MulticastGroupInfo info(ownerp, multicastAddress);
                    m->emit(ipv6MulticastGroupLeftSignal, &info);
                }
            }
        }
    }
}

bool Ipv6InterfaceData::hasMulticastListener(const Ipv6Address& multicastAddress) const
{
    const Ipv6AddressVector& multicastGroups = getRouterData()->reportedMulticastGroups;
    return find(multicastGroups.begin(), multicastGroups.end(), multicastAddress) != multicastGroups.end();
}

void Ipv6InterfaceData::addMulticastListener(const Ipv6Address& multicastAddress)
{
    if (!multicastAddress.isMulticast())
        throw cRuntimeError("Ipv6InterfaceData::addMulticastListener(): multicast address expected, received %s.", multicastAddress.str().c_str());

    if (!hasMulticastListener(multicastAddress)) {
        getRouterData()->reportedMulticastGroups.push_back(multicastAddress);
        changed1(F_MULTICAST_LISTENERS);
    }
}

void Ipv6InterfaceData::removeMulticastListener(const Ipv6Address& multicastAddress)
{
    Ipv6AddressVector& multicastGroups = getRouterData()->reportedMulticastGroups;

    int n = multicastGroups.size();
    int i;
    for (i = 0; i < n; i++)
        if (multicastGroups[i] == multicastAddress)
            break;

    if (i != n) {
        multicastGroups.erase(multicastGroups.begin() + i);
        changed1(F_MULTICAST_LISTENERS);
    }
}

#ifdef WITH_xMIPv6
//#############    Additional function definitions added by Zarrar Yousaf @ CNI UNI Dortmund 23.05.07######

const Ipv6Address& Ipv6InterfaceData::getGlobalAddress(AddressType type) const
{
    for (const auto & elem : addresses)
        if (elem.address.isGlobal() && elem.addrType == type) // FIXME and valid, 25.9.07 - CB
            return elem.address;

    return Ipv6Address::UNSPECIFIED_ADDRESS;
}

// =======Zarrar Yousaf @ CNI UNI Dortmund 08.07.07; ==============================

const Ipv6Address Ipv6InterfaceData::autoConfRouterGlobalScopeAddress(int i)    // removed return-by-reference - CB
{
    AdvPrefix& p = rtrVars.advPrefixList[i];
    Ipv6Address prefix = p.prefix;
    short length = p.prefixLength;
    Ipv6Address linkLocalAddr = getLinkLocalAddress();
    Ipv6Address globalAddress = linkLocalAddr.setPrefix(prefix, length);    //the global address gets autoconfigured, given its prefix, which during initialisation is supplied by the FlatnetworkConfigurator6
    p.rtrAddress = globalAddress;    //the newly formed global address from the respective adv prefix is stored in the AdvPrefix list, which will be used later by the RA prefix info option
    return globalAddress;
}

void Ipv6InterfaceData::autoConfRouterGlobalScopeAddress(AdvPrefix& p)
{
    Ipv6Address prefix = p.prefix;
    short length = p.prefixLength;
    Ipv6Address linkLocalAddr = getLinkLocalAddress();
    Ipv6Address globalAddress = linkLocalAddr.setPrefix(prefix, length);    //the global address gets autoconfigured, given its prefix, which during initialisation is supplied by the FlatnetworkConfigurator6
    p.rtrAddress = globalAddress;    //the newly formed global address from the respective adv prefix is stored in the AdvPrefix list, which will be used later by the RA prefix info option
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

    for (auto it = addresses.begin(); it != addresses.end(); ++it) {    // 24.9.07 - CB
        if ((*it).addrType == type) {
            addr = it->address;
            addresses.erase(it);
            break;    // it is assumed that we do not have more than one CoA
        }
    }

    // pick new address as we've removed the old one
    choosePreferredAddress();
    changed1(F_IP_ADDRESS);

    return addr;
}

std::ostream& operator<<(std::ostream& os, const Ipv6InterfaceData::HomeNetworkInfo& homeNetInfo)
{
    os << "HoA of MN:" << homeNetInfo.HoA << " HA Address: " << homeNetInfo.homeAgentAddr
       << " Home Network Prefix: " << homeNetInfo.prefix    /*.prefix()*/;
    return os;
}

void Ipv6InterfaceData::updateHomeNetworkInfo(const Ipv6Address& hoa, const Ipv6Address& ha, const Ipv6Address& prefix, const int prefixLength)
{
    EV_INFO << "\n++++++ Updating the Home Network Information \n";
    homeInfo.HoA = hoa;
    homeInfo.homeAgentAddr = ha;
    homeInfo.prefix = prefix;

    // check if we already have a HoA on this interface
    // if not, then we create one
    Ipv6Address addr = getGlobalAddress(HoA);

    if (addr == Ipv6Address::UNSPECIFIED_ADDRESS)
        this->assignAddress(hoa, false, SIMTIME_ZERO, SIMTIME_ZERO, true);
}

#endif /* WITH_xMIPv6 */

} // namespace inet

