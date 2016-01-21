//
// Copyright (C) 2015 Andras Varga
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#include "AARFRateControl.h"

namespace inet {
namespace ieee80211 {

Define_Module(AARFRateControl);

void AARFRateControl::initialize(const Ieee80211ModeSet* modeSet, const IIeee80211Mode *initialMode)
{
    Enter_Method_Silent("initialize()");
    RateControlBase::initialize(modeSet);
    currentMode = initialMode;
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
    updateDisplayString();
}

void AARFRateControl::handleMessage(cMessage* msg)
{
    throw cRuntimeError("This module doesn't handle self messages");
}

void AARFRateControl::updateDisplayString()
{
    getDisplayString().setTagArg("t", 0, currentMode->getName());
}

void AARFRateControl::frameTransmitted(const Ieee80211Frame* frame, const IIeee80211Mode* mode, int retryCount, bool isSuccessful, bool isGivenUp)
{
    increaseRateIfTimerIsExpired();

    if (!isSuccessful && probing) // probing packet failed
    {
        numberOfConsSuccTransmissions = 0;
        currentMode = decreaseRateIfPossible(currentMode);
        updateDisplayString();
        EV_DETAIL << "Decreased rate to " << *currentMode << endl;
        multiplyIncreaseThreshold(factor);
        resetTimer();
    }
    else if (!isSuccessful && retryCount >= decreaseThreshold - 1) // decreaseThreshold consecutive failed transmissions
    {
        numberOfConsSuccTransmissions = 0;
        currentMode = decreaseRateIfPossible(currentMode);
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
        updateDisplayString();
        EV_DETAIL << "Increased rate to " << *currentMode << endl;
        resetTimer();
        probing = true;
    }
    else
        probing = false;

}

void AARFRateControl::multiplyIncreaseThreshold(double factor)
{
    if (increaseThreshold * factor <= maxIncreaseThreshold)
        increaseThreshold *= factor;
}

void AARFRateControl::resetIncreaseThreshdold()
{
    increaseThreshold = par("increaseThreshold");
}

void AARFRateControl::resetTimer()
{
    timer = simTime();
}

void AARFRateControl::increaseRateIfTimerIsExpired()
{
    if (simTime() - timer >= interval)
    {
        currentMode = increaseRateIfPossible(currentMode);
        updateDisplayString();
        EV_DETAIL << "Increased rate to " << *currentMode << endl;
        resetTimer();
    }
}

void AARFRateControl::frameReceived(const Ieee80211Frame* frame, const Ieee80211ReceptionIndication* receptionIndication)
{
}

const IIeee80211Mode* AARFRateControl::getRate()
{
    Enter_Method_Silent("getRate()");
    increaseRateIfTimerIsExpired();
    EV_INFO << "The current mode is " << currentMode << " the net bitrate is " << currentMode->getDataMode()->getNetBitrate() << std::endl;
    return currentMode;
}


} /* namespace ieee80211 */
} /* namespace inet */
