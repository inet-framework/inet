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
class INET_API Icmpv6 : public OperationalBase, public DefaultProtocolRegistrationListener
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
    static void insertChecksum(ChecksumMode checksumMode, const Ptr<Icmpv6Header>& icmpHeader, Packet *packet);
    void insertChecksum(const Ptr<Icmpv6Header>& icmpHeader, Packet *packet) { insertChecksum(checksumMode, icmpHeader, packet); }

  protected:
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

