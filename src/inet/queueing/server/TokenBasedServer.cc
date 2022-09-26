//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/queueing/server/TokenBasedServer.h"

#include "inet/common/Simsignals.h"
#include "inet/common/StringFormat.h"

namespace inet {
namespace queueing {

Define_Module(TokenBasedServer);

void TokenBasedServer::initialize(int stage)
{
    PacketServerBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        tokenConsumptionPerPacketParameter = &par("tokenConsumptionPerPacket");
        tokenConsumptionPerBitParameter = &par("tokenConsumptionPerBit");
        displayStringTextFormat = par("displayStringTextFormat");
        numTokens = par("initialNumTokens");
        maxNumTokens = par("maxNumTokens");
        WATCH(numTokens);
    }
    else if (stage == INITSTAGE_LAST)
        updateDisplayString();
}

void TokenBasedServer::processPackets()
{
    while (true) {
        auto packet = provider->canPullPacket(inputGate->getPathStartGate());
        if (packet == nullptr)
            break;
        else {
            auto tokenConsumptionPerPacket = tokenConsumptionPerPacketParameter->doubleValue();
            auto tokenConsumptionPerBit = tokenConsumptionPerBitParameter->doubleValue();
            int numRequiredTokens = tokenConsumptionPerPacket + tokenConsumptionPerBit * packet->getTotalLength().get();
            if (numTokens >= numRequiredTokens) {
                packet = provider->pullPacket(inputGate->getPathStartGate());
                take(packet);
                emit(packetPulledSignal, packet);
                EV_INFO << "Processing packet" << EV_FIELD(packet) << EV_ENDL;
                processedTotalLength += packet->getDataLength();
                emit(packetPushedSignal, packet);
                pushOrSendPacket(packet, outputGate, consumer);
                numProcessedPackets++;
                numTokens -= numRequiredTokens;
                emit(tokensRemovedSignal, numTokens);
                updateDisplayString();
            }
            else {
                if (!tokensDepletedSignaled) {
                    tokensDepletedSignaled = true;
                    emit(tokensDepletedSignal, numTokens);
                }
                break;
            }
        }
    }
}

void TokenBasedServer::handleCanPushPacketChanged(cGate *gate)
{
    Enter_Method("handleCanPushPacketChanged");
}

void TokenBasedServer::handleCanPullPacketChanged(cGate *gate)
{
    Enter_Method("handleCanPullPacketChanged");
    processPackets();
    updateDisplayString();
}

void TokenBasedServer::addTokens(double tokens)
{
    Enter_Method("addTokens");
    numTokens += tokens;
    if (!std::isnan(maxNumTokens) && numTokens >= maxNumTokens)
        numTokens = maxNumTokens;
    emit(tokensAddedSignal, numTokens);
    tokensDepletedSignaled = false;
    processPackets();
    updateDisplayString();
}

std::string TokenBasedServer::resolveDirective(char directive) const
{
    switch (directive) {
        case 'n': {
            std::stringstream stream;
            stream << numTokens;
            return stream.str();
        }
        default:
            return PacketServerBase::resolveDirective(directive);
    }
}

} // namespace queueing
} // namespace inet

