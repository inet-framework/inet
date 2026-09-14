//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include <cmath>
#include <limits>

#include "inet/linklayer/ieee80211/mac/ratecontrol/AarfRateControl.h"

namespace inet {
namespace ieee80211 {

using namespace inet::physicallayer;

Define_Module(AarfRateControl);

void AarfRateControl::initialize(int stage)
{
    RateControlBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        factor = par("increaseThresholdFactor");
        initialIncreaseThreshold = par("increaseThreshold");
        maxIncreaseThreshold = par("maxIncreaseThreshold");
        decreaseThreshold = par("decreaseThreshold");
        minTimerThreshold = par("minTimerThreshold");
        timerThresholdFactor = par("timerThresholdFactor");
        if (initialIncreaseThreshold < 1 || maxIncreaseThreshold < initialIncreaseThreshold ||
            decreaseThreshold < 1 || minTimerThreshold < 1 ||
            !std::isfinite(factor) || factor < 1 ||
            !std::isfinite(timerThresholdFactor) || timerThresholdFactor < 1 ||
            timerThresholdFactor * maxIncreaseThreshold > std::numeric_limits<int>::max())
            throw cRuntimeError("Invalid AARF thresholds or factors");
        WATCH_EXPR("numStations", (int)stations.size());
    }
}

void AarfRateControl::handleMessage(cMessage *msg)
{
    throw cRuntimeError("This module doesn't handle self messages");
}

AarfRateControl::State& AarfRateControl::getState(const MacAddress& receiverAddress)
{
    auto it = stations.find(receiverAddress);
    if (it == stations.end()) {
        State state;
        state.address = receiverAddress;
        state.mode = getInitialMode();
        state.increaseThreshold = initialIncreaseThreshold;
        state.timerThreshold = minTimerThreshold;
        it = stations.insert({receiverAddress, state}).first;
        emitDatarateChangedSignal(state.address, state.mode);
    }
    return it->second;
}

void AarfRateControl::frameTransmitted(Packet *frame, int retryCount, bool isSuccessful, bool isGivenUp)
{
    Enter_Method("frameTransmitted");
    State& state = getState(getReceiverAddress(frame));
    auto previousMode = state.mode;

    // Lacage et al., INRIA RR-5208, Appendix A, pp. 22-24.
    // Retry counts belong to the packet's MAC recovery procedure, whereas the
    // adaptive thresholds and recovery phase persist for this receiver.
    if (isSuccessful) {
        if (state.numberOfConsSuccTransmissions < state.increaseThreshold)
            state.numberOfConsSuccTransmissions++;
        auto fasterMode = increaseRateIfPossible(state.mode);
        if ((state.numberOfConsSuccTransmissions >= state.increaseThreshold || state.timer >= state.timerThreshold) &&
            fasterMode != state.mode) {
            state.mode = fasterMode;
            state.timer = 0;
            state.numberOfConsSuccTransmissions = 0;
            state.probing = true;
        }
        else {
            if (state.timer < state.timerThreshold)
                state.timer++;
            state.probing = false;
        }
    }
    else {
        ASSERT(retryCount >= 1);
        if (state.timer < state.timerThreshold)
            state.timer++;
        state.numberOfConsSuccTransmissions = 0;
        if (state.probing) {
            state.timer = 0;
            if (retryCount == 1) {
                state.increaseThreshold = std::min(state.increaseThreshold * factor, double(maxIncreaseThreshold));
                state.timerThreshold = std::max(int(timerThresholdFactor * state.increaseThreshold), minTimerThreshold);
                state.mode = decreaseRateIfPossible(state.mode);
            }
        }
        else {
            // Generalize the appendix's retries 2, 4, 6, 8, 10 to the configured
            // spacing and MAC retry limit, without owning a second retry counter.
            if (retryCount % decreaseThreshold == 0) {
                state.increaseThreshold = initialIncreaseThreshold;
                state.timerThreshold = minTimerThreshold;
                state.mode = decreaseRateIfPossible(state.mode);
            }
            if (retryCount >= decreaseThreshold)
                state.timer = 0;
        }
    }
    if (state.mode != previousMode) {
        emitDatarateChangedSignal(state.address, state.mode);
        EV_DETAIL << "Changed rate to " << *state.mode << endl;
    }
}

void AarfRateControl::frameReceived(Packet *frame)
{
}

const IIeee80211Mode *AarfRateControl::getRate(const MacAddress& receiverAddress)
{
    Enter_Method("getRate");
    State& state = getState(receiverAddress);
    EV_INFO << "The current mode is " << state.mode << " the net bitrate is " << state.mode->getDataMode()->getNetBitrate() << std::endl;
    return state.mode;
}

} /* namespace ieee80211 */
} /* namespace inet */

