//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_LOCALDELIVERY_H
#define __INET_LOCALDELIVERY_H

#include "inet/common/ModuleRefByPar.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/queueing/base/PacketProcessorBase.h"
#include "inet/queueing/common/PassivePacketSinkRef.h"
#include "inet/queueing/contract/IPassivePacketSink.h"

namespace inet {

using namespace queueing;

class INET_API LocalDelivery : public PacketProcessorBase, public IPassivePacketSink
{
  protected:
    PassivePacketSinkRef deliveryOutConsumer;
    PassivePacketSinkRef forwardingOutConsumer;

    ModuleRefByPar<IInterfaceTable> interfaceTable;

  protected:
    virtual void initialize(int stage) override;

  public:
    virtual bool supportsPacketPushing(const cGate *gate) const override { return true; }
    virtual bool supportsPacketPulling(const cGate *gate) const override { return false; }

    virtual bool canPushSomePacket(const cGate *gate) const override { return true; }
    virtual bool canPushPacket(Packet *packet, const cGate *gate) const override { return true; }

    virtual void pushPacket(Packet *packet, const cGate *gate) override;

    virtual void pushPacketStart(Packet *packet, const cGate *gate, bps datarate) override { throw cRuntimeError("Unsupported operation"); }
    virtual void pushPacketEnd(Packet *packet, const cGate *gate) override { throw cRuntimeError("Unsupported operation"); }
    virtual void pushPacketProgress(Packet *packet, const cGate *gate, bps datarate, b position, b extraProcessableLength = b(0)) override { throw cRuntimeError("Unsupported operation"); }
};

} // namespace inet

#endif

