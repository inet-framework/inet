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
        queue = getModuleFromPar<IPacketQueue>(par("queueModule"), this);
        check_and_cast<cSimpleModule *>(queue)->subscribe(packetPulledSignal, this);
        numTokensParameter = &par("numTokens");
    }
    else if (stage == INITSTAGE_QUEUEING)
        if (queue->getNumPackets() < minNumPackets || queue->getTotalLength() < minTotalLength)
            generateTokens();
}

void QueueBasedTokenGenerator::receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details)
{
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
    server->addTokens(numTokens);
    updateDisplayString();
}

} // namespace queueing
} // namespace inet

