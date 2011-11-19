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

#include <sstream>
#include <algorithm>

#include "IPv6InterfaceData.h"

#ifdef WITH_xMIPv6
#include "RoutingTable6Access.h"
#endif /* WITH_xMIPv6 */

//FIXME invoked changed() from state-changing methods, to trigger notification...


IPv6InterfaceData::IPv6InterfaceData()
{
#ifdef WITH_xMIPv6
    rt6 = RoutingTable6Access().get();
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
    hostConstants.initialBindAckTimeout = MIPv6_INITIAL_BINDACK_TIMEOUT; //MIPv6: added by Zarrar Yousaf @ CNI UniDo 17.06.07
    hostConstants.maxBindAckTimeout = MIPv6_MAX_BINDACK_TIMEOUT; //MIPv6: added by Zarrar Yousaf @ CNI UniDo 17.06.07
    hostConstants.initialBindAckTimeoutFirst = MIPv6_INITIAL_BINDACK_TIMEOUT_FIRST; //MIPv6: 12.9.07 - CB
    hostConstants.maxRRBindingLifeTime = MIPv6_MAX_RR_BINDING_LIFETIME; // 14.9.07 - CB
    hostConstants.maxHABindingLifeTime = MIPv6_MAX_HA_BINDING_LIFETIME; // 14.9.07 - CB
    hostConstants.maxTokenLifeTime = MIPv6_MAX_TOKEN_LIFETIME; // 14.9.07 - CB
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
    hostVars.curHopLimit = IPv6_DEFAULT_ADVCURHOPLIMIT; //value specified in RFC 1700-can't find it
    hostVars.baseReachableTime = IPv6_REACHABLE_TIME;
    hostVars.reachableTime = generateReachableTime(_getMinRandomFactor(),
        _getMaxRandomFactor(), getBaseReachableTime());
    hostVars.retransTimer = IPv6_RETRANS_TIMER;

    //rtrVars.advSendAdvertisements is set in RoutingTable6.cc:line 143
    rtrVars.maxRtrAdvInterval = IPv6_DEFAULT_MAX_RTR_ADV_INT;
    rtrVars.minRtrAdvInterval = 0.33*rtrVars.maxRtrAdvInterval;
    rtrVars.advManagedFlag = false;
    rtrVars.advOtherConfigFlag = false;

#ifdef WITH_xMIPv6
    rtrVars.advHomeAgentFlag = false; //Zarrar Yousaf Feb-March 2007
#endif /* WITH_xMIPv6 */

    rtrVars.advLinkMTU = IPv6_MIN_MTU;
    rtrVars.advReachableTime = IPv6_DEFAULT_ADV_REACHABLE_TIME;
    rtrVars.advRetransTimer = IPv6_DEFAULT_ADV_RETRANS_TIMER;
    rtrVars.advCurHopLimit = IPv6_DEFAULT_ADVCURHOPLIMIT;
    rtrVars.advDefaultLifetime = 3*rtrVars.maxRtrAdvInterval;

#if USE_MOBILITY
    if (rtrVars.advDefaultLifetime<1)
        rtrVars.advDefaultLifetime = 1;
#endif
}

