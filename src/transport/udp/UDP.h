//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
// Copyright (C) 2004,2005 Andras Varga
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


//
// Author: Jochen Reber
// Rewrite: Andras Varga 2004,2005
//

#ifndef __INET_UDP_H
#define __INET_UDP_H

#include <map>
#include <list>
#include "UDPControlInfo_m.h"

class IPControlInfo;
class IPv6ControlInfo;
class ICMP;
class ICMPv6;
class UDPPacket;

const int UDP_HEADER_BYTES = 8;


/**
 * Implements the UDP protocol: encapsulates/decapsulates user data into/from UDP.
 *
 * More info in the NED file.
 */
class INET_API UDP : public cSimpleModule
{
  public:
    struct SockDesc
    {
        int sockId; // supposed to be unique across apps
        int userId; // we just send it back, but don't do anything with it
        int appGateIndex;
        bool onlyLocalPortIsSet;
        IPvXAddress localAddr;
        IPvXAddress remoteAddr;
        ushort localPort;
        ushort remotePort;
        int interfaceId; // FIXME do real sockets allow filtering by input interface??
    };

    typedef std::list<SockDesc *> SockDescList;
    typedef std::map<int,SockDesc *> SocketsByIdMap;
    typedef std::map<int,SockDescList> SocketsByPortMap;

  protected:
    // sockets
    SocketsByIdMap socketsByIdMap;
    SocketsByPortMap socketsByPortMap;

    // other state vars
    ushort lastEphemeralPort;
    ICMP *icmp;
    ICMPv6 *icmpv6;

    // statistics
    int numSent;
    int numPassedUp;
    int numDroppedWrongPort;
    int numDroppedBadChecksum;

  protected:
    // utility: show current statistics above the icon
    virtual void updateDisplayString();

    // bind socket
    virtual void bind(int gateIndex, UDPControlInfo *ctrl);

    // connect socket
    virtual void connect(int sockId, IPvXAddress addr, int port);

    // unbind socket
    virtual void unbind(int sockId);

    // ephemeral port
    virtual ushort getEphemeralPort();

    virtual bool matchesSocket(SockDesc *sd, UDPPacket *udp, IPControlInfo *ctrl);
    virtual bool matchesSocket(SockDesc *sd, UDPPacket *udp, IPv6ControlInfo *ctrl);
    virtual bool matchesSocket(SockDesc *sd, const IPvXAddress& localAddr, const IPvXAddress& remoteAddr, ushort remotePort);
    virtual void sendUp(cPacket *payload, UDPPacket *udpHeader, IPControlInfo *ctrl, SockDesc *sd);
    virtual void sendUp(cPacket *payload, UDPPacket *udpHeader, IPv6ControlInfo *ctrl, SockDesc *sd);
    virtual void processUndeliverablePacket(UDPPacket *udpPacket, cPolymorphic *ctrl);
    virtual void sendUpErrorNotification(SockDesc *sd, int msgkind, const IPvXAddress& localAddr, const IPvXAddress& remoteAddr, ushort remotePort);

    // process an ICMP error packet
    virtual void processICMPError(cPacket *icmpErrorMsg); // TODO use ICMPMessage

    // process UDP packets coming from IP
    virtual void processUDPPacket(UDPPacket *udpPacket);

    // process packets from application
    virtual void processMsgFromApp(cPacket *appData);

    // process commands from application
    virtual void processCommandFromApp(cMessage *msg);

    // create a blank UDP packet; override to subclass UDPPacket
    virtual UDPPacket *createUDPPacket(const char *name);

  public:
    UDP() {}
    virtual ~UDP();

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
};

#endif

