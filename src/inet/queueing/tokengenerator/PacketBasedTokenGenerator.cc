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
#include "inet/queueing/tokengenerator/PacketBasedTokenGenerator.h"

namespace inet {
namespace queueing {

Define_Module(PacketBasedTokenGenerator);

void PacketBasedTokenGenerator::initialize(int stage)
{
    PassivePacketSinkBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        numTokensPerPacketParameter = &par("numTokensPerPacket");
        numTokensPerBitParameter = &par("numTokensPerBit");
        inputGate = gate("in");
        producer = getConnectedModule<IActivePacketSource>(inputGate);
        server = getModuleFromPar<TokenBasedServer>(par("serverModule"), this);
        server->subscribe(TokenBasedServer::tokensDepletedSignal, this);
        numTokensGenerated = 0;
        WATCH(numTokensGenerated);
    }
    else if (stage == INITSTAGE_QUEUEING) {
        if (producer != nullptr)
            producer->handleCanPushPacket(inputGate);
    }
}

void PacketBasedTokenGenerator::pushPacket(Packet *packet, cGate *gate)
{
    Enter_Method("pushPacket");
    auto numTokens = numTokensPerPacketParameter->doubleValue() + numTokensPerBitParameter->doubleValue() * packet->getTotalLength().get();
    numTokensGenerated += numTokens;
    emit(TokenGeneratorBase::tokensCreatedSignal, numTokens);
    server->addTokens(numTokens);
    numProcessedPackets++;
    processedTotalLength += packet->getDataLength();
    updateDisplayString();
    delete packet;
}

const char *PacketBasedTokenGenerator::resolveDirective(char directive) const
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
            return PassivePacketSinkBase::resolveDirective(directive);
    }
    return result.c_str();
}

void PacketBasedTokenGenerator::receiveSignal(cComponent *source, simsignal_t signal, double value, cObject *details)
{
    if (signal == TokenBasedServer::tokensDepletedSignal) {
        Enter_Method("tokensDepleted");
        producer->handleCanPushPacket(inputGate->getPathStartGate());
    }
    else
        throw cRuntimeError("Unknown signal");
}

} // namespace queueing
} // namespace inet

