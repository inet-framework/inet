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

#include <map>
#include <vector>

#include "inet/common/INETDefs.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/routing/ospfv2/interface/OspfInterface.h"
#include "inet/routing/ospfv2/router/Lsa.h"
#include "inet/routing/ospfv2/router/OspfCommon.h"
#include "inet/routing/ospfv2/router/OspfRoutingTableEntry.h"

namespace inet {

namespace ospf {

class Router;

class INET_API Area : public cObject
{
  private:
    IInterfaceTable *ift;
    AreaId areaID;
    std::map<Ipv4AddressRange, bool> advertiseAddressRanges;
    std::vector<Ipv4AddressRange> areaAddressRanges;
    std::vector<OspfInterface *> associatedInterfaces;
    std::vector<HostRouteParameters> hostRoutes;
    std::map<LinkStateId, RouterLsa *> routerLSAsByID;
    std::vector<RouterLsa *> routerLSAs;
    std::map<LinkStateId, NetworkLsa *> networkLSAsByID;
    std::vector<NetworkLsa *> networkLSAs;
    std::map<LsaKeyType, SummaryLsa *, LsaKeyType_Less> summaryLSAsByID;
    std::vector<SummaryLsa *> summaryLSAs;
    bool transitCapability;
    bool externalRoutingCapability;
    Metric stubDefaultCost;
    RouterLsa *spfTreeRoot;

    Router *parentRouter;

  public:
    Area(IInterfaceTable *ift, AreaId id = BACKBONE_AREAID);
    virtual ~Area();

    void setAreaID(AreaId areaId) { areaID = areaId; }
    AreaId getAreaID() const { return areaID; }
    void addAddressRange(Ipv4AddressRange addressRange, bool advertise);
    unsigned int getAddressRangeCount() const { return areaAddressRanges.size(); }
    Ipv4AddressRange getAddressRange(unsigned int index) const { return areaAddressRanges[index]; }
    void addHostRoute(HostRouteParameters& hostRouteParameters) { hostRoutes.push_back(hostRouteParameters); }
    void setTransitCapability(bool transit) { transitCapability = transit; }
    bool getTransitCapability() const { return transitCapability; }
    void setExternalRoutingCapability(bool flooded) { externalRoutingCapability = flooded; }
    bool getExternalRoutingCapability() const { return externalRoutingCapability; }
    void setStubDefaultCost(Metric cost) { stubDefaultCost = cost; }
    Metric getStubDefaultCost() const { return stubDefaultCost; }
    void setSPFTreeRoot(RouterLsa *root) { spfTreeRoot = root; }
    RouterLsa *getSPFTreeRoot() { return spfTreeRoot; }
    const RouterLsa *getSPFTreeRoot() const { return spfTreeRoot; }

    void setRouter(Router *router) { parentRouter = router; }
    Router *getRouter() { return parentRouter; }
    const Router *getRouter() const { return parentRouter; }

    unsigned long getRouterLSACount() const { return routerLSAs.size(); }
    RouterLsa *getRouterLSA(unsigned long i) { return routerLSAs[i]; }
    const RouterLsa *getRouterLSA(unsigned long i) const { return routerLSAs[i]; }
    unsigned long getNetworkLSACount() const { return networkLSAs.size(); }
    NetworkLsa *getNetworkLSA(unsigned long i) { return networkLSAs[i]; }
    const NetworkLsa *getNetworkLSA(unsigned long i) const { return networkLSAs[i]; }
    unsigned long getSummaryLSACount() const { return summaryLSAs.size(); }
    SummaryLsa *getSummaryLSA(unsigned long i) { return summaryLSAs[i]; }
    const SummaryLsa *getSummaryLSA(unsigned long i) const { return summaryLSAs[i]; }

