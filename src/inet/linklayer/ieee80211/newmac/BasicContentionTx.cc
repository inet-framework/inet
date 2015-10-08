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

#include "BasicContentionTx.h"
#include "IUpperMac.h"
#include "IMacRadioInterface.h"
#include "inet/common/FSMA.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"

namespace inet {
namespace ieee80211 {

// don't forget to keep synchronized the C++ enum and the runtime enum definition
Register_Enum(Ieee80211NewMac,
   (BasicContentionTx::IDLE,
   BasicContentionTx::DEFER,
   BasicContentionTx::BACKOFF,
   BasicContentionTx::TRANSMIT,
   BasicContentionTx::WAIT_IFS));

Define_Module(BasicContentionTx);

// non-member utility function
void collectContentionTxModules(cModule *firstContentionTxModule, IContentionTx **& contentionTx)
{
    ASSERT(firstContentionTxModule != nullptr);
    int count = firstContentionTxModule->getVectorSize();

    contentionTx = new IContentionTx*[count+1];
    for (int i = 0; i < count; i++) {
        cModule *sibling = firstContentionTxModule->getParentModule()->getSubmodule(firstContentionTxModule->getName(), i);
        contentionTx[i] = check_and_cast<IContentionTx*>(sibling);
    }
    contentionTx[count] = nullptr;
}


void BasicContentionTx::initialize()
{
    mac = dynamic_cast<IMacRadioInterface*>(getModuleByPath("^"));
    upperMac = dynamic_cast<IUpperMac*>(getModuleByPath("^.upperMac"));
    txIndex = getIndex();

    fsm.setName("fsm");
    fsm.setState(IDLE);
    endIFS = new cMessage("IFS");
    endBackoff = new cMessage("Backoff");
    endEIFS = new cMessage("EIFS");
}

BasicContentionTx::~BasicContentionTx()
{
    cancelAndDelete(endIFS);
    cancelAndDelete(endBackoff);
    cancelAndDelete(endEIFS);
}

void BasicContentionTx::transmitContentionFrame(Ieee80211Frame* frame, simtime_t ifs, simtime_t eifs, int cwMin, int cwMax, simtime_t slotTime, int retryCount, ITxCallback *completionCallback)
{
    ASSERT(fsm.getState() == IDLE);
    this->frame = frame;
    this->ifs = ifs;
    this->eifs = eifs;
    this->cwMin = cwMin;
    this->cwMax = cwMax;
    this->slotTime = slotTime;
    this->retryCount = retryCount;
    this->completionCallback = completionCallback;

    int cw = computeCW(cwMin, cwMax, retryCount);
    backoffSlots = intrand(cw + 1);
    handleWithFSM(START, frame);
}

int BasicContentionTx::computeCW(int cwMin, int cwMax, int retryCount)
{
    int cw = ((cwMin+1) << retryCount) - 1;
    if (cw > cwMax)
        cw = cwMax;
    return cw;
}

void BasicContentionTx::handleWithFSM(EventType event, cMessage *msg)
{
    logState();
//    emit(stateSignal, fsm.getState()); TODO
    FSMA_Switch(fsm)
    {
        FSMA_State(IDLE)
        {
            FSMA_Enter(mac->sendDownPendingRadioConfigMsg());
//            FSMA_Event_Transition(Ready-To-Transmit,
//                                  event == START && mediumFree && !isIFSNecessary(),
//                                  TRANSMIT,
//                                  ;
//            );
            FSMA_Event_Transition(Need-IFS-Before-Transmit,
                                  event == START && mediumFree /*&& isIFSNecessary()*/,
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
            FSMA_Enter(mac->sendFrame(frame));
            FSMA_Event_Transition(TxFinished,
                                  event == TRANSMISSION_FINISHED,
                                  IDLE,
                                  upperMac->transmissionComplete(completionCallback, txIndex);
            );
        }
    }

    logState();
    // emit(stateSignal, fsm.getState()); TODO
}

void BasicContentionTx::mediumStateChanged(bool mediumFree)
{
    Enter_Method(mediumFree ? "medium FREE" : "medium BUSY");
    this->mediumFree = mediumFree;
    channelLastBusyTime = simTime();
    handleWithFSM(MEDIUM_STATE_CHANGED, nullptr);
}

void BasicContentionTx::radioTransmissionFinished()
{
    Enter_Method("radioTransmissionFinished()");
    handleWithFSM(TRANSMISSION_FINISHED, nullptr);
}

void BasicContentionTx::handleMessage(cMessage *msg)
{
    handleWithFSM(TIMER, msg);
}

void BasicContentionTx::lowerFrameReceived(bool isFcsOk)
{
    Enter_Method("lowerFrameReceived(%s)", isFcsOk ? "HEALTHY" : "CORRUPT");
    useEIFS = !isFcsOk;
}

void BasicContentionTx::scheduleIFSPeriod(simtime_t deferDuration)
{
    scheduleAt(simTime() + deferDuration, endIFS);
}

void BasicContentionTx::scheduleEIFSPeriod(simtime_t duration)
{
    cancelEvent(endEIFS);
    scheduleAt(simTime() + duration, endEIFS);
}

void BasicContentionTx::scheduleIFS()
{
    ASSERT(mediumFree);
//    simtime_t elapsedFreeChannelTime = simTime() - channelLastBusyTime;
//    if (ifs > elapsedFreeChannelTime)
//        scheduleIFSPeriod(ifs - elapsedFreeChannelTime);
//    if (useEIFS && eifs > elapsedFreeChannelTime)
//        scheduleEIFSPeriod(eifs - elapsedFreeChannelTime);
//    useEIFS = false;
    if (useEIFS)
        scheduleEIFSPeriod(eifs);
    scheduleIFSPeriod(ifs);
}


void BasicContentionTx::updateBackoffPeriod()
{
    simtime_t elapsedBackoffTime = simTime() - endBackoff->getSendingTime();
    backoffSlots -= ((int)(elapsedBackoffTime / slotTime)); //FIXME add some epsilon...?
}

void BasicContentionTx::scheduleBackoffPeriod(int backoffSlots)
{
    simtime_t backoffPeriod = backoffSlots * slotTime;
    scheduleAt(simTime() + backoffPeriod, endBackoff);
}

void BasicContentionTx::logState()
{
    EV  << "state information: " << "state = " << fsm.getStateName() << ", backoffPeriod = " << backoffPeriod << endl;
}

bool BasicContentionTx::isIFSNecessary()
{
    simtime_t elapsedFreeChannelTime = simTime() - channelLastBusyTime;
    return elapsedFreeChannelTime < ifs || (useEIFS && elapsedFreeChannelTime < eifs);
}

} // namespace ieee80211
} // namespace inet

