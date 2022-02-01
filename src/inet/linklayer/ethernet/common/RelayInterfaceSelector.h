//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_RELAYINTERFACESELECTOR_H
#define __INET_RELAYINTERFACESELECTOR_H

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ModuleRefByPar.h"
#include "inet/linklayer/ethernet/contract/IMacForwardingTable.h"
#include "inet/networklayer/common/NetworkInterface.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/queueing/base/PacketPusherBase.h"

namespace inet {

using namespace inet::queueing;

class INET_API RelayInterfaceSelector : public PacketPusherBase, public TransparentProtocolRegistrationListener
{
  protected:
    ModuleRefByPar<IInterfaceTable> interfaceTable;
    ModuleRefByPar<IMacForwardingTable> macForwardingTable;

    long numProcessedFrames = 0;
    long numDroppedFrames = 0;

  protected:
    virtual void initialize(int stage) override;
    virtual void pushPacket(Packet *packet, cGate *gate) override;

    virtual bool isForwardingInterface(NetworkInterface *networkInterface) const { return !networkInterface->isLoopback() && networkInterface->isBroadcast(); }
    virtual void broadcastPacket(Packet *packet, const MacAddress& destinationAddress, NetworkInterface *incomingInterface);
    virtual void sendPacket(Packet *packet, const MacAddress& destinationAddress, NetworkInterface *outgoingInterface);

    virtual bool isForwardingService(const Protocol& protocol, cGate *gate, ServicePrimitive servicePrimitive) const override { return servicePrimitive == SP_REQUEST; }
    virtual bool isForwardingServiceGroup(const ProtocolGroup& protocolGroup, cGate *gate, ServicePrimitive servicePrimitive) const override { return servicePrimitive == SP_REQUEST; }
    virtual bool isForwardingAnyService(cGate *gate, ServicePrimitive servicePrimitive) const override { return servicePrimitive == SP_REQUEST; }

    virtual bool isForwardingProtocol(const Protocol& protocol, cGate *gate, ServicePrimitive servicePrimitive) const override { return false; }
    virtual bool isForwardingProtocolGroup(const ProtocolGroup& protocolGroup, cGate *gate, ServicePrimitive servicePrimitive) const override { return false; }
    virtual bool isForwardingAnyProtocol(cGate *gate, ServicePrimitive servicePrimitive) const override { return false; }

    virtual cGate *getRegistrationForwardingGate(cGate *gate) override;
};

} // namespace inet

#endif

