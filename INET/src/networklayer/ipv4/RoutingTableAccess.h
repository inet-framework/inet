//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
// Copyright (C) 2004 Andras Varga
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


#ifndef __INET_ROUTING_TABLE_ACCESS_H
#define __INET_ROUTING_TABLE_ACCESS_H

//  Cleanup and rewrite: Andras Varga, 2004

#include <omnetpp.h>
#include "ModuleAccess.h"
#include "IRoutingTable.h"


/**
 * Gives access to the IRoutingTable.
 */
class INET_API RoutingTableAccess : public ModuleAccess<IRoutingTable>
{
    public:
        RoutingTableAccess() : ModuleAccess<IRoutingTable>("routingTable") {}
};

#endif

