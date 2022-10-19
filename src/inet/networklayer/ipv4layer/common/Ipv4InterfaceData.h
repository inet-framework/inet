//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_IPV4INTERFACEDATA_H
#define __INET_IPV4INTERFACEDATA_H

#include <vector>

#include "inet/networklayer/common/NetworkInterface.h"
#include "inet/networklayer/contract/ipv4/Ipv4Address.h"

namespace inet {

struct INET_API Ipv4MulticastSourceList
{
    typedef std::vector<Ipv4Address> Ipv4AddressVector;
    McastSourceFilterMode filterMode;
    Ipv4AddressVector sources; // sorted

    Ipv4MulticastSourceList()
        : filterMode(MCAST_INCLUDE_SOURCES) {}
    Ipv4MulticastSourceList(McastSourceFilterMode filterMode, const Ipv4AddressVector& sources)
        : filterMode(filterMode), sources(sources) {}
    static const Ipv4MulticastSourceList ALL_SOURCES;

    bool operator==(const Ipv4MulticastSourceList& other) { return filterMode == other.filterMode && sources == other.sources; }
    bool operator!=(const Ipv4MulticastSourceList& other) { return filterMode != other.filterMode || sources != other.sources; }
    bool isEmpty() const { return filterMode == MCAST_INCLUDE_SOURCES && sources.empty(); }
    bool containsAll() const { return filterMode == MCAST_EXCLUDE_SOURCES && sources.empty(); }
    bool contains(Ipv4Address source);
    bool add(Ipv4Address source);
    bool remove(Ipv4Address source);
    std::string str() const;
    std::string detailedInfo() const;
};

/*
 * Info for ipv4MulticastGroupJoinedSignal and ipv4MulticastGroupLeftSignal notifications
 */
struct INET_API Ipv4MulticastGroupInfo : public cObject
{
    Ipv4MulticastGroupInfo(NetworkInterface *const ie, const Ipv4Address& groupAddress)
        : ie(ie), groupAddress(groupAddress) {}
    NetworkInterface *ie;
    Ipv4Address groupAddress;
};

/*
 * Info for ipv4McastChangeSignal notifications
 */
struct INET_API Ipv4MulticastGroupSourceInfo : public Ipv4MulticastGroupInfo
{
    typedef std::vector<Ipv4Address> Ipv4AddressVector;

    Ipv4MulticastGroupSourceInfo(NetworkInterface *const ie, const Ipv4Address& groupAddress, const Ipv4MulticastSourceList& sourceList)
        : Ipv4MulticastGroupInfo(ie, groupAddress), sourceList(sourceList) {}

    Ipv4MulticastSourceList sourceList;
};

/**
 * Ipv4-specific data in an NetworkInterface. Stores interface Ipv4 address,
 * netmask, metric, etc.
 *
 * @see NetworkInterface
 */
// TODO pass Ipv4Address parameters as values
class INET_API Ipv4InterfaceData : public InterfaceProtocolData
{
  public:
    typedef std::vector<Ipv4Address> Ipv4AddressVector;

    // field ids for change notifications
    enum { F_IP_ADDRESS, F_NETMASK, F_METRIC, F_MULTICAST_TTL_THRESHOLD, F_MULTICAST_ADDRESSES, F_MULTICAST_LISTENERS };

  protected:

    struct INET_API HostMulticastGroupData {
        Ipv4Address multicastGroup;
        std::map<Ipv4Address, int> includeCounts;
        std::map<Ipv4Address, int> excludeCounts;
        int numOfExcludeModeSockets;

        // computed
        Ipv4MulticastSourceList sourceList;

        HostMulticastGroupData(Ipv4Address multicastGroup)
            : multicastGroup(multicastGroup), numOfExcludeModeSockets(0) {}
        bool updateSourceList();
    };

    typedef std::vector<HostMulticastGroupData *> HostMulticastGroupVector;

    struct INET_API HostMulticastData {
        HostMulticastGroupVector joinedMulticastGroups; // multicast groups this interface joined

        virtual ~HostMulticastData();
        std::string str();
        std::string detailedInfo();
    };

    struct INET_API RouterMulticastGroupData {
        Ipv4Address multicastGroup;
        Ipv4MulticastSourceList sourceList;

        RouterMulticastGroupData(Ipv4Address multicastGroup)
            : multicastGroup(multicastGroup) {}
    };

    typedef std::vector<RouterMulticastGroupData *> RouterMulticastGroupVector;

    struct INET_API RouterMulticastData {
        RouterMulticastGroupVector reportedMulticastGroups; ///< multicast groups that have listeners on the link connected to this interface
        int multicastTtlThreshold; ///< multicast ttl threshold, used by multicast routers to limit multicast scope

