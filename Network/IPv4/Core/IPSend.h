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


#ifndef __IPSENDCORE_H__
#define __IPSENDCORE_H__

#include "QueueWithQoS.h"
#include "RoutingTableAccess.h"
#include "IPInterfacePacket.h"
#include "IPDatagram.h"


/**
 * Encapsulate packet in an IP datagram.
 */
class IPSend : public QueueWithQoS
{
  private:
    RoutingTableAccess routingTableAccess;

    int defaultTimeToLive;
    int defaultMCTimeToLive;
    // type and value of curFragmentId is
    // just incremented per datagram sent out;
    //  Transport layer cannot give values
    long curFragmentId;

    void sendDatagram(IPInterfacePacket *);

  public:
    Module_Class_Members(IPSend, QueueWithQoS, 0);

  protected:
    IPDatagram *encapsulate(cMessage *msg);

    virtual void initialize();
    virtual void arrival(cMessage *msg);
    virtual cMessage *arrivalWhenIdle(cMessage *msg);
    virtual void endService(cMessage *msg);
};

#endif

