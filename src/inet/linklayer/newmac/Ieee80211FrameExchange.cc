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

#include "Ieee80211FrameExchange.h"
#include "IIeee80211UpperMacContext.h"
#include "Ieee80211UpperMac.h"
#include "IIeee80211MacTx.h"
#include "IIeee80211MacImmediateTx.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"

namespace inet {
namespace ieee80211 {

void Ieee80211FrameExchange::reportSuccess()
{
    finishedCallback->frameExchangeFinished(this, true);
}

void Ieee80211FrameExchange::reportFailure()
{
    finishedCallback->frameExchangeFinished(this, false);
}

//--------

void Ieee80211StepBasedFrameExchange::start()
{
    ASSERT(step == 0);
    step--;  // will be incremented in proceed()
    proceed();
}

void Ieee80211StepBasedFrameExchange::proceed()
{
    if (status == INPROGRESS) {
        step++;
        stepType = NONE;
        doStep(step);
        ASSERT(stepType != NONE);  // must do something
    }
}

bool Ieee80211StepBasedFrameExchange::lowerFrameReceived(Ieee80211Frame* frame)
{
    if (stepType != EXPECT_REPLY)
        return false;  // not ready to process frames
    else {
        bool accepted = processReply(step, frame);
        if (!accepted)
            return false;  // not for us
        else {
            proceed();
            return true;
        }
    }
}

void Ieee80211StepBasedFrameExchange::transmissionFinished()
{
    ASSERT(status == INPROGRESS);
    ASSERT(stepType == TRANSMIT_CONTENTION_FRAME || stepType == TRANSMIT_IMMEDIATE_FRAME);
    proceed();
}

void Ieee80211StepBasedFrameExchange::handleMessage(cMessage* msg)
{
    if (status != INPROGRESS)
        return;  // too late, frame exchange already finished
    ASSERT(msg == timeoutMsg);
    processTimeout(step);
    proceed();
}

void Ieee80211StepBasedFrameExchange::transmitContentionFrame(Ieee80211Frame* frame, int retryCount)
{
    ASSERT(status == INPROGRESS);
    ASSERT(stepType == NONE);
    mac->tx->transmitContentionFrame(frame, context->getDIFS(), context->getEIFS(), context->getMinCW(), context->getMaxCW(), retryCount, getUpperMac());
    stepType = TRANSMIT_CONTENTION_FRAME;
}

void Ieee80211StepBasedFrameExchange::transmitContentionFrame(Ieee80211Frame* frame, simtime_t ifs, simtime_t eifs, int cwMin, int cwMax, int retryCount)
{
    ASSERT(status == INPROGRESS);
    ASSERT(stepType == NONE);
    mac->tx->transmitContentionFrame(frame, ifs, eifs, cwMin, cwMax, retryCount, getUpperMac());
    stepType = TRANSMIT_CONTENTION_FRAME;
}

void Ieee80211StepBasedFrameExchange::transmitImmediateFrame(Ieee80211Frame* frame, simtime_t ifs)
{
    ASSERT(status == INPROGRESS);
    ASSERT(stepType == NONE);
    mac->immediateTx->transmitImmediateFrame(frame, ifs, getUpperMac());
    stepType = TRANSMIT_IMMEDIATE_FRAME;
}

void Ieee80211StepBasedFrameExchange::expectReply(simtime_t timeout)
{
    ASSERT(status == INPROGRESS);
    ASSERT(stepType == NONE);
    if (!timeoutMsg)
        timeoutMsg = new cMessage("timeout");
    scheduleAt(simTime() + timeout, timeoutMsg);
    stepType = EXPECT_REPLY;
}

void Ieee80211StepBasedFrameExchange::gotoStep(int step)
{
    ASSERT(status == INPROGRESS);
    this->step = step-1;  // will be incremented in proceed()
}

void Ieee80211StepBasedFrameExchange::fail()
{
    ASSERT(status == INPROGRESS);
    status = FAILED;
    reportFailure();
}

void Ieee80211StepBasedFrameExchange::succeed()
{
    ASSERT(status == INPROGRESS);
    status = SUCCEEDED;
    reportSuccess();
}

}
} /* namespace inet */

