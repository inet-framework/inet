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
#include "inet/networklayer/diffserv/SingleRateThreeColorMeter.h"

namespace inet {

using namespace DiffservUtil;

Define_Module(SingleRateThreeColorMeter);

void SingleRateThreeColorMeter::initialize(int stage)
{
    PacketMeterBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        numRcvd = 0;
        numYellow = 0;
        numRed = 0;
        WATCH(numRcvd);
        WATCH(numYellow);
        WATCH(numRed);

        CBS = 8 * par("cbs").intValue();
        EBS = 8 * par("ebs").intValue();
        colorAwareMode = par("colorAwareMode");
        Tc = CBS;
        Te = EBS;
    }
    else if (stage == INITSTAGE_NETWORK_LAYER) {
        const char *cirStr = par("cir");
        IInterfaceTable *ift = findModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
        CIR = parseInformationRate(cirStr, "cir", ift, *this, 0);
        lastUpdateTime = simTime();
    }
}

void SingleRateThreeColorMeter::pushPacket(Packet *packet, cGate *inputGate)
{
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

void SingleRateThreeColorMeter::refreshDisplay() const
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

int SingleRateThreeColorMeter::meterPacket(Packet *packet)
{
    // update token buckets
    simtime_t currentTime = simTime();
    long numTokens = (long)(SIMTIME_DBL(currentTime - lastUpdateTime) * CIR);
    lastUpdateTime = currentTime;
    if (Tc + numTokens <= CBS)
        Tc += numTokens;
    else {
        long excessTokens = Tc + numTokens - CBS;
        Tc = CBS;
        if (Te + excessTokens <= EBS)
            Te += excessTokens;
        else
            Te = EBS;
    }

    // update meter state
    int oldColor = colorAwareMode ? getColor(packet) : -1;
    int newColor;
    int packetSizeInBits = packet->getBitLength();
    if (oldColor <= GREEN && Tc - packetSizeInBits >= 0) {
        Tc -= packetSizeInBits;
        newColor = GREEN;
    }
    else if (oldColor <= YELLOW && Te - packetSizeInBits >= 0) {
        Te -= packetSizeInBits;
        newColor = YELLOW;
    }
    else
        newColor = RED;

    setColor(packet, newColor);
    return newColor;
}

} // namespace inet

