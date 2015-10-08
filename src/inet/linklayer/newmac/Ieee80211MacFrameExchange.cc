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

#include "Ieee80211MacFrameExchange.h"
#include "inet/common/FSMA.h"
#include "Ieee80211MacTransmission.h"

namespace inet {

void Ieee80211FrameExchange::reportSuccess()
{
    finishedCallback->frameExchangeFinished(this, true);
}

void Ieee80211FrameExchange::reportFailure()
{
    finishedCallback->frameExchangeFinished(this, false);
}

//--------

void Ieee80211SendDataWithAckFrameExchange::handleWithFSM(EventType event, cMessage *frameOrTimer)
{
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
                                  processFrame(receivedFrame);
            );
        }
        FSMA_State(WAITACK)
        {
            FSMA_Enter();
            FSMA_Event_Transition(Ack-arrived,
                                  event == EVENT_FRAMEARRIVED && isAck(receivedFrame),
                                  SUCCESS,
                                  delete receivedFrame;
            );
            FSMA_Event_Transition(Frame-arrived,
                                  event == EVENT_FRAMEARRIVED && !isAck(receivedFrame),
                                  FAILURE,
                                  processFrame(receivedFrame);
            );
            FSMA_Event_Transition(Ack-timeout-retry,
                                  event == EVENT_TIMER && retryCount < maxRetryCount,
                                  TRANSMITDATA,
                                  retryDataFrame();
            );
            FSMA_Event_Transition(Ack-timeout-giveup,
                                  event == EVENT_TIMER && retryCount == maxRetryCount,
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
}

void Ieee80211SendDataWithAckFrameExchange::transmitDataFrame()
{
    cw = cwMin;
    retryCount = 0;
    mac->transmission->transmitContentionFrame(frame, ifs, cw);
}

void Ieee80211SendDataWithAckFrameExchange::retryDataFrame()
{
    if (cw < cwMax)
        cw = ((cw+1)<<1)-1;
    retryCount++;
    frame->setRetry(true);
    mac->transmission->transmitContentionFrame(frame, ifs, cw);
}

void Ieee80211SendDataWithAckFrameExchange::scheduleAckTimeout()
{
    if (ackTimer == nullptr)
        ackTimer = new cMessage("timeout");
    simtime_t t = simTime() + 2*MAX_PROPAGATION_DELAY + SIFS + LENGTH_ACK / mac->basicBitrate + PHY_HEADER_LENGTH / BITRATE_HEADER; //TODO
    scheduleAt(t, ackTimer);
}

void Ieee80211SendDataWithAckFrameExchange::processFrame(Ieee80211Frame *receivedFrame)
{
    //TODO some totally unrelated frame arrived; process in the normal way
}

bool Ieee80211SendDataWithAckFrameExchange::isAck(Ieee80211Frame* frame)
{
    return dynamic_cast<Ieee80211ACKFrame *>(frame) != nullptr;
}


void Ieee80211SendRtsCtsFrameExchangeXXX::handleWithFSM(EventType event, cMessage *frameOrTimer)
{
    Ieee80211Frame *receivedFrame = event == EVENT_FRAMEARRIVED ? check_and_cast<Ieee80211Frame*>(frameOrTimer) : nullptr;

    FSMA_Switch(fsm)
    {
        FSMA_State(INIT)
        {
            FSMA_Enter();
            FSMA_Event_Transition(Start,
                                  event == EVENT_START,
                                  TRANSMITRTS,
                                  transmitRtsFrame();
            );
        }
        FSMA_State(TRANSMITRTS)
        {
            FSMA_Enter();
            FSMA_Event_Transition(Wait-CTS,
                                  event == EVENT_TXFINISHED,
                                  WAITCTS,
                                  scheduleCtsTimeout();
            );
            FSMA_Event_Transition(Frame-arrived,
                                  event == EVENT_FRAMEARRIVED,
                                  TRANSMITRTS,
                                  processFrame(receivedFrame);
            );
        }
        FSMA_State(WAITCTS)
        {
            FSMA_Enter();
            FSMA_Event_Transition(Cts-arrived,
                                  event == EVENT_FRAMEARRIVED && isCts(receivedFrame),
                                  SUCCESS,
                                  delete receivedFrame;
            );
            FSMA_Event_Transition(Frame-arrived,
                                  event == EVENT_FRAMEARRIVED && !isCts(receivedFrame),
                                  FAILURE,
                                  processFrame(receivedFrame);
            );
            FSMA_Event_Transition(Cts-timeout-retry,
                                  event == EVENT_TIMER && retryCount < maxRetryCount,
                                  TRANSMITRTS,
                                  retryRtsFrame();
            );
            FSMA_Event_Transition(Cts-timeout-giveup,
                                  event == EVENT_TIMER && retryCount == maxRetryCount,
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
}

void Ieee80211SendRtsCtsFrameExchangeXXX::transmitRtsFrame()
{
    cw = cwMin;
    retryCount = 0;
    mac->transmission->transmitContentionFrame(rtsFrame, ifs, cw);
}

void Ieee80211SendRtsCtsFrameExchangeXXX::retryRtsFrame()
{
    if (cw < cwMax)
        cw = ((cw+1)<<1)-1;
    retryCount++;
    mac->transmission->transmitContentionFrame(rtsFrame, ifs, cw);
}

void Ieee80211SendRtsCtsFrameExchangeXXX::scheduleCtsTimeout()
{
    if (ctsTimer == nullptr)
        ctsTimer = new cMessage("timeout");
    simtime_t t = simTime() + 2*MAX_PROPAGATION_DELAY + SIFS + LENGTH_ACK / mac->basicBitrate + PHY_HEADER_LENGTH / BITRATE_HEADER; //TODO
    scheduleAt(t, ctsTimer);
}

void Ieee80211SendRtsCtsFrameExchangeXXX::processFrame(Ieee80211Frame *receivedFrame)
{
    //TODO some totally unrelated frame arrived; process in the normal way
}

bool Ieee80211SendRtsCtsFrameExchangeXXX::isCts(Ieee80211Frame* frame)
{
    return dynamic_cast<Ieee80211CTSFrame *>(frame) != nullptr;
}

//------------------------------

/* IMPLEMENTATION DRAFT

void Ieee80211StepBasedFrameExchange::transmitImmediateFrame(Ieee80211Frame *frame, simtime_t ifs)
{
    ASSERT(action == NONE); // only one action allowed per step
    action = TRANSMIT_IMMEDIATE_FRAME;
    getMac()->transmitImmediateFrame(frame, ifs);
}

void Ieee80211StepBasedFrameExchange::transmitContentionFrame(Ieee80211DataOrMgmtFrame *frame, int maxRetryCount, simtime_t ifs, int cw)
{
    ASSERT(action == NONE); // only one action allowed per step
    action = TRANSMIT_CONTENTION_FRAME;
    getTransmission()->transmitContentionFrame(frame, ifs, cw);
}

void Ieee80211StepBasedFrameExchange::transmitAckedContentionFrame(Ieee80211DataOrMgmtFrame *frame, int maxRetryCount, simtime_t ifs, int cwMin, int cwMax, simtime_t timeout)
{
    ASSERT(action == NONE); // only one action allowed per step
    action = TRANSMIT_ACKED_CONTENTION_FRAME;
    getTransmission()->transmitContentionFrame(frame, ifs, cwMin, cwMax);
}

void Ieee80211StepBasedFrameExchange::expectReply(simtime_t timeout)
{
    ASSERT(action == NONE); // only one action allowed per step
    action = EXPECT_REPLY;
    scheduleAt(simTime()+timeout, timer);
}

void Ieee80211StepBasedFrameExchange::start()
{
    step = 0;
    isLastStep = doStep(step);
}

void Ieee80211StepBasedFrameExchange::nextStep()
{
    if (isLastStep)
        reportSuccess();
    else {
        step++;
        isLastStep = doStep(step);
    }
}

void Ieee80211StepBasedFrameExchange::lowerFrameReceived(Ieee80211Frame *frame)
{
    switch(action) {
        case TRANSMIT_IMMEDIATE_FRAME: ASSERT(false); break; //unexpected
        case TRANSMIT_CONTENTION_FRAME: break; // TODO let someone else process it
        case TRANSMIT_ACKED_CONTENTION_FRAME:
            if (not yet in WAITACK)
                letUpperProcessIt(); //XXX
            else if (isReply(step, frame))
                nextStep();
            else
                letUpperProcessIt()? or reportFailure()?
        default: ASSERT(false); //unexpected
    }
}

void Ieee80211StepBasedFrameExchange::transmissionFinished()
{
    switch(action) {
        case TRANSMIT_IMMEDIATE_FRAME: nextStep(); break;
        case TRANSMIT_CONTENTION_FRAME: nextStep(); break;
        case TRANSMIT_ACKED_CONTENTION_FRAME: scheduleAt(simTime()+timeout, timer); break;  //TODO plus state=WAITACK
        default: ASSERT(false); //unexpected
    }
}

void Ieee80211StepBasedFrameExchange::handleMessage(cMessage *timer)
{
    switch(action) {
        case TRANSMIT_ACKED_CONTENTION_FRAME: reportFailure(); break;
        case EXPECT_REPLY: reportFailure();  break;
        default: ASSERT(false); //unexpected
    }
}


//---

bool Ieee80211SendDataWithRtsCtsFrameExchange::doStep(int step)
{
    switch (step) {
        case 0: transmitAckedContentionFrame(buildRtsFrame(dataFrame), 10, getDIFS(), 3, 511, getCtsTimeout()); return false;
        case 1: transmitImmediateFrame(dataFrame, getSIFS()); return false;
        case 2: expectReply(getAckTimeout()); return false;
        default: return true; // done
    }
}

bool Ieee80211SendDataWithRtsCtsFrameExchange::processReply(int step, Ieee80211Frame *frame)
{
    switch (step) {
        case 0: return isFrameCtsFor(frame, dataFrame->getSourceAddress());
        case 2: return isFrameAckFor(frame, dataFrame->getSourceAddress());
        default: return false;
    }
}

IMPLEMENTATION DRAFT */


} /* namespace inet */



