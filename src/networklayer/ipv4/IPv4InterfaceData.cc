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

#include "IPv4InterfaceData.h"

std::string IPv4InterfaceData::HostMulticastData::info()
{
    std::stringstream out;
    if (!joinedMulticastGroups.empty() &&
            (joinedMulticastGroups[0]!=IPv4Address::ALL_HOSTS_MCAST || joinedMulticastGroups.size() > 1))
    {
        out << " mcastgrps:";
        bool addComma = false;
        for (int i = 0; i < (int)joinedMulticastGroups.size(); ++i)
        {
            if (joinedMulticastGroups[i] != IPv4Address::ALL_HOSTS_MCAST)
            {
                out << (addComma?",":"") << joinedMulticastGroups[i];
                addComma = true;
            }
        }
    }
    return out.str();
}

std::string IPv4InterfaceData::HostMulticastData::detailedInfo()
{

    std::stringstream out;
    out << "Joined Groups:";
    for (int i = 0; i < (int)joinedMulticastGroups.size(); ++i)
        out << " " << joinedMulticastGroups[i] << "(" << refCounts[i] << ")";
    out << "\n";
    return out.str();
}

std::string IPv4InterfaceData::RouterMulticastData::info()
{
    std::stringstream out;
    if (reportedMulticastGroups.size() > 0)
    {
        out << " mcast_listeners:";
        for (int i = 0; i < (int)reportedMulticastGroups.size(); ++i)
            out << (i>0?",":"") << reportedMulticastGroups[i];
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
    for (int i = 0; i < (int)reportedMulticastGroups.size(); ++i)
        out << " " << reportedMulticastGroups[i];
    out << "\n";
    return out.str();
}

IPv4InterfaceData::IPv4InterfaceData()
{
    netmask = IPv4Address::ALLONES_ADDRESS;
    metric = 0;
    hostData = NULL;
    routerData = NULL;
    nb = NULL;
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

bool IPv4InterfaceData::isMemberOfMulticastGroup(const IPv4Address &multicastAddress) const
{
    const IPv4AddressVector &multicastGroups = getJoinedMulticastGroups();
    return find(multicastGroups.begin(), multicastGroups.end(), multicastAddress) != multicastGroups.end();
}

void IPv4InterfaceData::joinMulticastGroup(const IPv4Address& multicastAddress)
{
    if(!multicastAddress.isMulticast())
        throw cRuntimeError("IPv4InterfaceData::joinMulticastGroup(): multicast address expected, received %s.", multicastAddress.str().c_str());

    IPv4AddressVector &multicastGroups = getHostData()->joinedMulticastGroups;
    std::vector<int> &refCounts = getHostData()->refCounts;
    for (int i = 0; i < (int)multicastGroups.size(); ++i)
    {
        if (multicastGroups[i] == multicastAddress)
        {
            refCounts[i]++;
            return;
        }
    }

    multicastGroups.push_back(multicastAddress);
    refCounts.push_back(1);

    changed1();

    if (!nb)
        nb = NotificationBoardAccess().get();
    IPv4MulticastGroupInfo info(ownerp, multicastAddress);
    nb->fireChangeNotification(NF_IPv4_MCAST_JOIN, &info);
}

void IPv4InterfaceData::leaveMulticastGroup(const IPv4Address& multicastAddress)
{
    if(!multicastAddress.isMulticast())
        throw cRuntimeError("IPv4InterfaceData::leaveMulticastGroup(): multicast address expected, received %s.", multicastAddress.str().c_str());

    IPv4AddressVector &multicastGroups = getHostData()->joinedMulticastGroups;
    std::vector<int> &refCounts = getHostData()->refCounts;
    for (int i = 0; i < (int)multicastGroups.size(); ++i)
    {
        if (multicastGroups[i] == multicastAddress)
        {
            if (--refCounts[i] == 0)
            {
                multicastGroups.erase(multicastGroups.begin()+i);
                refCounts.erase(refCounts.begin()+i);

                changed1();

                if (!nb)
                    nb = NotificationBoardAccess().get();
                IPv4MulticastGroupInfo info(ownerp, multicastAddress);
                nb->fireChangeNotification(NF_IPv4_MCAST_LEAVE, &info);
            }
        }
    }
}

bool IPv4InterfaceData::hasMulticastListener(const IPv4Address &multicastAddress) const
{
    const IPv4AddressVector &multicastGroups = getRouterData()->reportedMulticastGroups;
    return find(multicastGroups.begin(),  multicastGroups.end(), multicastAddress) != multicastGroups.end();
}

void IPv4InterfaceData::addMulticastListener(const IPv4Address &multicastAddress)
{
    if(!multicastAddress.isMulticast())
        throw cRuntimeError("IPv4InterfaceData::addMulticastListener(): multicast address expected, received %s.", multicastAddress.str().c_str());

    if (!hasMulticastListener(multicastAddress))
    {
        getRouterData()->reportedMulticastGroups.push_back(multicastAddress);
        changed1();
    }
}

void IPv4InterfaceData::removeMulticastListener(const IPv4Address &multicastAddress)
{
    IPv4AddressVector &multicastGroups = getRouterData()->reportedMulticastGroups;

    int n = multicastGroups.size();
    int i;
    for (i = 0; i < n; i++)
        if (multicastGroups[i] == multicastAddress)
            break;
    if (i != n)
    {
        multicastGroups.erase(multicastGroups.begin() + i);
        changed1();
    }
}
