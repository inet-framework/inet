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

#include "BasicStatistics.h"
#include "MacUtils.h"
#include "IRateControl.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"

namespace inet {
namespace ieee80211 {

Define_Module(BasicStatistics);

void BasicStatistics::initialize()
{
    resetStatistics();

    WATCH(numRetry);
    WATCH(numSentWithoutRetry);
    WATCH(numGivenUp);
    WATCH(numCollision);
    WATCH(numSent);
    WATCH(numSentBroadcast);

    WATCH(numReceivedUnicast);
    WATCH(numReceivedBroadcast);
    WATCH(numReceivedMulticast);
    WATCH(numReceivedNotForUs);
    WATCH(numReceivedErroneous);
}

void BasicStatistics::resetStatistics()
{
    numRetry = 0;
    numSentWithoutRetry = 0;
    numGivenUp = 0;
    numCollision = 0;
    numSent = 0;
    numSentBroadcast = 0;

    numReceivedUnicast = 0;
    numReceivedMulticast = 0;
    numReceivedBroadcast = 0;
    numReceivedNotForUs = 0;
    numReceivedErroneous = 0;
}

void BasicStatistics::finish()
{
    // transmit statistics
//    recordScalar("number of collisions", numCollision);
//    recordScalar("number of internal collisions", numInternalCollision);
//    for (int i = 0; i < numCategories(); i++) {
//        std::stringstream os;
//        os << i;
//        std::string th = "number of retry for AC " + os.str();
//        recordScalar(th.c_str(), numRetry(i));
//    }
//    recordScalar("sent and received bits", numBits);
//    for (int i = 0; i < numCategories(); i++) {
//        std::stringstream os;
//        os << i;
//        std::string th = "sent packet within AC " + os.str();
//        recordScalar(th.c_str(), numSent(i));
//    }
//    recordScalar("sent in TXOP ", numSentTXOP);
//    for (int i = 0; i < numCategories(); i++) {
//        std::stringstream os;
//        os << i;
//        std::string th = "sentWithoutRetry AC " + os.str();
//        recordScalar(th.c_str(), numSentWithoutRetry(i));
//    }
//    for (int i = 0; i < numCategories(); i++) {
//        std::stringstream os;
//        os << i;
//        std::string th = "numGivenUp AC " + os.str();
//        recordScalar(th.c_str(), numGivenUp(i));
//    }
//    for (int i = 0; i < numCategories(); i++) {
//        std::stringstream os;
//        os << i;
//        std::string th = "numDropped AC " + os.str();
//        recordScalar(th.c_str(), numDropped(i));
//    }

    // receive statistics
    recordScalar("numReceivedUnicast", numReceivedUnicast);
    recordScalar("numReceivedBroadcast", numReceivedBroadcast);
    recordScalar("numReceivedMulticast", numReceivedMulticast);
    recordScalar("numReceivedNotForUs", numReceivedNotForUs);
    recordScalar("numReceivedErroneous", numReceivedErroneous);
}

void BasicStatistics::setMacUtils(MacUtils *utils)
{
    this->utils = utils;
}

void BasicStatistics::setRateControl(IRateControl *rateControl)
{
    this->rateControl = rateControl;
}

void BasicStatistics::frameTransmissionSuccessful(Ieee80211DataOrMgmtFrame *frame, int retryCount)
{
    if (rateControl)
        rateControl->frameTransmitted(frame, utils->getFrameMode(frame), retryCount, true, false);
}

void BasicStatistics::frameTransmissionUnsuccessful(Ieee80211DataOrMgmtFrame *frame, int retryCount)
{
    if (rateControl)
        rateControl->frameTransmitted(frame, utils->getFrameMode(frame), retryCount, false, false);
}

void BasicStatistics::frameTransmissionUnsuccessfulGivingUp(Ieee80211DataOrMgmtFrame *frame, int retryCount)
{
    if (rateControl)
        rateControl->frameTransmitted(frame, utils->getFrameMode(frame), retryCount, false, true); //TODO for the last frame, both Unsuccessful() and GivenUp() will be called -- duplicate call!
}

void BasicStatistics::frameTransmissionGivenUp(Ieee80211DataOrMgmtFrame *frame)
{
}

void BasicStatistics::frameReceived(Ieee80211Frame *frame)
{
    if (dynamic_cast<Ieee80211DataOrMgmtFrame *>(frame)) {
        if (!utils->isForUs(frame))
            numReceivedNotForUs++;
        else if (utils->isBroadcast(frame))
            numReceivedBroadcast++;
        else if (utils->isBroadcastOrMulticast(frame))
            numReceivedMulticast++;
        else
            numReceivedUnicast++;
    }

    if (rateControl) {
        auto receptionIndication = check_and_cast<Ieee80211ReceptionIndication*>(frame->getControlInfo());
        rateControl->frameReceived(frame, receptionIndication);
    }
}

void BasicStatistics::erroneousFrameReceived()
{
    numReceivedErroneous++;
}

}  // namespace ieee80211
}  // namespace inet

