//
// Copyright (C) 2016 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see http://www.gnu.org/licenses/.
//

#include "inet/common/FSMA.h"
#include "inet/common/ModuleAccess.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#include "inet/linklayer/ieee80211/mac/contention/Contention.h"

namespace inet {
namespace ieee80211 {

simsignal_t Contention::stateChangedSignal = registerSignal("stateChanged");

// for @statistic; don't forget to keep synchronized the C++ enum and the runtime enum definition
Register_Enum(Contention::State,
        (Contention::IDLE,
         Contention::DEFER,
         Contention::IFS_AND_BACKOFF)
         );

Define_Module(Contention);

void Contention::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        backoffOptimization = par("backoffOptimization");
        lastIdleStartTime = simTime() - SimTime::getMaxTime() / 2;
        mac = check_and_cast<Ieee80211Mac *>(getContainingNicModule(this)->getSubmodule("mac"));
        startTxEvent = new cMessage("startTx");
        startTxEvent->setSchedulingPriority(1000); // low priority, i.e. processed later than most events for the same time
        // KLUDGE:
        // The callback->channelAccessGranted() call should be the last
        // event at a simulation time in order to handle internal collisions
        // properly.
        channelGrantedEvent = new cMessage("channelGranted");
        channelGrantedEvent->setSchedulingPriority(1000);
        fsm.setName("Backoff procedure");
        fsm.setState(IDLE, "IDLE");

        WATCH(ifs);
        WATCH(eifs);
        WATCH(slotTime);
        WATCH(endEifsTime);
        WATCH(backoffSlots);
        WATCH(scheduledTransmissionTime);
        WATCH(lastChannelBusyTime);
        WATCH(lastIdleStartTime);
        WATCH(backoffOptimizationDelta);
        WATCH(mediumFree);
        WATCH(backoffOptimization);
        WATCH(startTime);
        updateDisplayString(-1);
    }
    else if (stage == INITSTAGE_LAST) {
        if (!par("initialChannelBusy") && simTime() == 0)
            lastChannelBusyTime = simTime() - SimTime().getMaxTime() / 2;
    }
}

Contention::~Contention()
{
    cancelAndDelete(channelGrantedEvent);
    cancelAndDelete(startTxEvent);
}

void Contention::startContention(int cw, simtime_t ifs, simtime_t eifs, simtime_t slotTime, ICallback *callback)
{
    startTime = simTime();
    ASSERT(ifs >= 0 && eifs >= 0 && slotTime >= 0 && cw >= 0);
    Enter_Method_Silent("startContention");
    cancelEvent(channelGrantedEvent);
    ASSERT(fsm.getState() == IDLE);
    this->ifs = ifs;
    this->eifs = eifs;
    this->slotTime = slotTime;
    this->callback = callback;
    backoffSlots = intrand(cw + 1);
    emit(backoffPeriodGeneratedSignal, backoffSlots);
    EV_DETAIL << "Starting contention: cw = " << cw << ", slots = " << backoffSlots << ", slotTime = " << slotTime << ", ifs = " << ifs << ", eifs = " << eifs << endl;
    handleWithFSM(START);
}

void Contention::handleWithFSM(EventType event)
{
    emit(stateChangedSignal, fsm.getState());
    EV_TRACE << "handleWithFSM: processing event " << getEventName(event) << "\n";
    bool finallyReportChannelAccessGranted = false;
    FSMA_Switch(fsm) {
        FSMA_State(IDLE) {
            FSMA_Enter(mac->sendDownPendingRadioConfigMsg());
            FSMA_Event_Transition(Starting-IFS-and-Backoff,
                    event == START && mediumFree,
                    IFS_AND_BACKOFF,
                    scheduleTransmissionRequest();
                    );
            FSMA_Event_Transition(Busy,
                    event == START && !mediumFree,
                    DEFER,
                    ;
                    );
            FSMA_Ignore_Event(event==MEDIUM_STATE_CHANGED);
            FSMA_Ignore_Event(event==CORRUPTED_FRAME_RECEIVED);
            FSMA_Fail_On_Unhandled_Event();
        }
        FSMA_State(DEFER) {
            FSMA_Enter(mac->sendDownPendingRadioConfigMsg());
            FSMA_Event_Transition(Restarting-IFS-and-Backoff,
                    event == MEDIUM_STATE_CHANGED && mediumFree,
                    IFS_AND_BACKOFF,
                    scheduleTransmissionRequest();
                    );
            FSMA_Event_Transition(Use-EIFS,
                    event == CORRUPTED_FRAME_RECEIVED,
                    DEFER,
                    endEifsTime = simTime() + eifs;
                    );
            FSMA_Fail_On_Unhandled_Event();
        }
        FSMA_State(IFS_AND_BACKOFF) {
            FSMA_Enter();
            FSMA_Event_Transition(Backoff-expired,
                    event == CHANNEL_ACCESS_GRANTED,
                    IDLE,
                    lastIdleStartTime = simTime();
                    finallyReportChannelAccessGranted = true;
                    );
            FSMA_Event_Transition(Defer-on-channel-busy,
                    event == MEDIUM_STATE_CHANGED && !mediumFree,
                    DEFER,
                    cancelTransmissionRequest();
                    computeRemainingBackoffSlots();
                    );
            FSMA_Event_Transition(Use-EIFS,
                    event == CORRUPTED_FRAME_RECEIVED,
                    IFS_AND_BACKOFF,
                    switchToEifs();
                    );
            FSMA_Fail_On_Unhandled_Event();
        }
    }
    emit(stateChangedSignal, fsm.getState());
    if (finallyReportChannelAccessGranted)
        scheduleAfter(SIMTIME_ZERO, channelGrantedEvent);
    if (hasGUI()) {
        if (startTxEvent->isScheduled())
            updateDisplayString(startTxEvent->getArrivalTime());
        else
            updateDisplayString(-1);
    }
}

