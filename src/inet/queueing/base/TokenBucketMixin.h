//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
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

    const virtual char* resolveDirective(char directive) const override
    {
        static std::string result;
        switch (directive) {
            case 'n': {
                std::stringstream stream;
                stream << tokenBucket.getNumTokens();
                result = stream.str();
                break;
            }
            default:
                return T::resolveDirective(directive);
        }
        return result.c_str();
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

