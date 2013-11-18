// Copyright (C) 2013 Brno University of Technology (http://nes.fit.vutbr.cz/ansa)
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
/**
 * @file AnsaRoutingTable.cc
 * @date 25.1.2013
 * @author Tomas Prochazka (mailto:xproch21@stud.fit.vutbr.cz), Vladimir Vesely (mailto:ivesely@fit.vutbr.cz)
 * @brief Extended RoutingTable with new features for PIM
 */

#include <algorithm>

#include "PIMRoutingTable.h"
#include "IPv4InterfaceData.h"

Define_Module(PIMRoutingTable);

/** Defined in RoutingTable.cc */
std::ostream& operator<<(std::ostream& os, const IPv4Route& e);

/** Printout of structure PIMMulticastRoute (one route in table). */
std::ostream& operator<<(std::ostream& os, const PIMMulticastRoute& e)
{
    os << e.info();
    return os;
};

/**
 * UPDATE DISPLAY STRING
 *
 * Update string under multicast table icon - number of multicast routes.
 */
void PIMRoutingTable::updateDisplayString()
{
    if (!ev.isGUI())
        return;

    char buf[80];
    sprintf(buf, "%d routes\n%d multicast routes", getNumRoutes(), getNumMulticastRoutes());

    getDisplayString().setTagArg("t", 0, buf);
}

/**
 * ADD ROUTE
 *
 * Function check new multicast table entry and then add new entry to multicast table.
 *
 * @param entry New entry about new multicast group.
 * @see MulticastIPRoute
 * @see updateDisplayString()
 */
void PIMRoutingTable::addMulticastRoute(PIMMulticastRoute *entry)
{
    Enter_Method("addMulticastRoute(...)");

    // check for null multicast group address
    if (entry->getMulticastGroup().isUnspecified())
        error("addMulticastRoute(): multicast group address cannot be NULL");

    // check for source or RP address
    if (entry->getOrigin().isUnspecified() && entry->getRP().isUnspecified())
        error("addMulticastRoute(): source or RP address has to be specified");

    // check that the incoming interface exists
    //FIXME for PIM-SM is needed unspecified next hop (0.0.0.0)
    //if (!entry->getInIntPtr() || entry->getInIntNextHop().isUnspecified())
        //error("addMulticastRoute(): incoming interface has to be specified");
    //if (!entry->getInIntPtr())
        //error("addMulticastRoute(): incoming interface has to be specified");

    // add to tables
    internalAddMulticastRoute(entry);

    updateDisplayString();
}



/**
 * DELETE ROUTE
 *
 * Function deletes a multicast route if it is found in the multicast routing table.
 *
 * @param entry Multicast entry which should be deleted from multicast table.
 * @return False if entry was not found in table. True if entry was deleted.
 * @see MulticastIPRoute
 * @see updateDisplayString()
 */
bool PIMRoutingTable::deleteMulticastRoute(PIMMulticastRoute *entry)
{
    Enter_Method("deleteMulticastRoute(...)");

    // if entry was found, it can be deleted
    if (internalRemoveMulticastRoute(entry))
    {
        // first delete all timers assigned to route
        cancelAndDelete(entry->getSrt());
        cancelAndDelete(entry->getGrt());
        cancelAndDelete(entry->getSat());
        cancelAndDelete(entry->getKat());
        cancelAndDelete(entry->getEt());
        cancelAndDelete(entry->getJt());
        cancelAndDelete(entry->getPpt());
        // delete timers from outgoing interfaces
        for (unsigned int j = 0;j < entry->getNumOutInterfaces(); j++)
            cancelAndDelete(entry->getAnsaOutInterface(j)->pruneTimer);

        // delete route
        delete entry;
        updateDisplayString();
        return true;
    }
    return false;
}

/**
 * GET ROUTE FOR
 *
 * The method returns one route from multicast routing table for given group and source IP addresses.
 *
 * @param group IP address of multicast group.
 * @param source IP address of multicast source.
 * @return Pointer to found multicast route.
 * @see getRoute()
 */
PIMMulticastRoute *PIMRoutingTable::getRouteFor(IPv4Address group, IPv4Address source)
{
    Enter_Method("getMulticastRoutesFor(%x, %x)", group.getInt(), source.getInt()); // note: str().c_str() too slow here here
    EV << "MulticastRoutingTable::getRouteFor - group = " << group << ", source = " << source << endl;

    // search in multicast routing table
    PIMMulticastRoute *route = NULL;

    int n = getNumMulticastRoutes();
    int i;
    // go through all multicast routes
    for (i = 0; i < n; i++)
    {
        route = dynamic_cast<PIMMulticastRoute*>(getMulticastRoute(i));
        if (route && route->getMulticastGroup() == group && route->getOrigin() == source)
            break;
    }

    if (i == n)
        return NULL;
    return route;
}

/**
 * GET ROUTE FOR
 *
 * The method returns all routes from multicast routing table for given multicast group.
 *
 * @param group IP address of multicast group.
 * @return Vecotr of pointers to routes in multicast table.
 * @see getRoute()
 */
std::vector<PIMMulticastRoute*> PIMRoutingTable::getRouteFor(IPv4Address group)
{
    Enter_Method("getMulticastRoutesFor(%x)", group.getInt()); // note: str().c_str() too slow here here
    EV << "MulticastRoutingTable::getRouteFor - address = " << group << endl;
    std::vector<PIMMulticastRoute*> routes;

    // search in multicast table
    int n = getNumMulticastRoutes();
    for (int i = 0; i < n; i++)
    {
        PIMMulticastRoute *route = dynamic_cast<PIMMulticastRoute*>(getMulticastRoute(i));
        if (route && route->getMulticastGroup() == group)
            routes.push_back(route);
    }

    return routes;
}

/**
 * GET ROUTES FOR SOURCES
 *
 * The method returns all routes from multicast routing table for given source.
 *
 * @param source IP address of multicast source.
 * @return Vector of found multicast routes.
 * @see getNetwork()
 */
std::vector<PIMMulticastRoute*> PIMRoutingTable::getRoutesForSource(IPv4Address source)
{
    Enter_Method("getRoutesForSource(%x)", source.getInt()); // note: str().c_str() too slow here here
    EV << "MulticastRoutingTable::getRoutesForSource - source = " << source << endl;
    std::vector<PIMMulticastRoute*> routes;

    // search in multicast table
    int n = getNumMulticastRoutes();
    int i;
    for (i = 0; i < n; i++)
    {
        //FIXME works only for classfull adresses (function getNetwork) !!!!
        PIMMulticastRoute *route = dynamic_cast<PIMMulticastRoute*>(getMulticastRoute(i));
        if (route && route->getOrigin().getNetwork().getInt() == source.getInt())
            routes.push_back(route);
    }
    return routes;
}
