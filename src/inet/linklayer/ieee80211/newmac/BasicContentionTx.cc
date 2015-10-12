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

// for @statistic; don't forget to keep synchronized the C++ enum and the runtime enum definition
Register_Enum(BasicContentionTx::State,
        (BasicContentionTx::IDLE,
         BasicContentionTx::DEFER,
         BasicContentionTx::IFS_AND_BACKOFF,
         BasicContentionTx::TRANSMIT));

Define_Module(BasicContentionTx);

// non-member utility function
void collectContentionTxModules(cModule *firstContentionTxModule, IContentionTx **& contentionTx)
{
    ASSERT(firstContentionTxModule != nullptr);
    int count = firstContentionTxModule->getVectorSize();

    contentionTx = new IContentionTx *[count + 1];
    for (int i = 0; i < count; i++) {
        cModule *sibling = firstContentionTxModule->getParentModule()->getSubmodule(firstContentionTxModule->getName(), i);
        contentionTx[i] = check_and_cast<IContentionTx *>(sibling);
    }
    contentionTx[count] = nullptr;
}

void BasicContentionTx::initialize()
{
    mac = check_and_cast<IMacRadioInterface *>(getModuleByPath(par("macModule")));
    upperMac = check_and_cast<IUpperMac *>(getModuleByPath(par("upperMacModule")));
    collisionController = dynamic_cast<ICollisionController *>(getModuleByPath(par("collisionControllerModule")));

    txIndex = getIndex();
    if (txIndex > 0 && !collisionController)
        throw cRuntimeError("No collision controller module -- one is needed when multiple ContentionTx instances are present");

    if (!collisionController)
        startTxEvent = new cMessage("startTx");

    fsm.setName("fsm");
    fsm.setState(IDLE, "IDLE");

    WATCH(txIndex);
    WATCH(ifs);
    WATCH(eifs);
    WATCH(cwMin);
    WATCH(cwMax);
    WATCH(slotTime);
    WATCH(retryCount);
    WATCH(endEifsTime);
    WATCH(backoffSlots);
    WATCH(scheduledTransmissionTime);
    WATCH(channelLastBusyTime);
    WATCH(mediumFree);
    updateDisplayString();
}

BasicContentionTx::~BasicContentionTx()
{
    if (frame && fsm.getState() != TRANSMIT)
        delete frame;
    cancelAndDelete(startTxEvent);
}

void BasicContentionTx::transmitContentionFrame(Ieee80211Frame *frame, simtime_t ifs, simtime_t eifs, int cwMin, int cwMax, simtime_t slotTime, int retryCount, ITxCallback *completionCallback)
{
    Enter_Method("transmitContentionFrame(\"%s\")", frame->getName());
    ASSERT(fsm.getState() == IDLE);
    take(frame);
    this->frame = frame;
    this->ifs = ifs;
    this->eifs = eifs;
    this->cwMin = cwMin;
    this->cwMax = cwMax;
    this->slotTime = slotTime;
    this->retryCount = retryCount;
    this->completionCallback = completionCallback;

    int cw = computeCw(cwMin, cwMax, retryCount);
    backoffSlots = intrand(cw + 1);
    handleWithFSM(START, frame);
}

int BasicContentionTx::computeCw(int cwMin, int cwMax, int retryCount)
{
    int cw = ((cwMin + 1) << retryCount) - 1;
    if (cw > cwMax)
        cw = cwMax;
    return cw;
}

void BasicContentionTx::handleWithFSM(EventType event, cMessage *msg)
{
    // emit(stateSignal, fsm.getState()); TODO
    EV_DEBUG << "handleWithFSM: processing event " << getEventName(event) << "\n";
    bool finallyReportInternalCollision = false;
    bool finallyReportTransmissionComplete = false;
    FSMA_Switch(fsm) {
        FSMA_State(IDLE) {
            FSMA_Enter(mac->sendDownPendingRadioConfigMsg(); frame = nullptr);
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
            FSMA_Ignore_Event(event==TRANSMISSION_FINISHED);
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
            FSMA_Ignore_Event(event==TRANSMISSION_FINISHED);  // i.e. of another Tx
            FSMA_Fail_On_Unhandled_Event();
        }
        FSMA_State(IFS_AND_BACKOFF) {
            FSMA_Enter();
            FSMA_Event_Transition(Backoff-expired,
                    event == TRANSMISSION_GRANTED,
                    TRANSMIT,
                    ;
                    );
            FSMA_Event_Transition(Defer-on-channel-busy,
                    event == MEDIUM_STATE_CHANGED && !mediumFree,
                    DEFER,
                    cancelTransmissionRequest();
                    computeRemainingBackoffSlots();
                    );
            FSMA_Event_Transition(Internal-collision,
                    event == INTERNAL_COLLISION,
                    IDLE,
                    finallyReportInternalCollision = true; delete frame;
                    );
            FSMA_Event_Transition(Use-EIFS,
                    event == CORRUPTED_FRAME_RECEIVED,
                    IFS_AND_BACKOFF,
                    switchToEifs();
                    );
            FSMA_Fail_On_Unhandled_Event();
        }
        FSMA_State(TRANSMIT) {
            FSMA_Enter(mac->sendFrame(frame));
            FSMA_Event_Transition(TxFinished,
                    event == TRANSMISSION_FINISHED,
                    IDLE,
                    finallyReportTransmissionComplete = true;
                    );
            FSMA_Ignore_Event(event==MEDIUM_STATE_CHANGED);
            FSMA_Fail_On_Unhandled_Event();
        }
    }
    // emit(stateSignal, fsm.getState()); TODO
    if (finallyReportTransmissionComplete)
        reportTransmissionComplete();
    if (finallyReportInternalCollision)
        reportInternalCollision();
    if (hasGUI())
        updateDisplayString();
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
    Enter_Method_Silent();
    handleWithFSM(TRANSMISSION_FINISHED, nullptr);
}

