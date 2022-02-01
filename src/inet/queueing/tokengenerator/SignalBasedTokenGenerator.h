//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_SIGNALBASEDTOKENGENERATOR_H
#define __INET_SIGNALBASEDTOKENGENERATOR_H

#include "inet/queueing/base/TokenGeneratorBase.h"

namespace inet {
namespace queueing {

class INET_API SignalBasedTokenGenerator : public TokenGeneratorBase, public cListener
{
  protected:
    cPar *numTokensParameter = nullptr;

  protected:
    virtual void initialize(int stage) override;
    virtual void generateTokens();

  public:
    virtual bool supportsPacketPushing(cGate *gate) const override { return false; }
    virtual bool supportsPacketPulling(cGate *gate) const override { return false; }

    virtual void receiveSignal(cComponent *source, simsignal_t signal, intval_t value, cObject *details) override;
    virtual void receiveSignal(cComponent *source, simsignal_t signal, double value, cObject *details) override;
    virtual void receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details) override;
};

} // namespace queueing
} // namespace inet

#endif

