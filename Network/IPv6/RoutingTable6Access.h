//
// Copyright (C) 2005 Wei Yang, Ng
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
//

#ifndef __ROUTING_TABLE6_ACCESS_H__
#define __ROUTING_TABLE6_ACCESS_H__

#include <omnetpp.h>
#include "ModuleAccess.h"
#include "RoutingTable6.h"


/**
 * Gives access to RoutingTable6
 */
class INET_API RoutingTable6Access : public ModuleAccess<RoutingTable6>
{
    public:
        RoutingTable6Access() : ModuleAccess<RoutingTable6>("routingTable6") {}
};

#endif


