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

#include "Ieee80211FrameExchanges.h"
#include "inet/common/FSMA.h"
#include "Ieee80211UpperMac.h"
#include "IIeee80211UpperMacContext.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"

namespace inet {
namespace ieee80211 {

Ieee80211SendDataWithAckFrameExchange::Ieee80211SendDataWithAckFrameExchange(Ieee80211NewMac *mac, IIeee80211UpperMacContext *context, IFinishedCallback *callback, Ieee80211DataOrMgmtFrame *frame) :
    Ieee80211FSMBasedFrameExchange(mac, context, callback), frame(frame)
{
}

Ieee80211SendDataWithAckFrameExchange::~Ieee80211SendDataWithAckFrameExchange()
{
    delete frame;
    if (ackTimer)
        delete cancelEvent(ackTimer);
}

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
                                  event == EVENT_FRAMEARRIVED && isAck(receivedFrame), //TODO is from right STA
                                  SUCCESS,
                                  delete receivedFrame;
            );
            FSMA_Event_Transition(Frame-arrived,
                                  event == EVENT_FRAMEARRIVED && !isAck(receivedFrame), //TODO is from right STA
                                  FAILURE,
                                  processFrame(receivedFrame);
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
}

void Ieee80211SendDataWithAckFrameExchange::transmitDataFrame()
{
    retryCount = 0;
    int txIndex = 0; //TODO
    context->transmitContentionFrame(txIndex, frame, context->getDIFS(), context->getEIFS(), context->getMinCW(), context->getMaxCW(), context->getSlotTime(), retryCount, this);
}

void Ieee80211SendDataWithAckFrameExchange::retryDataFrame()
{
    retryCount++;
    frame->setRetry(true);
    int txIndex = 0; //TODO
    context->transmitContentionFrame(txIndex, frame, context->getDIFS(), context->getEIFS(), context->getMinCW(), context->getMaxCW(), context->getSlotTime(), retryCount, this);
}

void Ieee80211SendDataWithAckFrameExchange::scheduleAckTimeout()
{
    if (ackTimer == nullptr)
        ackTimer = new cMessage("timeout");
    simtime_t t = simTime() + context->getAckTimeout();
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

//------------------------------

Ieee80211SendDataWithRtsCtsFrameExchange::Ieee80211SendDataWithRtsCtsFrameExchange(Ieee80211NewMac *mac, IIeee80211UpperMacContext *context, IFinishedCallback *callback, Ieee80211DataOrMgmtFrame *dataFrame) :
    Ieee80211StepBasedFrameExchange(mac, context, callback), dataFrame(dataFrame)
{
}

void Ieee80211SendDataWithRtsCtsFrameExchange::doStep(int step)
{
    switch (step) {
        case 0: transmitContentionFrame(context->buildRtsFrame(dataFrame), retryCount); break;
        case 1: expectReply(context->getCtsTimeout()); break;
        case 2: transmitImmediateFrame(dataFrame, context->getSIFS()); break;
        case 3: expectReply(context->getAckTimeout()); break;
    }
}

bool Ieee80211SendDataWithRtsCtsFrameExchange::processReply(int step, Ieee80211Frame *frame)
{
    switch (step) {
        case 1: return context->isCts(frame);  // true=accepted
        case 3: return context->isAck(frame);
        default: return false;
    }
}

void Ieee80211SendDataWithRtsCtsFrameExchange::processTimeout(int step)
{
    switch (step) {
        case 1: if (++retryCount < context->getShortRetryLimit()) {dataFrame->setRetry(true); gotoStep(0);} else fail(); break;
        case 3: fail(); break;
    }
}

}

} /* namespace inet */
