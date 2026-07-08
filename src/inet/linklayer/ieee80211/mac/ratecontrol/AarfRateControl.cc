//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


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
        maxIncreaseThreshold = par("maxIncreaseThreshold");
        decreaseThreshold = par("decreaseThreshold");
        interval = par("interval");
        WATCH(factor);
        WATCH(maxIncreaseThreshold);
        WATCH(decreaseThreshold);
        WATCH(interval);
    }
    else if (stage == INITSTAGE_LINK_LAYER) {
    }
}

void AarfRateControl::handleMessage(cMessage *msg)
{
    throw cRuntimeError("This module doesn't handle self messages");
}

void AarfRateControl::refreshDisplay() const
{
    getDisplayString().setTagArg("t", 0, (std::to_string(stations.size()) + " stations").c_str());
}

AarfRateControl::State& AarfRateControl::stateFor(const MacAddress& receiverAddress)
{
    auto it = stations.find(receiverAddress);
    if (it == stations.end()) {
        State state;
        state.mode = getInitialMode();
        state.increaseThreshold = par("increaseThreshold");
        it = stations.insert({receiverAddress, state}).first;
        emitDatarateChangedSignal(state.mode);
    }
    return it->second;
}

void AarfRateControl::frameTransmitted(Packet *frame, int retryCount, bool isSuccessful, bool isGivenUp)
{
    State& state = stateFor(getReceiverAddress(frame));
    increaseRateIfTimerIsExpired(state);

    if (!isSuccessful && state.probing) { // probing packet failed
        state.numberOfConsSuccTransmissions = 0;
        state.mode = decreaseRateIfPossible(state.mode);
        emitDatarateChangedSignal(state.mode);
        EV_DETAIL << "Decreased rate to " << *state.mode << endl;
        multiplyIncreaseThreshold(state, factor);
        resetTimer(state);
    }
    else if (!isSuccessful && retryCount >= decreaseThreshold - 1) { // decreaseThreshold consecutive failed transmissions
        state.numberOfConsSuccTransmissions = 0;
        state.mode = decreaseRateIfPossible(state.mode);
        emitDatarateChangedSignal(state.mode);
        EV_DETAIL << "Decreased rate to " << *state.mode << endl;
        resetIncreaseThreshdold(state);
        resetTimer(state);
    }
    else if (isSuccessful && retryCount == 0)
        state.numberOfConsSuccTransmissions++;

    if (state.numberOfConsSuccTransmissions == state.increaseThreshold) {
        state.numberOfConsSuccTransmissions = 0;
        state.mode = increaseRateIfPossible(state.mode);
        emitDatarateChangedSignal(state.mode);
        EV_DETAIL << "Increased rate to " << *state.mode << endl;
        resetTimer(state);
        state.probing = true;
    }
    else
        state.probing = false;
}

void AarfRateControl::multiplyIncreaseThreshold(State& state, double factor)
{
    if (state.increaseThreshold * factor <= maxIncreaseThreshold)
        state.increaseThreshold *= factor;
}

void AarfRateControl::resetIncreaseThreshdold(State& state)
{
    state.increaseThreshold = par("increaseThreshold");
}

void AarfRateControl::resetTimer(State& state)
{
    state.timer = simTime();
}

void AarfRateControl::increaseRateIfTimerIsExpired(State& state)
{
    if (simTime() - state.timer >= interval) {
        state.mode = increaseRateIfPossible(state.mode);
        emitDatarateChangedSignal(state.mode);
        EV_DETAIL << "Increased rate to " << *state.mode << endl;
        resetTimer(state);
    }
}

void AarfRateControl::frameReceived(Packet *frame)
{
}

const IIeee80211Mode *AarfRateControl::getRate(const MacAddress& receiverAddress)
{
    Enter_Method("getRate");
    State& state = stateFor(receiverAddress);
    increaseRateIfTimerIsExpired(state);
    EV_INFO << "The current mode is " << state.mode << " the net bitrate is " << state.mode->getDataMode()->getNetBitrate() << std::endl;
    return state.mode;
}

} /* namespace ieee80211 */
} /* namespace inet */
