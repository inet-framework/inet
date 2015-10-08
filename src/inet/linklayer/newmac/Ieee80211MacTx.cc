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

#include "Ieee80211MacTx.h"
#include "Ieee80211NewMac.h"
#include "inet/common/FSMA.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"

namespace inet {

namespace ieee80211 {

// don't forget to keep synchronized the C++ enum and the runtime enum definition
Register_Enum(Ieee80211NewMac,
   (Ieee80211MacTx::IDLE,
   Ieee80211MacTx::DEFER,
   Ieee80211MacTx::BACKOFF,
   Ieee80211MacTx::TRANSMIT,
   Ieee80211MacTx::WAIT_IFS));

Ieee80211MacTx::Ieee80211MacTx(Ieee80211NewMac* mac) : Ieee80211MacPlugin(mac)
{
    fsm.setName("Ieee80211NewMac State Machine");
    fsm.setState(IDLE);
    endIFS = new cMessage("IFS");
    endBackoff = new cMessage("Backoff");
    frameDuration = new cMessage("FrameDuration");
    endEIFS = new cMessage("EIFS");
}

Ieee80211MacTx::~Ieee80211MacTx()
{
    cancelEvent(endIFS);
    cancelEvent(endBackoff);
    cancelEvent(frameDuration);
    cancelEvent(endEIFS);
    delete endIFS;
    delete endBackoff;
    delete frameDuration;
    delete endEIFS;
}

void Ieee80211MacTx::transmitContentionFrame(Ieee80211Frame* frame, simtime_t ifs, simtime_t eifs, int cwMin, int cwMax, int retryCount, ICallback *completionCallback)
{
    ASSERT(fsm.getState() == IDLE);
    this->frame = frame;
    this->ifs = ifs;
    this->eifs = eifs;
    this->cwMin = cwMin;
    this->cwMax = cwMax;
    this->retryCount = retryCount;
    this->completionCallback = completionCallback;

    int cw = computeCW(cwMin, cwMax, retryCount);
    backoffSlots = intrand(cw + 1);
    handleWithFSM(START, frame);
}

int Ieee80211MacTx::computeCW(int cwMin, int cwMax, int retryCount)
{
    int cw = ((cwMin+1) << retryCount) - 1;
    if (cw > cwMax)
        cw = cwMax;
    return cw;
}

void Ieee80211MacTx::handleWithFSM(EventType event, cMessage *msg)
{
    if (frame == nullptr)
        return; //FIXME ????????????????
    logState();
//    emit(stateSignal, fsm.getState()); TODO
    FSMA_Switch(fsm)
    {
        FSMA_State(IDLE)
        {
            FSMA_Enter(mac->sendDownPendingRadioConfigMsg());
            FSMA_Event_Transition(Ready-To-Transmit,
                                  event == START && mediumFree && !isIFSNecessary(),
                                  TRANSMIT,
                                  ;
            );
            FSMA_Event_Transition(Need-IFS-Before-Transmit,
                                  event == START && mediumFree && isIFSNecessary(),
                                  WAIT_IFS,
                                  ;
            );
            FSMA_Event_Transition(Busy,
                                  event == START && !mediumFree,
                                  DEFER,
                                  ;
            );
        }
        FSMA_State(DEFER)
        {
            FSMA_Enter(mac->sendDownPendingRadioConfigMsg());
            FSMA_Event_Transition(WAIT-Ifs,
                                     event == MEDIUM_STATE_CHANGED && mediumFree,
                                     WAIT_IFS,
                                     ;
            );
        }
        FSMA_State(WAIT_IFS)
        {
            FSMA_Enter(scheduleIFS());
            FSMA_Event_Transition(Backoff,
                                  event == TIMER && !endIFS->isScheduled() && !endEIFS->isScheduled(),
                                  BACKOFF,
                                  ;
            );
            FSMA_Event_Transition(Busy,
                                  event == MEDIUM_STATE_CHANGED && !mediumFree,
                                  DEFER,
                                  cancelEvent(endIFS);
            );
        }
        FSMA_State(BACKOFF)
        {
            FSMA_Enter(scheduleBackoffPeriod(backoffSlots));
            FSMA_Event_Transition(Transmit-Data,
                                  event == TIMER && msg == endBackoff,
                                  TRANSMIT,
                                  ;
            );
            FSMA_Event_Transition(Backoff-Busy,
                                  event == MEDIUM_STATE_CHANGED && !mediumFree,
                                  DEFER,
                                  updateBackoffPeriod();
                                  cancelEvent(endBackoff);
            );
        }
        FSMA_State(TRANSMIT)
        {
            FSMA_Enter(mac->sendFrame(frame->dup()));  //FIXME why is the dup() here!!!
            FSMA_Event_Transition(TxFinished,
                                  event == TRANSMISSION_FINISHED,
                                  IDLE,
                                  completionCallback->transmissionComplete(this);
            );
        }
    }

    logState();
    // emit(stateSignal, fsm.getState()); TODO
}

void Ieee80211MacTx::mediumStateChanged(bool mediumFree)
{
    this->mediumFree = mediumFree;
    channelLastBusyTime = simTime();
    handleWithFSM(MEDIUM_STATE_CHANGED, nullptr);
}

void Ieee80211MacTx::transmissionFinished()
{
    handleWithFSM(TRANSMISSION_FINISHED, nullptr);
}

void Ieee80211MacTx::handleMessage(cMessage *msg)
{
    handleWithFSM(TIMER, msg);
}

void Ieee80211MacTx::lowerFrameReceived(bool isFcsOk)
{
    useEIFS = !isFcsOk;
}

void Ieee80211MacTx::scheduleIFSPeriod(simtime_t deferDuration)
{
    scheduleAt(simTime() + deferDuration, endIFS);
}

void Ieee80211MacTx::scheduleEIFSPeriod(simtime_t duration)
{
    cancelEvent(endEIFS);
    scheduleAt(simTime() + duration, endEIFS);
}

void Ieee80211MacTx::scheduleIFS()
{
    ASSERT(mediumFree);
    simtime_t elapsedFreeChannelTime = simTime() - channelLastBusyTime;
    if (ifs > elapsedFreeChannelTime)
        scheduleIFSPeriod(ifs - elapsedFreeChannelTime);
    if (useEIFS && eifs > elapsedFreeChannelTime)
        scheduleEIFSPeriod(eifs - elapsedFreeChannelTime);
    useEIFS = false;
}


void Ieee80211MacTx::updateBackoffPeriod()
{
    simtime_t elapsedBackoffTime = simTime() - endBackoff->getSendingTime();
    backoffSlots -= ((int)(elapsedBackoffTime / mac->getSlotTime()));  //FIXME slot should be a parameter
}

void Ieee80211MacTx::scheduleBackoffPeriod(int backoffSlots)
{
    simtime_t backoffPeriod = backoffSlots * mac->getSlotTime();
    scheduleAt(simTime() + backoffPeriod, endBackoff);
}

void Ieee80211MacTx::logState()
{
    EV  << "state information: " << "state = " << fsm.getStateName() << ", backoffPeriod = " << backoffPeriod << endl;
}

bool Ieee80211MacTx::isIFSNecessary()
{
    simtime_t elapsedFreeChannelTime = simTime() - channelLastBusyTime;
    return elapsedFreeChannelTime < ifs || (useEIFS && elapsedFreeChannelTime < eifs);
}


}

} //namespace