        RouterMulticastData() : multicastTtlThreshold(0) {}
        virtual ~RouterMulticastData();
        std::string str();
        std::string detailedInfo();
    };

    Ipv4Address inetAddr; ///< Ipv4 address of interface
    Ipv4Address netmask; ///< netmask
    int metric; ///< link "cost"; see e.g. MS KB article Q299540
    HostMulticastData *hostData;
    RouterMulticastData *routerData;

  protected:
    void changed1(int fieldId) { changed(interfaceIpv4ConfigChangedSignal, fieldId); }
    HostMulticastData *getHostData() { if (!hostData) hostData = new HostMulticastData(); return hostData; }
    const HostMulticastData *getHostData() const { return const_cast<Ipv4InterfaceData *>(this)->getHostData(); }
    HostMulticastGroupData *findHostGroupData(Ipv4Address multicastAddress);
    bool removeHostGroupData(Ipv4Address multicastAddress);
    RouterMulticastData *getRouterData() { if (!routerData) routerData = new RouterMulticastData(); return routerData; }
    const RouterMulticastData *getRouterData() const { return const_cast<Ipv4InterfaceData *>(this)->getRouterData(); }
    RouterMulticastGroupData *findRouterGroupData(Ipv4Address multicastAddress) const;
    bool removeRouterGroupData(Ipv4Address multicastAddress);

  private:
    // copying not supported: following are private and also left undefined
    Ipv4InterfaceData(const Ipv4InterfaceData& obj);
    Ipv4InterfaceData& operator=(const Ipv4InterfaceData& obj);

  public:
    Ipv4InterfaceData();
    virtual ~Ipv4InterfaceData();
    virtual std::string str() const override;
    virtual std::string detailedInfo() const;

    /** @name Getters */
    //@{
    Ipv4Address getIPAddress() const { return inetAddr; }
    Ipv4Address getNetmask() const { return netmask; }
    Ipv4Address getNetworkBroadcastAddress() const { return inetAddr.makeBroadcastAddress(netmask); }
    int getMetric() const { return metric; }
    int getMulticastTtlThreshold() const { return getRouterData()->multicastTtlThreshold; }
    int getNumOfJoinedMulticastGroups() const { return getHostData()->joinedMulticastGroups.size(); }
    Ipv4Address getJoinedMulticastGroup(int index) const { return getHostData()->joinedMulticastGroups[index]->multicastGroup; }
    const Ipv4MulticastSourceList& getJoinedMulticastSources(int index) { return getHostData()->joinedMulticastGroups[index]->sourceList; }
    int getNumOfReportedMulticastGroups() const { return getRouterData()->reportedMulticastGroups.size(); }
    Ipv4Address getReportedMulticastGroup(int index) const { return getRouterData()->reportedMulticastGroups[index]->multicastGroup; }
    const Ipv4MulticastSourceList& getReportedMulticastSources(int index) const { return getRouterData()->reportedMulticastGroups[index]->sourceList; }
    bool isMemberOfMulticastGroup(const Ipv4Address& multicastAddress) const;
    bool hasMulticastListener(Ipv4Address multicastAddress) const;
    bool hasMulticastListener(Ipv4Address multicastAddress, Ipv4Address sourceAddress) const;
    //@}

    /** @name Setters */
    //@{
    virtual void setIPAddress(Ipv4Address a) { inetAddr = a; changed1(F_IP_ADDRESS); }
    virtual void setNetmask(Ipv4Address m) { netmask = m; changed1(F_NETMASK); }
    virtual void setMetric(int m) { metric = m; changed1(F_METRIC); }
    virtual void setMulticastTtlThreshold(int threshold) { getRouterData()->multicastTtlThreshold = threshold; changed1(F_MULTICAST_TTL_THRESHOLD); }
    virtual void joinMulticastGroup(const Ipv4Address& multicastAddress);
    virtual void leaveMulticastGroup(const Ipv4Address& multicastAddress);
    virtual void changeMulticastGroupMembership(Ipv4Address multicastAddress, McastSourceFilterMode oldFilterMode, const Ipv4AddressVector& oldSourceList,
            McastSourceFilterMode newFilterMode, const Ipv4AddressVector& newSourceList);
    virtual void addMulticastListener(const Ipv4Address& multicastAddress);
    virtual void addMulticastListener(Ipv4Address multicastAddress, Ipv4Address sourceAddress);
    virtual void removeMulticastListener(const Ipv4Address& multicastAddress);
    virtual void removeMulticastListener(Ipv4Address multicastAddress, Ipv4Address sourceAddress);
    virtual void setMulticastListeners(Ipv4Address multicastAddress, McastSourceFilterMode filterMode, const Ipv4AddressVector& sourceList);
    //@}
};

} // namespace inet

#endif

