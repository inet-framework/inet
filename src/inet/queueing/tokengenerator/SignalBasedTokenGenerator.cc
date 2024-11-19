//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/queueing/tokengenerator/SignalBasedTokenGenerator.h"

#include "inet/common/ModuleAccess.h"

namespace inet {
namespace queueing {

Define_Module(SignalBasedTokenGenerator);

void SignalBasedTokenGenerator::initialize(int stage)
{
    TokenGeneratorBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        intSignalValue = par("intSignalValue");
        doubleSignalValue = par("doubleSignalValue");
        numTokensParameter = &par("numTokens");
        auto subscriptionModule = getModuleFromPar<cModule>(par("subscriptionModule"), this);
        cStringTokenizer tokenizer(par("signals"));
        while (tokenizer.hasMoreTokens()) {
            auto signal = tokenizer.nextToken();
            subscriptionModule->subscribe(signal, this);
        }
    }
}

void SignalBasedTokenGenerator::generateTokens()
{
    auto numTokens = numTokensParameter->doubleValue();
    numTokensGenerated += numTokens;
    emit(tokensCreatedSignal, numTokens);
    storage->addTokens(numTokens);
    updateDisplayString();
}

void SignalBasedTokenGenerator::receiveSignal(cComponent *source, simsignal_t signal, intval_t value, cObject *details)
{
    Enter_Method("%s", cComponent::getSignalName(signal));
    if (intSignalValue == -1 || intSignalValue == value)
        generateTokens();
}

void SignalBasedTokenGenerator::receiveSignal(cComponent *source, simsignal_t signal, double value, cObject *details)
{
    Enter_Method("%s", cComponent::getSignalName(signal));
    if (std::isnan(doubleSignalValue) || doubleSignalValue == value)
        generateTokens();
}

void SignalBasedTokenGenerator::receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details)
{
    Enter_Method("%s", cComponent::getSignalName(signal));

    generateTokens();
}

} // namespace queueing
} // namespace inet

