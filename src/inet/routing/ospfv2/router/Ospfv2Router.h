//
// Copyright (C) 2006 Andras Babos and Andras Varga
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_OSPFV2ROUTER_H
#define __INET_OSPFV2ROUTER_H

#include <map>
#include <vector>

#include "inet/routing/ospfv2/interface/Ospfv2Interface.h"
#include "inet/routing/ospfv2/messagehandler/MessageHandler.h"
#include "inet/routing/ospfv2/router/Lsa.h"
#include "inet/routing/ospfv2/router/Ospfv2Area.h"
#include "inet/routing/ospfv2/router/Ospfv2Common.h"
#include "inet/routing/ospfv2/router/Ospfv2RoutingTableEntry.h"

namespace inet {

namespace ospfv2 {

/**
 * All OSPF classes are in this namespace.
 */
/**
 * Represents the full OSPF data structure as laid out in RFC2328.
 */
class INET_API Router
{
  private:
    IInterfaceTable *ift = nullptr;
    IIpv4RoutingTable *rt = nullptr;
    RouterId routerID; ///< The router ID assigned by the IP layer.
    std::map<AreaId, Ospfv2Area *> areasByID; ///< A map of the contained areas with the AreaId as key.
    std::vector<Ospfv2Area *> areas; ///< A list of the contained areas.
    std::map<LsaKeyType, AsExternalLsa *, LsaKeyType_Less> asExternalLSAsByID; ///< A map of the ASExternalLSAs advertised by this router.
    std::vector<AsExternalLsa *> asExternalLSAs; ///< A list of the ASExternalLSAs advertised by this router.
    std::map<Ipv4Address, Ospfv2AsExternalLsaContents> externalRoutes; ///< A map of the external route advertised by this router.
    cMessage *ageTimer; ///< Database age timer - fires every second.
    std::vector<Ospfv2RoutingTableEntry *> ospfRoutingTable; ///< The OSPF routing table - contains more information than the one in the IP layer.
    MessageHandler *messageHandler; ///< The message dispatcher class.
    bool rfc1583Compatibility; ///< Decides whether to handle the preferred routing table entry to an AS boundary router as defined in RFC1583 or not.

  public:
    /**
     * Constructor.
     * Initializes internal variables, adds a MessageHandler and starts the Database Age timer.
     */
    Router(cSimpleModule *containingModule, IInterfaceTable *ift, IIpv4RoutingTable *rt);

    /**
     * Destructor.
     * Clears all LSA lists and kills the Database Age timer.
     */
    virtual ~Router();

    void setRouterID(RouterId id) { routerID = id; }
    RouterId getRouterID() const { return routerID; }
    void setRFC1583Compatibility(bool compatibility) { rfc1583Compatibility = compatibility; }
    bool getRFC1583Compatibility() const { return rfc1583Compatibility; }
    unsigned long getAreaCount() const { return areas.size(); }
    std::vector<AreaId> getAreaIds();

    MessageHandler *getMessageHandler() { return messageHandler; }

    unsigned long getASExternalLSACount() const { return asExternalLSAs.size(); }
    AsExternalLsa *getASExternalLSA(unsigned long i) { return asExternalLSAs[i]; }
    const AsExternalLsa *getASExternalLSA(unsigned long i) const { return asExternalLSAs[i]; }
    bool getASBoundaryRouter() const { return externalRoutes.size() > 0; }

    unsigned long getRoutingTableEntryCount() const { return ospfRoutingTable.size(); }
    Ospfv2RoutingTableEntry *getRoutingTableEntry(unsigned long i) { return ospfRoutingTable[i]; }
    const Ospfv2RoutingTableEntry *getRoutingTableEntry(unsigned long i) const { return ospfRoutingTable[i]; }
    void addRoutingTableEntry(Ospfv2RoutingTableEntry *entry) { ospfRoutingTable.push_back(entry); }

    /**
     * Adds OMNeT++ watches for the routerID, the list of Areas and the list of AS External LSAs.
     */
    void addWatches();

    /**
     * Adds a new Area to the Area list.
     * @param area [in] The Area to add.
     */
    void addArea(Ospfv2Area *area);

