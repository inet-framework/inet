//
// Copyright (C) 2012 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/networklayer/diffserv/TokenBucketMeter.h"

#include "inet/common/ModuleAccess.h"
#include "inet/networklayer/diffserv/DiffservUtil.h"

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

