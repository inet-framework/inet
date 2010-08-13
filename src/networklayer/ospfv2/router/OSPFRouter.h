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

#ifndef __INET_OSPFROUTER_H
#define __INET_OSPFROUTER_H

#include "OSPFcommon.h"
#include "OSPFArea.h"
#include "MessageHandler.h"
#include "OSPFInterface.h"
#include "LSA.h"
#include "OSPFRoutingTableEntry.h"
#include <map>

/**
 * All OSPF classes are in this namespace.
 */
namespace OSPF {

/**
 * Represents the full OSPF datastructure as laid out in RFC2328.
 */
class Router {
private:
    RouterID                                                           routerID;                ///< The router ID assigned by the IP layer.
    std::map<AreaID, Area*>                                            areasByID;               ///< A map of the contained areas with the AreaID as key.
    std::vector<Area*>                                                 areas;                   ///< A list of the contained areas.
    std::map<LSAKeyType, ASExternalLSA*, LSAKeyType_Less>              asExternalLSAsByID;      ///< A map of the ASExternalLSAs advertised by this router.
    std::vector<ASExternalLSA*>                                        asExternalLSAs;          ///< A list of the ASExternalLSAs advertised by this router.
    std::map<IPv4Address, OSPFASExternalLSAContents, IPv4Address_Less> externalRoutes;          ///< A map of the external route advertised by this router.
    OSPFTimer*                                                         ageTimer;                ///< Database age timer - fires every second.
    std::vector<RoutingTableEntry*>                                    routingTable;            ///< The OSPF routing table - contains more information than the one in the IP layer.
    MessageHandler*                                                    messageHandler;          ///< The message dispatcher class.
    bool                                                               rfc1583Compatibility;    ///< Decides whether to handle the preferred routing table entry to an AS boundary router as defined in RFC1583 or not.

public:
            Router(RouterID id, cSimpleModule* containingModule);
    virtual ~Router(void);

    void                     setRouterID               (RouterID id)              { routerID = id; }
    RouterID                 getRouterID               (void) const               { return routerID; }
    void                     setRFC1583Compatibility   (bool compatibility)       { rfc1583Compatibility = compatibility; }
    bool                     getRFC1583Compatibility   (void) const               { return rfc1583Compatibility; }
    unsigned long            getAreaCount              (void) const               { return areas.size(); }

    MessageHandler*          getMessageHandler         (void)                     { return messageHandler; }

    unsigned long            getASExternalLSACount     (void) const               { return asExternalLSAs.size(); }
    ASExternalLSA*           getASExternalLSA          (unsigned long i)          { return asExternalLSAs[i]; }
    const ASExternalLSA*     getASExternalLSA          (unsigned long i) const    { return asExternalLSAs[i]; }
    bool                     getASBoundaryRouter       (void) const               { return (externalRoutes.size() > 0); }

    unsigned long            getRoutingTableEntryCount(void) const               { return routingTable.size(); }
    RoutingTableEntry*       getRoutingTableEntry      (unsigned long i)          { return routingTable[i]; }
    const RoutingTableEntry* getRoutingTableEntry      (unsigned long i) const    { return routingTable[i]; }
    void                     AddRoutingTableEntry      (RoutingTableEntry* entry) { routingTable.push_back(entry); }

    void                 AddWatches                           (void);

    void                 AddArea                              (Area* area);
    Area*                getArea                              (AreaID areaID);
    Area*                getArea                              (IPv4Address address);
    Interface*           getNonVirtualInterface               (unsigned char ifIndex);

    bool                 InstallLSA                           (OSPFLSA* lsa, AreaID areaID = BackboneAreaID);
    OSPFLSA*             FindLSA                              (LSAType lsaType, LSAKeyType lsaKey, AreaID areaID);
    void                 AgeDatabase                          (void);
    bool                 HasAnyNeighborInStates               (int states) const;
    void                 RemoveFromAllRetransmissionLists     (LSAKeyType lsaKey);
    bool                 IsOnAnyRetransmissionList            (LSAKeyType lsaKey) const;
    bool                 FloodLSA                             (OSPFLSA* lsa, AreaID areaID = BackboneAreaID, Interface* intf = NULL, Neighbor* neighbor = NULL);
    bool                 IsLocalAddress                       (IPv4Address address) const;
    bool                 HasAddressRange                      (IPv4AddressRange addressRange) const;
    bool                 IsDestinationUnreachable             (OSPFLSA* lsa) const;
    RoutingTableEntry*   Lookup                               (IPAddress destination, std::vector<RoutingTableEntry*>* table = NULL) const;
    void                 RebuildRoutingTable                  (void);
    IPv4AddressRange     getContainingAddressRange            (IPv4AddressRange addressRange, bool* advertise = NULL) const;
    void                 UpdateExternalRoute                  (IPv4Address networkAddress, const OSPFASExternalLSAContents& externalRouteContents, int ifIndex);
    void                 RemoveExternalRoute                  (IPv4Address networkAddress);
    RoutingTableEntry*   getPreferredEntry                    (const OSPFLSA& lsa, bool skipSelfOriginated, std::vector<RoutingTableEntry*>* fromRoutingTable = NULL);

private:
    bool                 InstallASExternalLSA                 (OSPFASExternalLSA* lsa);
    ASExternalLSA*       FindASExternalLSA                    (LSAKeyType lsaKey);
    const ASExternalLSA* FindASExternalLSA                    (LSAKeyType lsaKey) const;
    ASExternalLSA*       OriginateASExternalLSA               (ASExternalLSA* lsa);
    LinkStateID          getUniqueLinkStateID                 (IPv4AddressRange destination,
                                                               Metric destinationCost,
                                                               OSPF::ASExternalLSA*& lsaToReoriginate,
                                                               bool externalMetricIsType2 = false) const;
    void                 CalculateASExternalRoutes            (std::vector<RoutingTableEntry*>& newRoutingTable);
    void                 NotifyAboutRoutingTableChanges       (std::vector<RoutingTableEntry*>& oldRoutingTable);
    bool                 HasRouteToASBoundaryRouter           (const std::vector<RoutingTableEntry*>& inRoutingTable, OSPF::RouterID routerID) const;
    std::vector<RoutingTableEntry*>
                         getRoutesToASBoundaryRouter          (const std::vector<RoutingTableEntry*>& fromRoutingTable, OSPF::RouterID routerID) const;
    void                 PruneASBoundaryRouterEntries         (std::vector<RoutingTableEntry*>& asbrEntries) const;
    RoutingTableEntry*   SelectLeastCostRoutingEntry          (std::vector<RoutingTableEntry*>& entries) const;
};

} // namespace OSPF

#endif // __INET_OSPFROUTER_H
