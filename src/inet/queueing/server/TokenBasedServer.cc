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
        tokenProductionTimer = new cMessage("TokenProductionTimer");
        WATCH(numTokens);
    }
    else if (stage == INITSTAGE_QUEUEING)
        scheduleTokenProductionTimer();
    else if (stage == INITSTAGE_LAST)
        updateDisplayString();
}

void TokenBasedServer::handleMessage(cMessage *message)
{
    if (message == tokenProductionTimer) {
        numTokens++;
        if (!std::isnan(maxNumTokens) && numTokens >= maxNumTokens)
            numTokens = maxNumTokens;
        processPackets();
        updateDisplayString();
        scheduleTokenProductionTimer();
    }
    else
        PacketServerBase::handleMessage(message);
}

void TokenBasedServer::scheduleTokenProductionTimer()
{
    simtime_t interval = par("tokenProductionInterval");
    if (interval != 0)
        scheduleAt(simTime() + interval, tokenProductionTimer);
}

void TokenBasedServer::processPackets()
{
    while (true) {
        auto packet = provider->canPopPacket(inputGate->getPathStartGate());
        if (packet == nullptr)
            break;
        else {
            auto tokenConsumptionPerPacket = tokenConsumptionPerPacketParameter->doubleValue();
            auto tokenConsumptionPerBit = tokenConsumptionPerBitParameter->doubleValue();
            int numRequiredTokens = tokenConsumptionPerPacket + tokenConsumptionPerBit * packet->getTotalLength().get();
            if (numTokens >= numRequiredTokens) {
                packet = provider->popPacket(inputGate->getPathStartGate());
                EV_INFO << "Processing packet " << packet->getName() << ".\n";
                processedTotalLength += packet->getDataLength();
                pushOrSendPacket(packet, outputGate, consumer);
                numProcessedPackets++;
                numTokens -= numRequiredTokens;
            }
            else {
                emit(tokensDepletedSignal, this);
                break;
            }
        }
    }
    updateDisplayString();
}

void TokenBasedServer::handleCanPushPacket(cGate *gate)
{
}

void TokenBasedServer::handleCanPopPacket(cGate *gate)
{
    Enter_Method_Silent();
    processPackets();
    updateDisplayString();
}

void TokenBasedServer::addTokens(double tokens)
{
    Enter_Method("addTokens");
    numTokens += tokens;
    if (!std::isnan(maxNumTokens) && numTokens >= maxNumTokens)
        numTokens = maxNumTokens;
    processPackets();
    updateDisplayString();
}

const char *TokenBasedServer::resolveDirective(char directive)
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

