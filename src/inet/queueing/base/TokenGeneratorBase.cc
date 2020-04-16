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
#include "inet/queueing/base/TokenGeneratorBase.h"

namespace inet {
namespace queueing {

simsignal_t TokenGeneratorBase::tokensCreatedSignal = cComponent::registerSignal("tokensCreated");

void TokenGeneratorBase::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        displayStringTextFormat = par("displayStringTextFormat");
        server = getModuleFromPar<TokenBasedServer>(par("serverModule"), this);
        numTokensGenerated = 0;
        WATCH(numTokensGenerated);
    }
}

void TokenGeneratorBase::updateDisplayString()
{
    if (getEnvir()->isGUI()) {
        auto text = StringFormat::formatString(displayStringTextFormat, this);
        getDisplayString().setTagArg("t", 0, text);
    }
}

const char *TokenGeneratorBase::resolveDirective(char directive) const
{
    static std::string result;
    switch (directive) {
        case 's': {
            result = par("serverModule").stringValue();
            break;
        }
        case 't': {
            std::stringstream stream;
            stream << numTokensGenerated;
            result = stream.str();
            break;
        }
        default:
            throw cRuntimeError("Unknown directive: %c", directive);
    }
    return result.c_str();
}

} // namespace queueing
} // namespace inet

