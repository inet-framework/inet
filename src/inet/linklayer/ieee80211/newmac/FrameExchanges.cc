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
#include "inet/common/INETUtils.h"
#include "inet/common/FSMA.h"
#include "IContentionTx.h"
#include "IImmediateTx.h"
#include "IMacParameters.h"
#include "MacUtils.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"

using namespace inet::utils;

namespace inet {
namespace ieee80211 {

//TODO utilize rx->isReceptionInProgress() after AckTimeout-AckDuration to determine if a frame (expectedly an ACK) is being received, and start retransmission immediately if not

SendDataWithAckFsmBasedFrameExchange::SendDataWithAckFsmBasedFrameExchange(FrameExchangeContext *context, IFinishedCallback *callback, Ieee80211DataOrMgmtFrame *frame, int txIndex, AccessCategory accessCategory) :
    FsmBasedFrameExchange(context, callback), frame(frame), txIndex(txIndex), accessCategory(accessCategory)
{
    frame->setDuration(params->getSifsTime() + utils->getAckDuration());
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
                    event == EVENT_FRAMEARRIVED && isAck(receivedFrame),
                    SUCCESS,
                    { frameProcessed = true;
                      delete receivedFrame;
                    }
                    );
            FSMA_Event_Transition(Frame - arrived,
                    event == EVENT_FRAMEARRIVED && !isAck(receivedFrame),
                    FAILURE,
                    ;
                    );
            FSMA_Event_Transition(Ack - timeout - retry,
                    event == EVENT_TIMER && retryCount < params->getShortRetryLimit(),
                    TRANSMITDATA,
                    retryDataFrame();
                    );
            FSMA_Event_Transition(Ack - timeout - giveup,
                    event == EVENT_TIMER && retryCount == params->getShortRetryLimit(),
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
    AccessCategory ac = accessCategory; // abbreviate
    contentionTx[txIndex]->transmitContentionFrame(dupPacketAndControlInfo(frame), params->getAifsTime(ac), params->getEifsTime(ac), params->getCwMin(ac), params->getCwMax(ac), params->getSlotTime(), retryCount, this);
}

void SendDataWithAckFsmBasedFrameExchange::retryDataFrame()
{
    retryCount++;
    frame->setRetry(true);
    AccessCategory ac = accessCategory; // abbreviate
    contentionTx[txIndex]->transmitContentionFrame(dupPacketAndControlInfo(frame), params->getAifsTime(ac), params->getEifsTime(ac), params->getCwMin(ac), params->getCwMax(ac), params->getSlotTime(), retryCount, this);
}

void SendDataWithAckFsmBasedFrameExchange::scheduleAckTimeout()
{
    if (ackTimer == nullptr)
        ackTimer = new cMessage("timeout");
    simtime_t t = simTime() + utils->getAckFullTimeout();
    scheduleAt(t, ackTimer);
}

bool SendDataWithAckFsmBasedFrameExchange::isAck(Ieee80211Frame *frame)
{
    return dynamic_cast<Ieee80211ACKFrame *>(frame) != nullptr;
}

//------------------------------

SendDataWithAckFrameExchange::SendDataWithAckFrameExchange(FrameExchangeContext *context, IFinishedCallback *callback, Ieee80211DataOrMgmtFrame *dataFrame, int txIndex, AccessCategory accessCategory) :
    StepBasedFrameExchange(context, callback, txIndex, accessCategory), dataFrame(dataFrame)
{
    dataFrame->setDuration(params->getSifsTime() + utils->getAckDuration());
}

SendDataWithAckFrameExchange::~SendDataWithAckFrameExchange()
{
    delete dataFrame;
}

void SendDataWithAckFrameExchange::doStep(int step)
{
    switch (step) {
        case 0: transmitContentionFrame(dupPacketAndControlInfo(dataFrame), retryCount); break;
        case 1: expectReply(utils->getAckFullTimeout()); break;
        case 2: succeed(); break;
        default: ASSERT(false);
    }
}

bool SendDataWithAckFrameExchange::processReply(int step, Ieee80211Frame *frame)
{
    switch (step) {
        case 1: if (utils->isAck(frame)) {delete frame; return true;} else return false;
        default: ASSERT(false); return false;
    }
}

void SendDataWithAckFrameExchange::processTimeout(int step)
{
    switch (step) {
        case 1: if (++retryCount < params->getShortRetryLimit()) {dataFrame->setRetry(true); gotoStep(0);} else fail(); break;
        default: ASSERT(false);
    }
}

void SendDataWithAckFrameExchange::processInternalCollision(int step)
{
    switch (step) {
        case 0: if (++retryCount < params->getShortRetryLimit()) {gotoStep(0);} else fail(); break;
        default: ASSERT(false);
    }
}

//------------------------------

SendDataWithRtsCtsFrameExchange::SendDataWithRtsCtsFrameExchange(FrameExchangeContext *context, IFinishedCallback *callback, Ieee80211DataOrMgmtFrame *dataFrame, int txIndex, AccessCategory accessCategory) :
    StepBasedFrameExchange(context, callback, txIndex, accessCategory), dataFrame(dataFrame)
{
    dataFrame->setDuration(params->getSifsTime() + utils->getAckDuration());
}

SendDataWithRtsCtsFrameExchange::~SendDataWithRtsCtsFrameExchange()
{
    delete dataFrame;
}

void SendDataWithRtsCtsFrameExchange::doStep(int step)
{
    switch (step) {
        case 0: transmitContentionFrame(utils->buildRtsFrame(dataFrame, params->getDefaultDataFrameMode()), retryCount); break;
        case 1: expectReply(utils->getCtsFullTimeout()); break;
        case 2: transmitImmediateFrame(dupPacketAndControlInfo(dataFrame), params->getSifsTime()); break;
        case 3: expectReply(utils->getAckFullTimeout()); break;
        case 4: succeed(); break;
        default: ASSERT(false);
    }
}

bool SendDataWithRtsCtsFrameExchange::processReply(int step, Ieee80211Frame *frame)
{
    switch (step) {
        case 1: if (utils->isCts(frame)) {delete frame; return true;} else return false;
        case 3: if (utils->isAck(frame)) {delete frame; return true;} else return false;
        default: ASSERT(false); return false;
    }
}

void SendDataWithRtsCtsFrameExchange::processTimeout(int step)
{
    switch (step) {
        case 1: if (++retryCount < params->getShortRetryLimit()) {gotoStep(0);} else fail(); break;
        case 3: fail(); break;
        default: ASSERT(false);
    }
}

void SendDataWithRtsCtsFrameExchange::processInternalCollision(int step)
{
    switch (step) {
        case 0: if (++retryCount < params->getShortRetryLimit()) {gotoStep(0);} else fail(); break;
        default: ASSERT(false);
    }
}

//------------------------------

SendMulticastDataFrameExchange::SendMulticastDataFrameExchange(FrameExchangeContext *context, IFinishedCallback *callback, Ieee80211DataOrMgmtFrame *dataFrame, int txIndex, AccessCategory accessCategory) :
    FrameExchange(context, callback), dataFrame(dataFrame), txIndex(txIndex), accessCategory(accessCategory)
{
    ASSERT(utils->isBroadcast(dataFrame) || utils->isMulticast(dataFrame));
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
    if (++retryCount < params->getShortRetryLimit()) {
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
    AccessCategory ac = accessCategory;  // abbreviate
    contentionTx[txIndex]->transmitContentionFrame(dupPacketAndControlInfo(dataFrame), params->getAifsTime(ac), params->getEifsTime(ac), params->getCwMulticast(ac), params->getCwMulticast(ac), params->getSlotTime(), 0, this);
}

} // namespace ieee80211
} // namespace inet