    /**
     * Returns the pointer to the Area identified by the input areaID, if it's on the Area list,
     * nullptr otherwise.
     * @param areaID [in] The Area identifier.
     */
    Ospfv2Area *getAreaByID(AreaId areaID);

    /**
     * Returns the Area pointer from the Area list which contains the input Ipv4 address,
     * nullptr if there's no such area connected to the Router.
     * @param address [in] The Ipv4 address whose containing Area we're looking for.
     */
    Ospfv2Area *getAreaByAddr(Ipv4Address address);

    /**
     * Returns the pointer of the physical Interface identified by the input interface index,
     * nullptr if the Router doesn't have such an interface.
     * @param ifIndex [in] The interface index to look for.
     */
    Ospfv2Interface *getNonVirtualInterface(unsigned char ifIndex);

    /**
     * Installs a new LSA into the Router database.
     * Checks the input LSA's type and installs it into either the selected Area's database,
     * or if it's an AS External LSA then into the Router's common asExternalLSAs list.
     * @param lsa    [in] The LSA to install. It will be copied into the database.
     * @param areaID [in] Identifies the input Router, Network and Summary LSA's Area.
     * @return True if the routing table needs to be updated, false otherwise.
     */
    bool installLSA(const Ospfv2Lsa *lsa, AreaId areaID = BACKBONE_AREAID);

    /**
     * Find the LSA identified by the input lsaKey in the database.
     * @param lsaType [in] Look for an LSA of this type.
     * @param lsaKey  [in] Look for the LSA which is identified by this key.
     * @param areaID  [in] In case of Router, Network and Summary LSAs, look in the Area's database
     *                     identified by this parameter.
     * @return The pointer to the LSA if it was found, nullptr otherwise.
     */
    Ospfv2Lsa *findLSA(Ospfv2LsaType lsaType, LsaKeyType lsaKey, AreaId areaID);

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
    void removeFromAllRetransmissionLists(LsaKeyType lsaKey);

    /**
     * Returns true if there's at least one LSA on any Neighbor's retransmission list
     * identified by the input lsaKey, false otherwise.
     * @param lsaKey [in] Identifies the LSAs to look for on the retransmission lists.
     */
    bool isOnAnyRetransmissionList(LsaKeyType lsaKey) const;

    /**
     * Floods out the input lsa on a set of Interfaces.
     * @sa RFC2328 Section 13.3.
     * @param lsa      [in] The LSA to be flooded out.
     * @param areaID   [in] If the lsa is a Router, Network or Summary LSA, then flood it only in this Area.
     * @param intf     [in] The Interface this LSA arrived on.
     * @param neighbor [in] The Nieghbor this LSA arrived from.
     * @return True if the LSA was floooded back out on the receiving Interface, false otherwise.
     */
    bool floodLSA(const Ospfv2Lsa *lsa, AreaId areaID = BACKBONE_AREAID, Ospfv2Interface *intf = nullptr, Neighbor *neighbor = nullptr);

    /**
     * Returns true if the input Ipv4 address falls into any of the Router's Areas' configured
     * Ipv4 address ranges, false otherwise.
     * @param address [in] The Ipv4 address to look for.
     */
    bool isLocalAddress(Ipv4Address address) const;

    /**
     * Returns true if one of the Router's Areas the same Ipv4 address range configured as the
     * input Ipv4 address range, false otherwise.
     * @param addressRange [in] The Ipv4 address range to look for.
     */
    bool hasAddressRange(const Ipv4AddressRange& addressRange) const;

    /**
     * Returns true if the destination described by the input lsa is in the routing table, false otherwise.
     * @param lsa [in] The LSA which describes the destination to look for.
     */
    bool isDestinationUnreachable(Ospfv2Lsa *lsa) const;

    /**
     * Do a lookup in either the input OSPF routing table, or if it's nullptr then in the Router's own routing table.
     * @sa RFC2328 Section 11.1.
     * @param destination [in] The destination to look up in the routing table.
     * @param table       [in] The routing table to do the lookup in.
     * @return The RoutingTableEntry describing the input destination if there's one, false otherwise.
     */
    Ospfv2RoutingTableEntry *lookup(Ipv4Address destination, std::vector<Ospfv2RoutingTableEntry *> *table = nullptr) const;

