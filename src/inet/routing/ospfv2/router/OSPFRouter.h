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

#include <map>
#include <vector>

#include "inet/routing/ospfv2/router/LSA.h"
#include "inet/routing/ospfv2/messagehandler/MessageHandler.h"
#include "inet/routing/ospfv2/router/OSPFArea.h"
#include "inet/routing/ospfv2/router/OSPFcommon.h"
#include "inet/routing/ospfv2/interface/OSPFInterface.h"
#include "inet/routing/ospfv2/router/OSPFRoutingTableEntry.h"

namespace inet {

namespace ospf {

/**
 * All OSPF classes are in this namespace.
 */
/**
 * Represents the full OSPF data structure as laid out in RFC2328.
 */
class Router
{
  private:
    IInterfaceTable *ift;
    IIPv4RoutingTable *rt;
    RouterID routerID;    ///< The router ID assigned by the IP layer.
    std::map<AreaID, Area *> areasByID;    ///< A map of the contained areas with the AreaID as key.
    std::vector<Area *> areas;    ///< A list of the contained areas.
    std::map<LSAKeyType, ASExternalLSA *, LSAKeyType_Less> asExternalLSAsByID;    ///< A map of the ASExternalLSAs advertised by this router.
    std::vector<ASExternalLSA *> asExternalLSAs;    ///< A list of the ASExternalLSAs advertised by this router.
    std::map<IPv4Address, OSPFASExternalLSAContents> externalRoutes;    ///< A map of the external route advertised by this router.
    cMessage *ageTimer;    ///< Database age timer - fires every second.
    std::vector<RoutingTableEntry *> routingTable;    ///< The OSPF routing table - contains more information than the one in the IP layer.
    MessageHandler *messageHandler;    ///< The message dispatcher class.
    bool rfc1583Compatibility;    ///< Decides whether to handle the preferred routing table entry to an AS boundary router as defined in RFC1583 or not.

  public:
    /**
     * Constructor.
     * Initializes internal variables, adds a MessageHandler and starts the Database Age timer.
     */
    Router(RouterID id, cSimpleModule *containingModule, IInterfaceTable *ift, IIPv4RoutingTable *rt);

    /**
     * Destructor.
     * Clears all LSA lists and kills the Database Age timer.
     */
    virtual ~Router();

    void setRouterID(RouterID id) { routerID = id; }
    RouterID getRouterID() const { return routerID; }
    void setRFC1583Compatibility(bool compatibility) { rfc1583Compatibility = compatibility; }
    bool getRFC1583Compatibility() const { return rfc1583Compatibility; }
    unsigned long getAreaCount() const { return areas.size(); }

    MessageHandler *getMessageHandler() { return messageHandler; }

    unsigned long getASExternalLSACount() const { return asExternalLSAs.size(); }
    ASExternalLSA *getASExternalLSA(unsigned long i) { return asExternalLSAs[i]; }
    const ASExternalLSA *getASExternalLSA(unsigned long i) const { return asExternalLSAs[i]; }
    bool getASBoundaryRouter() const { return externalRoutes.size() > 0; }

    unsigned long getRoutingTableEntryCount() const { return routingTable.size(); }
    RoutingTableEntry *getRoutingTableEntry(unsigned long i) { return routingTable[i]; }
    const RoutingTableEntry *getRoutingTableEntry(unsigned long i) const { return routingTable[i]; }
    void addRoutingTableEntry(RoutingTableEntry *entry) { routingTable.push_back(entry); }

    /**
     * Adds OMNeT++ watches for the routerID, the list of Areas and the list of AS External LSAs.
     */
    void addWatches();

    /**
     * Adds a new Area to the Area list.
     * @param area [in] The Area to add.
     */
    void addArea(Area *area);

    /**
     * Returns the pointer to the Area identified by the input areaID, if it's on the Area list,
     * NULL otherwise.
     * @param areaID [in] The Area identifier.
     */
    Area *getAreaByID(AreaID areaID);

