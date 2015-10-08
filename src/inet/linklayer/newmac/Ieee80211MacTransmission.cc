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

namespace ieee80211 {

// don't forget to keep synchronized the C++ enum and the runtime enum definition
Register_Enum(Ieee80211NewMac,
   (Ieee80211MacTransmission::IDLE,
   Ieee80211MacTransmission::DEFER,
   Ieee80211MacTransmission::BACKOFF,
   Ieee80211MacTransmission::TRANSMIT,
   Ieee80211MacTransmission::WAIT_IFS));

Ieee80211MacTransmission::Ieee80211MacTransmission(Ieee80211NewMac* mac) : Ieee80211MacPlugin(mac)
{
    fsm.setName("Ieee80211NewMac State Machine");
    fsm.setState(IDLE);
    endIFS = new cMessage("IFS");
    endBackoff = new cMessage("Backoff");
    frameDuration = new cMessage("FrameDuration");
    endEIFS = new cMessage("EIFS");
}

Ieee80211MacTransmission::~Ieee80211MacTransmission()
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

void Ieee80211MacTransmission::transmitContentionFrame(Ieee80211Frame* frame, int retryCount, ITransmissionCompleteCallback *transmissionCompleteCallback)
{
    transmitContentionFrame(frame,
            mac->dataFrameMode->getDifsTime(), mac->dataFrameMode->getEifsTime(nullptr,0), //FIXME getEifsTime() args are dummy!!!
            mac->dataFrameMode->getCwMin(), mac->dataFrameMode->getCwMax(),
            retryCount, transmissionCompleteCallback);
}

void Ieee80211MacTransmission::transmitContentionFrame(Ieee80211Frame* frame, simtime_t ifs, simtime_t eifs, int cwMin, int cwMax, int retryCount, ITransmissionCompleteCallback *transmissionCompleteCallback)
{
    ASSERT(fsm.getState() == IDLE);
    this->frame = frame;
    this->ifs = ifs;
    this->eifs = eifs;
    this->cwMin = cwMin;
    this->cwMax = cwMax;
    this->retryCount = retryCount;
    this->transmissionCompleteCallback = transmissionCompleteCallback;

    int cw = computeCW(cwMin, cwMax, retryCount);
    backoffSlots = intrand(cw + 1);
    handleWithFSM(START_TRANSMISSION, frame);
}

int Ieee80211MacTransmission::computeCW(int cwMin, int cwMax, int retryCount)
{
    int cw = ((cwMin+1) << retryCount) - 1;
    if (cw > cwMax)
        cw = cwMax;
    return cw;
}

void Ieee80211MacTransmission::handleWithFSM(EventType event, cMessage *msg)
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
                                  event == START_TRANSMISSION && mediumFree && !isIFSNecessary(),
                                  TRANSMIT,
                                  ;
            );
            FSMA_Event_Transition(Need-IFS-Before-Transmit,
                                  event == START_TRANSMISSION && mediumFree && isIFSNecessary(),
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
            FSMA_Enter(mac->sendDataFrame(frame));
            FSMA_Event_Transition(TxFinished,
                                  event == MEDIUM_STATE_CHANGED && transmissionState == IRadio::TRANSMISSION_STATE_IDLE,
                                  IDLE,
                                  transmissionCompleteCallback->transmissionComplete(this);
            );
        }
    }

    logState();
    // emit(stateSignal, fsm.getState()); TODO
}

void Ieee80211MacTransmission::mediumStateChanged(bool mediumFree)
{
    this->mediumFree = mediumFree;
    channelBecameFree = simTime();
    handleWithFSM(MEDIUM_STATE_CHANGED, nullptr);
}

void Ieee80211MacTransmission::transmissionStateChanged(IRadio::TransmissionState transmissionState)
{
    this->transmissionState = transmissionState;
    handleWithFSM(MEDIUM_STATE_CHANGED, nullptr);
}

void Ieee80211MacTransmission::handleMessage(cMessage *msg)
{
    handleWithFSM(TIMER, msg);
}

void Ieee80211MacTransmission::lowerFrameReceived(bool isFcsOk)
{
    useEIFS = !isFcsOk;
}

void Ieee80211MacTransmission::scheduleIFSPeriod(simtime_t deferDuration)
{
    scheduleAt(simTime() + deferDuration, endIFS);
}

void Ieee80211MacTransmission::scheduleEIFSPeriod(simtime_t duration)
{
    cancelEvent(endEIFS);
    scheduleAt(simTime() + duration, endEIFS);
}

void Ieee80211MacTransmission::scheduleIFS()
{
    ASSERT(mediumFree);
    simtime_t elapsedFreeChannelTime = simTime() - channelBecameFree;
    if (ifs > elapsedFreeChannelTime)
        scheduleIFSPeriod(ifs - elapsedFreeChannelTime);
    if (useEIFS && eifs > elapsedFreeChannelTime)
        scheduleEIFSPeriod(eifs - elapsedFreeChannelTime);
    useEIFS = false;
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

bool Ieee80211MacTransmission::isIFSNecessary()
{
    simtime_t elapsedFreeChannelTime = simTime() - channelBecameFree;
    return elapsedFreeChannelTime < ifs || (useEIFS && elapsedFreeChannelTime < eifs);
}


}

} //namespace

