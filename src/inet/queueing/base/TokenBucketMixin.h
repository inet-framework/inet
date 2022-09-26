//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_TOKENBUCKETMIXIN_H
#define __INET_TOKENBUCKETMIXIN_H

#include "inet/common/ModuleAccess.h"
#include "inet/common/Simsignals.h"
#include "inet/queueing/common/TokenBucket.h"
#include "inet/queueing/contract/ITokenStorage.h"

namespace inet {
namespace queueing {

template<typename T>
class INET_API TokenBucketMixin : public T, public ITokenStorage
{
  protected:
    TokenBucket tokenBucket;

    cMessage *overflowTimer = nullptr;

  protected:
    virtual void initialize(int stage) override
    {
        T::initialize(stage);
        if (stage == INITSTAGE_LOCAL) {
            double numTokens = T::par("initialNumTokens");
            double maxNumTokens = T::par("maxNumTokens");
            double tokenProductionRate = T::par("tokenProductionRate");
            auto excessTokenModule = findModuleFromPar<ITokenStorage>(T::par("excessTokenModule"), this);
            tokenBucket = TokenBucket(numTokens, maxNumTokens, tokenProductionRate, excessTokenModule);
            overflowTimer = new cMessage("OverflowTimer");
            WATCH(tokenBucket);
        }
        else if (stage == INITSTAGE_QUEUEING) {
            T::emit(tokensChangedSignal, getNumTokens());
            rescheduleOverflowTimer();
        }
    }

    virtual void handleMessage(cMessage *message) override
    {
        if (message == overflowTimer)
            // initiate overflow
            tokenBucket.getNumTokens();
        else
            throw cRuntimeError("Unknown message");
    }

    virtual void finish() override
    {
        T::emit(tokensChangedSignal, getNumTokens());
    }

    std::string resolveDirective(char directive) const override
    {
        switch (directive) {
            case 'n':
                return std::to_string(tokenBucket.getNumTokens());
            default:
                return T::resolveDirective(directive);
        }
    }

    virtual void rescheduleOverflowTimer() {
        if (tokenBucket.getExcessTokenStorage() != nullptr && tokenBucket.getOverflowTime() > simTime())
            T::rescheduleAt(tokenBucket.getOverflowTime(), overflowTimer);
    }

  public:
    virtual ~TokenBucketMixin<T>() { T::cancelAndDelete(overflowTimer); }

    virtual double getNumTokens() const override
    {
        return tokenBucket.getNumTokens();
    }

    virtual void addTokens(double numTokens) override
    {
        Enter_Method("addTokens");
        EV_INFO << "Adding tokens" << EV_FIELD(numTokens) << EV_FIELD(tokenBucket) << EV_ENDL;
        T::emit(tokensChangedSignal, getNumTokens());
        tokenBucket.addTokens(numTokens);
        T::emit(tokensChangedSignal, tokenBucket.getNumTokens());
        rescheduleOverflowTimer();
    }

    virtual void removeTokens(double numTokens) override
    {
        Enter_Method("removeTokens");
        EV_INFO << "Removing tokens" << EV_FIELD(numTokens) << EV_FIELD(tokenBucket) << EV_ENDL;
        T::emit(tokensChangedSignal, getNumTokens());
        tokenBucket.removeTokens(numTokens);
        T::emit(tokensChangedSignal, tokenBucket.getNumTokens());
        rescheduleOverflowTimer();
    }

    virtual void addTokenProductionRate(double tokenRate) override
    {
        Enter_Method("addTokenProductionRate");
        EV_INFO << "Adding token production rate" << EV_FIELD(tokenRate) << EV_FIELD(tokenBucket) << EV_ENDL;
        tokenBucket.addTokenProductionRate(tokenRate);
        rescheduleOverflowTimer();
    }

    virtual void removeTokenProductionRate(double tokenRate) override
    {
        Enter_Method("removeTokenProductionRate");
        EV_INFO << "Removing token production rate" << EV_FIELD(tokenRate) << EV_FIELD(tokenBucket) << EV_ENDL;
        tokenBucket.removeTokenProductionRate(tokenRate);
        rescheduleOverflowTimer();
    }
};

} // namespace queueing
} // namespace inet

#endif

