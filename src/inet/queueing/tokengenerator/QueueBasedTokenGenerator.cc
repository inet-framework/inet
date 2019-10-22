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
#include "inet/queueing/tokengenerator/QueueBasedTokenGenerator.h"

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
        check_and_cast<cSimpleModule *>(queue)->subscribe(packetPoppedSignal, this);
        numTokensParameter = &par("numTokens");
    }
    else if (stage == INITSTAGE_QUEUEING)
        if (queue->getNumPackets() < minNumPackets || queue->getTotalLength() < minTotalLength)
            generateTokens();
}

void QueueBasedTokenGenerator::receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details)
{
    if (signal == packetPoppedSignal) {
        Enter_Method("packetPopped");
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