    /**
     * Returns the Area pointer from the Area list which contains the input IPv4 address,
     * NULL if there's no such area connected to the Router.
     * @param address [in] The IPv4 address whose containing Area we're looking for.
     */
    Area *getAreaByAddr(IPv4Address address);

    /**
     * Returns the pointer of the physical Interface identified by the input interface index,
     * NULL if the Router doesn't have such an interface.
     * @param ifIndex [in] The interface index to look for.
     */
    Interface *getNonVirtualInterface(unsigned char ifIndex);

    /**
     * Installs a new LSA into the Router database.
     * Checks the input LSA's type and installs it into either the selected Area's database,
     * or if it's an AS External LSA then into the Router's common asExternalLSAs list.
     * @param lsa    [in] The LSA to install. It will be copied into the database.
     * @param areaID [in] Identifies the input Router, Network and Summary LSA's Area.
     * @return True if the routing table needs to be updated, false otherwise.
     */
    bool installLSA(OSPFLSA *lsa, AreaID areaID = BACKBONE_AREAID);

    /**
     * Find the LSA identified by the input lsaKey in the database.
     * @param lsaType [in] Look for an LSA of this type.
     * @param lsaKey  [in] Look for the LSA which is identified by this key.
     * @param areaID  [in] In case of Router, Network and Summary LSAs, look in the Area's database
     *                     identified by this parameter.
     * @return The pointer to the LSA if it was found, NULL otherwise.
     */
    OSPFLSA *findLSA(LSAType lsaType, LSAKeyType lsaKey, AreaID areaID);

    /**
     * Ages the LSAs in the Router's database.
     * This method is called on every firing of the DATABASE_AGE_TIMER(every second).
     * @sa RFC2328 Section 14.
     */
    void ageDatabase();

    /**
     * Returns true if any Neighbor on any Interface in any of the Router's Areas is
     * in any of the input states, false otherwise.
     * @param states [in] A bitfield combination of NeighborStateType values.
     */
    bool hasAnyNeighborInStates(int states) const;

    /**
     * Removes all LSAs from all Neighbor's retransmission lists which are identified by
     * the input lsaKey.
     * @param lsaKey [in] Identifies the LSAs to remove from the retransmission lists.
     */
    void removeFromAllRetransmissionLists(LSAKeyType lsaKey);

    /**
     * Returns true if there's at least one LSA on any Neighbor's retransmission list
     * identified by the input lsaKey, false otherwise.
     * @param lsaKey [in] Identifies the LSAs to look for on the retransmission lists.
     */
    bool isOnAnyRetransmissionList(LSAKeyType lsaKey) const;

    /**
     * Floods out the input lsa on a set of Interfaces.
     * @sa RFC2328 Section 13.3.
     * @param lsa      [in] The LSA to be flooded out.
     * @param areaID   [in] If the lsa is a Router, Network or Summary LSA, then flood it only in this Area.
     * @param intf     [in] The Interface this LSA arrived on.
     * @param neighbor [in] The Nieghbor this LSA arrived from.
     * @return True if the LSA was floooded back out on the receiving Interface, false otherwise.
     */
    bool floodLSA(OSPFLSA *lsa, AreaID areaID = BACKBONE_AREAID, Interface *intf = NULL, Neighbor *neighbor = NULL);

    /**
     * Returns true if the input IPv4 address falls into any of the Router's Areas' configured
     * IPv4 address ranges, false otherwise.
     * @param address [in] The IPv4 address to look for.
     */
    bool isLocalAddress(IPv4Address address) const;

    /**
     * Returns true if one of the Router's Areas the same IPv4 address range configured as the
     * input IPv4 address range, false otherwise.
     * @param addressRange [in] The IPv4 address range to look for.
     */
    bool hasAddressRange(const IPv4AddressRange& addressRange) const;

    /**
     * Returns true if the destination described by the input lsa is in the routing table, false otherwise.
     * @param lsa [in] The LSA which describes the destination to look for.
     */
    bool isDestinationUnreachable(OSPFLSA *lsa) const;

