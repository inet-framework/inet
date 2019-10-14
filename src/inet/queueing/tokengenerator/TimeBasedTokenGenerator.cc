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
#include "inet/queueing/tokengenerator/TimeBasedTokenGenerator.h"

namespace inet {
namespace queueing {

Define_Module(TimeBasedTokenGenerator);

void TimeBasedTokenGenerator::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        displayStringTextFormat = par("displayStringTextFormat");
        generationIntervalParameter = &par("generationInterval");
        numTokensParameter = &par("numTokens");
        server = getModuleFromPar<TokenBasedServer>(par("serverModule"), this);
        generationTimer = new cMessage("GenerationTimer");
        numTokensGenerated = 0;
        WATCH(numTokensGenerated);
    }
    else if (stage == INITSTAGE_QUEUEING)
        scheduleGenerationTimer();
}

void TimeBasedTokenGenerator::scheduleGenerationTimer()
{
    scheduleAt(simTime() + generationIntervalParameter->doubleValue(), generationTimer);
}

void TimeBasedTokenGenerator::handleMessage(cMessage *message)
{
    if (message == generationTimer) {
        auto numTokens = numTokensParameter->doubleValue();
        numTokensGenerated += numTokens;
        server->addTokens(numTokens);
        scheduleGenerationTimer();
        updateDisplayString();
    }
    else
        throw cRuntimeError("Unknown message");
}

void TimeBasedTokenGenerator::updateDisplayString()
{
    auto text = StringFormat::formatString(displayStringTextFormat, this);
    getDisplayString().setTagArg("t", 0, text);
}

const char *TimeBasedTokenGenerator::resolveDirective(char directive)
{
    static std::string result;
    switch (directive) {
        case 't':
            result = std::to_string(numTokensGenerated);
            break;
        default:
            throw cRuntimeError("Unknown directive: %c", directive);
    }
    return result.c_str();
}

} // namespace queueing
} // namespace inet

