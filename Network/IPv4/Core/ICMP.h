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


#ifndef __ICMP_H__
#define __ICMP_H__

#include "QueueBase.h"
#include "RoutingTableAccess.h"
#include "ICMPMessage.h"
#include "IPDatagram.h"


/**
 * ICMP module.
 */
class ICMP : public QueueBase
{
  protected:
    RoutingTableAccess routingTableAccess;

    //void processError (cMessage *);
    void processICMPMessage(IPInterfacePacket *);
    void errorOut(ICMPMessage *);
    void recEchoRequest (ICMPMessage *, const IPAddress& dest);
    void recEchoReply (ICMPMessage *);
    void sendEchoRequest(cMessage *);
    void sendInterfacePacket(ICMPMessage *, const IPAddress& dest);

  public:
    Module_Class_Members(ICMP, QueueBase, 0);

    /**
     * This method can be called from other modules to send an ICMP error packet.
     */
    void sendErrorMessage(IPDatagram *datagram, ICMPType type, ICMPCode code);

  protected:
    virtual void endService(cMessage *msg);

};

#endif