    /**
     * Do a lookup in either the input OSPF routing table, or if it's NULL then in the Router's own routing table.
     * @sa RFC2328 Section 11.1.
     * @param destination [in] The destination to look up in the routing table.
     * @param table       [in] The routing table to do the lookup in.
     * @return The RoutingTableEntry describing the input destination if there's one, false otherwise.
     */
    RoutingTableEntry *lookup(IPv4Address destination, std::vector<RoutingTableEntry *> *table = NULL) const;

    /**
     * Rebuilds the routing table from scratch(based on the LSA database).
     * @sa RFC2328 Section 16.
     */
    void rebuildRoutingTable();

    /**
     * Scans through the router's areas' preconfigured address ranges and returns
     * the one containing the input addressRange.
     * @param addressRange [in] The address range to look for.
     * @param advertise    [out] Whether the advertise flag is set in the returned
     *                           preconfigured address range.
     * @return The containing preconfigured address range if found,
     *         NULL_IPV4ADDRESSRANGE otherwise.
     */
    IPv4AddressRange getContainingAddressRange(const IPv4AddressRange& addressRange, bool *advertise = NULL) const;

    /**
     * Stores information on an AS External Route in externalRoutes and intalls(or
     * updates) a new ASExternalLSA into the database.
     * @param networkAddress        [in] The external route's network address.
     * @param externalRouteContents [in] Route configuration data for the external route.
     * @param ifIndex               [in]
     */
    void updateExternalRoute(IPv4Address networkAddress, const OSPFASExternalLSAContents& externalRouteContents, int ifIndex);

    /**
     * Add an AS External Route in IPRoutingTable
     * @param networkAddress        [in] The external route's network address.
     * @param externalRouteContents [in] Route configuration data for the external route.
     * @param ifIndex               [in]
     */
    void addExternalRouteInIPTable(IPv4Address networkAddress, const OSPFASExternalLSAContents& externalRouteContents, int ifIndex);

    /**
     * Removes an AS External Route from the database.
     * @param networkAddress [in] The network address of the external route which
     *                            needs to be removed.
     */
    void removeExternalRoute(IPv4Address networkAddress);

    /**
     * Selects the preferred routing table entry for the input LSA(which is either
     * an ASExternalLSA or a SummaryLSA) according to the algorithm defined in
     * RFC2328 Section 16.4. points(1) through(3). This method is used when
     * calculating the AS external routes and also when originating an SummaryLSA
     * for an AS Boundary Router.
     * @param lsa                [in] The LSA describing the destination for which
     *                                the preferred Routing Entry is sought for.
     * @param skipSelfOriginated [in] Whether to disregard this LSA if it was
     *                                self-originated.
     * @param fromRoutingTable   [in] The Routing Table from which to select the
     *                                preferred RoutingTableEntry. If it is NULL
     *                                then the router's current routing table is
     *                                used instead.
     * @return The preferred RoutingTableEntry, or NULL if no such entry exists.
     * @sa RFC2328 Section 16.4. points(1) through(3)
     * @sa Area::originateSummaryLSA
     */
    RoutingTableEntry *getPreferredEntry(const OSPFLSA& lsa, bool skipSelfOriginated, std::vector<RoutingTableEntry *> *fromRoutingTable = NULL);

  private:
    /**
     * Installs a new AS External LSA into the Router's database.
     * It tries to install keep one of multiple functionally equivalent AS External LSAs in the database.
     * (See the comment in the method implementation.)
     * @param lsa [in] The LSA to install. It will be copied into the database.
     * @return True if the routing table needs to be updated, false otherwise.
     */
    bool installASExternalLSA(OSPFASExternalLSA *lsa);

    /**
     * Find the AS External LSA identified by the input lsaKey in the database.
     * @param lsaKey [in] Look for the AS External LSA which is identified by this key.
     * @return The pointer to the AS External LSA if it was found, NULL otherwise.
     */
    ASExternalLSA *findASExternalLSA(LSAKeyType lsaKey);

