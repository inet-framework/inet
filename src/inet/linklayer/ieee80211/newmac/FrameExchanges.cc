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
#include "inet/common/INETUtils.h"
#include "IUpperMacContext.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"

using namespace inet::utils;

namespace inet {
namespace ieee80211 {

SendDataWithAckFsmBasedFrameExchange::SendDataWithAckFsmBasedFrameExchange(cSimpleModule *ownerModule, IUpperMacContext *context, IFinishedCallback *callback, Ieee80211DataOrMgmtFrame *frame, int txIndex, int accessCategory) :
    FsmBasedFrameExchange(ownerModule, context, callback), frame(frame), txIndex(txIndex), accessCategory(accessCategory)
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
    Ieee80211Frame *receivedFrame = event == EVENT_FRAMEARRIVED ? check_and_cast<Ieee80211Frame *>(frameOrTimer) : nullptr;

    FSMA_Switch(fsm) {
        FSMA_State(INIT) {
            FSMA_Enter();
            FSMA_Event_Transition(Start,
                    event == EVENT_START,
                    TRANSMITDATA,
                    transmitDataFrame();
                    );
        }
        FSMA_State(TRANSMITDATA) {
            FSMA_Enter();
            FSMA_Event_Transition(Wait - ACK,
                    event == EVENT_TXFINISHED,
                    WAITACK,
                    scheduleAckTimeout();
                    );
            FSMA_Event_Transition(Frame - arrived,
                    event == EVENT_FRAMEARRIVED,
                    TRANSMITDATA,
                    ;
                    );
        }
        FSMA_State(WAITACK) {
            FSMA_Enter();
            FSMA_Event_Transition(Ack - arrived,
                    event == EVENT_FRAMEARRIVED && isAck(receivedFrame),    //TODO is from right STA
                    SUCCESS,
                    { frameProcessed = true;
                      delete receivedFrame;
                    }
                    );
            FSMA_Event_Transition(Frame - arrived,
                    event == EVENT_FRAMEARRIVED && !isAck(receivedFrame),    //TODO is from right STA
                    FAILURE,
                    ;
                    );
            FSMA_Event_Transition(Ack - timeout - retry,
                    event == EVENT_TIMER && retryCount < context->getShortRetryLimit(),
                    TRANSMITDATA,
                    retryDataFrame();
                    );
            FSMA_Event_Transition(Ack - timeout - giveup,
                    event == EVENT_TIMER && retryCount == context->getShortRetryLimit(),
                    FAILURE,
                    ;
                    );
        }
        FSMA_State(SUCCESS) {
            FSMA_Enter(reportSuccess());
        }
        FSMA_State(FAILURE) {
            FSMA_Enter(reportFailure());
        }
    }
    return frameProcessed;
}

void SendDataWithAckFsmBasedFrameExchange::transmitDataFrame()
{
    retryCount = 0;
    int ac = accessCategory; // abbreviate
    context->transmitContentionFrame(txIndex, dupPacketAndControlInfo(frame), context->getAifsTime(ac), context->getEifsTime(ac), context->getCwMin(ac), context->getCwMax(ac), context->getSlotTime(), retryCount, this);
}

void SendDataWithAckFsmBasedFrameExchange::retryDataFrame()
{
    retryCount++;
    frame->setRetry(true);
    int ac = accessCategory; // abbreviate
    context->transmitContentionFrame(txIndex, dupPacketAndControlInfo(frame), context->getAifsTime(ac), context->getEifsTime(ac), context->getCwMin(ac), context->getCwMax(ac), context->getSlotTime(), retryCount, this);
}

void SendDataWithAckFsmBasedFrameExchange::scheduleAckTimeout()
{
    if (ackTimer == nullptr)
        ackTimer = new cMessage("timeout");
    simtime_t t = simTime() + context->getAckTimeout();
    scheduleAt(t, ackTimer);
}

bool SendDataWithAckFsmBasedFrameExchange::isAck(Ieee80211Frame *frame)
{
    return dynamic_cast<Ieee80211ACKFrame *>(frame) != nullptr;
}

//------------------------------

SendDataWithAckFrameExchange::SendDataWithAckFrameExchange(cSimpleModule *ownerModule, IUpperMacContext *context, IFinishedCallback *callback, Ieee80211DataOrMgmtFrame *dataFrame, int txIndex, int accessCategory) :
    StepBasedFrameExchange(ownerModule, context, callback, txIndex, accessCategory), dataFrame(dataFrame)
{
    dataFrame->setDuration(context->getSifsTime() + context->getAckDuration());
}

SendDataWithAckFrameExchange::~SendDataWithAckFrameExchange()
{
    delete dataFrame;
}

void SendDataWithAckFrameExchange::doStep(int step)
{
    switch (step) {
        case 0: transmitContentionFrame(dupPacketAndControlInfo(dataFrame), retryCount); break;
        case 1: expectReply(context->getAckTimeout()); break;
        case 2: succeed(); break;
        default: ASSERT(false);
    }
}

bool SendDataWithAckFrameExchange::processReply(int step, Ieee80211Frame *frame)
{
    switch (step) {
        case 1: if (context->isAck(frame)) {delete frame; return true;} else return false;
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

void SendDataWithAckFrameExchange::processInternalCollision(int step)
{
    switch (step) {
        case 0: if (++retryCount < context->getShortRetryLimit()) {gotoStep(0);} else fail(); break;
        default: ASSERT(false);
    }
}

//------------------------------

SendDataWithRtsCtsFrameExchange::SendDataWithRtsCtsFrameExchange(cSimpleModule *ownerModule, IUpperMacContext *context, IFinishedCallback *callback, Ieee80211DataOrMgmtFrame *dataFrame, int txIndex, int accessCategory) :
    StepBasedFrameExchange(ownerModule, context, callback, txIndex, accessCategory), dataFrame(dataFrame)
{
    dataFrame->setDuration(context->getSifsTime() + context->getAckDuration());
}

SendDataWithRtsCtsFrameExchange::~SendDataWithRtsCtsFrameExchange()
{
    delete dataFrame;
}

void SendDataWithRtsCtsFrameExchange::doStep(int step)
{
    switch (step) {
        case 0: transmitContentionFrame(context->buildRtsFrame(dataFrame), retryCount); break;
        case 1: expectReply(context->getCtsTimeout()); break;
        case 2: transmitImmediateFrame(dupPacketAndControlInfo(dataFrame), context->getSifsTime()); break;
        case 3: expectReply(context->getAckTimeout()); break;
        case 4: succeed(); break;
        default: ASSERT(false);
    }
}

bool SendDataWithRtsCtsFrameExchange::processReply(int step, Ieee80211Frame *frame)
{
    switch (step) {
        case 1: if (context->isCts(frame)) {delete frame; return true;} else return false;
        case 3: if (context->isAck(frame)) {delete frame; return true;} else return false;
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

void SendDataWithRtsCtsFrameExchange::processInternalCollision(int step)
{
    switch (step) {
        case 0: if (++retryCount < context->getShortRetryLimit()) {gotoStep(0);} else fail(); break;
        default: ASSERT(false);
    }
}

//------------------------------

SendMulticastDataFrameExchange::SendMulticastDataFrameExchange(cSimpleModule *ownerModule, IUpperMacContext *context, IFinishedCallback *callback, Ieee80211DataOrMgmtFrame *dataFrame, int txIndex, int accessCategory) :
    FrameExchange(ownerModule, context, callback), dataFrame(dataFrame), txIndex(txIndex), accessCategory(accessCategory)
{
    ASSERT(context->isBroadcast(dataFrame) || context->isMulticast(dataFrame));
    dataFrame->setDuration(0);
}

SendMulticastDataFrameExchange::~SendMulticastDataFrameExchange()
{
    delete dataFrame;
}

void SendMulticastDataFrameExchange::start()
{
    transmitFrame();
}

bool SendMulticastDataFrameExchange::lowerFrameReceived(Ieee80211Frame *frame)
{
    return false;  // not ours
}

void SendMulticastDataFrameExchange::transmissionComplete(int txIndex)
{
    reportSuccess();
}

void SendMulticastDataFrameExchange::internalCollision(int txIndex)
{
    if (++retryCount < context->getShortRetryLimit()) {
        dataFrame->setRetry(true);
        transmitFrame();
    }
    else {
        reportFailure();
    }
}

void SendMulticastDataFrameExchange::handleSelfMessage(cMessage *msg)
{
    ASSERT(false);
}

void SendMulticastDataFrameExchange::transmitFrame()
{
    int ac = accessCategory;  // abbreviate
    context->transmitContentionFrame(txIndex, dupPacketAndControlInfo(dataFrame), context->getAifsTime(ac), context->getEifsTime(ac), context->getCwMulticast(ac), context->getCwMulticast(ac), context->getSlotTime(), 0, this);
}

} // namespace ieee80211
} // namespace inet

