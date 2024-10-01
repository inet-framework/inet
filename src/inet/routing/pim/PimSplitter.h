//
// Copyright (C) 2013 Brno University of Technology (http://nes.fit.vutbr.cz/ansa)
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

// Authors: Veronika Rybova, Vladimir Vesely (ivesely@fit.vutbr.cz),
//          Tamas Borbely (tomi@omnetpp.org)

#ifndef __INET_PIMSPLITTER_H
#define __INET_PIMSPLITTER_H

#include "inet/common/ModuleRefByPar.h"
#include "inet/common/packet/Packet.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/queueing/common/PassivePacketSinkRef.h"
#include "inet/routing/pim/PimPacket_m.h"
#include "inet/routing/pim/tables/PimInterfaceTable.h"

namespace inet {

using namespace inet::queueing;

/**
 * PimSplitter register itself for PIM protocol (103) in the network layer,
 * and dispatches the received packets either to PimDm or PimSm according
 * to the PIM mode of the incoming interface.
 * Packets received from the PIM modules are simply forwarded to the
 * network layer.
 */
class INET_API PimSplitter : public cSimpleModule, public IPassivePacketSink
{
  private:
    ModuleRefByPar<IInterfaceTable> ift;
    ModuleRefByPar<PimInterfaceTable> pimIft;
    PassivePacketSinkRef ipSink;
    PassivePacketSinkRef pimDMSink;
    PassivePacketSinkRef pimSMSink;

    cGate *ipIn = nullptr;
    cGate *ipOut = nullptr;
    cGate *pimDMIn = nullptr;
    cGate *pimDMOut = nullptr;
    cGate *pimSMIn = nullptr;
    cGate *pimSMOut = nullptr;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void processPIMPacket(Packet *pkt);

  public:
    virtual bool canPushSomePacket(const cGate *gate) const override { return gate->isName("ipIn") || gate->isName("pimDMIn") || gate->isName("pimSMIn"); }
    virtual bool canPushPacket(Packet *packet, const cGate *gate) const override { return gate->isName("ipIn") || gate->isName("pimDMIn") || gate->isName("pimSMIn"); }
    virtual void pushPacket(Packet *packet, const cGate *gate) override;
    virtual void pushPacketStart(Packet *packet, const cGate *gate, bps datarate) override { throw cRuntimeError("TODO"); }
    virtual void pushPacketEnd(Packet *packet, const cGate *gate) override { throw cRuntimeError("TODO"); }
    virtual void pushPacketProgress(Packet *packet, const cGate *gate, bps datarate, b position, b extraProcessableLength = b(0)) override { throw cRuntimeError("TODO"); }
};

} // namespace inet

#endif

