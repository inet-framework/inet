//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_TOKENBUCKET_H
#define __INET_TOKENBUCKET_H

#include "inet/common/packet/Packet.h"
#include "inet/queueing/contract/ITokenStorage.h"

namespace inet {
namespace queueing {

class INET_API TokenBucket : public ITokenStorage
{
  protected:
    double numTokens = 0;
    double maxNumTokens = -1;
    double tokenProductionRate = 0;
    double excessTokenProductionRate = 0;
    ITokenStorage *excessTokenStorage = nullptr;

    simtime_t lastUpdate;

  protected:
    void updateNumTokens();

  public:
    TokenBucket() {}
    TokenBucket(double numTokens, double maxNumTokens, double tokenProductionRate, ITokenStorage *excessTokenStorage);

    virtual ITokenStorage *getExcessTokenStorage() const { return excessTokenStorage; }
    virtual double getNumTokens() const override;
    virtual void addTokens(double numTokens) override;
    virtual void removeTokens(double numTokens) override;
    virtual void addTokenProductionRate(double tokenRate) override;
    virtual void removeTokenProductionRate(double tokenRate) override;

    /**
     * Returns the simulation time when the token bucket becomes full with the
     * current rate of token production.
     */
    virtual simtime_t getOverflowTime();
};

inline std::ostream& operator<<(std::ostream& out, const TokenBucket& tokenBucket)
{
    return out << tokenBucket.getNumTokens();
}

} // namespace queueing
} // namespace inet

#endif

