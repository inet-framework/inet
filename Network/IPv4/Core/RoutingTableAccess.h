//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

#ifndef __RROUTING_TABLE_ACCESS_H__
#define __RROUTING_TABLE_ACCESS_H__

#include <omnetpp.h>
#include "RoutingTable.h"

/**
 * Gives access to the RoutingTable.
 */
class RoutingTableAccess
{
  private:
    RoutingTable *rt;

  public:
    RoutingTableAccess() {rt=NULL;}
    RoutingTable *get();
};

#endif

