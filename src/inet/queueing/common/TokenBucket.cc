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

#include "inet/queueing/common/TokenBucket.h"

namespace inet {
namespace queueing {

TokenBucket::TokenBucket(double numTokens, double maxNumTokens, double tokenProductionRate, ITokenStorage *excessTokenStorage) :
    maxNumTokens(maxNumTokens),
    tokenProductionRate(tokenProductionRate),
    excessTokenStorage(excessTokenStorage)
{
    addTokens(numTokens);
}

double TokenBucket::getNumTokens() const
{
    const_cast<TokenBucket*>(this)->updateNumTokens();
    return numTokens;
}

void TokenBucket::addTokens(double addedNumTokens)
{
    EV_TRACE << " Adding tokens: numTokens = " << numTokens << ", addedNumTokens = " << addedNumTokens << std::endl;
    ASSERT(addedNumTokens >= 0);
    numTokens += addedNumTokens;
    if (maxNumTokens > 0 && numTokens >= maxNumTokens) {
        if (excessTokenStorage != nullptr) {
            excessTokenStorage->addTokens(numTokens - maxNumTokens);
            excessTokenStorage->addTokenProductionRate(tokenProductionRate - excessTokenProductionRate);
            excessTokenProductionRate = tokenProductionRate;
        }
        numTokens = maxNumTokens;
    }
    EV_TRACE << " Added tokens: numTokens = " << numTokens << ", addedNumTokens = " << addedNumTokens << std::endl;
}

void TokenBucket::removeTokens(double removedNumTokens)
{
    EV_TRACE << " Removing tokens: numTokens = " << numTokens << ", removedNumTokens = " << removedNumTokens << std::endl;
    ASSERT(removedNumTokens >= 0);
    if (numTokens == maxNumTokens && excessTokenStorage != nullptr) {
        ASSERT(tokenProductionRate == excessTokenProductionRate);
        excessTokenStorage->removeTokenProductionRate(excessTokenProductionRate);
        excessTokenProductionRate = 0;
    }
    if (numTokens < removedNumTokens)
        throw cRuntimeError("Insufficient number of tokens");
    numTokens -= removedNumTokens;
    EV_TRACE << " Removed tokens: numTokens = " << numTokens << ", removedNumTokens = " << removedNumTokens << std::endl;
}

void TokenBucket::addTokenProductionRate(double tokenRate)
{
    EV_TRACE << " Adding rate: tokenProductionRate = " << tokenProductionRate << ", excessTokenProductionRate = " << excessTokenProductionRate << ", tokenRate = " << tokenRate << std::endl;
    ASSERT(tokenRate >= 0);
    if (numTokens == maxNumTokens) {
        if (excessTokenStorage != nullptr) {
            ASSERT(tokenProductionRate == excessTokenProductionRate);
            excessTokenStorage->addTokenProductionRate(tokenRate);
            excessTokenProductionRate += tokenRate;
        }
    }
    tokenProductionRate += tokenRate;
    ASSERT(tokenProductionRate >= 0);
    EV_TRACE << " Added rate: tokenProductionRate = " << tokenProductionRate << ", excessTokenProductionRate = " << excessTokenProductionRate << ", tokenRate = " << tokenRate << std::endl;
}

void TokenBucket::removeTokenProductionRate(double tokenRate)
{
    EV_TRACE << " Removing rate: tokenProductionRate = " << tokenProductionRate << ", excessTokenProductionRate = " << excessTokenProductionRate << ", tokenRate = " << tokenRate << std::endl;
    ASSERT(tokenRate >= 0);
    if (numTokens == maxNumTokens) {
        if (excessTokenStorage != nullptr) {
            ASSERT(tokenProductionRate == excessTokenProductionRate);
            excessTokenStorage->removeTokenProductionRate(tokenRate);
            excessTokenProductionRate -= tokenRate;
        }
    }
    tokenProductionRate -= tokenRate;
    ASSERT(tokenProductionRate >= 0);
    EV_TRACE << " Removed rate: tokenProductionRate = " << tokenProductionRate << ", excessTokenProductionRate = " << excessTokenProductionRate << ", tokenRate = " << tokenRate << std::endl;
}

simtime_t TokenBucket::getOverflowTime()
{
    double numTokens = getNumTokens();
    if (maxNumTokens > 0 && maxNumTokens != numTokens) {
        simtime_t overflowTime = simTime() + (maxNumTokens - numTokens) / tokenProductionRate;
        return SimTime::fromRaw(overflowTime.raw() + 1);
    }
    else
        return -1;
}

void TokenBucket::updateNumTokens()
{
    simtime_t now = simTime();
    simtime_t elapsedTime = now - lastUpdate;
    addTokens(tokenProductionRate * elapsedTime.dbl());
    lastUpdate = now;
}

} // namespace queueing
} // namespace inet

