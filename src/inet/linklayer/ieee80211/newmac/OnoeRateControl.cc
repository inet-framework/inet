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

#include "OnoeRateControl.h"

namespace inet {
namespace ieee80211 {

Define_Module(OnoeRateControl);

void OnoeRateControl::initialize(const Ieee80211ModeSet* modeSet, const IIeee80211Mode *initialRate)
{
    this->modeSet = modeSet;
    this->currentMode = initialRate;
    interval = par("interval");
}

void OnoeRateControl::resetStatisticalVariables()
{
    numOfRetries = 0;
    numOfSuccTransmissions = 0;
    numOfGivenUpTransmissions = 0;
}

void OnoeRateControl::handleMessage(cMessage* msg)
{
    if (msg == timer)
    {
        computeMode();
        scheduleAt(simTime() + interval, timer);
    }
    else
        throw cRuntimeError("Unknown msg = %s", msg->getName());
}

void OnoeRateControl::frameTransmitted(const Ieee80211Frame* frame, const IIeee80211Mode* mode, int retryCount, bool isSuccessful, bool isGivenUp)
{
    if (isSuccessful)
        numOfSuccTransmissions++;
    else if (isGivenUp)
        numOfGivenUpTransmissions++;
    if (retryCount > 0)
        numOfRetries++;
}

void OnoeRateControl::frameReceived(const Ieee80211Frame* frame, const Ieee80211ReceptionIndication* receptionIndication)
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
            currentMode = modeSet->getSlowerMode(currentMode);
            credit = 0;
        }
        else if (avgRetriesPerFrame >= 0.1)
            credit--;
        else
            credit++;

        if (credit >= 10)
        {
            currentMode = modeSet->getFasterMode(currentMode);
            credit = 0;
        }

        resetStatisticalVariables();
    }
}

const IIeee80211Mode* OnoeRateControl::getRate() const
{
    return currentMode;
}


} /* namespace ieee80211 */
} /* namespace inet */
