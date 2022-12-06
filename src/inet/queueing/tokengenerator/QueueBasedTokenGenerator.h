//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_QUEUEBASEDTOKENGENERATOR_H
#define __INET_QUEUEBASEDTOKENGENERATOR_H

#include "inet/common/ModuleRefByPar.h"
#include "inet/queueing/base/TokenGeneratorBase.h"
#include "inet/queueing/contract/IPacketQueue.h"

namespace inet {
namespace queueing {

class INET_API QueueBasedTokenGenerator : public TokenGeneratorBase, public cListener
{
  protected:
    int minNumPackets = -1;
    b minTotalLength = b(-1);
    ModuleRefByPar<IPacketQueue> queue;
    cPar *numTokensParameter = nullptr;

  protected:
    virtual void initialize(int stage) override;

    virtual void generateTokens();

  public:
    virtual void receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details) override;
};

} // namespace queueing
} // namespace inet

#endif

