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

