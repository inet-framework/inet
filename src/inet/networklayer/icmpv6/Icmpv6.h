//
// Copyright (C) 2005 OpenSim Ltd.
// Copyright (C) 2005 Wei Yang, Ng
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_ICMPV6_H
#define __INET_ICMPV6_H

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/lifecycle/OperationalBase.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/packet/Message.h"
#include "inet/common/packet/Packet.h"
#include "inet/networklayer/common/L3Address.h"
#include "inet/networklayer/contract/INetfilter.h"
#include "inet/networklayer/icmpv6/Icmpv6Header_m.h"
#include "inet/common/checksum/ChecksumMode_m.h"

namespace inet {

// foreign declarations:
class Ipv6Address;
class Ipv6Header;
class PingPayload;

/**
 * ICMPv6 implementation.
 */
class INET_API Icmpv6 : public OperationalBase, public DefaultProtocolRegistrationListener, public NetfilterBase::HookBase
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
    /**
     * Sends an ICMPv6 error message about the given datagram. mtu is the MTU of the
     * next-hop link and is only read for ICMPv6_PACKET_TOO_BIG, whose MTU field RFC 4443
     * section 3.2 requires; every other type ignores it.
     */
    virtual void sendErrorMessage(Packet *datagram, Icmpv6Type type, int code, int mtu = 0);

    /**
     * Checks the checksum of a received ICMPv6 message. The source and destination
     * addresses that RFC 4443 Section 2.3 puts into the pseudo-header are taken from the
     * packet's L3AddressInd tag; a packet that has no such tag, as one being dissected on
     * its own does, is accepted unless its checksum is declared incorrect.
     */
    static bool verifyChecksum(const Packet *packet);

  protected:
    // internal helper functions
    virtual void sendToIP(Packet *msg, const Ipv6Address& dest);
    virtual void sendToIP(Packet *msg); // FIXME check if really needed

    virtual Packet *createDestUnreachableMsg(Icmpv6DestUnav code);
    virtual Packet *createPacketTooBigMsg(int mtu);
    virtual Packet *createTimeExceededMsg(Icmpv6TimeEx code);
    virtual Packet *createParamProblemMsg(Icmpv6ParameterProblem code); // TODOSection 3.4 describes a pointer. What is it?

  protected:
    /**
     * Initialization
     */
    virtual void initialize(int stage) override;

    /**
     *  Processing of messages that arrive in this module. Messages arrived here
     *  could be for ICMP ping requests or ICMPv6 messages that require processing.
     */
    virtual void handleMessageWhenUp(cMessage *msg) override;

    // lifecycle:
    virtual bool isInitializeStage(int stage) const override { return stage == INITSTAGE_NETWORK_LAYER; }
    virtual bool isModuleStartStage(int stage) const override { return stage == ModuleStartOperation::STAGE_NETWORK_LAYER; }
    virtual bool isModuleStopStage(int stage) const override { return stage == ModuleStopOperation::STAGE_NETWORK_LAYER; }
    virtual void handleStartOperation(LifecycleOperation *operation) override {}
    virtual void handleStopOperation(LifecycleOperation *operation) override {}
    virtual void handleCrashOperation(LifecycleOperation *operation) override {}
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

    virtual void errorOut(Indication *indication);

    virtual void handleRegisterService(const Protocol& protocol, cGate *gate, ServicePrimitive servicePrimitive) override;
    virtual void handleRegisterProtocol(const Protocol& protocol, cGate *gate, ServicePrimitive servicePrimitive) override;

  public:
    /**
     * Sets the checksum mode on the header and, for the declared modes, the recognizable
     * placeholder value. In CHECKSUM_COMPUTED mode it only zeroes the field: the RFC 4443
     * checksum covers an IPv6 pseudo-header, so it cannot be computed before Ipv6 has
     * chosen the source address. The value is filled in by datagramPostRoutingHook().
     */
    static void insertChecksum(ChecksumMode checksumMode, const Ptr<Icmpv6Header>& icmpHeader, Packet *packet);
    void insertChecksum(const Ptr<Icmpv6Header>& icmpHeader, Packet *packet) { insertChecksum(checksumMode, icmpHeader, packet); }

    /**
     * Sets the checksum on a message whose source and destination addresses are already
     * known, computing it over the RFC 4443 pseudo-header in CHECKSUM_COMPUTED mode.
     */
    static void insertChecksum(ChecksumMode checksumMode, const L3Address& srcAddress, const L3Address& destAddress, const Ptr<Icmpv6Header>& icmpHeader, Packet *packet);

    // Fills in the checksum of an outgoing ICMPv6 message, at the point where the source
    // and destination addresses the datagram is sent with are settled.
    virtual Result datagramPreRoutingHook(Packet *packet) override { return ACCEPT; }
    virtual Result datagramForwardHook(Packet *packet) override { return ACCEPT; }
    virtual Result datagramPostRoutingHook(Packet *packet) override;
    virtual Result datagramLocalInHook(Packet *packet) override { return ACCEPT; }
    virtual Result datagramLocalOutHook(Packet *packet) override { return ACCEPT; }

  protected:
    /**
     * Computes the RFC 4443 Section 2.3 checksum: the one's complement sum over an IPv6
     * pseudo-header (source address, destination address, upper-layer packet length, next
     * header 58) followed by the ICMPv6 message, whose checksum field must be zero.
     */
    static uint16_t computeChecksum(const L3Address& srcAddress, const L3Address& destAddress, const Ptr<const Icmpv6Header>& icmpHeader, const Ptr<const Chunk>& icmpData);

    ChecksumMode checksumMode = CHECKSUM_MODE_UNDEFINED;
    typedef std::map<long, int> PingMap;
    PingMap pingMap;
    std::set<int> transportProtocols; // where to send up packets
    int numEchoReplied = 0;
    long numErrorsSent = 0;
    long numErrorsReceived = 0;
};

} // namespace inet

#endif

