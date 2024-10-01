//
// Copyright (C) 2004 OpenSim Ltd.
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_IPVXTRAFSINK_H
#define __INET_IPVXTRAFSINK_H

#include <vector>

#include "inet/applications/base/ApplicationBase.h"
#include "inet/common/IModuleInterfaceLookup.h"
#include "inet/common/Protocol.h"
#include "inet/common/packet/Packet.h"
#include "inet/networklayer/common/L3Address.h"
#include "inet/queueing/contract/IPassivePacketSink.h"

namespace inet {

using namespace inet::queueing;

/**
 * Consumes and prints packets received from the IP module. See NED for more info.
 */
class INET_API IpvxTrafSink : public ApplicationBase, public IPassivePacketSink, public IModuleInterfaceLookup
{
  protected:
    const Protocol *protocol = nullptr;

    int numReceived;

  protected:
    virtual void printPacket(Packet *msg);
    virtual void processPacket(Packet *msg);

    virtual void initialize(int stage) override;
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void handleMessageWhenUp(cMessage *msg) override;
    virtual void refreshDisplay() const override;

    virtual void handleStartOperation(LifecycleOperation *operation) override {}
    virtual void handleStopOperation(LifecycleOperation *operation) override {}
    virtual void handleCrashOperation(LifecycleOperation *operation) override {}

  public:
    virtual bool canPushSomePacket(const cGate *gate) const override { return gate->isName("ipIn"); }
    virtual bool canPushPacket(Packet *packet, const cGate *gate) const override { return gate->isName("ipIn"); }
    virtual void pushPacket(Packet *packet, const cGate *gate) override;
    virtual void pushPacketStart(Packet *packet, const cGate *gate, bps datarate) override { throw cRuntimeError("TODO"); }
    virtual void pushPacketEnd(Packet *packet, const cGate *gate) override { throw cRuntimeError("TODO"); }
    virtual void pushPacketProgress(Packet *packet, const cGate *gate, bps datarate, b position, b extraProcessableLength = b(0)) override { throw cRuntimeError("TODO"); }

    virtual cGate *lookupModuleInterface(cGate *gate, const std::type_info& type, const cObject *arguments, int direction) override;
};

} // namespace inet

#endif

