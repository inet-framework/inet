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

#include "Ieee80211MacTransmission.h"
#include "Ieee80211NewMac.h"
#include "Ieee80211UpperMac.h"

namespace inet {

// don't forget to keep synchronized the C++ enum and the runtime enum definition
Register_Enum(Ieee80211NewMac,
   (Ieee80211MacTransmission::IDLE,
   Ieee80211MacTransmission::DEFER,
   Ieee80211MacTransmission::BACKOFF,
   Ieee80211MacTransmission::TRANSMIT,
   Ieee80211MacTransmission::WAIT_IFS));

/**
 * Msg can be upper, lower, self or NULL (when radio state changes)
 */
void Ieee80211MacTransmission::handleWithFSM(EventType event, cMessage *msg)
{
    if (frame == nullptr)
        return;
    logState();
//    emit(stateSignal, fsm.getState()); TODO

    FSMA_Switch(fsm)
    {
        FSMA_State(IDLE)
        {
            FSMA_Enter(mac->sendDownPendingRadioConfigMsg());
            FSMA_Event_Transition(Data-Ready,
                                  event == START_TRANSMISSION && mediumFree,
                                  WAIT_IFS,
                                  ;
            );
            FSMA_Event_Transition(Busy,
                                  event == START_TRANSMISSION && !mediumFree,
                                  DEFER,
                                  ;
            );
        }
        FSMA_State(DEFER)
        {
            FSMA_Enter(mac->sendDownPendingRadioConfigMsg());
            FSMA_Event_Transition(WAIT-Ifs,
                                     event == CHANNEL_STATE_CHANGED && mediumFree,
                                     WAIT_IFS,
                                     ;
            );
        }
        FSMA_State(WAIT_IFS)
        {
            FSMA_Enter(scheduleIFSPeriod(deferDuration));
            FSMA_Event_Transition(Backoff,
                                  event == TIMER && msg == endIFS,
                                  BACKOFF,
                                  ;
            );
            FSMA_Event_Transition(Busy,
                                  event == CHANNEL_STATE_CHANGED && !mediumFree,
                                  DEFER,
                                  ;
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
                                  event == CHANNEL_STATE_CHANGED && !mediumFree,
                                  DEFER,
                                  updateBackoffPeriod(); // TODO: update txState backoff peroid with remaining time
            );
        }
        FSMA_State(TRANSMIT)
        {
            FSMA_Enter(mac->sendDataFrame(frame));
            FSMA_Event_Transition(TxFinished,
                                  event == CHANNEL_STATE_CHANGED && mediumFree,
                                  IDLE,
                                  mac->upperMac->transmissionFinished();
            );
        }
    }

    logState();
    // emit(stateSignal, fsm.getState()); TODO
}

void Ieee80211MacTransmission::transmitContentionFrame(Ieee80211Frame* frame, simtime_t deferDuration, int cw)
{
    ASSERT(fsm.getState() == IDLE);
    this->frame = frame;
    this->deferDuration = deferDuration;
    backoffSlots = intrand(cw + 1);
    handleWithFSM(START_TRANSMISSION, frame);
}

void Ieee80211MacTransmission::mediumStateChanged(bool mediumFree)
{
    this->mediumFree = mediumFree;
    handleWithFSM(CHANNEL_STATE_CHANGED, nullptr);
}

void Ieee80211MacTransmission::handleMessage(cMessage *msg)
{
    handleWithFSM(TIMER, msg);
}

Ieee80211MacTransmission::Ieee80211MacTransmission(Ieee80211NewMac* mac) : Ieee80211MacPlugin(mac)
{
    fsm.setName("Ieee80211NewMac State Machine");
    fsm.setState(IDLE);
    endIFS = new cMessage("IFS");
    endBackoff = new cMessage("Backoff");
    mediumStateChange = new cMessage("MediumStateChange");
    frameDuration = new cMessage("FrameDuration");
}

void Ieee80211MacTransmission::scheduleIFSPeriod(simtime_t deferDuration)
{
    scheduleAt(simTime() + deferDuration, endIFS);
}

void Ieee80211MacTransmission::updateBackoffPeriod()
{
    simtime_t elapsedBackoffTime = simTime() - endBackoff->getSendingTime();
    backoffSlots -= ((int)(elapsedBackoffTime / mac->getSlotTime()));
}

void Ieee80211MacTransmission::scheduleBackoffPeriod(int backoffSlots)
{
    simtime_t backoffPeriod = backoffSlots * mac->getSlotTime();
    scheduleAt(simTime() + backoffPeriod, endBackoff);
}

void Ieee80211MacTransmission::logState()
{
    EV  << "state information: " << "state = " << fsm.getStateName() << ", backoffPeriod = " << backoffPeriod << endl;
}


} //namespace
