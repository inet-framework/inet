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

#ifndef DYMO_ROUTINGTABLE_H
#define DYMO_ROUTINGTABLE_H

#include <omnetpp.h>
#include <vector>
#include "INETDefs.h"

#include "NotificationBoard.h"

#include "DYMO_RoutingEntry.h"

#include "IRoutingTable.h"

/**
  * class describes the functionality of the routing table
**/
class DYMO_RoutingTable : public cObject
{
  public:
    DYMO_RoutingTable(cObject* host, const IPAddress& myAddr, const char* DYMO_INTERFACES, const IPAddress& LL_MANET_ROUTERS) {DYMO_RoutingTable(host,myAddr);}
    DYMO_RoutingTable(cObject* host, const IPAddress& myAddr);
    virtual ~DYMO_RoutingTable();

    /** @brief inherited from cObject */
    virtual const char* getFullName() const;

    /** @brief inherited from cObject */
    virtual std::string info() const;

    /** @brief inherited from cObject */
    virtual std::string detailedInfo() const;

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
    void deleteRoute (DYMO_RoutingEntry *entry);
    /** @removes invalid routes from the network layer routing table **/
    void maintainAssociatedRoutingTable ();
    /** @searchs an entry (exact match) and gives back a pointer to it, or 0 if none is found **/
    DYMO_RoutingEntry* getByAddress(IPAddress addr);
    /** @searchs an entry (longest-prefix match) and gives back a pointer to it, or 0 if none is found **/
    DYMO_RoutingEntry* getForAddress(IPAddress addr);
    /** @returns the routing table **/
    std::vector<DYMO_RoutingEntry *> getRoutingTable();

  private:
    typedef std::vector<DYMO_RoutingEntry *> RouteVector;
    RouteVector routeVector;
    cObject * dymoProcess;
    /**
     * add or delete network layer routing table entry for given DYMO routing table entry, based on whether it's valid
     */
    void maintainAssociatedRoutingEntryFor(DYMO_RoutingEntry* entry);

  public:
    friend std::ostream& operator<<(std::ostream& os, const DYMO_RoutingTable& o);

};

#endif