void BasicContentionTx::handleMessage(cMessage *msg)
{
    ASSERT(msg == startTxEvent);
    transmissionGranted(txIndex);
}

void BasicContentionTx::corruptedFrameReceived()
{
    Enter_Method("corruptedFrameReceived()");
    handleWithFSM(CORRUPTED_FRAME_RECEIVED, nullptr);
}

void BasicContentionTx::transmissionGranted(int txIndex)
{
    Enter_Method("transmissionGranted()");
    handleWithFSM(TRANSMISSION_GRANTED, nullptr);
}

void BasicContentionTx::internalCollision(int txIndex)
{
    Enter_Method("internalCollision()");
    handleWithFSM(INTERNAL_COLLISION, nullptr);
}

void BasicContentionTx::scheduleTransmissionRequestFor(simtime_t txStartTime)
{
    if (collisionController)
        collisionController->scheduleTransmissionRequest(txIndex, txStartTime, this);
    else
        scheduleAt(txStartTime, startTxEvent);
}

void BasicContentionTx::cancelTransmissionRequest()
{
    if (collisionController)
        collisionController->cancelTransmissionRequest(txIndex);
    else
        cancelEvent(startTxEvent);
}

void BasicContentionTx::scheduleTransmissionRequest()
{
    ASSERT(mediumFree);

    simtime_t now = simTime();
    bool useEifs = endEifsTime > now + ifs;
    simtime_t waitInterval = (useEifs ? eifs : ifs) + backoffSlots * slotTime;

    if (retryCount == 0) {
        // we can pretend the frame has arrived into the queue a little bit earlier, and may be able to start transmitting immediately
        simtime_t elapsedFreeChannelTime = now - channelLastBusyTime;
        if (elapsedFreeChannelTime > waitInterval)
            waitInterval = 0;
        else
            waitInterval -= elapsedFreeChannelTime;
    }
    scheduledTransmissionTime = now + waitInterval;
    scheduleTransmissionRequestFor(scheduledTransmissionTime);
}

void BasicContentionTx::switchToEifs()
{
    endEifsTime = simTime() + eifs;
    cancelTransmissionRequest();
    scheduleTransmissionRequest();
}

void BasicContentionTx::computeRemainingBackoffSlots()
{
    simtime_t remainingTime = scheduledTransmissionTime - simTime();
    int remainingSlots = (int)ceil(remainingTime / slotTime);  //TODO this is not accurate
    if (remainingSlots < backoffSlots) // don't count IFS
        backoffSlots = remainingSlots;
}

void BasicContentionTx::reportTransmissionComplete()
{
    upperMac->transmissionComplete(completionCallback, txIndex);
}

void BasicContentionTx::reportInternalCollision()
{
    upperMac->internalCollision(completionCallback, txIndex);
}

const char *BasicContentionTx::getEventName(EventType event)
{
#define CASE(x)   case x: return #x;
    switch (event) {
        CASE(START);
        CASE(MEDIUM_STATE_CHANGED);
        CASE(CORRUPTED_FRAME_RECEIVED);
        CASE(TRANSMISSION_GRANTED);
        CASE(INTERNAL_COLLISION);
        CASE(TRANSMISSION_FINISHED);
        default: ASSERT(false); return "";
    }
#undef CASE
}

void BasicContentionTx::updateDisplayString()
{
    // faster version is just to display the state: getDisplayString().setTagArg("t", 0, fsm.getStateName());
    std::stringstream os;
    if (frame)
        os << frame->getName() << "\n";
    const char *stateName = fsm.getStateName();
    if (strcmp(stateName, "IFS_AND_BACKOFF") == 0)
        stateName = "IFS+BKOFF";  // original text is too long
    os << stateName;
    getDisplayString().setTagArg("t", 0, os.str().c_str());
}

} // namespace ieee80211
} // namespace inet

