//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
// Copyright (C) 2004 Andras Varga
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

//
//  Author: Andras Varga
//

#ifndef __INET_IPV4INTERFACEDATA_H
#define __INET_IPV4INTERFACEDATA_H

#include <vector>

#include "inet/common/INETDefs.h"

#include "inet/networklayer/common/InterfaceEntry.h"
#include "inet/networklayer/contract/ipv4/IPv4Address.h"

namespace inet {

struct INET_API IPv4MulticastSourceList
{
    typedef std::vector<IPv4Address> IPv4AddressVector;
    McastSourceFilterMode filterMode;
    IPv4AddressVector sources;    // sorted

    IPv4MulticastSourceList()
        : filterMode(MCAST_INCLUDE_SOURCES) {}
    IPv4MulticastSourceList(McastSourceFilterMode filterMode, const IPv4AddressVector& sources)
        : filterMode(filterMode), sources(sources) {}
    static const IPv4MulticastSourceList ALL_SOURCES;

    bool operator==(const IPv4MulticastSourceList& other) { return filterMode == other.filterMode && sources == other.sources; }
    bool operator!=(const IPv4MulticastSourceList& other) { return filterMode != other.filterMode || sources != other.sources; }
    bool isEmpty() const { return filterMode == MCAST_INCLUDE_SOURCES && sources.empty(); }
    bool containsAll() const { return filterMode == MCAST_EXCLUDE_SOURCES && sources.empty(); }
    bool contains(IPv4Address source);
    bool add(IPv4Address source);
    bool remove(IPv4Address source);
    std::string info() const;
    std::string detailedInfo() const;
};

/*
 * Info for NF_IPv4_MCAST_JOIN and NF_IPv4_MCAST_LEAVE notifications
 */
struct INET_API IPv4MulticastGroupInfo : public cObject
{
    IPv4MulticastGroupInfo(InterfaceEntry *const ie, const IPv4Address& groupAddress)
        : ie(ie), groupAddress(groupAddress) {}
    InterfaceEntry *ie;
    IPv4Address groupAddress;
};

/*
 * Info for NF_IPv4_MCAST_CHANGE notifications
 */
struct INET_API IPv4MulticastGroupSourceInfo : public IPv4MulticastGroupInfo
{
    typedef std::vector<IPv4Address> IPv4AddressVector;

    IPv4MulticastGroupSourceInfo(InterfaceEntry *const ie, const IPv4Address& groupAddress, const IPv4MulticastSourceList& sourceList)
        : IPv4MulticastGroupInfo(ie, groupAddress), sourceList(sourceList) {}

    IPv4MulticastSourceList sourceList;
};

/**
 * IPv4-specific data in an InterfaceEntry. Stores interface IPv4 address,
 * netmask, metric, etc.
 *
 * @see InterfaceEntry
 */
// XXX pass IPv4Address parameters as values
class INET_API IPv4InterfaceData : public InterfaceProtocolData
{
  public:
    typedef std::vector<IPv4Address> IPv4AddressVector;

    // field ids for change notifications
    enum { F_IP_ADDRESS, F_NETMASK, F_METRIC, F_MULTICAST_TTL_THRESHOLD, F_MULTICAST_ADDRESSES, F_MULTICAST_LISTENERS };

  protected:

    struct INET_API HostMulticastGroupData
    {
        IPv4Address multicastGroup;
        std::map<IPv4Address, int> includeCounts;
        std::map<IPv4Address, int> excludeCounts;
        int numOfExcludeModeSockets;

        // computed
        IPv4MulticastSourceList sourceList;

        HostMulticastGroupData(IPv4Address multicastGroup)
            : multicastGroup(multicastGroup), numOfExcludeModeSockets(0) {}
        bool updateSourceList();
    };

    typedef std::vector<HostMulticastGroupData *> HostMulticastGroupVector;

    struct INET_API HostMulticastData
    {
        HostMulticastGroupVector joinedMulticastGroups;    // multicast groups this interface joined

        virtual ~HostMulticastData();
        std::string info();
        std::string detailedInfo();
    };

    struct INET_API RouterMulticastGroupData
    {
        IPv4Address multicastGroup;
        IPv4MulticastSourceList sourceList;

        RouterMulticastGroupData(IPv4Address multicastGroup)
            : multicastGroup(multicastGroup) {}
    };

    typedef std::vector<RouterMulticastGroupData *> RouterMulticastGroupVector;

    struct INET_API RouterMulticastData
    {
        RouterMulticastGroupVector reportedMulticastGroups;    ///< multicast groups that have listeners on the link connected to this interface
        int multicastTtlThreshold;    ///< multicast ttl threshold, used by multicast routers to limit multicast scope

