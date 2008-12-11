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


#ifndef __INET_ICMP_H
#define __INET_ICMP_H

//  Cleanup and rewrite: Andras Varga, 2004

#include "RoutingTableAccess.h"
#include "ICMPMessage.h"

class IPDatagram;
class IPControlInfo;


/**
 * ICMP module.
 */
class INET_API ICMP : public cSimpleModule
{
  protected:
    RoutingTableAccess routingTableAccess;

    virtual void processICMPMessage(ICMPMessage *);
    virtual void errorOut(ICMPMessage *);
    virtual void processEchoRequest (ICMPMessage *);
    virtual void processEchoReply (ICMPMessage *);
    virtual void sendEchoRequest(cPacket *);
    virtual void sendToIP(ICMPMessage *, const IPAddress& dest);
    virtual void sendToIP(ICMPMessage *msg);

  public:
    /**
     * This method can be called from other modules to send an ICMP error packet
     * in response to a received bogus packet.
     */
    virtual void sendErrorMessage(IPDatagram *datagram, ICMPType type, ICMPCode code);

    /**
     * This method can be called from other modules to send an ICMP error packet
     * in response to a received bogus packet from the transport layer (like UDP).
     * The ICMP error packet needs to include (part of) the original IP datagram,
     * so this function will wrap back the transport packet into the IP datagram
     * based on its IPControlInfo.
     */
    virtual void sendErrorMessage(cPacket *transportPacket, IPControlInfo *ctrl, ICMPType type, ICMPCode code);

  protected:
    virtual void handleMessage(cMessage *msg);

};

#endif

