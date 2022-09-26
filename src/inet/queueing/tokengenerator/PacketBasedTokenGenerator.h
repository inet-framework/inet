//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PACKETBASEDTOKENGENERATOR_H
#define __INET_PACKETBASEDTOKENGENERATOR_H

#include "inet/common/ModuleRefByPar.h"
#include "inet/queueing/base/PassivePacketSinkBase.h"
#include "inet/queueing/contract/ITokenStorage.h"

namespace inet {
namespace queueing {

class INET_API PacketBasedTokenGenerator : public PassivePacketSinkBase, public cListener
{
  protected:
    cPar *numTokensPerPacketParameter = nullptr;
    cPar *numTokensPerBitParameter = nullptr;

    cGate *inputGate = nullptr;
    opp_component_ptr<IActivePacketSource> producer;
    ModuleRefByPar<ITokenStorage> storage;

    int numTokensGenerated = -1;

  protected:
    virtual void initialize(int stage) override;

  public:
    virtual bool supportsPacketPushing(cGate *gate) const override { return true; }
    virtual bool supportsPacketPulling(cGate *gate) const override { return false; }

    virtual bool canPushSomePacket(cGate *gate) const override { return storage->getNumTokens() == 0; }
    virtual bool canPushPacket(Packet *packet, cGate *gate) const override { return storage->getNumTokens() == 0; }
    virtual void pushPacket(Packet *packet, cGate *gate) override;

    virtual std::string resolveDirective(char directive) const override;
    virtual void receiveSignal(cComponent *source, simsignal_t signal, double value, cObject *details) override;
};

} // namespace queueing
} // namespace inet

#endif

