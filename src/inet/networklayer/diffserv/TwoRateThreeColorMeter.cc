//
// Copyright (C) 2012 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/networklayer/diffserv/TwoRateThreeColorMeter.h"

#include "inet/common/ModuleAccess.h"
#include "inet/networklayer/diffserv/DiffservUtil.h"

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
    auto consumer = findConnectedModule<IPassivePacketSink>(outputGate);
    pushOrSendPacket(packet, outputGate, consumer);
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

