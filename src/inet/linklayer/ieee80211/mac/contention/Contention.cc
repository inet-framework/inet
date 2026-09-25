//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/contention/Contention.h"

#include "inet/common/ModuleAccess.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"

namespace inet {
namespace ieee80211 {

simsignal_t Contention::stateChangedSignal = registerSignal("stateChanged");

// for @statistic; don't forget to keep synchronized the C++ enum and the runtime enum definition
Register_Enum(Contention::State,
        (Contention::IDLE,
         Contention::DEFER,
         Contention::IFS_AND_BACKOFF));

Define_Module(Contention);

void Contention::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        backoffOptimization = par("backoffOptimization");
        lastIdleStartTime = simTime() - SimTime::getMaxTime() / 2;
        mac = check_and_cast<Ieee80211Mac *>(getContainingNicModule(this)->getSubmodule("mac"));
        startTxEvent = new cMessage("startTx");
        startTxEvent->setSchedulingPriority(1000); // low priority, i.e. processed later than most events for the same time
        // KLUDGE
        // The callback->channelAccessGranted() call should be the last
        // event at a simulation time in order to handle internal collisions
        // properly.
        channelGrantedEvent = new cMessage("channelGranted");
        channelGrantedEvent->setSchedulingPriority(1000);
        fsm.setName("Backoff procedure");
        fsm.setState(IDLE, "IDLE");

        WATCH(fsm);
        WATCH_EXPR("fsmState", fsm.getStateName());
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
    Enter_Method("startContention");
    cancelEvent(channelGrantedEvent);
    ASSERT(fsm.getState() == IDLE);
    this->ifs = ifs;
    this->eifs = eifs;
    this->slotTime = slotTime;
    this->callback = callback;
    backoffSlots = intrand(cw + 1);
    auto revision = cancellationRevision;
    emit(backoffPeriodGeneratedSignal, backoffSlots);
    if (revision != cancellationRevision)
        return;
    EV_DETAIL << "Starting contention: cw = " << cw << ", slots = " << backoffSlots << ", slotTime = " << slotTime << ", ifs = " << ifs << ", eifs = " << eifs << endl;
    handleWithFSM(START);
}

void Contention::handleWithFSM(EventType event)
{
    auto revision = cancellationRevision;
    emit(stateChangedSignal, fsm.getState());
    if (revision != cancellationRevision)
        return;
    EV_TRACE << "handleWithFSM: processing event " << getEventName(event) << "\n";
    bool finallyReportChannelAccessGranted = false;
    FSMA_Switch(fsm) {
        FSMA_State(IDLE) {
            FSMA_Enter(mac->sendDownPendingRadioConfigMsg());
            FSMA_Event_Transition(Starting-IFS-and-Backoff,
                    event == START && mediumFree,
                    IFS_AND_BACKOFF,
                    scheduleTransmissionRequest();
                    if (revision != cancellationRevision) return;
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
                    if (revision != cancellationRevision) return;
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
                    if (revision != cancellationRevision) return;
                    computeRemainingBackoffSlots();
                    );
            FSMA_Event_Transition(Use-EIFS,
                    event == CORRUPTED_FRAME_RECEIVED,
                    IFS_AND_BACKOFF,
                    switchToEifs();
                    if (revision != cancellationRevision) return;
                    );
            FSMA_Fail_On_Unhandled_Event();
        }
    }
    emit(stateChangedSignal, fsm.getState());
    if (revision != cancellationRevision)
        return;
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
    Enter_Method(mediumFree ? "medium FREE" : "medium BUSY");
    this->mediumFree = mediumFree;
    lastChannelBusyTime = simTime();
    handleWithFSM(MEDIUM_STATE_CHANGED);
}

void Contention::handleMessage(cMessage *msg)
{
    if (msg == startTxEvent) {
        auto revision = cancellationRevision;
        emit(backoffStoppedSignal, SimTime::ZERO);
        if (revision == cancellationRevision)
            handleWithFSM(CHANNEL_ACCESS_GRANTED);
    }
    else if (msg == channelGrantedEvent) {
        EV_INFO << "Channel granted: startTime = " << startTime << std::endl;
        auto grantedCallback = callback;
        auto revision = cancellationRevision;
        // Keep the request cancellable until the grant signal returns.
        // A listener can cancel it and install a replacement request.
        emit(channelAccessGrantedSignal, this);
        if (revision == cancellationRevision) {
            callback = nullptr;
            grantedCallback->channelAccessGranted();
        }
    }
    else
        throw cRuntimeError("Unknown msg");
}

void Contention::cancelContention()
{
    Enter_Method("cancelContention");
    ++cancellationRevision;
    cancelEvent(startTxEvent);
    cancelEvent(channelGrantedEvent);
    auto cancelledCallback = callback;
    callback = nullptr;
    fsm.setState(IDLE, "IDLE");
    if (cancelledCallback)
        cancelledCallback->expectedChannelAccess(-1);
}

void Contention::corruptedFrameReceived()
{
    Enter_Method("corruptedFrameReceived");
    handleWithFSM(CORRUPTED_FRAME_RECEIVED);
}

void Contention::scheduleTransmissionRequestFor(simtime_t txStartTime)
{
    auto revision = cancellationRevision;
    scheduleAt(txStartTime, startTxEvent);
    callback->expectedChannelAccess(txStartTime);
    if (revision != cancellationRevision)
        return;
    emit(backoffStartedSignal, txStartTime);
    if (hasGUI())
        updateDisplayString(txStartTime);
}

void Contention::cancelTransmissionRequest()
{
    auto revision = cancellationRevision;
    cancelEvent(startTxEvent);
    callback->expectedChannelAccess(-1);
    if (revision != cancellationRevision)
        return;
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
    EV_INFO << ", waitInterval = " << waitInterval << ".\n";
    scheduleTransmissionRequestFor(scheduledTransmissionTime);
}

void Contention::switchToEifs()
{
    EV_DEBUG << "Switching to EIFS from DISF.\n";
    endEifsTime = simTime() + eifs;
    auto revision = cancellationRevision;
    cancelTransmissionRequest();
    if (revision != cancellationRevision)
        return;
    scheduleTransmissionRequest();
}

void Contention::computeRemainingBackoffSlots()
{
    simtime_t remainingTime = scheduledTransmissionTime - simTime();
    int remainingSlots = (remainingTime.raw() + slotTime.raw() - 1) / slotTime.raw();
    if (remainingSlots < backoffSlots) // don't count IFS
        backoffSlots = remainingSlots;
}

// TODO we should call it when internal collision occurs after backoff optimization
void Contention::revokeBackoffOptimization()
{
    EV_DEBUG << "Revoking backoff optimization: backoffOptimizationDelta = " << backoffOptimizationDelta << std::endl;
    scheduledTransmissionTime += backoffOptimizationDelta;
    backoffOptimizationDelta = SIMTIME_ZERO;
    auto revision = cancellationRevision;
    cancelTransmissionRequest();
    if (revision != cancellationRevision)
        return;
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
        default: ASSERT(false); return "?";
    }
#undef CASE
}

void Contention::updateDisplayString(simtime_t expectedChannelAccess) const
{
}

} // namespace ieee80211
} // namespace inet