        RouterMulticastData() : multicastTtlThreshold(0) {}
        virtual ~RouterMulticastData();
        std::string info();
        std::string detailedInfo();
    };

    IPv4Address inetAddr;    ///< IPv4 address of interface
    IPv4Address netmask;    ///< netmask
    int metric;    ///< link "cost"; see e.g. MS KB article Q299540
    HostMulticastData *hostData;
    RouterMulticastData *routerData;

  protected:
    void changed1(int fieldId) { changed(NF_INTERFACE_IPv4CONFIG_CHANGED, fieldId); }
    HostMulticastData *getHostData() { if (!hostData) hostData = new HostMulticastData(); return hostData; }
    const HostMulticastData *getHostData() const { return const_cast<IPv4InterfaceData *>(this)->getHostData(); }
    HostMulticastGroupData *findHostGroupData(IPv4Address multicastAddress);
    bool removeHostGroupData(IPv4Address multicastAddress);
    RouterMulticastData *getRouterData() { if (!routerData) routerData = new RouterMulticastData(); return routerData; }
    const RouterMulticastData *getRouterData() const { return const_cast<IPv4InterfaceData *>(this)->getRouterData(); }
    RouterMulticastGroupData *findRouterGroupData(IPv4Address multicastAddress) const;
    bool removeRouterGroupData(IPv4Address multicastAddress);

  private:
    // copying not supported: following are private and also left undefined
    IPv4InterfaceData(const IPv4InterfaceData& obj);
    IPv4InterfaceData& operator=(const IPv4InterfaceData& obj);

  public:
    IPv4InterfaceData();
    virtual ~IPv4InterfaceData();
    virtual std::string info() const override;
    virtual std::string detailedInfo() const override;

    /** @name Getters */
    //@{
    IPv4Address getIPAddress() const { return inetAddr; }
    IPv4Address getNetmask() const { return netmask; }
    IPv4Address getNetworkBroadcastAddress() const { return inetAddr.makeBroadcastAddress(netmask); }
    int getMetric() const { return metric; }
    int getMulticastTtlThreshold() const { return getRouterData()->multicastTtlThreshold; }
    int getNumOfJoinedMulticastGroups() const { return getHostData()->joinedMulticastGroups.size(); }
    IPv4Address getJoinedMulticastGroup(int index) const { return getHostData()->joinedMulticastGroups[index]->multicastGroup; }
    const IPv4MulticastSourceList& getJoinedMulticastSources(int index) { return getHostData()->joinedMulticastGroups[index]->sourceList; }
    int getNumOfReportedMulticastGroups() const { return getRouterData()->reportedMulticastGroups.size(); }
    IPv4Address getReportedMulticastGroup(int index) const { return getRouterData()->reportedMulticastGroups[index]->multicastGroup; }
    const IPv4MulticastSourceList& getReportedMulticastSources(int index) const { return getRouterData()->reportedMulticastGroups[index]->sourceList; }
    bool isMemberOfMulticastGroup(const IPv4Address& multicastAddress) const;
    bool hasMulticastListener(IPv4Address multicastAddress) const;
    bool hasMulticastListener(IPv4Address multicastAddress, IPv4Address sourceAddress) const;
    //@}

    /** @name Setters */
    //@{
    virtual void setIPAddress(IPv4Address a) { inetAddr = a; changed1(F_IP_ADDRESS); }
    virtual void setNetmask(IPv4Address m) { netmask = m; changed1(F_NETMASK); }
    virtual void setMetric(int m) { metric = m; changed1(F_METRIC); }
    virtual void setMulticastTtlThreshold(int threshold) { getRouterData()->multicastTtlThreshold = threshold; changed1(F_MULTICAST_TTL_THRESHOLD); }
    virtual void joinMulticastGroup(const IPv4Address& multicastAddress);
    virtual void leaveMulticastGroup(const IPv4Address& multicastAddress);
    virtual void changeMulticastGroupMembership(IPv4Address multicastAddress, McastSourceFilterMode oldFilterMode, const IPv4AddressVector& oldSourceList,
            McastSourceFilterMode newFilterMode, const IPv4AddressVector& newSourceList);
    virtual void addMulticastListener(const IPv4Address& multicastAddress);
    virtual void addMulticastListener(IPv4Address multicastAddress, IPv4Address sourceAddress);
    virtual void removeMulticastListener(const IPv4Address& multicastAddress);
    virtual void removeMulticastListener(IPv4Address multicastAddress, IPv4Address sourceAddress);
    virtual void setMulticastListeners(IPv4Address multicastAddress, McastSourceFilterMode filterMode, const IPv4AddressVector& sourceList);
    //@}
};

} // namespace inet

#endif // ifndef __INET_IPV4INTERFACEDATA_H

