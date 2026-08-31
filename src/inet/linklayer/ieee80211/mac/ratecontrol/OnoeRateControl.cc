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
    computeModeIfTimerIsExpired(state);
    if (isSuccessful)
        state.numOfSuccTransmissions++;
    else if (isGivenUp)
        state.numOfGivenUpTransmissions++;
    if (retryCount > 0)
        state.numOfRetries++;
}

void OnoeRateControl::computeModeIfTimerIsExpired(State& state)
{
    if (simTime() - state.timer >= interval) {
        computeMode(state);
        state.timer = simTime();
    }
}

void OnoeRateControl::frameReceived(Packet *frame)
{
}

void OnoeRateControl::computeMode(State& state)
{
    int numOfFrameTransmitted = state.numOfSuccTransmissions + state.numOfGivenUpTransmissions + state.numOfRetries;
    state.avgRetriesPerFrame = double(state.numOfRetries) / (state.numOfSuccTransmissions + state.numOfGivenUpTransmissions);

    if (state.numOfSuccTransmissions > 0) {
        if (numOfFrameTransmitted >= 10 && state.avgRetriesPerFrame > 1) {
            state.mode = decreaseRateIfPossible(state.mode);
            emitDatarateChangedSignal(state.address, state.mode);
            EV_DETAIL << "Decreased rate to " << *state.mode << endl;
            state.credit = 0;
        }
        else if (state.avgRetriesPerFrame >= 0.1)
            state.credit--;
        else
            state.credit++;

        if (state.credit >= 10) {
            state.mode = increaseRateIfPossible(state.mode);
            emitDatarateChangedSignal(state.address, state.mode);
            EV_DETAIL << "Increased rate to " << *state.mode << endl;
            state.credit = 0;
        }

        resetStatisticalVariables(state);
    }
}

const IIeee80211Mode *OnoeRateControl::getRate(const MacAddress& receiverAddress)
{
    Enter_Method("getRate");
    State& state = getState(receiverAddress);
    computeModeIfTimerIsExpired(state);
    EV_INFO << "The current mode is " << state.mode << " the net bitrate is " << state.mode->getDataMode()->getNetBitrate() << std::endl;
    return state.mode;
}

} /* namespace ieee80211 */
} /* namespace inet */

