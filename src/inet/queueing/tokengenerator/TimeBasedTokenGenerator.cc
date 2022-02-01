//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/queueing/tokengenerator/TimeBasedTokenGenerator.h"

namespace inet {
namespace queueing {

Define_Module(TimeBasedTokenGenerator);

void TimeBasedTokenGenerator::initialize(int stage)
{
    ClockUserModuleMixin::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        generationIntervalParameter = &par("generationInterval");
        numTokensParameter = &par("numTokens");
        generationTimer = new ClockEvent("GenerationTimer");
    }
    else if (stage == INITSTAGE_QUEUEING)
        scheduleGenerationTimer();
}

void TimeBasedTokenGenerator::scheduleGenerationTimer()
{
    scheduleClockEventAfter(generationIntervalParameter->doubleValue(), generationTimer);
}

void TimeBasedTokenGenerator::handleMessage(cMessage *message)
{
    if (message == generationTimer) {
        auto numTokens = numTokensParameter->doubleValue();
        numTokensGenerated += numTokens;
        emit(tokensCreatedSignal, numTokens);
        storage->addTokens(numTokens);
        scheduleGenerationTimer();
        updateDisplayString();
    }
    else
        throw cRuntimeError("Unknown message");
}

} // namespace queueing
} // namespace inet