    /**
     * Rebuilds the routing table from scratch(based on the LSA database).
     * @sa RFC2328 Section 16.
     */
    void rebuildRoutingTable();

    // delete an entry from the OSPF routing table
    bool deleteRoute(Ospfv2RoutingTableEntry *entry);

    /**
     * Scans through the router's areas' preconfigured address ranges and returns
     * the one containing the input addressRange.
     * @param addressRange [in] The address range to look for.
     * @param advertise    [out] Whether the advertise flag is set in the returned
     *                           preconfigured address range.
     * @return The containing preconfigured address range if found,
     *         NULL_IPV4ADDRESSRANGE otherwise.
     */
    Ipv4AddressRange getContainingAddressRange(const Ipv4AddressRange& addressRange, bool *advertise = nullptr) const;

    /**
     * get the default route in the routing table.
     */
    Ipv4Route *getDefaultRoute();

    /**
     * Stores information on an AS External Route in externalRoutes and intalls(or
     * updates) a new AsExternalLsa into the database.
     * @param networkAddress        [in] The external route's network address.
     * @param externalRouteContents [in] Route configuration data for the external route.
     * @param ifIndex               [in]
     */
    void updateExternalRoute(Ipv4Address networkAddress, const Ospfv2AsExternalLsaContents& externalRouteContents, int ifIndex = -1);

    /**
     * Add an AS External Route in IPRoutingTable
     * @param networkAddress        [in] The external route's network address.
     * @param externalRouteContents [in] Route configuration data for the external route.
     * @param ifIndex               [in]
     */
    void addExternalRouteInIPTable(Ipv4Address networkAddress, const Ospfv2AsExternalLsaContents& externalRouteContents, int ifIndex);

    /**
     * Removes an AS External Route from the database.
     * @param networkAddress [in] The network address of the external route which
     *                            needs to be removed.
     */
    void removeExternalRoute(Ipv4Address networkAddress);

    /**
     * Selects the preferred routing table entry for the input LSA(which is either
     * an AsExternalLsa or a SummaryLsa) according to the algorithm defined in
     * RFC2328 Section 16.4. points(1) through(3). This method is used when
     * calculating the AS external routes and also when originating an SummaryLsa
     * for an AS Boundary Router.
     * @param lsa                [in] The LSA describing the destination for which
     *                                the preferred Routing Entry is sought for.
     * @param skipSelfOriginated [in] Whether to disregard this LSA if it was
     *                                self-originated.
     * @param fromRoutingTable   [in] The Routing Table from which to select the
     *                                preferred RoutingTableEntry. If it is nullptr
     *                                then the router's current routing table is
     *                                used instead.
     * @return The preferred RoutingTableEntry, or nullptr if no such entry exists.
     * @sa RFC2328 Section 16.4. points(1) through(3)
     * @sa Area::originateSummaryLSA
     */
    Ospfv2RoutingTableEntry *getPreferredEntry(const Ospfv2Lsa& lsa, bool skipSelfOriginated, std::vector<Ospfv2RoutingTableEntry *> *fromRoutingTable = nullptr);

  private:
    /**
     * Installs a new AS External LSA into the Router's database.
     * It tries to install keep one of multiple functionally equivalent AS External LSAs in the database.
     * (See the comment in the method implementation.)
     * @param lsa [in] The LSA to install. It will be copied into the database.
     * @return True if the routing table needs to be updated, false otherwise.
     */
    bool installASExternalLSA(const Ospfv2AsExternalLsa *lsa);

    /**
     * Find the AS External LSA identified by the input lsaKey in the database.
     * @param lsaKey [in] Look for the AS External LSA which is identified by this key.
     * @return The pointer to the AS External LSA if it was found, nullptr otherwise.
     */
    AsExternalLsa *findASExternalLSA(LsaKeyType lsaKey);

    /**
     * Find the AS External LSA identified by the input lsaKey in the database.
     * @param lsaKey [in] Look for the AS External LSA which is identified by this key.
     * @return The const pointer to the AS External LSA if it was found, nullptr otherwise.
     */
    const AsExternalLsa *findASExternalLSA(LsaKeyType lsaKey) const;

