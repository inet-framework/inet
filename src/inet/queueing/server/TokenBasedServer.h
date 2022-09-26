//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_TOKENBASEDSERVER_H
#define __INET_TOKENBASEDSERVER_H

#include "inet/queueing/base/PacketServerBase.h"
#include "inet/queueing/contract/ITokenStorage.h"

namespace inet {
namespace queueing {

class INET_API TokenBasedServer : public PacketServerBase, public ITokenStorage
{
  protected:
    cPar *tokenConsumptionPerPacketParameter = nullptr;
    cPar *tokenConsumptionPerBitParameter = nullptr;
    double maxNumTokens = NaN;

    bool tokensDepletedSignaled = true;
    double numTokens = 0;

  protected:
    virtual void initialize(int stage) override;
    virtual void processPackets();

  public:
    virtual double getNumTokens() const override { return numTokens; }
    virtual void addTokens(double tokens) override;
    virtual void removeTokens(double tokens) override { throw cRuntimeError("TODO"); }
    virtual void addTokenProductionRate(double tokenRate) override { throw cRuntimeError("TODO"); }
    virtual void removeTokenProductionRate(double tokenRate) override { throw cRuntimeError("TODO"); }

    virtual void handleCanPushPacketChanged(cGate *gate) override;
    virtual void handleCanPullPacketChanged(cGate *gate) override;

    virtual std::string resolveDirective(char directive) const override;
};

} // namespace queueing
} // namespace inet

#endif