    bool containsAddress(Ipv4Address address) const;
    bool hasAddressRange(Ipv4AddressRange addressRange) const;
    void addWatches();
    Ipv4AddressRange getContainingAddressRange(Ipv4AddressRange addressRange, bool *advertise = nullptr) const;
    void addInterface(OspfInterface *intf);
    int getInterfaceCount() const {return associatedInterfaces.size();}
    OspfInterface *getInterface(unsigned char ifIndex);
    OspfInterface *getInterface(Ipv4Address address);
    std::vector<int> getInterfaceIndices();
    bool hasVirtualLink(AreaId withTransitArea) const;
    OspfInterface *findVirtualLink(RouterId routerID);

    bool installRouterLSA(const OspfRouterLsa *lsa);
    bool installNetworkLSA(const OspfNetworkLsa *lsa);
    bool installSummaryLSA(const OspfSummaryLsa *lsa);
    RouterLsa *findRouterLSA(LinkStateId linkStateID);
    const RouterLsa *findRouterLSA(LinkStateId linkStateID) const;
    NetworkLsa *findNetworkLSA(LinkStateId linkStateID);
    const NetworkLsa *findNetworkLSA(LinkStateId linkStateID) const;
    SummaryLsa *findSummaryLSA(LsaKeyType lsaKey);
    const SummaryLsa *findSummaryLSA(LsaKeyType lsaKey) const;
    void ageDatabase();
    bool hasAnyNeighborInStates(int states) const;
    void removeFromAllRetransmissionLists(LsaKeyType lsaKey);
    bool isOnAnyRetransmissionList(LsaKeyType lsaKey) const;
    bool floodLSA(const OspfLsa *lsa, OspfInterface *intf = nullptr, Neighbor *neighbor = nullptr);
    bool isLocalAddress(Ipv4Address address) const;
    RouterLsa *originateRouterLSA();
    NetworkLsa *originateNetworkLSA(const OspfInterface *intf);
    SummaryLsa *originateSummaryLSA(const OspfRoutingTableEntry *entry,
            const std::map<LsaKeyType, bool, LsaKeyType_Less>& originatedLSAs,
            SummaryLsa *& lsaToReoriginate);
    SummaryLsa *originateSummaryLSA_Stub();
    void calculateShortestPathTree(std::vector<OspfRoutingTableEntry *>& newRoutingTable);
    void calculateInterAreaRoutes(std::vector<OspfRoutingTableEntry *>& newRoutingTable);
    void recheckSummaryLSAs(std::vector<OspfRoutingTableEntry *>& newRoutingTable);

    std::string str() const override;
    std::string info() const OMNETPP5_CODE(override);
    std::string detailedInfo() const OMNETPP5_CODE(override);

  private:
    SummaryLsa *originateSummaryLSA(const SummaryLsa *summaryLSA);
    bool hasLink(OspfLsa *fromLSA, OspfLsa *toLSA) const;
    std::vector<NextHop> *calculateNextHops(OspfLsa *destination, OspfLsa *parent) const;
    std::vector<NextHop> *calculateNextHops(const Link& destination, OspfLsa *parent) const;

    LinkStateId getUniqueLinkStateID(Ipv4AddressRange destination,
            Metric destinationCost,
            SummaryLsa *& lsaToReoriginate) const;

    bool findSameOrWorseCostRoute(const std::vector<OspfRoutingTableEntry *>& newRoutingTable,
            const SummaryLsa& currentLSA,
            unsigned short currentCost,
            bool& destinationInRoutingTable,
            std::list<OspfRoutingTableEntry *>& sameOrWorseCost) const;

    OspfRoutingTableEntry *createRoutingTableEntryFromSummaryLSA(const SummaryLsa& summaryLSA,
            unsigned short entryCost,
            const OspfRoutingTableEntry& borderRouterEntry) const;
    void printLSDB();
    void printSummaryLsa();
    bool isDefaultRoute(OspfRoutingTableEntry *entry) const;
    bool isAllZero(Ipv4AddressRange entry) const;
};

inline std::ostream& operator<<(std::ostream& ostr, Area& area)
{
    ostr << area.info();
    return ostr;
}

} // namespace ospf

} // namespace inet

#endif // ifndef __INET_OSPFAREA_H

