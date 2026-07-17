//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/ratecontrol/RateStatistic.h"

namespace inet {
namespace ieee80211 {

Define_Module(RateStatistic);

simsignal_t RateStatistic::datarateChangedSignal = cComponent::registerSignal("datarateChanged");

void RateStatistic::initialize(int stage)
{
    SimpleModule::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        WATCH(peerAddress);
        WATCH(currentRate);
    }
}

void RateStatistic::handleMessage(cMessage *msg)
{
    throw cRuntimeError("RateStatistic does not process messages");
}

void RateStatistic::setRate(double bitrate)
{
    if (bitrate != currentRate) {
        currentRate = bitrate;
        emit(datarateChangedSignal, currentRate);
    }
}

void RateStatistic::refreshDisplay() const
{
    char buf[32];
    snprintf(buf, sizeof(buf), "%g Mbps", currentRate / 1e6);
    getDisplayString().setTagArg("t", 0, buf);
}

} // namespace ieee80211
} // namespace inet