    /**
     * Originates a new AS External LSA based on the input lsa.
     * @param lsa [in] The LSA whose contents should be copied into the newly originated LSA.
     * @return The newly originated LSA.
     */
    AsExternalLsa *originateASExternalLSA(AsExternalLsa *lsa);

    /**
     * Generates a unique LinkStateId for a given destination. This may require the
     * reorigination of an LSA already in the database(with a different
     * LinkStateId).
     * @param destination           [in] The destination for which a unique
     *                                   LinkStateId is required.
     * @param destinationCost       [in] The path cost to the destination.
     * @param lsaToReoriginate      [out] The LSA to reoriginate(which was already
     *                                    in the database, and had to be changed).
     * @param externalMetricIsType2 [in] True if the destinationCost is given as a
     *                                   Type2 external metric.
     * @return the LinkStateId for the destination.
     * @sa RFC2328 Appendix E.
     * @sa Area::getUniqueLinkStateID
     */
    LinkStateId getUniqueLinkStateID(const Ipv4AddressRange& destination,
            Metric destinationCost,
            AsExternalLsa *& lsaToReoriginate,
            bool externalMetricIsType2 = false) const;

    /**
     * Calculate the AS External Routes from the ASExternalLSAs in the database.
     * @param newRoutingTable [in/out] Push the new RoutingTableEntries into this
     *                                 routing table, and also use this for path
     *                                 calculations.
     * @sa RFC2328 Section 16.4.
     */
    void calculateASExternalRoutes(std::vector<Ospfv2RoutingTableEntry *>& newRoutingTable);

    /**
     * After a routing table rebuild the changes in the routing table are
     * identified and new SummaryLSAs are originated or old ones are flooded out
     * in each area as necessary.
     * @param oldRoutingTable [in] The previous version of the routing table(which
     *                             is then compared with the one in routingTable).
     * @sa RFC2328 Section 12.4. points(5) through(6).
     */
    void notifyAboutRoutingTableChanges(std::vector<Ospfv2RoutingTableEntry *>& oldRoutingTable);

    /**
     * Returns true if there is a route to the AS Boundary Router identified by
     * asbrRouterID in the input inRoutingTable, false otherwise.
     * @param inRoutingTable [in] The routing table to look in.
     * @param asbrRouterID   [in] The ID of the AS Boundary Router to look for.
     */
    bool hasRouteToASBoundaryRouter(const std::vector<Ospfv2RoutingTableEntry *>& inRoutingTable, RouterId routerID) const;

    /**
     * Returns an std::vector of routes leading to the AS Boundary Router
     * identified by asbrRouterID from the input fromRoutingTable. If there are no
     * routes leading to the AS Boundary Router, the returned std::vector is empty.
     * @param fromRoutingTable [in] The routing table to look in.
     * @param asbrRouterID     [in] The ID of the AS Boundary Router to look for.
     */
    std::vector<Ospfv2RoutingTableEntry *> getRoutesToASBoundaryRouter(const std::vector<Ospfv2RoutingTableEntry *>& fromRoutingTable, RouterId routerID) const;

    /**
     * Prunes the input std::vector of RoutingTableEntries according to the RFC2328
     * Section 16.4.1.
     * @param asbrEntries [in/out] The list of RoutingTableEntries to prune.
     * @sa RFC2328 Section 16.4.1.
     */
    void pruneASBoundaryRouterEntries(std::vector<Ospfv2RoutingTableEntry *>& asbrEntries) const;

    /**
     * Selects the least cost RoutingTableEntry from the input std::vector of
     * RoutingTableEntries.
     * @param entries [in] The RoutingTableEntries to choose the least cost one from.
     * @return The least cost entry or nullptr if entries is empty.
     */
    Ospfv2RoutingTableEntry *selectLeastCostRoutingEntry(std::vector<Ospfv2RoutingTableEntry *>& entries) const;

    void printAsExternalLsa();
    bool isDirectRoute(Ospfv2RoutingTableEntry& entry);
};

} // namespace ospfv2

} // namespace inet

#endif

