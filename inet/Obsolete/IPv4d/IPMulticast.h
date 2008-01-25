//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
// Copyright (C) 2004 Andras Varga
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


#ifndef __IPMULTICAST_H__
#define __IPMULTICAST_H__

//  Cleanup and rewrite: Andras Varga, 2004

#include "IPDatagram.h"

class RoutingTable;
class InterfaceTable;

/**
 * Receive datagram with multicast address, duplicate it if it is sent
 * to more than one output ports, then send it to appropriate direction
 * using the multicast routing table.
 * More detailed info in the NED file.
 */
class INET_API IPMulticast : public cSimpleModule
{
  private:
    InterfaceTable *ift;
    RoutingTable *rt;

  public:
    Module_Class_Members(IPMulticast, cSimpleModule, 0);

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
};

#endif

