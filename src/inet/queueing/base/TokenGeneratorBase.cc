//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/queueing/base/TokenGeneratorBase.h"

#include "inet/common/ModuleAccess.h"

namespace inet {
namespace queueing {

simsignal_t TokenGeneratorBase::tokensCreatedSignal = cComponent::registerSignal("tokensCreated");

void TokenGeneratorBase::initialize(int stage)
{
    PacketProcessorBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        displayStringTextFormat = par("displayStringTextFormat");
        storage.reference(this, "storageModule", true);
        numTokensGenerated = 0;
        WATCH(numTokensGenerated);
    }
}

const char *TokenGeneratorBase::resolveDirective(char directive) const
{
    static std::string result;
    switch (directive) {
        case 's': {
            result = par("storageModule").stringValue();
            break;
        }
        case 't': {
            std::stringstream stream;
            stream << numTokensGenerated;
            result = stream.str();
            break;
        }
        default:
            result = PacketProcessorBase::resolveDirective(directive);
    }
    return result.c_str();
}

} // namespace queueing
} // namespace inet

