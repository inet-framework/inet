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

//FIXME invoked changed() from state-changing methods, to trigger notification...


IPv6InterfaceData::IPv6InterfaceData()
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
    hostVars.curHopLimit = IPv6_DEFAULT_ADVCURHOPLIMIT;//value specified in RFC 1700-can't find it
    hostVars.baseReachableTime = IPv6_REACHABLE_TIME;
    hostVars.reachableTime = generateReachableTime(_getMinRandomFactor(),
        _getMaxRandomFactor(), getBaseReachableTime());
    hostVars.retransTimer = IPv6_RETRANS_TIMER;

    //rtrVars.advSendAdvertisements is set in RoutingTable6.cc:line 143
    rtrVars.maxRtrAdvInterval = IPv6_DEFAULT_MAX_RTR_ADV_INT;
    rtrVars.minRtrAdvInterval = 0.33*rtrVars.maxRtrAdvInterval;
    rtrVars.advManagedFlag = false;
    rtrVars.advOtherConfigFlag = false;
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

void IPv6InterfaceData::assignAddress(const IPv6Address& addr, bool tentative,
                                      simtime_t expiryTime, simtime_t prefExpiryTime)
{
    addresses.push_back(AddressData());
    AddressData& a = addresses.back();
    a.address = addr;
    a.tentative = tentative;
    a.expiryTime = expiryTime;
    a.prefExpiryTime = prefExpiryTime;
    choosePreferredAddress();
}

void IPv6InterfaceData::updateMatchingAddressExpiryTimes(const IPv6Address& prefix, int length,
                                                         simtime_t expiryTime, simtime_t prefExpiryTime)
{
    for (AddressDataVector::iterator it=addresses.begin(); it!=addresses.end(); it++)
    {
        if (it->address.matches(prefix,length))
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


