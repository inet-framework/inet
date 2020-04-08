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

#include "inet/common/Simsignals.h"
#include "inet/common/StringFormat.h"
#include "inet/queueing/server/TokenBasedServer.h"

namespace inet {
namespace queueing {

Define_Module(TokenBasedServer);

simsignal_t TokenBasedServer::tokensAddedSignal = cComponent::registerSignal("tokensAdded");
simsignal_t TokenBasedServer::tokensRemovedSignal = cComponent::registerSignal("tokensRemoved");
simsignal_t TokenBasedServer::tokensDepletedSignal = cComponent::registerSignal("tokensDepleted");

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
                EV_INFO << "Processing packet " << packet->getName() << ".\n";
                processedTotalLength += packet->getDataLength();
                emit(packetServedSignal, packet);
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

void TokenBasedServer::handleCanPushPacket(cGate *gate)
{
    Enter_Method("handleCanPushPacket");
}

void TokenBasedServer::handleCanPullPacket(cGate *gate)
{
    Enter_Method("handleCanPullPacket");
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

const char *TokenBasedServer::resolveDirective(char directive) const
{
    static std::string result;
    switch (directive) {
        case 'n': {
            std::stringstream stream;
            stream << numTokens;
            result = stream.str();
            break;
        }
        default:
            return PacketServerBase::resolveDirective(directive);
    }
    return result.c_str();
}

} // namespace queueing
} // namespace inet

