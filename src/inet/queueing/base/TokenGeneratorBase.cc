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

std::string TokenGeneratorBase::resolveDirective(char directive) const
{
    switch (directive) {
        case 's': {
            return par("storageModule").stringValue();
        }
        case 't': {
            std::stringstream stream;
            stream << numTokensGenerated;
            return stream.str();
        }
        default:
            return PacketProcessorBase::resolveDirective(directive);
    }
}

} // namespace queueing
} // namespace inet

