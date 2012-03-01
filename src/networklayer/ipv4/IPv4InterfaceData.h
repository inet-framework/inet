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

#ifndef __IPv4INTERFACEDATA_H
#define __IPv4INTERFACEDATA_H

#include <vector>

#include "INETDefs.h"

#include "NotificationBoard.h"
#include "InterfaceEntry.h"
#include "IPv4Address.h"

/*
 * Info for NF_IPv4_MCAST_JOIN and NF_IPv4_MCAST_LEAVE notifications
 */
struct INET_API IPv4MulticastGroupInfo : public cObject
{
    IPv4MulticastGroupInfo(InterfaceEntry * const ie, const IPv4Address &groupAddress)
        : ie(ie), groupAddress(groupAddress) {}
    InterfaceEntry* ie;
    IPv4Address groupAddress;
};


/**
 * IPv4-specific data in an InterfaceEntry. Stores interface IPv4 address,
 * netmask, metric, etc.
 *
 * @see InterfaceEntry
 */
class INET_API IPv4InterfaceData : public InterfaceProtocolData
{
  public:
    typedef std::vector<IPv4Address> IPv4AddressVector;


  protected:
    struct HostMulticastData
    {
        IPv4AddressVector joinedMulticastGroups; // multicast groups this interface joined
        std::vector<int> refCounts;              // ref count of the corresponding multicast group

        std::string info();
        std::string detailedInfo();
    };

    struct RouterMulticastData
    {
        IPv4AddressVector reportedMulticastGroups; ///< multicast groups that have listeners on the link connected to this interface
        int multicastTtlThreshold;          ///< multicast ttl threshold, used by multicast routers to limit multicast scope

        RouterMulticastData() : multicastTtlThreshold(0) {}
        std::string info();
        std::string detailedInfo();
    };

    IPv4Address inetAddr;             ///< IPv4 address of interface
    IPv4Address netmask;              ///< netmask
    int metric;                       ///< link "cost"; see e.g. MS KB article Q299540
    HostMulticastData *hostData;
    RouterMulticastData *routerData;

    NotificationBoard *nb; // cached pointer

  protected:
    void changed1() {changed(NF_INTERFACE_IPv4CONFIG_CHANGED);}
    HostMulticastData *getHostData() { if (!hostData) hostData = new HostMulticastData(); return hostData; }
    const HostMulticastData *getHostData() const { return const_cast<IPv4InterfaceData*>(this)->getHostData(); }
    RouterMulticastData *getRouterData() { if (!routerData) routerData = new RouterMulticastData(); return routerData; }
    const RouterMulticastData *getRouterData() const { return const_cast<IPv4InterfaceData*>(this)->getRouterData(); }

  private:
    // copying not supported: following are private and also left undefined
    IPv4InterfaceData(const IPv4InterfaceData& obj);
    IPv4InterfaceData& operator=(const IPv4InterfaceData& obj);

  public:
    IPv4InterfaceData();
    virtual ~IPv4InterfaceData();
    virtual std::string info() const;
    virtual std::string detailedInfo() const;

    /** @name Getters */
    //@{
    IPv4Address getIPAddress() const {return inetAddr;}
    IPv4Address getNetmask() const {return netmask;}
    int getMetric() const  {return metric;}
    int getMulticastTtlThreshold() const {return getRouterData()->multicastTtlThreshold;}
    const IPv4AddressVector& getJoinedMulticastGroups() const {return getHostData()->joinedMulticastGroups;}
    const IPv4AddressVector& getReportedMulticastGroups() const {return getRouterData()->reportedMulticastGroups;}
    bool isMemberOfMulticastGroup(const IPv4Address &multicastAddress) const;
    bool hasMulticastListener(const IPv4Address &multicastAddress) const;
    //@}

    /** @name Setters */
    //@{
    virtual void setIPAddress(IPv4Address a) {inetAddr = a; changed1();}
    virtual void setNetmask(IPv4Address m) {netmask = m; changed1();}
    virtual void setMetric(int m) {metric = m; changed1();}
    virtual void setMulticastTtlThreshold(int threshold) {getRouterData()->multicastTtlThreshold=threshold; changed1();}
    virtual void joinMulticastGroup(const IPv4Address& multicastAddress);
    virtual void leaveMulticastGroup(const IPv4Address& multicastAddress);
    virtual void addMulticastListener(const IPv4Address &multicastAddress);
    virtual void removeMulticastListener(const IPv4Address &multicastAddress);
    //@}
};

#endif

