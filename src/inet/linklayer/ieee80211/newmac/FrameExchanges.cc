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
#include "UpperMac.h"
#include "IUpperMacContext.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"

namespace inet {
namespace ieee80211 {

Ieee80211SendDataWithAckFSMBasedFrameExchange::Ieee80211SendDataWithAckFSMBasedFrameExchange(cSimpleModule *ownerModule, IUpperMacContext *context, IFinishedCallback *callback, Ieee80211DataOrMgmtFrame *frame) :
    Ieee80211FSMBasedFrameExchange(ownerModule, context, callback), frame(frame)
{
    frame->setDuration(context->getSIFS() + context->getAckDuration());
}

Ieee80211SendDataWithAckFSMBasedFrameExchange::~Ieee80211SendDataWithAckFSMBasedFrameExchange()
{
    delete frame;
    if (ackTimer)
        delete cancelEvent(ackTimer);
}

bool Ieee80211SendDataWithAckFSMBasedFrameExchange::handleWithFSM(EventType event, cMessage *frameOrTimer)
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

void Ieee80211SendDataWithAckFSMBasedFrameExchange::transmitDataFrame()
{
    retryCount = 0;
    int txIndex = 0; //TODO
    context->transmitContentionFrame(txIndex, frame->dup(), context->getDIFS(), context->getEIFS(), context->getMinCW(), context->getMaxCW(), context->getSlotTime(), retryCount, this);
}

void Ieee80211SendDataWithAckFSMBasedFrameExchange::retryDataFrame()
{
    retryCount++;
    frame->setRetry(true);
    int txIndex = 0; //TODO
    context->transmitContentionFrame(txIndex, frame->dup(), context->getDIFS(), context->getEIFS(), context->getMinCW(), context->getMaxCW(), context->getSlotTime(), retryCount, this);
}

void Ieee80211SendDataWithAckFSMBasedFrameExchange::scheduleAckTimeout()
{
    if (ackTimer == nullptr)
        ackTimer = new cMessage("timeout");
    simtime_t t = simTime() + context->getAckTimeout();
    scheduleAt(t, ackTimer);
}

bool Ieee80211SendDataWithAckFSMBasedFrameExchange::isAck(Ieee80211Frame* frame)
{
    return dynamic_cast<Ieee80211ACKFrame *>(frame) != nullptr;
}

//------------------------------

Ieee80211SendDataWithAckFrameExchange::Ieee80211SendDataWithAckFrameExchange(cSimpleModule *ownerModule, IUpperMacContext *context, IFinishedCallback *callback, Ieee80211DataOrMgmtFrame *dataFrame) :
    Ieee80211StepBasedFrameExchange(ownerModule, context, callback), dataFrame(dataFrame)
{
}

void Ieee80211SendDataWithAckFrameExchange::doStep(int step)
{
    switch (step) {
        case 0: transmitContentionFrame(dataFrame->dup(), retryCount); break;
        case 1: expectReply(context->getAckTimeout()); break;
        case 2: succeed(); break;
        default: ASSERT(false);
    }
}

bool Ieee80211SendDataWithAckFrameExchange::processReply(int step, Ieee80211Frame *frame)
{
    switch (step) {
        case 1: return context->isAck(frame);
        default: ASSERT(false); return false;
    }
}

void Ieee80211SendDataWithAckFrameExchange::processTimeout(int step)
{
    switch (step) {
        case 1: if (++retryCount < context->getShortRetryLimit()) {dataFrame->setRetry(true); gotoStep(0);} else fail(); break;
        default: ASSERT(false);
    }
}

void Ieee80211SendDataWithAckFrameExchange::processInternalCollision()
{
    switch (step) {
        case 0: if (++retryCount < context->getShortRetryLimit()) {gotoStep(0);} else fail(); break;
        default: ASSERT(false);
    }
}


//------------------------------


Ieee80211SendDataWithRtsCtsFrameExchange::Ieee80211SendDataWithRtsCtsFrameExchange(cSimpleModule *ownerModule, IUpperMacContext *context, IFinishedCallback *callback, Ieee80211DataOrMgmtFrame *dataFrame) :
    Ieee80211StepBasedFrameExchange(ownerModule, context, callback), dataFrame(dataFrame)
{
}

void Ieee80211SendDataWithRtsCtsFrameExchange::doStep(int step)
{
    switch (step) {
        case 0: transmitContentionFrame(context->buildRtsFrame(dataFrame), retryCount); break;
        case 1: expectReply(context->getCtsTimeout()); break;
        case 2: transmitImmediateFrame(dataFrame->dup(), context->getSIFS()); break;
        case 3: expectReply(context->getAckTimeout()); break;
        case 4: succeed(); break;
        default: ASSERT(false);
    }
}

bool Ieee80211SendDataWithRtsCtsFrameExchange::processReply(int step, Ieee80211Frame *frame)
{
    switch (step) {
        case 1: return context->isCts(frame);  // true=accepted
        case 3: return context->isAck(frame);
        default: ASSERT(false); return false;
    }
}

void Ieee80211SendDataWithRtsCtsFrameExchange::processTimeout(int step)
{
    switch (step) {
        case 1: if (++retryCount < context->getShortRetryLimit()) {gotoStep(0);} else fail(); break;
        case 3: fail(); break;
        default: ASSERT(false);
    }
}

void Ieee80211SendDataWithRtsCtsFrameExchange::processInternalCollision()
{
    switch (step) {
        case 0: if (++retryCount < context->getShortRetryLimit()) {gotoStep(0);} else fail(); break;
        default: ASSERT(false);
    }
}

} // namespace ieee80211
} // namespace inet

