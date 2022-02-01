//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/queueing/tokengenerator/QueueBasedTokenGenerator.h"

#include "inet/common/ModuleAccess.h"

namespace inet {
namespace queueing {

Define_Module(QueueBasedTokenGenerator);

void QueueBasedTokenGenerator::initialize(int stage)
{
    TokenGeneratorBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        minNumPackets = par("minNumPackets");
        minTotalLength = b(par("minTotalLength"));
        queue.reference(this, "queueModule", true);
        check_and_cast<cSimpleModule *>(queue.get())->subscribe(packetPulledSignal, this);
        numTokensParameter = &par("numTokens");
    }
    else if (stage == INITSTAGE_QUEUEING)
        if (queue->getNumPackets() < minNumPackets || queue->getTotalLength() < minTotalLength)
            generateTokens();
}

void QueueBasedTokenGenerator::receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details)
{
    Enter_Method("%s", cComponent::getSignalName(signal));

    if (signal == packetPulledSignal) {
        Enter_Method("packetPulled");
        if (queue->getNumPackets() < minNumPackets || queue->getTotalLength() < minTotalLength)
            generateTokens();
    }
    else
        throw cRuntimeError("Unknown signal");
}

void QueueBasedTokenGenerator::generateTokens()
{
    auto numTokens = numTokensParameter->doubleValue();
    numTokensGenerated += numTokens;
    emit(tokensCreatedSignal, numTokens);
    storage->addTokens(numTokens);
    updateDisplayString();
}

} // namespace queueing
} // namespace inet

