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

#ifndef __INET_DYMO_ROUTINGENTRY_H
#define __INET_DYMO_ROUTINGENTRY_H

#include <string.h>
#include <sstream>

#include "inet/common/INETDefs.h"

#include "inet/networklayer/contract/ipv4/IPv4Address.h"
#include "inet/networklayer/common/InterfaceTable.h"
#include "inet/networklayer/ipv4/IPv4RoutingTable.h"
#include "inet/routing/extras/dymo_fau/DYMO_Timer.h"

namespace inet {

namespace inetmanet {

class DYMOFau;

/**
 * DYMO Route Table Entry
 */
class DYMO_RoutingEntry
{
  public:
    DYMO_RoutingEntry(DYMOFau* dymo);
    virtual ~DYMO_RoutingEntry();

    /**
     * @name DYMO Mandatory Fields
     */
    /*@{*/
    IPv4Address routeAddress; /**< The IPv4 destination address of the getNode(s) associated with the routing table entry. */
    unsigned int routeSeqNum = 0; /**< The DYMO SeqNum associated with this routing information. */
    IPv4Address routeNextHopAddress; /**< The IPv4 address of the next DYMO router on the path toward the Route.Address. */
    InterfaceEntry* routeNextHopInterface = nullptr; /**< The interface used to send packets toward the Route.Address. */
    bool routeBroken = false; /**< A flag indicating whether this Route is broken.  This flag is set if the next hop becomes unreachable or in response to processing a RERR (see Section 5.5.4). */
    /*@}*/

    /**
     * @name DYMO Optional Fields
     */
    /*@{*/
    unsigned int routeDist = 0; /**< A metric indicating the distance traversed before reaching the Route.Address node. */
    int routePrefix = 0; /**< Indicates that the associated address is a network address, rather than a host address.  The value is the length of the netmask/prefix.  If an address block does not have an associated PREFIX_LENGTH TLV [I-D.ietf-manet-packetbb], the prefix may be considered to have a prefix length equal to the address length (in bits). */
    /*@}*/

    /**
     * @name DYMO Timers
     * Each set to the simulation time at which it is meant to be considered expired or -1 if it's not running
     */
    /*@{*/
    DYMO_Timer routeAgeMin; /**< Minimum Delete Timeout. After updating a route table entry, it should be maintained for at least ROUTE_AGE_MIN */
    DYMO_Timer routeAgeMax; /**< After the ROUTE_AGE_MAX timeout a route must be deleted. */
    DYMO_Timer routeNew; /**< After the ROUTE_NEW timeout if the route has not been used, a timer for deleting the route (ROUTE_DELETE) is set to ROUTE_DELETE_TIMEOUT. */
    DYMO_Timer routeUsed; /**< When a route is used to forward data packets, this timer is set to expire after ROUTE_USED_TIMEOUT.  This operation is also discussed in Section 5.5.2. */
    DYMO_Timer routeDelete; /**< After the ROUTE_DELETE timeout, the routing table entry should be deleted. */
    /*@}*/

  protected:
    DYMOFau* dymo = nullptr; /**< DYMO module */

  public:
    friend std::ostream& operator<<(std::ostream& os, const DYMO_RoutingEntry& e);
    bool hasActiveTimer() { return routeAgeMin.isActive() || routeAgeMax.isActive() || routeNew.isActive() || routeUsed.isActive() || routeDelete.isActive(); }
};

} // namespace inetmanet

} // namespace inet

#endif

