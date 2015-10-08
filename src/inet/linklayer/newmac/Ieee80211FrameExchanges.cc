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
#include "Ieee80211MacTransmission.h"
#include "Ieee80211UpperMac.h"

namespace inet {
namespace ieee80211 {

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
    retryCount = 0;
    mac->transmission->transmitContentionFrame(frame, retryCount, getUpperMac());
}

void Ieee80211SendDataWithAckFrameExchange::retryDataFrame()
{
    retryCount++;
    frame->setRetry(true);
    mac->transmission->transmitContentionFrame(frame, retryCount, getUpperMac());
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

//------------------------------

SendDataWithRtsCtsFrameExchange::SendDataWithRtsCtsFrameExchange(Ieee80211NewMac *mac, IFinishedCallback *callback, Ieee80211DataOrMgmtFrame *dataFrame) :
    Ieee80211StepBasedFrameExchange(mac, callback), dataFrame(dataFrame)
{
}

/*TODO
void SendDataWithRtsCtsFrameExchange::doStep(int step)
{
    switch (step) {
        case 0: transmitContentionFrame(buildRtsFrame(dataFrame, retryCount)); break;
        case 1: expectReply(ctsTimeout); break;
        case 2: transmitImmediateFrame(dataFrame, sifs); break;
        case 3: expectReply(ackTimeout); break;
    }
}

//context? for getting channel parameters, building ACK frames, computing ACK timeouts, etc....
bool SendDataWithRtsCtsFrameExchange::processReply(int step, Ieee80211Frame *frame)
{
    const MACAddress& destAddress = dataFrame->getReceiverAddress();
    switch (step) {
        case 1: return isCtsFrom(frame, destAddress);  // true=accepted
        case 3: return isAckFrom(frame, destAddress);
    }
}
*/

void SendDataWithRtsCtsFrameExchange::processTimeout(int step)
{
    switch (step) {
        case 1: if (retryCount < maxRetryCount) {retryCount++; gotoStep(0);} else fail(); break;
        case 3: fail(); break;
    }
}

}

} /* namespace inet */