    /**
     * Find the AS External LSA identified by the input lsaKey in the database.
     * @param lsaKey [in] Look for the AS External LSA which is identified by this key.
     * @return The const pointer to the AS External LSA if it was found, NULL otherwise.
     */
    const ASExternalLSA *findASExternalLSA(LSAKeyType lsaKey) const;

    /**
     * Originates a new AS External LSA based on the input lsa.
     * @param lsa [in] The LSA whose contents should be copied into the newly originated LSA.
     * @return The newly originated LSA.
     */
    ASExternalLSA *originateASExternalLSA(ASExternalLSA *lsa);

    /**
     * Generates a unique LinkStateID for a given destination. This may require the
     * reorigination of an LSA already in the database(with a different
     * LinkStateID).
     * @param destination           [in] The destination for which a unique
     *                                   LinkStateID is required.
     * @param destinationCost       [in] The path cost to the destination.
     * @param lsaToReoriginate      [out] The LSA to reoriginate(which was already
     *                                    in the database, and had to be changed).
     * @param externalMetricIsType2 [in] True if the destinationCost is given as a
     *                                   Type2 external metric.
     * @return the LinkStateID for the destination.
     * @sa RFC2328 Appendix E.
     * @sa Area::getUniqueLinkStateID
     */
    LinkStateID getUniqueLinkStateID(const IPv4AddressRange& destination,
            Metric destinationCost,
            ASExternalLSA *& lsaToReoriginate,
            bool externalMetricIsType2 = false) const;

    /**
     * Calculate the AS External Routes from the ASExternalLSAs in the database.
     * @param newRoutingTable [in/out] Push the new RoutingTableEntries into this
     *                                 routing table, and also use this for path
     *                                 calculations.
     * @sa RFC2328 Section 16.4.
     */
    void calculateASExternalRoutes(std::vector<RoutingTableEntry *>& newRoutingTable);

    /**
     * After a routing table rebuild the changes in the routing table are
     * identified and new SummaryLSAs are originated or old ones are flooded out
     * in each area as necessary.
     * @param oldRoutingTable [in] The previous version of the routing table(which
     *                             is then compared with the one in routingTable).
     * @sa RFC2328 Section 12.4. points(5) through(6).
     */
    void notifyAboutRoutingTableChanges(std::vector<RoutingTableEntry *>& oldRoutingTable);

    /**
     * Returns true if there is a route to the AS Boundary Router identified by
     * asbrRouterID in the input inRoutingTable, false otherwise.
     * @param inRoutingTable [in] The routing table to look in.
     * @param asbrRouterID   [in] The ID of the AS Boundary Router to look for.
     */
    bool hasRouteToASBoundaryRouter(const std::vector<RoutingTableEntry *>& inRoutingTable, RouterID routerID) const;

    /**
     * Returns an std::vector of routes leading to the AS Boundary Router
     * identified by asbrRouterID from the input fromRoutingTable. If there are no
     * routes leading to the AS Boundary Router, the returned std::vector is empty.
     * @param fromRoutingTable [in] The routing table to look in.
     * @param asbrRouterID     [in] The ID of the AS Boundary Router to look for.
     */
    std::vector<RoutingTableEntry *>
    getRoutesToASBoundaryRouter(const std::vector<RoutingTableEntry *>& fromRoutingTable, RouterID routerID) const;

    /**
     * Prunes the input std::vector of RoutingTableEntries according to the RFC2328
     * Section 16.4.1.
     * @param asbrEntries [in/out] The list of RoutingTableEntries to prune.
     * @sa RFC2328 Section 16.4.1.
     */
    void pruneASBoundaryRouterEntries(std::vector<RoutingTableEntry *>& asbrEntries) const;

    /**
     * Selects the least cost RoutingTableEntry from the input std::vector of
     * RoutingTableEntries.
     * @param entries [in] The RoutingTableEntries to choose the least cost one from.
     * @return The least cost entry or NULL if entries is empty.
     */
    RoutingTableEntry *selectLeastCostRoutingEntry(std::vector<RoutingTableEntry *>& entries) const;
};

} // namespace ospf

} // namespace inet

#endif // ifndef __INET_OSPFROUTER_H

