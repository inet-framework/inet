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
        WATCH(numOfRetries);
        WATCH(credit);
        WATCH(numOfSuccTransmissions);
        WATCH(numOfGivenUpTransmissions);
        WATCH(avgRetriesPerFrame);
        updateDisplayString();
    }
}

void OnoeRateControl::updateDisplayString()
{
    getDisplayString().setTagArg("t", 0, currentMode->getName());
}

void OnoeRateControl::resetStatisticalVariables()
{
    numOfRetries = 0;
    numOfSuccTransmissions = 0;
    numOfGivenUpTransmissions = 0;
}

void OnoeRateControl::handleMessage(cMessage* msg)
{
    throw cRuntimeError("This module doesn't handle self messages");
}

void OnoeRateControl::frameTransmitted(Packet *frame, int retryCount, bool isSuccessful, bool isGivenUp)
{
    computeModeIfTimerIsExpired();
    if (isSuccessful)
        numOfSuccTransmissions++;
    else if (isGivenUp)
        numOfGivenUpTransmissions++;
    if (retryCount > 0)
        numOfRetries++;
}

void OnoeRateControl::computeModeIfTimerIsExpired()
{
    if (simTime() - timer >= interval)
    {
        computeMode();
        timer = simTime();
    }
}

void OnoeRateControl::frameReceived(Packet *frame)
{
}

void OnoeRateControl::computeMode()
{
    int numOfFrameTransmitted = numOfSuccTransmissions + numOfGivenUpTransmissions + numOfRetries;
    avgRetriesPerFrame = double(numOfRetries) / (numOfSuccTransmissions + numOfGivenUpTransmissions);

    if (numOfSuccTransmissions > 0)
    {
        if (numOfFrameTransmitted >= 10 && avgRetriesPerFrame > 1)
        {
            currentMode = decreaseRateIfPossible(currentMode);
            emitDatarateChangedSignal();
            updateDisplayString();
            EV_DETAIL << "Decreased rate to " << *currentMode << endl;
            credit = 0;
        }
        else if (avgRetriesPerFrame >= 0.1)
            credit--;
        else
            credit++;

        if (credit >= 10)
        {
            currentMode = increaseRateIfPossible(currentMode);
            emitDatarateChangedSignal();
            updateDisplayString();
            EV_DETAIL << "Increased rate to " << *currentMode << endl;
            credit = 0;
        }

        resetStatisticalVariables();
    }
}

const IIeee80211Mode* OnoeRateControl::getRate()
{
    Enter_Method_Silent("getRate");
    computeModeIfTimerIsExpired();
    EV_INFO << "The current mode is " << currentMode << " the net bitrate is " << currentMode->getDataMode()->getNetBitrate() << std::endl;
    return currentMode;
}


} /* namespace ieee80211 */
} /* namespace inet */
