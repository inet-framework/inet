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

#include <stdexcept>
#include <sstream>
#include <algorithm>
#include "DYMO_RoutingTable.h"
#include "DYMO.h"


DYMO_RoutingTable::DYMO_RoutingTable(DYMO* host, const IPv4Address& myAddr)
{
    // get our host module
    if (!host) throw cRuntimeError("No parent module found");

    dymoProcess = host;

    // get our routing table
    // routingTable = IPvXAddressResolver().routingTableOf(host);
    // if (!routingTable) throw cRuntimeError("No routing table found");

    // get our interface table
    // IInterfaceTable *ift = IPvXAddressResolver().interfaceTableOf(host);
    // look at all interface table entries
}

DYMO_RoutingTable::~DYMO_RoutingTable()
{
    while (!routeVector.empty())
    {
        delete routeVector.back();
        routeVector.pop_back();
    }
}

const char* DYMO_RoutingTable::getFullName() const
{
    return "DYMO_RoutingTable";
}

std::string DYMO_RoutingTable::info() const
{
    std::ostringstream ss;

    ss << getNumRoutes() << " entries";

    int broken = 0;
    for (RouteVector::const_iterator iter = routeVector.begin(); iter < routeVector.end(); iter++)
    {
        DYMO_RoutingEntry* e = *iter;
        if (e->routeBroken) broken++;
    }
    ss << " (" << broken << " broken)";

    ss << " {" << std::endl;
    for (RouteVector::const_iterator iter = routeVector.begin(); iter < routeVector.end(); iter++)
    {
        DYMO_RoutingEntry* e = *iter;
        ss << "  " << *e << std::endl;
    }
    ss << "}";

    return ss.str();
}

std::string DYMO_RoutingTable::detailedInfo() const
{
    return info();
}

//=================================================================================================
/*
 * Function returns the size of the table
 */
//=================================================================================================
int DYMO_RoutingTable::getNumRoutes() const
{
    return (int)routeVector.size();
}

//=================================================================================================
/*
 * Function gets an routing entry at the given position
 */
//=================================================================================================
DYMO_RoutingEntry* DYMO_RoutingTable::getRoute(int k)
{
    if (k < (int)routeVector.size())
        return routeVector[k];
    else
        return NULL;
}

//=================================================================================================
/*
 *
 */
//=================================================================================================
void DYMO_RoutingTable::addRoute(DYMO_RoutingEntry *entry)
{
    routeVector.push_back(entry);
}

//=================================================================================================
/*
 */
//=================================================================================================
void DYMO_RoutingTable::deleteRoute(DYMO_RoutingEntry *entry)
{

    // update standard routingTable
//  if (entry->routingEntry) {
//      routingTable->deleteRoute(entry->routingEntry);
//      entry->routingEntry = 0;
//  }

    // update DYMO routingTable
    RouteVector::iterator iter;
    for (iter = routeVector.begin(); iter < routeVector.end(); iter++)
    {
        if (entry == *iter)
        {
            routeVector.erase(iter);
            Uint128 dest(entry->routeAddress.getInt());
            dymoProcess->omnet_chg_rte(dest, dest, dest, 0, true);
            //updateDisplayString();
            delete entry;
            return;
        }
    }

    throw cRuntimeError("unknown routing entry requested to be deleted");
}

//=================================================================================================
/*
 */
//=================================================================================================
void DYMO_RoutingTable::maintainAssociatedRoutingTable()
{
    RouteVector::iterator iter;
    for (iter = routeVector.begin(); iter < routeVector.end(); iter++)
    {
        maintainAssociatedRoutingEntryFor(*iter);
    }
}

//=================================================================================================
/*
 */
//=================================================================================================
DYMO_RoutingEntry* DYMO_RoutingTable::getByAddress(IPv4Address addr)
{

    RouteVector::iterator iter;

    for (iter = routeVector.begin(); iter < routeVector.end(); iter++)
    {
        DYMO_RoutingEntry *entry = *iter;

        if (entry->routeAddress == addr)
        {
            return entry;
        }
    }

    return 0;
}

//=================================================================================================
/*
 */
//=================================================================================================
DYMO_RoutingEntry* DYMO_RoutingTable::getForAddress(IPv4Address addr)
{
    RouteVector::iterator iter;

    int longestPrefix = 0;
    DYMO_RoutingEntry* longestPrefixEntry = 0;
    for (iter = routeVector.begin(); iter < routeVector.end(); iter++)
    {
        DYMO_RoutingEntry *entry = *iter;

        // skip if we already have a more specific match
        if (!(entry->routePrefix > longestPrefix)) continue;

        // skip if address is not in routeAddress/routePrefix block
        if (!addr.prefixMatches(entry->routeAddress, entry->routePrefix)) continue;

        // we have a match
        longestPrefix = entry->routePrefix;
        longestPrefixEntry = entry;
    }

    return longestPrefixEntry;
}

//=================================================================================================
/*
 */
//=================================================================================================
DYMO_RoutingTable::RouteVector DYMO_RoutingTable::getRoutingTable()
{
    return routeVector;
}

void DYMO_RoutingTable::maintainAssociatedRoutingEntryFor(DYMO_RoutingEntry* entry)
{
    Uint128 dest(entry->routeAddress.getInt());
    if (!entry->routeBroken)
    {
        // entry is valid
        Uint128 mask(IPv4Address::ALLONES_ADDRESS.getInt());
        Uint128 gtw(entry->routeNextHopAddress.getInt());
        dymoProcess->setIpEntry(dest, gtw, mask, entry->routeDist);
    }
    else
    {
        dymoProcess->deleteIpEntry(dest);
    }
}

std::ostream& operator<<(std::ostream& os, const DYMO_RoutingTable& o)
{
    os << o.info();
    return os;
}

