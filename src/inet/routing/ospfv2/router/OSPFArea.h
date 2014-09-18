//
// Copyright (C) 2006 Andras Babos and Andras Varga
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

#ifndef __INET_OSPFAREA_H
#define __INET_OSPFAREA_H

#include <vector>
#include <map>

#include "inet/routing/ospfv2/router/LSA.h"
#include "inet/routing/ospfv2/router/OSPFcommon.h"
#include "inet/routing/ospfv2/interface/OSPFInterface.h"
#include "inet/routing/ospfv2/router/OSPFRoutingTableEntry.h"
#include "inet/networklayer/common/IInterfaceTable.h"

namespace inet {

namespace ospf {

class Router;

class Area : public cObject
{
  private:
    IInterfaceTable *ift;
    AreaID areaID;
    std::map<IPv4AddressRange, bool> advertiseAddressRanges;
    std::vector<IPv4AddressRange> areaAddressRanges;
    std::vector<Interface *> associatedInterfaces;
    std::vector<HostRouteParameters> hostRoutes;
    std::map<LinkStateID, RouterLSA *> routerLSAsByID;
    std::vector<RouterLSA *> routerLSAs;
    std::map<LinkStateID, NetworkLSA *> networkLSAsByID;
    std::vector<NetworkLSA *> networkLSAs;
    std::map<LSAKeyType, SummaryLSA *, LSAKeyType_Less> summaryLSAsByID;
    std::vector<SummaryLSA *> summaryLSAs;
    bool transitCapability;
    bool externalRoutingCapability;
    Metric stubDefaultCost;
    RouterLSA *spfTreeRoot;

    Router *parentRouter;

  public:
    Area(IInterfaceTable *ift, AreaID id = BACKBONE_AREAID);
    virtual ~Area();

    void setAreaID(AreaID areaId) { areaID = areaId; }
    AreaID getAreaID() const { return areaID; }
    void addAddressRange(IPv4AddressRange addressRange, bool advertise);
    unsigned int getAddressRangeCount() const { return areaAddressRanges.size(); }
    IPv4AddressRange getAddressRange(unsigned int index) const { return areaAddressRanges[index]; }
    void addHostRoute(HostRouteParameters& hostRouteParameters) { hostRoutes.push_back(hostRouteParameters); }
    void setTransitCapability(bool transit) { transitCapability = transit; }
    bool getTransitCapability() const { return transitCapability; }
    void setExternalRoutingCapability(bool flooded) { externalRoutingCapability = flooded; }
    bool getExternalRoutingCapability() const { return externalRoutingCapability; }
    void setStubDefaultCost(Metric cost) { stubDefaultCost = cost; }
    Metric getStubDefaultCost() const { return stubDefaultCost; }
    void setSPFTreeRoot(RouterLSA *root) { spfTreeRoot = root; }
    RouterLSA *getSPFTreeRoot() { return spfTreeRoot; }
    const RouterLSA *getSPFTreeRoot() const { return spfTreeRoot; }

    void setRouter(Router *router) { parentRouter = router; }
    Router *getRouter() { return parentRouter; }
    const Router *getRouter() const { return parentRouter; }

    unsigned long getRouterLSACount() const { return routerLSAs.size(); }
    RouterLSA *getRouterLSA(unsigned long i) { return routerLSAs[i]; }
    const RouterLSA *getRouterLSA(unsigned long i) const { return routerLSAs[i]; }
    unsigned long getNetworkLSACount() const { return networkLSAs.size(); }
    NetworkLSA *getNetworkLSA(unsigned long i) { return networkLSAs[i]; }
    const NetworkLSA *getNetworkLSA(unsigned long i) const { return networkLSAs[i]; }
    unsigned long getSummaryLSACount() const { return summaryLSAs.size(); }
    SummaryLSA *getSummaryLSA(unsigned long i) { return summaryLSAs[i]; }
    const SummaryLSA *getSummaryLSA(unsigned long i) const { return summaryLSAs[i]; }

    bool containsAddress(IPv4Address address) const;
    bool hasAddressRange(IPv4AddressRange addressRange) const;
    IPv4AddressRange getContainingAddressRange(IPv4AddressRange addressRange, bool *advertise = NULL) const;
    void addInterface(Interface *intf);
    Interface *getInterface(unsigned char ifIndex);
    Interface *getInterface(IPv4Address address);
    bool hasVirtualLink(AreaID withTransitArea) const;
    Interface *findVirtualLink(RouterID routerID);

    bool installRouterLSA(OSPFRouterLSA *lsa);
    bool installNetworkLSA(OSPFNetworkLSA *lsa);
    bool installSummaryLSA(OSPFSummaryLSA *lsa);
    RouterLSA *findRouterLSA(LinkStateID linkStateID);
    const RouterLSA *findRouterLSA(LinkStateID linkStateID) const;
    NetworkLSA *findNetworkLSA(LinkStateID linkStateID);
    const NetworkLSA *findNetworkLSA(LinkStateID linkStateID) const;
    SummaryLSA *findSummaryLSA(LSAKeyType lsaKey);
    const SummaryLSA *findSummaryLSA(LSAKeyType lsaKey) const;
    void ageDatabase();
    bool hasAnyNeighborInStates(int states) const;
    void removeFromAllRetransmissionLists(LSAKeyType lsaKey);
    bool isOnAnyRetransmissionList(LSAKeyType lsaKey) const;
    bool floodLSA(OSPFLSA *lsa, Interface *intf = NULL, Neighbor *neighbor = NULL);
    bool isLocalAddress(IPv4Address address) const;
    RouterLSA *originateRouterLSA();
    NetworkLSA *originateNetworkLSA(const Interface *intf);
    SummaryLSA *originateSummaryLSA(const RoutingTableEntry *entry,
            const std::map<LSAKeyType, bool, LSAKeyType_Less>& originatedLSAs,
            SummaryLSA *& lsaToReoriginate);
    void calculateShortestPathTree(std::vector<RoutingTableEntry *>& newRoutingTable);
    void calculateInterAreaRoutes(std::vector<RoutingTableEntry *>& newRoutingTable);
    void recheckSummaryLSAs(std::vector<RoutingTableEntry *>& newRoutingTable);

    std::string info() const;
    std::string detailedInfo() const;

  private:
    SummaryLSA *originateSummaryLSA(const SummaryLSA *summaryLSA);
    bool hasLink(OSPFLSA *fromLSA, OSPFLSA *toLSA) const;
    std::vector<NextHop> *calculateNextHops(OSPFLSA *destination, OSPFLSA *parent) const;
    std::vector<NextHop> *calculateNextHops(Link& destination, OSPFLSA *parent) const;

    LinkStateID getUniqueLinkStateID(IPv4AddressRange destination,
            Metric destinationCost,
            SummaryLSA *& lsaToReoriginate) const;

    bool findSameOrWorseCostRoute(const std::vector<RoutingTableEntry *>& newRoutingTable,
            const SummaryLSA& currentLSA,
            unsigned short currentCost,
            bool& destinationInRoutingTable,
            std::list<RoutingTableEntry *>& sameOrWorseCost) const;

    RoutingTableEntry *createRoutingTableEntryFromSummaryLSA(const SummaryLSA& summaryLSA,
            unsigned short entryCost,
            const RoutingTableEntry& borderRouterEntry) const;
};

inline std::ostream& operator<<(std::ostream& ostr, Area& area)
{
    ostr << area.detailedInfo();
    return ostr;
}

} // namespace ospf

} // namespace inet

#endif // ifndef __INET_OSPFAREA_H

