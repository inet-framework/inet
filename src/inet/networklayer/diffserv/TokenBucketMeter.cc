//
// Copyright (C) 2012 Opensim Ltd.
// Author: Tamas Borbely
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
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "inet/common/ModuleAccess.h"
#include "inet/networklayer/diffserv/DiffservUtil.h"
#include "inet/networklayer/diffserv/TokenBucketMeter.h"

namespace inet {

using namespace DiffservUtil;

Define_Module(TokenBucketMeter);

void TokenBucketMeter::initialize(int stage)
{
    PacketMeterBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        numRcvd = 0;
        numRed = 0;
        WATCH(numRcvd);
        WATCH(numRed);

        CBS = 8 * par("cbs").intValue();
        colorAwareMode = par("colorAwareMode");
        Tc = CBS;
    }
    else if (stage == INITSTAGE_NETWORK_LAYER) {
        const char *cirStr = par("cir");
        IInterfaceTable *ift = findModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
        CIR = parseInformationRate(cirStr, "cir", ift, *this, 0);
        lastUpdateTime = simTime();
    }
}

void TokenBucketMeter::pushPacket(Packet *packet, cGate *inputGate)
{
    numRcvd++;
    cGate *outputGate = nullptr;
    int color = meterPacket(packet);
    if (color == GREEN)
        outputGate = gate("greenOut");
    else {
        numRed++;
        outputGate = gate("redOut");
    }
    auto consumer = findConnectedModule<IPassivePacketSink>(outputGate);
    pushOrSendPacket(packet, outputGate, consumer);
}

void TokenBucketMeter::refreshDisplay() const
{
    char buf[50] = "";
    if (numRcvd > 0)
        sprintf(buf + strlen(buf), "rcvd: %d ", numRcvd);
    if (numRed > 0)
        sprintf(buf + strlen(buf), "red:%d ", numRed);
    getDisplayString().setTagArg("t", 0, buf);
}

int TokenBucketMeter::meterPacket(Packet *packet)
{
    // update token buckets
    simtime_t currentTime = simTime();
    long numTokens = (long)(SIMTIME_DBL(currentTime - lastUpdateTime) * CIR);
    lastUpdateTime = currentTime;
    if (Tc + numTokens <= CBS)
        Tc += numTokens;
    else
        Tc = CBS;

    // update meter state
    int oldColor = colorAwareMode ? getColor(packet) : -1;
    int newColor;
    int packetSizeInBits = packet->getBitLength();
    if (oldColor <= GREEN && Tc - packetSizeInBits >= 0) {
        Tc -= packetSizeInBits;
        newColor = GREEN;
    }
    else
        newColor = RED;

    setColor(packet, newColor);
    return newColor;
}

} // namespace inet