void Contention::mediumStateChanged(bool mediumFree)
{
    Enter_Method_Silent(mediumFree ? "medium FREE" : "medium BUSY");
    this->mediumFree = mediumFree;
    lastChannelBusyTime = simTime();
    handleWithFSM(MEDIUM_STATE_CHANGED);
}

void Contention::handleMessage(cMessage *msg)
{
    if (msg == startTxEvent) {
        emit(backoffStoppedSignal, SimTime::ZERO);
        handleWithFSM(CHANNEL_ACCESS_GRANTED);
    }
    else if (msg == channelGrantedEvent) {
        EV_INFO << "Channel granted: startTime = " << startTime << std::endl;
        emit(channelAccessGrantedSignal, this);
        callback->channelAccessGranted();
        callback = nullptr;
    }
    else
        throw cRuntimeError("Unknown msg");
}

void Contention::corruptedFrameReceived()
{
    Enter_Method_Silent("corruptedFrameReceived");
    handleWithFSM(CORRUPTED_FRAME_RECEIVED);
}

void Contention::scheduleTransmissionRequestFor(simtime_t txStartTime)
{
    scheduleAt(txStartTime, startTxEvent);
    callback->expectedChannelAccess(txStartTime);
    emit(backoffStartedSignal, txStartTime);
    if (hasGUI())
        updateDisplayString(txStartTime);
}

void Contention::cancelTransmissionRequest()
{
    cancelEvent(startTxEvent);
    callback->expectedChannelAccess(-1);
    emit(backoffStoppedSignal, SimTime::ZERO);
    if (hasGUI())
        updateDisplayString(-1);
}

void Contention::scheduleTransmissionRequest()
{
    ASSERT(mediumFree);
    simtime_t now = simTime();
    bool useEifs = endEifsTime > now + ifs;
    simtime_t waitInterval = (useEifs ? eifs : ifs) + backoffSlots * slotTime;
    EV_INFO << "Scheduling contention end: backoffslots = " << backoffSlots << ", slotTime = " << slotTime;
    if (backoffOptimization && fsm.getState() == IDLE) {
        // we can pretend the frame has arrived into the queue a little bit earlier, and may be able to start transmitting immediately
        simtime_t elapsedFreeChannelTime = now - lastChannelBusyTime;
        simtime_t elapsedIdleTime = now - lastIdleStartTime;
        EV_INFO << ", lastBusyTime = " << lastChannelBusyTime << ", lastIdle = " << lastIdleStartTime;
        backoffOptimizationDelta = std::min(waitInterval, std::min(elapsedFreeChannelTime, elapsedIdleTime));
        if (backoffOptimizationDelta > SIMTIME_ZERO)
            waitInterval -= backoffOptimizationDelta;
    }
    scheduledTransmissionTime = now + waitInterval;
    EV_INFO << ", waitInterval = " <<  waitInterval << ".\n";
    scheduleTransmissionRequestFor(scheduledTransmissionTime);
}

void Contention::switchToEifs()
{
    EV_DEBUG << "Switching to EIFS from DISF.\n";
    endEifsTime = simTime() + eifs;
    cancelTransmissionRequest();
    scheduleTransmissionRequest();
}

void Contention::computeRemainingBackoffSlots()
{
    simtime_t remainingTime = scheduledTransmissionTime - simTime();
    int remainingSlots = (remainingTime.raw() + slotTime.raw() - 1) / slotTime.raw();
    if (remainingSlots < backoffSlots) // don't count IFS
        backoffSlots = remainingSlots;
}

// TODO: we should call it when internal collision occurs after backoff optimization
void Contention::revokeBackoffOptimization()
{
    EV_DEBUG << "Revoking backoff optimization: backoffOptimizationDelta = " << backoffOptimizationDelta << std::endl;
    scheduledTransmissionTime += backoffOptimizationDelta;
    backoffOptimizationDelta = SIMTIME_ZERO;
    cancelTransmissionRequest();
    computeRemainingBackoffSlots();
    scheduleTransmissionRequest();
}

const char *Contention::getEventName(EventType event)
{
#define CASE(x)   case x: return #x;
    switch (event) {
        CASE(START);
        CASE(MEDIUM_STATE_CHANGED);
        CASE(CORRUPTED_FRAME_RECEIVED);
        CASE(CHANNEL_ACCESS_GRANTED);
        default: ASSERT(false); return "";
    }
#undef CASE
}

void Contention::updateDisplayString(simtime_t expectedChannelAccess)
{
    getDisplayString().setTagArg("t", 0, fsm.getStateName());
}

} // namespace ieee80211
} // namespace inet
