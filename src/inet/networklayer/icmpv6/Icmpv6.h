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

#ifndef __INET_ICMPV6_H
#define __INET_ICMPV6_H

#include "inet/common/INETDefs.h"
#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/lifecycle/LifecycleUnsupported.h"
#include "inet/common/packet/Packet.h"
#include "inet/networklayer/icmpv6/Icmpv6Header_m.h"
#include "inet/transportlayer/common/CrcMode_m.h"

namespace inet {

//foreign declarations:
class Ipv6Address;
class Ipv6Header;
class PingPayload;

/**
 * ICMPv6 implementation.
 */
class INET_API Icmpv6 : public cSimpleModule, public LifecycleUnsupported, public IProtocolRegistrationListener
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
    virtual void sendErrorMessage(Packet *datagram, Icmpv6Type type, int code);

    static bool verifyCrc(const Packet *packet);

  protected:
    // internal helper functions
    virtual void sendToIP(Packet *msg, const Ipv6Address& dest);
    virtual void sendToIP(Packet *msg);    // FIXME check if really needed

    virtual Packet *createDestUnreachableMsg(Icmpv6DestUnav code);
    virtual Packet *createPacketTooBigMsg(int mtu);
    virtual Packet *createTimeExceededMsg(Icmpv6TimeEx code);
    virtual Packet *createParamProblemMsg(Icmpv6ParameterProblem code);    //TODO:Section 3.4 describes a pointer. What is it?

  protected:
    /**
     * Initialization
     */
    virtual void initialize(int stage) override;
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }

    /**
     *  Processing of messages that arrive in this module. Messages arrived here
     *  could be for ICMP ping requests or ICMPv6 messages that require processing.
     */
    virtual void handleMessage(cMessage *msg) override;
    virtual void processICMPv6Message(Packet *packet);

    /**
     *  Respond to the machine that tried to ping us.
     */
    virtual void processEchoRequest(Packet *packet, const Ptr<const Icmpv6EchoRequestMsg>& header);

    /**
     *  Forward the ping reply to the "pingOut" of this module.
     */
    virtual void processEchoReply(Packet *packet, const Ptr<const Icmpv6EchoReplyMsg>& header);

    /**
     * Validate the received Ipv6 datagram before responding with error message.
     */
    virtual bool validateDatagramPromptingError(Packet *packet);

    virtual void errorOut(const Ptr<const Icmpv6Header>& header);

    virtual void handleRegisterService(const Protocol& protocol, cGate *out, ServicePrimitive servicePrimitive) override;
    virtual void handleRegisterProtocol(const Protocol& protocol, cGate *in, ServicePrimitive servicePrimitive) override;

  public:
    static void insertCrc(CrcMode crcMode, const Ptr<Icmpv6Header>& icmpHeader, Packet *packet);
    void insertCrc(const Ptr<Icmpv6Header>& icmpHeader, Packet *packet) { insertCrc(crcMode, icmpHeader, packet); }

  protected:
    CrcMode crcMode = CRC_MODE_UNDEFINED;
    typedef std::map<long, int> PingMap;
    PingMap pingMap;
    std::set<int> transportProtocols;    // where to send up packets
};

} // namespace inet

#endif // ifndef __INET_ICMPV6_H

