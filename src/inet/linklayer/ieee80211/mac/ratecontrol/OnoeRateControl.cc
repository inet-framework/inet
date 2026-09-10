//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/ratecontrol/OnoeRateControl.h"

namespace inet {
namespace ieee80211 {

using namespace inet::physicallayer;

Define_Module(OnoeRateControl);

void OnoeRateControl::initialize(int stage)
{
    RateControlBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        interval = par("interval");
        WATCH_EXPR("numStations", (int)stations.size());
    }
}

OnoeRateControl::State& OnoeRateControl::getState(const MacAddress& receiverAddress)
{
    auto it = stations.find(receiverAddress);
    if (it == stations.end()) {
        State state;
        state.address = receiverAddress;
        state.mode = getInitialMode();
        state.timer = simTime(); // the interval starts when the station is first seen, not at t=0
        it = stations.insert({receiverAddress, state}).first;
        emitDatarateChangedSignal(state.address, state.mode);
    }
    return it->second;
}

void OnoeRateControl::resetStatisticalVariables(State& state)
{
    state.numOfRetries = 0;
    state.numOfSuccTransmissions = 0;
    state.numOfGivenUpTransmissions = 0;
}

void OnoeRateControl::handleMessage(cMessage *msg)
{
    throw cRuntimeError("This module doesn't handle self messages");
}

void OnoeRateControl::frameTransmitted(Packet *frame, int retryCount, bool isSuccessful, bool isGivenUp)
{
    State& state = getState(getReceiverAddress(frame));
    if (!isSuccessful && !isGivenUp)
        return;
    // Recovery owns the per-packet counter. Commit it once, at completion, so
    // unfinished retries (including other TIDs) cannot enter an evaluated sample.
    ASSERT(retryCount >= 0);
    state.numOfRetries += retryCount;
    if (isSuccessful)
        state.numOfSuccTransmissions++;
    else
        state.numOfGivenUpTransmissions++;
    computeModeIfTimerIsExpired(state);
}

void OnoeRateControl::frameTransmitted(Packet *frame, int retryCount, int totalRetryCount, bool isSuccessful, bool isGivenUp)
{
    frameTransmitted(frame, totalRetryCount, isSuccessful, isGivenUp);
}

void OnoeRateControl::rtsFrameTransmissionFailed(Packet *frame, int totalRetryCount, bool isGivenUp)
{
    frameTransmitted(frame, totalRetryCount, false, isGivenUp);
}

void OnoeRateControl::computeModeIfTimerIsExpired(State& state)
{
    if (simTime() - state.timer >= interval) {
        state.timer = simTime();
        computeMode(state);
    }
}

void OnoeRateControl::frameReceived(Packet *frame)
{
}

void OnoeRateControl::computeMode(State& state)
{
    // Decision semantics: MadWifi ath_rate/onoe/onoe.c, ath_rate_ctl(), revision
    // a7531fd223a1f454d3fd74a975b4581cde5411bb. This is an implementation reference,
    // not an IEEE 802.11 normative algorithm.
    bool enough = state.numOfSuccTransmissions + state.numOfGivenUpTransmissions >= 10;
    auto previousMode = state.mode;
    if ((state.numOfGivenUpTransmissions > 0 && state.numOfSuccTransmissions == 0) ||
        (enough && state.numOfSuccTransmissions < state.numOfRetries))
    {
        state.mode = decreaseRateIfPossible(state.mode);
        state.credit = 0;
    }
    // Ten percent is integer-truncated in the reference. Dividing by ten is
    // equivalent to multiplying by ten then dividing by 100, without overflow.
    else if (enough && state.numOfGivenUpTransmissions == 0 && state.numOfRetries < state.numOfSuccTransmissions / 10) {
        if (++state.credit == 10) {
            state.mode = increaseRateIfPossible(state.mode);
            state.credit = 0;
        }
    }
    else if (enough && state.credit > 0)
        state.credit--;

    // A small failed-only sample at the floor is retained: attempting to lower
    // the rate does not count as a rate change.
    if (state.mode != previousMode || enough)
        resetStatisticalVariables(state);
    if (state.mode != previousMode) {
        EV_DETAIL << "Changed rate to " << *state.mode << endl;
        emitDatarateChangedSignal(state.address, state.mode);
    }
}

const IIeee80211Mode *OnoeRateControl::getRate(const MacAddress& receiverAddress)
{
    Enter_Method("getRate");
    State& state = getState(receiverAddress);
    EV_INFO << "The current mode is " << state.mode << " the net bitrate is " << state.mode->getDataMode()->getNetBitrate() << std::endl;
    return state.mode;
}

} /* namespace ieee80211 */
} /* namespace inet */
