//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IEEE8021ADPOLICY_H
#define __INET_IEEE8021ADPOLICY_H

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ModuleRefByPar.h"
#include "inet/linklayer/common/VlanTagType.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/queueing/base/PacketProcessorBase.h"
#include "inet/queueing/contract/IPassivePacketSink.h"

namespace inet {

using namespace inet::queueing;

class INET_API Ieee8021adPolicy : public PacketProcessorBase, public virtual IPassivePacketSink, public TransparentProtocolRegistrationListener, public cListener
{
  public:
    enum PortType {
        PORT_TYPE_ACCESS,
        PORT_TYPE_TRUNK,
        PORT_TYPE_CNP,
        PORT_TYPE_PNP,
    };

    struct Port {
        PortType type = PORT_TYPE_ACCESS;
        std::set<int> vlanIds; // The single VLAN id of an access or cnp port, the VLAN ids of a trunk or pnp port
    };

  protected:
    VlanTagType vlanTagType = VLAN_TAG_TYPE_C;
    const Protocol *vlanTagProtocol = nullptr;
    ModuleRefByPar<IInterfaceTable> interfaceTable;
    std::map<std::string, Port> ports; // interface name or "*" -> port

    cGate *upperLayerInGate = nullptr;
    cGate *upperLayerOutGate = nullptr;
    cGate *lowerLayerInGate = nullptr;
    cGate *lowerLayerOutGate = nullptr;
    PassivePacketSinkRef upperLayerSink;
    PassivePacketSinkRef lowerLayerSink;

    int numDroppedPackets = 0;

  protected:
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *message) override;

    virtual cGate *getRegistrationForwardingGate(cGate *gate) override;

    virtual Port parsePort(const char *interfaceName, const cValue& value) const;
    virtual void checkPort(const char *interfaceName, const Port& port) const;
    virtual void checkInterfaces() const;
    virtual const Port& getPort(const NetworkInterface *networkInterface) const;

    virtual void processReceivedPacket(Packet *packet);
    virtual void processSentPacket(Packet *packet);
    virtual void dropNotAcceptedPacket(Packet *packet, const char *reason);

    static bool isUntaggedPort(PortType type) { return type == PORT_TYPE_ACCESS || type == PORT_TYPE_CNP; }

    virtual void receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details) override;

  public:
    /**
     * Replaces the port of the interface, which must be in the interface
     * table of this module: its type and its VLAN ids, one for an access or
     * cnp port, one or more for a trunk or pnp port.
     */
    void setPort(const NetworkInterface *networkInterface, PortType type, const std::set<int>& vlanIds);

    /**
     * Removes the port of the interface, so that the entry of "*" applies to
     * it, or, without such an entry, a frame of the interface is an error.
     */
    void removePort(const NetworkInterface *networkInterface);

    virtual bool supportsPacketPushing(const cGate *gate) const override { return true; }
    virtual bool supportsPacketPulling(const cGate *gate) const override { return false; }

    virtual bool canPushSomePacket(const cGate *gate) const override { return true; }
    virtual bool canPushPacket(Packet *packet, const cGate *gate) const override { return true; }
    virtual void pushPacket(Packet *packet, const cGate *gate) override;

    virtual void pushPacketStart(Packet *packet, const cGate *gate, bps datarate) override { throw cRuntimeError("Packet streaming is not supported"); }
    virtual void pushPacketEnd(Packet *packet, const cGate *gate) override { throw cRuntimeError("Packet streaming is not supported"); }
    virtual void pushPacketProgress(Packet *packet, const cGate *gate, bps datarate, b position, b extraProcessableLength = b(0)) override { throw cRuntimeError("Packet streaming is not supported"); }
};

INET_API std::ostream& operator<<(std::ostream& os, const Ieee8021adPolicy::Port& port);

} // namespace inet

#endif