std::string IPv6InterfaceData::info() const
{
    // FIXME FIXME FIXME FIXME info() should never print a newline
    std::ostringstream os;
    os << "IPv6:{" << endl;
    for (int i=0; i<getNumAddresses(); i++)
    {
        os << (i?"\t            , ":"\tAddrs:") << getAddress(i)
           << "(" << IPv6Address::scopeName(getAddress(i).getScope())
           << (isTentativeAddress(i)?" tent":"") << ") "

#ifdef WITH_xMIPv6
           << ((rt6->isMobileNode() && getAddress(i).isGlobal())
               ? (addresses[i].addrType==HoA ? "HoA" : "CoA") : "")
#endif /* WITH_xMIPv6 */

           << " expiryTime: " << (addresses[i].expiryTime==0 ? "inf" : SIMTIME_STR(addresses[i].expiryTime))
           << " prefExpiryTime: " << (addresses[i].prefExpiryTime==0 ? "inf" : SIMTIME_STR(addresses[i].prefExpiryTime))
           << endl;
    }

    for (int i=0; i<getNumAdvPrefixes(); i++)
    {
        const AdvPrefix& a = getAdvPrefix(i);
        os << (i?", ":"\tAdvPrefixes: ") << a.prefix << "/" << a.prefixLength << "("
           << (a.advOnLinkFlag?"":"off-link ")
           << (a.advAutonomousFlag?"":"non-auto ");

#ifdef WITH_xMIPv6
        os << "R-Flag = " << (a.advRtrAddr ? "1 " : "0 ");
#endif /* WITH_xMIPv6 */

        if (a.advValidLifetime==0)
           os  << "lifetime:inf";
        else if (a.advValidLifetime>0)
           os  << "expires:" << a.advValidLifetime;
        else
           os  << "lifetime:+" << (-1 * a.advValidLifetime);
        os << ")" << endl;
    }
    os << " ";

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
    if ( rt6->isMobileNode() )
        os << "\tHome Network Info: " << " HoA="<< homeInfo.HoA << ", HA=" << homeInfo.homeAgentAddr
           << ", home prefix=" << homeInfo.prefix/*.prefix()*/ << "\n";
#endif /* WITH_xMIPv6 */

    if (rtrVars.advSendAdvertisements)
    {
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

std::string IPv6InterfaceData::detailedInfo() const
{
    return info(); // TBD this could be improved: multi-line text, etc
}

#ifndef WITH_xMIPv6
void IPv6InterfaceData::assignAddress(const IPv6Address& addr, bool tentative,
                                      simtime_t expiryTime, simtime_t prefExpiryTime)
#else /* WITH_xMIPv6 */
void IPv6InterfaceData::assignAddress(const IPv6Address& addr, bool tentative,
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
    if ( addr.isGlobal() )  //only tag a global scope address as HoA or CoA, depending on the status of the H-Flag
    {
        if (hFlag == true)
            a.addrType = HoA; //if H-Flag is set then the auto-conf address is the Home address -.....
        else
            a.addrType = CoA; // else it is a care of address (CoA)
    }
    // FIXME else a.addrType = ???;
#endif /* WITH_xMIPv6 */

    choosePreferredAddress();
}

void IPv6InterfaceData::updateMatchingAddressExpiryTimes(const IPv6Address& prefix, int length,
                                                         simtime_t expiryTime, simtime_t prefExpiryTime)
{
    for (AddressDataVector::iterator it=addresses.begin(); it!=addresses.end(); it++)
    {
        if (it->address.matches(prefix, length))
        {
            it->expiryTime = expiryTime;
            it->prefExpiryTime = prefExpiryTime;
        }
    }
    choosePreferredAddress();
}

int IPv6InterfaceData::findAddress(const IPv6Address& addr) const
{
    for (AddressDataVector::const_iterator it=addresses.begin(); it!=addresses.end(); it++)
        if (it->address==addr)
            return it-addresses.begin();
    return -1;
}

const IPv6Address& IPv6InterfaceData::getAddress(int i) const
{
    ASSERT(i>=0 && i<(int)addresses.size());
    return addresses[i].address;
}

bool IPv6InterfaceData::isTentativeAddress(int i) const
{
    ASSERT(i>=0 && i<(int)addresses.size());
    return addresses[i].tentative;
}

#ifdef WITH_xMIPv6
IPv6InterfaceData::AddressType IPv6InterfaceData::getAddressType(int i) const
{
    ASSERT(i>=0 && i<(int)addresses.size());
    return addresses[i].addrType;
}

IPv6InterfaceData::AddressType IPv6InterfaceData::getAddressType(const IPv6Address& addr) const
{
    return getAddressType(findAddress(addr));
}

#endif /* WITH_xMIPv6 */
bool IPv6InterfaceData::hasAddress(const IPv6Address& addr) const
{
    return findAddress(addr)!=-1;
}

bool IPv6InterfaceData::matchesSolicitedNodeMulticastAddress(const IPv6Address& solNodeAddr) const
{
    for (AddressDataVector::const_iterator it=addresses.begin(); it!=addresses.end(); it++)
        if (it->address.formSolicitedNodeMulticastAddress()==solNodeAddr)
            return true;
    return false;
}

bool IPv6InterfaceData::isTentativeAddress(const IPv6Address& addr) const
{
    int k = findAddress(addr);
    return k!=-1 && addresses[k].tentative;
}

void IPv6InterfaceData::permanentlyAssign(const IPv6Address& addr)
{
    int k = findAddress(addr);
    ASSERT(k!=-1);
    addresses[k].tentative = false;
    choosePreferredAddress();
}

#ifdef WITH_xMIPv6
void IPv6InterfaceData::tentativelyAssign(int i)
{
    ASSERT(i>=0 && i<(int)addresses.size());
    addresses[i].tentative = true;
    choosePreferredAddress();
}
#endif /* WITH_xMIPv6 */

const IPv6Address& IPv6InterfaceData::getLinkLocalAddress() const
{
    for (AddressDataVector::const_iterator it=addresses.begin(); it!=addresses.end(); it++)
        if (it->address.isLinkLocal())  // FIXME and valid
            return it->address;
    return IPv6Address::UNSPECIFIED_ADDRESS;
}

void IPv6InterfaceData::removeAddress(const IPv6Address& address)
{
    int k = findAddress(address);
    ASSERT(k!=-1);
    addresses.erase(addresses.begin()+k);
    choosePreferredAddress();
}

bool IPv6InterfaceData::addrLess(const AddressData& a, const AddressData& b)
{
    // This method is used in choosePreferredAddress().
    // sort() produces increasing order, so "better" addresses should
    // compare as "less", to make them appear first in the array
    if (a.tentative!=b.tentative)
        return !a.tentative; // tentative=false is better
    if (a.address.getScope()!=b.address.getScope())
        return a.address.getScope()>b.address.getScope(); // bigger scope is better

#ifdef WITH_xMIPv6
    // FIXME  check a.address.isGlobal() != b.address.isGlobal()

    if ( a.address.isGlobal() && b.address.isGlobal() && a.addrType != b.addrType)
        return a.addrType == CoA; // HoA is better than CoA, 24.9.07 - CB
#endif /* WITH_xMIPv6 */

    return (a.expiryTime==0 && b.expiryTime!=0) || a.expiryTime>b.expiryTime;  // longer expiry time is better
}

void IPv6InterfaceData::choosePreferredAddress()
{
    // do we have addresses?
    if (addresses.size()==0)
    {
        preferredAddr = IPv6Address();
        return;
    }

    // FIXME shouldn't we stick to the current preferredAddress if its prefLifetime
    // hasn't expired yet?

    // FIXME TBD throw out expired addresses! 0 should be treated as infinity

    // sort addresses by scope and expiry time, then pick the first one
    std::sort(addresses.begin(), addresses.end(), addrLess);
    preferredAddr = addresses[0].address;
    preferredAddrExpiryTime = addresses[0].expiryTime;
}

void IPv6InterfaceData::addAdvPrefix(const AdvPrefix& advPrefix)
{
    rtrVars.advPrefixList.push_back(advPrefix);
}

const IPv6InterfaceData::AdvPrefix& IPv6InterfaceData::getAdvPrefix(int i) const
{
    ASSERT(i>=0 && i<(int)rtrVars.advPrefixList.size());
    return rtrVars.advPrefixList[i];
}

void IPv6InterfaceData::setAdvPrefix(int i, const AdvPrefix& advPrefix)
{
    ASSERT(i>=0 && i<(int)rtrVars.advPrefixList.size());
    ASSERT(rtrVars.advPrefixList[i].prefix == advPrefix.prefix);
    ASSERT(rtrVars.advPrefixList[i].prefixLength == advPrefix.prefixLength);
    rtrVars.advPrefixList[i] = advPrefix;
}

void IPv6InterfaceData::removeAdvPrefix(int i)
{
    ASSERT(i>=0 && i<(int)rtrVars.advPrefixList.size());
    rtrVars.advPrefixList.erase(rtrVars.advPrefixList.begin()+i);
}

simtime_t IPv6InterfaceData::generateReachableTime(double MIN_RANDOM_FACTOR,
    double MAX_RANDOM_FACTOR, uint baseReachableTime)
{
    return uniform(MIN_RANDOM_FACTOR, MAX_RANDOM_FACTOR) * baseReachableTime;
}

simtime_t IPv6InterfaceData::generateReachableTime()
{
    return uniform(_getMinRandomFactor(), _getMaxRandomFactor()) * getBaseReachableTime();
}

#ifdef WITH_xMIPv6
//#############    Additional function definitions added by Zarrar Yousaf @ CNI UNI Dortmund 23.05.07######

const IPv6Address& IPv6InterfaceData::getGlobalAddress(AddressType type) const
{
    for (AddressDataVector::const_iterator it=addresses.begin(); it!=addresses.end(); it++)
        if (it->address.isGlobal() && it->addrType == type )  // FIXME and valid, 25.9.07 - CB
            return it->address;
    return IPv6Address::UNSPECIFIED_ADDRESS;
}

// =======Zarrar Yousaf @ CNI UNI Dortmund 08.07.07; ==============================

const IPv6Address IPv6InterfaceData::autoConfRouterGlobalScopeAddress(int i) // removed return-by-reference - CB
{
    AdvPrefix& p = rtrVars.advPrefixList[i];
    IPv6Address prefix = p.prefix;
    short length = p.prefixLength;
    IPv6Address linkLocalAddr = getLinkLocalAddress();
    IPv6Address globalAddress = linkLocalAddr.setPrefix(prefix, length); //the global address gets autoconfigured, given its prefix, which during initialisation is supplied by the FlatnetworkConfigurator6
    p.rtrAddress = globalAddress; //the newly formed global address from the respective adv prefix is stored in the AdvPrefix list, which will be used later by the RA prefix info option
    return globalAddress;
}

void IPv6InterfaceData::autoConfRouterGlobalScopeAddress(AdvPrefix &p)
{
    IPv6Address prefix = p.prefix;
    short length = p.prefixLength;
    IPv6Address linkLocalAddr = getLinkLocalAddress();
    IPv6Address globalAddress = linkLocalAddr.setPrefix(prefix, length); //the global address gets autoconfigured, given its prefix, which during initialisation is supplied by the FlatnetworkConfigurator6
    p.rtrAddress = globalAddress; //the newly formed global address from the respective adv prefix is stored in the AdvPrefix list, which will be used later by the RA prefix info option
}

void IPv6InterfaceData::deduceAdvPrefix()
{
    for (int i=0; i < getNumAdvPrefixes(); i++)
    {
        IPv6InterfaceData::AdvPrefix& p = rtrVars.advPrefixList[i];
        /*IPv6Address globalAddr = */
        autoConfRouterGlobalScopeAddress(p);
        assignAddress(p.rtrAddress, false, 0, 0);
    }
}

/**
 * This method traverses the address list and searches for a specific address.
 * The element is removed and returned.
 */
IPv6Address IPv6InterfaceData::removeAddress(IPv6InterfaceData::AddressType type)
{
    IPv6Address addr;

    for (AddressDataVector::iterator it=addresses.begin(); it!=addresses.end(); ++it ) // 24.9.07 - CB
    {
        if ( (*it).addrType == type )
        {
            addr = it->address;
            addresses.erase(it);
            break; // it is assumed that we do not have more than one CoA
        }
    }

    // pick new address as we've removed the old one
    choosePreferredAddress();

    return addr;
}

std::ostream& operator<<(std::ostream& os, const IPv6InterfaceData::HomeNetworkInfo& homeNetInfo)
{
    os << "HoA of MN:" << homeNetInfo.HoA << " HA Address: " << homeNetInfo.homeAgentAddr
       << " Home Network Prefix: " << homeNetInfo.prefix/*.prefix()*/;
    return os;
}

void IPv6InterfaceData::updateHomeNetworkInfo(const IPv6Address& hoa, const IPv6Address& ha, const IPv6Address& prefix, const int prefixLength)
{
    EV<< "\n++++++ Updating the Home Network Information \n";
    homeInfo.HoA = hoa;
    homeInfo.homeAgentAddr = ha;
    homeInfo.prefix = prefix;

    // check if we already have a HoA on this interface
    // if not, then we create one
    IPv6Address addr = getGlobalAddress(HoA);

    if ( addr == IPv6Address::UNSPECIFIED_ADDRESS )
        this->assignAddress(hoa, false, 0, 0, true);
}
#endif /* WITH_xMIPv6 */
