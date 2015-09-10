/*
 *  Copyright (C) 2005 Mohamed Louizi
 *  Copyright (C) 2006,2007 Christoph Sommer <christoph.sommer@informatik.uni-erlangen.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef __INET_DYMO_ROUTINGTABLE_H
#define __INET_DYMO_ROUTINGTABLE_H

#include <vector>

#include "inet/common/INETDefs.h"

#include "inet/routing/extras/dymo_fau/DYMO_RoutingEntry.h"
#include "inet/networklayer/ipv4/IIPv4RoutingTable.h"

namespace inet {

namespace inetmanet {

/**
  * class describes the functionality of the routing table
**/
class DYMO_RoutingTable : public cObject
{
  public:
    DYMO_RoutingTable(DYMOFau* host, const IPv4Address& myAddr);
    virtual ~DYMO_RoutingTable();

    /** @brief inherited from cObject */
    virtual const char* getFullName() const override;

    /** @brief inherited from cObject */
    virtual std::string info() const override;

    /** @brief inherited from cObject */
    virtual std::string detailedInfo() const override;

    //-----------------------------------------------------------------------
    //Route table manupilation operations
    //-----------------------------------------------------------------------
    /** @returns the size of the table **/
    int getNumRoutes() const;
    /** @gets an routing entry at the given position **/
    DYMO_RoutingEntry* getRoute(int k);
    /** @adds a new entry to the table **/
    void addRoute(DYMO_RoutingEntry *entry);
    /** @deletes an entry from the table **/
    void deleteRoute(DYMO_RoutingEntry *entry);
    /** @removes invalid routes from the network layer routing table **/
    void maintainAssociatedRoutingTable();
    /** @searchs an entry (exact match) and gives back a pointer to it, or 0 if none is found **/
    DYMO_RoutingEntry* getByAddress(IPv4Address addr);
    /** @searchs an entry (longest-prefix match) and gives back a pointer to it, or 0 if none is found **/
    DYMO_RoutingEntry* getForAddress(IPv4Address addr);
    /** @returns the routing table **/
    std::vector<DYMO_RoutingEntry *> getRoutingTable();

  private:
    typedef std::vector<DYMO_RoutingEntry *> RouteVector;
    RouteVector routeVector;
    DYMOFau *dymoProcess;
    /**
     * add or delete network layer routing table entry for given DYMO routing table entry, based on whether it's valid
     */
    void maintainAssociatedRoutingEntryFor(DYMO_RoutingEntry* entry);

  public:
    friend std::ostream& operator<<(std::ostream& os, const DYMO_RoutingTable& o);

};

} // namespace inetmanet

} // namespace inet

#endif

