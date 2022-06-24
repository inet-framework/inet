//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_MESSAGEDISPATCHER_H
#define __INET_MESSAGEDISPATCHER_H

#include "inet/common/IInterfaceRegistrationListener.h"
#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/packet/Message.h"
#include "inet/common/packet/Packet.h"
#ifdef INET_WITH_QUEUEING
#include "inet/queueing/base/PacketProcessorBase.h"
#include "inet/queueing/contract/IActivePacketSource.h"
#include "inet/queueing/contract/IPassivePacketSink.h"
#endif // #ifdef INET_WITH_QUEUEING

namespace inet {

/**
 * This class implements the corresponding module. See module documentation for more details.
 */
class INET_API MessageDispatcher :
#ifdef INET_WITH_QUEUEING
    public queueing::PacketProcessorBase, public queueing::IActivePacketSource, public queueing::IPassivePacketSink,
#else
    public cSimpleModule,
#endif // #ifdef INET_WITH_QUEUEING
    public DefaultProtocolRegistrationListener, public IInterfaceRegistrationListener
{
  public:
    class INET_API Key {
      protected:
        int protocolId;
        int servicePrimitive;

      public:
        Key(int protocolId, ServicePrimitive servicePrimitive) : protocolId(protocolId), servicePrimitive(servicePrimitive) {}

        bool operator<(const MessageDispatcher::Key& other) const {
            if (protocolId < other.protocolId)
                return true;
            else if (protocolId > other.protocolId)
                return false;
            else
                return servicePrimitive < other.servicePrimitive;
        }

        friend std::ostream& operator<<(std::ostream& out, const MessageDispatcher::Key& foo);
    };

  protected:
    bool forwardServiceRegistration;
    bool forwardProtocolRegistration;

    std::map<int, int> socketIdToGateIndex;
    std::map<int, int> interfaceIdToGateIndex;
    std::map<Key, int> serviceToGateIndex;
    std::map<Key, int> protocolToGateIndex;
    const Protocol *registeringProtocol = nullptr;
    bool registeringAny = false;

  protected:
    virtual void initialize(int stage) override;
    virtual void arrived(cMessage *message, cGate *gate, const SendOptions& options, simtime_t time) override;
    virtual cGate *handlePacket(Packet *packet, cGate *inGate);
    virtual cGate *handleMessage(Message *request, cGate *inGate);

  public:
#ifdef INET_WITH_QUEUEING
    virtual bool supportsPacketSending(cGate *gate) const override { return true; }
    virtual bool supportsPacketPushing(cGate *gate) const override { return true; }
    virtual bool supportsPacketPulling(cGate *gate) const override { return false; }
    virtual bool supportsPacketPassing(cGate *gate) const override { return true; }
    virtual bool supportsPacketStreaming(cGate *gate) const override { return false; }

    virtual IPassivePacketSink *getConsumer(cGate *gate) override { throw cRuntimeError("Invalid operation"); }

    virtual bool canPushSomePacket(cGate *gate) const override;
    virtual bool canPushPacket(Packet *packet, cGate *gate) const override;

    virtual void pushPacket(Packet *packet, cGate *gate) override;
    virtual void pushPacketStart(Packet *packet, cGate *gate, bps datarate) override;
    virtual void pushPacketEnd(Packet *packet, cGate *gate) override;
    virtual void pushPacketProgress(Packet *packet, cGate *gate, bps datarate, b position, b extraProcessableLength = b(0)) override { throw cRuntimeError("Invalid operation"); }

    virtual void handleCanPushPacketChanged(cGate *gate) override;
    virtual void handlePushPacketProcessed(Packet *packet, cGate *gate, bool successful) override;
#endif // #ifdef INET_WITH_QUEUEING

    virtual void handleRegisterInterface(const NetworkInterface& interface, cGate *out, cGate *in) override;

    virtual void handleRegisterService(const Protocol& protocol, cGate *gate, ServicePrimitive servicePrimitive) override;
    virtual void handleRegisterAnyService(cGate *gate, ServicePrimitive servicePrimitive) override;

    virtual void handleRegisterProtocol(const Protocol& protocol, cGate *gate, ServicePrimitive servicePrimitive) override;
    virtual void handleRegisterAnyProtocol(cGate *gate, ServicePrimitive servicePrimitive) override;
};

std::ostream& operator<<(std::ostream& out, const MessageDispatcher::Key& foo) {
    out << "[" << foo.protocolId << ", " << omnetpp::cEnum::get("inet::ServicePrimitive")->getStringFor(foo.servicePrimitive) << "]";
    return out;
}

} // namespace inet

#endif

