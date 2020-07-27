//
// Copyright (C) OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see http://www.gnu.org/licenses/.
//

#include "inet/queueing/tokengenerator/TimeBasedTokenGenerator.h"

namespace inet {
namespace queueing {

Define_Module(TimeBasedTokenGenerator);

void TimeBasedTokenGenerator::initialize(int stage)
{
    TokenGeneratorBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        generationIntervalParameter = &par("generationInterval");
        numTokensParameter = &par("numTokens");
        generationTimer = new cMessage("GenerationTimer");
    }
    else if (stage == INITSTAGE_QUEUEING)
        scheduleGenerationTimer();
}

void TimeBasedTokenGenerator::scheduleGenerationTimer()
{
    scheduleAfter(generationIntervalParameter->doubleValue(), generationTimer);
}

void TimeBasedTokenGenerator::handleMessage(cMessage *message)
{
    if (message == generationTimer) {
        auto numTokens = numTokensParameter->doubleValue();
        numTokensGenerated += numTokens;
        emit(tokensCreatedSignal, numTokens);
        server->addTokens(numTokens);
        scheduleGenerationTimer();
        updateDisplayString();
    }
    else
        throw cRuntimeError("Unknown message");
}

} // namespace queueing
} // namespace inet

