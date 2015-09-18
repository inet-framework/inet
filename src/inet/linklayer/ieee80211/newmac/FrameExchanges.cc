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
// Author: Andras Varga
//

#include "FrameExchanges.h"
#include "inet/common/FSMA.h"
#include "IUpperMacContext.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"

namespace inet {
namespace ieee80211 {

SendDataWithAckFsmBasedFrameExchange::SendDataWithAckFsmBasedFrameExchange(cSimpleModule *ownerModule, IUpperMacContext *context, IFinishedCallback *callback, Ieee80211DataOrMgmtFrame *frame) :
    FsmBasedFrameExchange(ownerModule, context, callback), frame(frame)
{
    frame->setDuration(context->getSifsTime() + context->getAckDuration());
}

SendDataWithAckFsmBasedFrameExchange::~SendDataWithAckFsmBasedFrameExchange()
{
    delete frame;
    if (ackTimer)
        delete cancelEvent(ackTimer);
}

bool SendDataWithAckFsmBasedFrameExchange::handleWithFSM(EventType event, cMessage *frameOrTimer)
{
    bool frameProcessed = false;
    Ieee80211Frame *receivedFrame = event == EVENT_FRAMEARRIVED ? check_and_cast<Ieee80211Frame*>(frameOrTimer) : nullptr;

    FSMA_Switch(fsm)
    {
        FSMA_State(INIT)
        {
            FSMA_Enter();
            FSMA_Event_Transition(Start,
                                  event == EVENT_START,
                                  TRANSMITDATA,
                                  transmitDataFrame();
            );
        }
        FSMA_State(TRANSMITDATA)
        {
            FSMA_Enter();
            FSMA_Event_Transition(Wait-ACK,
                                  event == EVENT_TXFINISHED,
                                  WAITACK,
                                  scheduleAckTimeout();
            );
            FSMA_Event_Transition(Frame-arrived,
                                  event == EVENT_FRAMEARRIVED,
                                  TRANSMITDATA,
                                  ;
            );
        }
        FSMA_State(WAITACK)
        {
            FSMA_Enter();
            FSMA_Event_Transition(Ack-arrived,
                                  event == EVENT_FRAMEARRIVED && isAck(receivedFrame), //TODO is from right STA
                                  SUCCESS,
                                  {frameProcessed = true; delete receivedFrame;}
            );
            FSMA_Event_Transition(Frame-arrived,
                                  event == EVENT_FRAMEARRIVED && !isAck(receivedFrame), //TODO is from right STA
                                  FAILURE,
                                  ;
            );
            FSMA_Event_Transition(Ack-timeout-retry,
                                  event == EVENT_TIMER && retryCount < context->getShortRetryLimit(),
                                  TRANSMITDATA,
                                  retryDataFrame();
            );
            FSMA_Event_Transition(Ack-timeout-giveup,
                                  event == EVENT_TIMER && retryCount == context->getShortRetryLimit(),
                                  FAILURE,
                                  ;
            );
        }
        FSMA_State(SUCCESS)
        {
            FSMA_Enter(reportSuccess());
        }
        FSMA_State(FAILURE)
        {
            FSMA_Enter(reportFailure());
        }
    }
    return frameProcessed;
}

void SendDataWithAckFsmBasedFrameExchange::transmitDataFrame()
{
    retryCount = 0;
    int txIndex = 0; //TODO
    context->transmitContentionFrame(txIndex, frame->dup(), context->getDifsTime(), context->getEifsTime(), context->getCwMin(), context->getCwMax(), context->getSlotTime(), retryCount, this);
}

void SendDataWithAckFsmBasedFrameExchange::retryDataFrame()
{
    retryCount++;
    frame->setRetry(true);
    int txIndex = 0; //TODO
    context->transmitContentionFrame(txIndex, frame->dup(), context->getDifsTime(), context->getEifsTime(), context->getCwMin(), context->getCwMax(), context->getSlotTime(), retryCount, this);
}

void SendDataWithAckFsmBasedFrameExchange::scheduleAckTimeout()
{
    if (ackTimer == nullptr)
        ackTimer = new cMessage("timeout");
    simtime_t t = simTime() + context->getAckTimeout();
    scheduleAt(t, ackTimer);
}

bool SendDataWithAckFsmBasedFrameExchange::isAck(Ieee80211Frame* frame)
{
    return dynamic_cast<Ieee80211ACKFrame *>(frame) != nullptr;
}

//------------------------------

SendDataWithAckFrameExchange::SendDataWithAckFrameExchange(cSimpleModule *ownerModule, IUpperMacContext *context, IFinishedCallback *callback, Ieee80211DataOrMgmtFrame *dataFrame) :
    StepBasedFrameExchange(ownerModule, context, callback), dataFrame(dataFrame)
{
}

void SendDataWithAckFrameExchange::doStep(int step)
{
    switch (step) {
        case 0: transmitContentionFrame(dataFrame->dup(), retryCount); break;
        case 1: expectReply(context->getAckTimeout()); break;
        case 2: succeed(); break;
        default: ASSERT(false);
    }
}

bool SendDataWithAckFrameExchange::processReply(int step, Ieee80211Frame *frame)
{
    switch (step) {
        case 1: return context->isAck(frame);
        default: ASSERT(false); return false;
    }
}

void SendDataWithAckFrameExchange::processTimeout(int step)
{
    switch (step) {
        case 1: if (++retryCount < context->getShortRetryLimit()) {dataFrame->setRetry(true); gotoStep(0);} else fail(); break;
        default: ASSERT(false);
    }
}

void SendDataWithAckFrameExchange::processInternalCollision()
{
    switch (step) {
        case 0: if (++retryCount < context->getShortRetryLimit()) {gotoStep(0);} else fail(); break;
        default: ASSERT(false);
    }
}


//------------------------------


SendDataWithRtsCtsFrameExchange::SendDataWithRtsCtsFrameExchange(cSimpleModule *ownerModule, IUpperMacContext *context, IFinishedCallback *callback, Ieee80211DataOrMgmtFrame *dataFrame) :
    StepBasedFrameExchange(ownerModule, context, callback), dataFrame(dataFrame)
{
}

void SendDataWithRtsCtsFrameExchange::doStep(int step)
{
    switch (step) {
        case 0: transmitContentionFrame(context->buildRtsFrame(dataFrame), retryCount); break;
        case 1: expectReply(context->getCtsTimeout()); break;
        case 2: transmitImmediateFrame(dataFrame->dup(), context->getSifsTime()); break;
        case 3: expectReply(context->getAckTimeout()); break;
        case 4: succeed(); break;
        default: ASSERT(false);
    }
}

bool SendDataWithRtsCtsFrameExchange::processReply(int step, Ieee80211Frame *frame)
{
    switch (step) {
        case 1: return context->isCts(frame);  // true=accepted
        case 3: return context->isAck(frame);
        default: ASSERT(false); return false;
    }
}

void SendDataWithRtsCtsFrameExchange::processTimeout(int step)
{
    switch (step) {
        case 1: if (++retryCount < context->getShortRetryLimit()) {gotoStep(0);} else fail(); break;
        case 3: fail(); break;
        default: ASSERT(false);
    }
}

void SendDataWithRtsCtsFrameExchange::processInternalCollision()
{
    switch (step) {
        case 0: if (++retryCount < context->getShortRetryLimit()) {gotoStep(0);} else fail(); break;
        default: ASSERT(false);
    }
}

} // namespace ieee80211
} // namespace inet

