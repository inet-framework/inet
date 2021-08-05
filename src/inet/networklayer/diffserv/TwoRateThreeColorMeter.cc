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
#include "inet/networklayer/diffserv/TwoRateThreeColorMeter.h"

namespace inet {

using namespace DiffservUtil;

Define_Module(TwoRateThreeColorMeter);

void TwoRateThreeColorMeter::initialize(int stage)
{
    PacketMeterBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        numRcvd = 0;
        numYellow = 0;
        numRed = 0;
        WATCH(numRcvd);
        WATCH(numYellow);
        WATCH(numRed);

        PBS = 8 * par("pbs").intValue();
        CBS = 8 * par("cbs").intValue();
        colorAwareMode = par("colorAwareMode");
        Tp = PBS;
        Tc = CBS;
    }
    else if (stage == INITSTAGE_NETWORK_LAYER) {
        IInterfaceTable *ift = findModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
        PIR = parseInformationRate(par("pir"), "pir", ift, *this, 0);
        CIR = parseInformationRate(par("cir"), "cir", ift, *this, 0);
        lastUpdateTime = simTime();
    }
}

void TwoRateThreeColorMeter::pushPacket(Packet *packet, cGate *inputGate)
{
    Enter_Method("pushPacket");
    take(packet);
    numRcvd++;
    cGate *outputGate = nullptr;
    int color = meterPacket(packet);
    switch (color) {
        case GREEN:
            outputGate = gate("greenOut");
            break;

        case YELLOW:
            numYellow++;
            outputGate = gate("yellowOut");
            break;

        case RED:
            numRed++;
            outputGate = gate("redOut");
            break;
    }
    pushOrSendPacket(packet, outputGate);
}

void TwoRateThreeColorMeter::refreshDisplay() const
{
    char buf[80] = "";
    if (numRcvd > 0)
        sprintf(buf + strlen(buf), "rcvd: %d ", numRcvd);
    if (numYellow > 0)
        sprintf(buf + strlen(buf), "yellow:%d ", numYellow);
    if (numRed > 0)
        sprintf(buf + strlen(buf), "red:%d ", numRed);
    getDisplayString().setTagArg("t", 0, buf);
}

int TwoRateThreeColorMeter::meterPacket(Packet *packet)
{
    // update token buckets
    simtime_t currentTime = simTime();
    double elapsedTime = SIMTIME_DBL(currentTime - lastUpdateTime);
    lastUpdateTime = currentTime;
    long numTokens = (long)(elapsedTime * PIR);
    if (Tp + numTokens <= PBS)
        Tp += numTokens;
    else
        Tp = PBS;
    numTokens = (long)(elapsedTime * CIR);
    if (Tc + numTokens <= CBS)
        Tc += numTokens;
    else
        Tc = CBS;

    // update meter state
    int oldColor = colorAwareMode ? getColor(packet) : -1;
    int newColor;
    int packetSizeInBits = 8 * packet->getByteLength();
    if (oldColor == RED || Tp - packetSizeInBits < 0) {
        newColor = RED;
    }
    else if (oldColor == YELLOW || Tc - packetSizeInBits < 0) {
        Tp -= packetSizeInBits;
        newColor = YELLOW;
    }
    else {
        Tp -= packetSizeInBits;
        Tc -= packetSizeInBits;
        newColor = GREEN;
    }

    setColor(packet, newColor);
    return newColor;
}

} // namespace inet

