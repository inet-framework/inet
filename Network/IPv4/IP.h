//
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

#ifndef __IP_H__
#define __IP_H__


#include "RoutingTableAccess.h"
#include "IPControlInfo_m.h"
#include "IPDatagram.h"


/**
 * Implements the IP protocol.
 */
class IP : public cSimpleModule
{
  private:
    RoutingTableAccess routingTableAccess;

    int defaultTimeToLive;
    int defaultMCTimeToLive;
    long curFragmentId;  // counter, used to assign unique fragmentIds to datagrams

  public:
    Module_Class_Members(IP, cSimpleModule, 0);

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    IPDatagram *encapsulate(cMessage *transportPacket);
};

#endif

