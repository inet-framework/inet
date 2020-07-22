//
// Copyright (C) OpenSim Ltd
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
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __INET_OPERATIONALMIXINIMPL_H
#define __INET_OPERATIONALMIXINIMPL_H

#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/common/lifecycle/OperationalMixin.h"
#include "inet/common/ModuleAccess.h"

namespace inet {

template <typename T>
OperationalMixin<T>::~OperationalMixin()
{
    T::cancelAndDelete(activeOperationExtraTimer);
    T::cancelAndDelete(activeOperationTimeout);
}

template <typename T>
void OperationalMixin<T>::initialize(int stage)
{
    T::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        WATCH(operationalState);
        activeOperationTimeout = new cMessage("ActiveOperationTimeout");
        activeOperationExtraTimer = new cMessage("ActiveOperationExtraTimer");
        activeOperationExtraTimer->setSchedulingPriority(std::numeric_limits<short>::max());
    }
    if (isInitializeStage(stage)) {
        auto state = getInitialOperationalState();
        ASSERT(state == NOT_OPERATING || state == OPERATING);
        setOperationalState(state);
        if (operationalState == OPERATING)
            handleStartOperation(nullptr);     //TODO use an InitializeOperation with current stage value
    }
}

template <typename T>
void OperationalMixin<T>::handleMessage(cMessage *message)
{
    if (message == activeOperationExtraTimer)
        finishActiveOperation();
    else if (message == activeOperationTimeout) {
        T::cancelEvent(activeOperationExtraTimer);
        handleActiveOperationTimeout(message);
    }
    else {
        switch (operationalState) {
            case STARTING_OPERATION:
            case OPERATING:
            case STOPPING_OPERATION:
                handleMessageWhenUp(message);
                break;
            case NOT_OPERATING:
                handleMessageWhenDown(message);
                break;
            case CRASHING_OPERATION:
            default:
                throw cRuntimeError("Invalid operational state: %d", (int)operationalState);
        }
    }
}

template <typename T>
void OperationalMixin<T>::handleMessageWhenDown(cMessage *message)
{
    if (message->isSelfMessage())
        throw cRuntimeError("Self message '%s' received when %s is down", message->getName(), T::getComponentType()->getName());
    else if (simTime() == lastChange)
        EV_WARN << T::getComponentType()->getName() << " is down, dropping '" << message->getName() << "' message\n";
    else
        throw cRuntimeError("Message '%s' received when %s is down", message->getName(), T::getComponentType()->getName());
    delete message;
}

template <typename T>
bool OperationalMixin<T>::handleOperationStage(LifecycleOperation *operation, IDoneCallback *doneCallback)
{
    Enter_Method_Silent();
    int stage = operation->getCurrentStage();
    if (dynamic_cast<ModuleStartOperation *>(operation)) {
        if (isModuleStartStage(stage)) {
            operationalState = STARTING_OPERATION;
            setupActiveOperation(operation, doneCallback, OPERATING);
            handleStartOperation(operation);
            if (activeOperation.operation != nullptr && !activeOperation.isDelayedFinish)
                finishActiveOperation();
            return activeOperation.operation == nullptr;
        }
    }
    else if (dynamic_cast<ModuleStopOperation *>(operation)) {
        if (isModuleStopStage(stage)) {
            operationalState = STOPPING_OPERATION;
            setupActiveOperation(operation, doneCallback, NOT_OPERATING);
            handleStopOperation(operation);
            if (activeOperation.operation != nullptr && !activeOperation.isDelayedFinish)
                finishActiveOperation();
            return activeOperation.operation == nullptr;
        }
    }
    else if (dynamic_cast<ModuleCrashOperation *>(operation)) {
        if (stage == ModuleCrashOperation::STAGE_CRASH) {
            operationalState = CRASHING_OPERATION;
            setupActiveOperation(operation, doneCallback, NOT_OPERATING);
            handleCrashOperation(operation);
            finishActiveOperation();
            return true;
        }
    }
    else
        throw cRuntimeError("unaccepted Lifecycle operation: (%s)%s", operation->getClassName(), operation->getName());
    return true;
}

template <typename T>
void OperationalMixin<T>::scheduleOperationTimeout(simtime_t timeout)
{
    ASSERT(activeOperation.isDelayedFinish);
    ASSERT(!activeOperationTimeout->isScheduled());
    activeOperationTimeout->setContextPointer(activeOperation.operation);
    T::scheduleAt(simTime() + timeout, activeOperationTimeout);
    // TODO: schedule timer and use module parameter
}

template <typename T>
void OperationalMixin<T>::handleActiveOperationTimeout(cMessage *message)
{
    handleCrashOperation(activeOperation.operation);
    finishActiveOperation();
}

template <typename T>
void OperationalMixin<T>::setupActiveOperation(LifecycleOperation *operation, IDoneCallback *doneCallback, State endState)
{
    ASSERT(activeOperation.operation == nullptr);
    activeOperation.set(operation, doneCallback, endState);
}

template <typename T>
void OperationalMixin<T>::setOperationalState(State newState)
{
    operationalState = newState;
    lastChange = simTime();
}

template <typename T>
typename OperationalMixin<T>::State OperationalMixin<T>::getInitialOperationalState() const
{
    cModule *node = findContainingNode(this);
    NodeStatus *nodeStatus = node ? check_and_cast_nullable<NodeStatus *>(node->getSubmodule("status")) : nullptr;
    return (!nodeStatus || nodeStatus->getState() == NodeStatus::UP) ? OPERATING : NOT_OPERATING;
}

template <typename T>
void OperationalMixin<T>::delayActiveOperationFinish(simtime_t timeout)
{
    ASSERT(activeOperation.operation != nullptr);
    activeOperation.delayFinish();
    scheduleOperationTimeout(timeout);
}

template <typename T>
void OperationalMixin<T>::startActiveOperationExtraTime(simtime_t extraTime)
{
    ASSERT(extraTime >= SIMTIME_ZERO);
    ASSERT(!activeOperationExtraTimer->isScheduled());
    activeOperation.delayFinish();
    setOperationalState(activeOperation.endState);
    T::scheduleAt(simTime() + extraTime, activeOperationExtraTimer);
}

template <typename T>
void OperationalMixin<T>::startActiveOperationExtraTimeOrFinish(simtime_t extraTime)
{
    if (extraTime >= SIMTIME_ZERO)
        startActiveOperationExtraTime(extraTime);
    else
        finishActiveOperation();
}

template <typename T>
void OperationalMixin<T>::finishActiveOperation()
{
    setOperationalState(activeOperation.endState);
    if (activeOperation.isDelayedFinish) {
        T::cancelEvent(activeOperationExtraTimer);
        T::cancelEvent(activeOperationTimeout);
        activeOperation.doneCallback->invoke();
    }
    activeOperation.clear();
}

template <typename T>
void OperationalMixin<T>::refreshDisplay() const
{
    auto& displayString = T::getDisplayString();
    switch (operationalState) {
    case STARTING_OPERATION:
        displayString.setTagArg("i2", 0, "status/up");
        break;
    case OPERATING:
        displayString.removeTag("i2");
        break;
    case STOPPING_OPERATION:
    case CRASHING_OPERATION:
        displayString.setTagArg("i2", 0, "status/down");
        break;
    case NOT_OPERATING:
        displayString.setTagArg("i2", 0, "status/cross");
        break;
    default:
        break;
    }
}

} // namespace inet

#endif // ifndef __INET_OPERATIONALMIXINIMPL_H

