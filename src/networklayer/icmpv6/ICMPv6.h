//
// Copyright (C) 2005 Andras Varga
// Copyright (C) 2005 Wei Yang, Ng
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


#ifndef __ICMPv6_H__
#define __ICMPv6_H__

#include "INETDefs.h"

#include "ICMPv6Message_m.h"


//foreign declarations:
class IPv6Address;
class IPv6ControlInfo;
class IPv6Datagram;
class PingPayload;

/**
 * ICMPv6 implementation.
 */
class INET_API ICMPv6 : public cSimpleModule
{
  public:
    /**
     *  This method can be called from other modules to send an ICMPv6 error packet.
     *  RFC 2463, Section 3: ICMPv6 Error Messages
     *  There are a total of 4 ICMPv6 error messages as described in the RFC.
     *  This method will construct and send error messages corresponding to the
     *  given type.
     *  Error Types:
     *      - Destination Unreachable Message - 1
     *      - Packet Too Big Message          - 2
     *      - Time Exceeded Message           - 3
     *      - Parameter Problem Message       - 4
     *  Code Types have different semantics for each error type. See RFC 2463.
     */
    virtual void sendErrorMessage(IPv6Datagram *datagram, ICMPv6Type type, int code);

    /**
     * This method can be called from other modules to send an ICMP error packet
     * in response to a received bogus packet from the transport layer (like UDP).
     * The ICMP error packet needs to include (part of) the original IP datagram,
     * so this function will wrap back the transport packet into the IP datagram
     * based on its IPv4ControlInfo.
     */
    virtual void sendErrorMessage(cPacket *transportPacket, IPv6ControlInfo *ctrl, ICMPv6Type type, int code);

  protected:
    // internal helper functions
    virtual void sendToIP(ICMPv6Message *msg, const IPv6Address& dest);
    virtual void sendToIP(ICMPv6Message *msg); // FIXME check if really needed

    virtual ICMPv6Message *createDestUnreachableMsg(int code);
    virtual ICMPv6Message *createPacketTooBigMsg(int mtu);
    virtual ICMPv6Message *createTimeExceededMsg(int code);
    virtual ICMPv6Message *createParamProblemMsg(int code); //TODO:Section 3.4 describes a pointer. What is it?

  protected:
    /**
     * Initialization
     */
    virtual void initialize();

    /**
     *  Processing of messages that arrive in this module. Messages arrived here
     *  could be for ICMP ping requests or ICMPv6 messages that require processing.
     */
    virtual void handleMessage(cMessage *msg);
    virtual void processICMPv6Message(ICMPv6Message *);

    /**
     *  Respond to the machine that tried to ping us.
     */
    virtual void processEchoRequest(ICMPv6EchoRequestMsg *);

    /**
     *  Forward the ping reply to the "pingOut" of this module.
     */
    virtual void processEchoReply(ICMPv6EchoReplyMsg *);

    /**
     *  Ping a machine. The information needed to do this is in the cMessage
     *  parameter.  TODO where in cMessage? document!!!
     */
    virtual void sendEchoRequest(PingPayload *);

    /**
     * Validate the received IPv6 datagram before responding with error message.
     */
    virtual bool validateDatagramPromptingError(IPv6Datagram *datagram);

    virtual void errorOut(ICMPv6Message *);

  protected:
    typedef std::map<long,int> PingMap;
    PingMap pingMap;
};


#endif


