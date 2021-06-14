//
// Copyright (C) 2020 OpenSim Ltd.
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

#ifndef __INET_MULTITOKENBUCKETMIXIN_H
#define __INET_MULTITOKENBUCKETMIXIN_H

#include "inet/common/ModuleAccess.h"
#include "inet/common/Simsignals.h"
#include "inet/queueing/common/TokenBucket.h"
#include "inet/queueing/contract/ITokenStorage.h"

namespace inet {
namespace queueing {

inline std::ostream& operator<<(std::ostream& out, const std::vector<TokenBucket>& vector)
{
    out << "[";
    for (int i = 0; i < vector.size(); i++) {
        auto& element = vector[i];
        if (i != 0)
            out << ", ";
        out << element;
    }
    return out << "]";
}

template<typename T>
class INET_API MultiTokenBucketMixin : public T, public ITokenStorage
{
  protected:
    std::vector<TokenBucket> tokenBuckets;

  protected:
    virtual void initialize(int stage) override
    {
        T::initialize(stage);
        if (stage == INITSTAGE_LOCAL) {
            cValueArray *bucketConfigurations = check_and_cast<cValueArray*>(T::par("buckets").objectValue());
            tokenBuckets.resize(bucketConfigurations->size());
            for (int i = 0; i < bucketConfigurations->size(); i++) {
                cValueMap *bucketConfiguration = check_and_cast<cValueMap*>(bucketConfigurations->get(i).objectValue());
                double numTokens = bucketConfiguration->containsKey("initialNumTokens") ? bucketConfiguration->get("initialNumTokens").doubleValue() : 0;
                double maxNumTokens = bucketConfiguration->containsKey("maxNumTokens") ? bucketConfiguration->get("maxNumTokens").doubleValue() : -1;
                double tokenProductionRate = bucketConfiguration->get("tokenProductionRate");
                auto excessTokenStorage = bucketConfiguration->containsKey("excessTokenModule") ? getExcessTokenStorage(bucketConfiguration->get("excessTokenModule")) : (i < bucketConfigurations->size() - 1 ? &tokenBuckets[i + 1] : nullptr);
                tokenBuckets[i] = (TokenBucket(numTokens, maxNumTokens, tokenProductionRate, excessTokenStorage));
            }
            WATCH_VECTOR(tokenBuckets);
        }
        else if (stage == INITSTAGE_QUEUEING)
            T::emit(tokensChangedSignal, getNumTokens());
    }

    virtual void finish() override
    {
        T::emit(tokensChangedSignal, getNumTokens());
    }

    virtual ITokenStorage* getExcessTokenStorage(const char *designator)
    {
        try {
            return &tokenBuckets[std::stoi(designator)];
        }
        catch (...) {
            const char *colon = strchr(designator, ':');
            if (colon == nullptr)
                return check_and_cast<ITokenStorage*>(T::getModuleByPath(designator));
            else {
                auto multiTokenBucketMixin = check_and_cast<MultiTokenBucketMixin<T>*>(T::getModuleByPath(designator));
                int tokenBucketIndex = std::stoi(colon + 1);
                return &multiTokenBucketMixin->tokenBuckets[tokenBucketIndex];
            }
        }
    }

    const virtual char* resolveDirective(char directive) const override
    {
        static std::string result;
        switch (directive) {
            case 'n': {
                std::stringstream stream;
                for (int i = 0; i < tokenBuckets.size(); i++) {
                    auto &tokenBucket = tokenBuckets[i];
                    if (i != 0)
                        stream << ", ";
                    stream << tokenBucket.getNumTokens();
                }
                result = stream.str();
                break;
            }
            default:
                return T::resolveDirective(directive);
        }
        return result.c_str();
    }

  public:
    virtual double getNumTokens() const override
    {
        double numTokens = 0;
        for (auto &tokenBucket : tokenBuckets)
            numTokens += tokenBucket.getNumTokens();
        return numTokens;
    }

    virtual void addTokens(double numTokens) override
    {
        Enter_Method("addTokens");
        EV_INFO << "Adding tokens" << EV_FIELD(numTokens) << EV_FIELD(tokenBuckets) << EV_ENDL;
        T::emit(tokensChangedSignal, getNumTokens());
        tokenBuckets[0].addTokens(numTokens);
        T::emit(tokensChangedSignal, getNumTokens());
    }

    virtual void removeTokens(double numTokens) override
    {
        Enter_Method("removeTokens");
        EV_INFO << "Removing tokens" << EV_FIELD(numTokens) << EV_ENDL;
        T::emit(tokensChangedSignal, getNumTokens());
        tokenBuckets[0].removeTokens(numTokens);
        T::emit(tokensChangedSignal, getNumTokens());
    }

    virtual void addTokenProductionRate(double tokenRate) override
    {
        Enter_Method("addTokenProductionRate");
        auto &tokenBucket = tokenBuckets[0];
        EV_INFO << "Adding token production rate" << EV_FIELD(tokenRate) << EV_FIELD(tokenBucket) << EV_ENDL;
        tokenBucket.addTokenProductionRate(tokenRate);
    }

    virtual void removeTokenProductionRate(double tokenRate) override
    {
        Enter_Method("removeTokenProductionRate");
        auto &tokenBucket = tokenBuckets[0];
        EV_INFO << "Removing token production rate" << EV_FIELD(tokenRate) << EV_FIELD(tokenBucket) << EV_ENDL;
        tokenBucket.removeTokenProductionRate(tokenRate);
    }
};

} // namespace queueing
} // namespace inet

#endif

