//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/queueing/tokengenerator/PacketBasedTokenGenerator.h"

#include "inet/common/ModuleAccess.h"
#include "inet/queueing/base/TokenGeneratorBase.h"

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
        storage.reference(this, "storageModule", true);
        getModuleFromPar<cModule>(par("storageModule"), this)->subscribe(tokensDepletedSignal, this);
        numTokensGenerated = 0;
        WATCH(numTokensGenerated);
    }
    else if (stage == INITSTAGE_QUEUEING) {
        if (producer != nullptr)
            producer->handleCanPushPacketChanged(inputGate->getPathStartGate());
    }
}

void PacketBasedTokenGenerator::pushPacket(Packet *packet, cGate *gate)
{
    Enter_Method("pushPacket");
    take(packet);
    auto numTokens = numTokensPerPacketParameter->doubleValue() + numTokensPerBitParameter->doubleValue() * packet->getTotalLength().get();
    numTokensGenerated += numTokens;
    emit(TokenGeneratorBase::tokensCreatedSignal, numTokens);
    storage->addTokens(numTokens);
    numProcessedPackets++;
    processedTotalLength += packet->getDataLength();
    updateDisplayString();
    delete packet;
}

std::string PacketBasedTokenGenerator::resolveDirective(char directive) const
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
            return PassivePacketSinkBase::resolveDirective(directive);
    }
}

void PacketBasedTokenGenerator::receiveSignal(cComponent *source, simsignal_t signal, double value, cObject *details)
{
    Enter_Method("%s", cComponent::getSignalName(signal));

    if (signal == tokensDepletedSignal) {
        Enter_Method("tokensDepleted");
        producer->handleCanPushPacketChanged(inputGate->getPathStartGate());
    }
    else
        throw cRuntimeError("Unknown signal");
}

} // namespace queueing
} // namespace inet

