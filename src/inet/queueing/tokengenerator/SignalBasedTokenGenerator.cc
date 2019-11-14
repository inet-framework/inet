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

#include "inet/common/ModuleAccess.h"
#include "inet/queueing/tokengenerator/SignalBasedTokenGenerator.h"

namespace inet {
namespace queueing {

Define_Module(SignalBasedTokenGenerator);

void SignalBasedTokenGenerator::initialize(int stage)
{
    TokenGeneratorBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
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
    server->addTokens(numTokens);
    updateDisplayString();
}

void SignalBasedTokenGenerator::receiveSignal(cComponent *source, simsignal_t signal, intval_t value, cObject *details)
{
    Enter_Method(cComponent::getSignalName(signal));
    generateTokens();
}

void SignalBasedTokenGenerator::receiveSignal(cComponent *source, simsignal_t signal, double value, cObject *details)
{
    Enter_Method(cComponent::getSignalName(signal));
    generateTokens();
}

void SignalBasedTokenGenerator::receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details)
{
    Enter_Method(cComponent::getSignalName(signal));
    generateTokens();
}

} // namespace queueing
} // namespace inet

