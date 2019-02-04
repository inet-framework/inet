//
// Copyright (C) 2016 OpenSim Ltd.
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
// along with this program; if not, see http://www.gnu.org/licenses/.
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
        increaseThreshold = par("increaseThreshold");
        maxIncreaseThreshold = par("maxIncreaseThreshold");
        decreaseThreshold = par("decreaseThreshold");
        interval = par("interval");
        WATCH(factor);
        WATCH(increaseThreshold);
        WATCH(maxIncreaseThreshold);
        WATCH(decreaseThreshold);
        WATCH(interval);
        WATCH(probing);
        WATCH(numberOfConsSuccTransmissions);
    }
    else if (stage == INITSTAGE_LINK_LAYER) {
        updateDisplayString();
    }
}

void AarfRateControl::handleMessage(cMessage* msg)
{
    throw cRuntimeError("This module doesn't handle self messages");
}

void AarfRateControl::updateDisplayString()
{
    getDisplayString().setTagArg("t", 0, currentMode->getName());
}

void AarfRateControl::frameTransmitted(Packet *frame, int retryCount, bool isSuccessful, bool isGivenUp)
{
    increaseRateIfTimerIsExpired();

    if (!isSuccessful && probing) // probing packet failed
    {
        numberOfConsSuccTransmissions = 0;
        currentMode = decreaseRateIfPossible(currentMode);
        emitDatarateChangedSignal();
        updateDisplayString();
        EV_DETAIL << "Decreased rate to " << *currentMode << endl;
        multiplyIncreaseThreshold(factor);
        resetTimer();
    }
    else if (!isSuccessful && retryCount >= decreaseThreshold - 1) // decreaseThreshold consecutive failed transmissions
    {
        numberOfConsSuccTransmissions = 0;
        currentMode = decreaseRateIfPossible(currentMode);
        emitDatarateChangedSignal();
        updateDisplayString();
        EV_DETAIL << "Decreased rate to " << *currentMode << endl;
        resetIncreaseThreshdold();
        resetTimer();
    }
    else if (isSuccessful && retryCount == 0)
        numberOfConsSuccTransmissions++;

    if (numberOfConsSuccTransmissions == increaseThreshold)
    {
        numberOfConsSuccTransmissions = 0;
        currentMode = increaseRateIfPossible(currentMode);
        emitDatarateChangedSignal();
        updateDisplayString();
        EV_DETAIL << "Increased rate to " << *currentMode << endl;
        resetTimer();
        probing = true;
    }
    else
        probing = false;

}

void AarfRateControl::multiplyIncreaseThreshold(double factor)
{
    if (increaseThreshold * factor <= maxIncreaseThreshold)
        increaseThreshold *= factor;
}

void AarfRateControl::resetIncreaseThreshdold()
{
    increaseThreshold = par("increaseThreshold");
}

void AarfRateControl::resetTimer()
{
    timer = simTime();
}

void AarfRateControl::increaseRateIfTimerIsExpired()
{
    if (simTime() - timer >= interval)
    {
        currentMode = increaseRateIfPossible(currentMode);
        emitDatarateChangedSignal();
        updateDisplayString();
        EV_DETAIL << "Increased rate to " << *currentMode << endl;
        resetTimer();
    }
}

void AarfRateControl::frameReceived(Packet *frame)
{
}

const IIeee80211Mode* AarfRateControl::getRate()
{
    Enter_Method_Silent("getRate");
    increaseRateIfTimerIsExpired();
    EV_INFO << "The current mode is " << currentMode << " the net bitrate is " << currentMode->getDataMode()->getNetBitrate() << std::endl;
    return currentMode;
}


} /* namespace ieee80211 */
} /* namespace inet */
