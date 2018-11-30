//
// Copyright (C) 2013 OpenSim Ltd
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
// author: Zoltan Bojthe
//

#include "inet/common/lifecycle/OperationalBase.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/lifecycle/NodeStatus.h"

namespace inet {

OperationalBase::OperationalBase() :
    operational(NOT_OPERATING)
{
}

OperationalBase::~OperationalBase()
{
}

void OperationalBase::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        WATCH(operational);
    }
    if (isInitializeStage(stage)) {
        cModule *node = findContainingNode(this);
        NodeStatus *nodeStatus = node ? check_and_cast_nullable<NodeStatus *>(node->getSubmodule("status")) : nullptr;
        setOperational((!nodeStatus || nodeStatus->getState() == NodeStatus::UP) ? OPERATING : NOT_OPERATING);
        if (operational != NOT_OPERATING)
            handleStartOperation(nullptr);     //TODO use an InitializeOperation with current stage value
    }
}

void OperationalBase::handleMessage(cMessage *message)
{
    if (isOperationTimeout(message))
        handleOperationTimeout(message);
    else {
        switch (operational) {
            case STARTING_OPERATION:
            case OPERATING:
            case STOPPING_OPERATION:
            case SUSPENDING_OPERATION:
            case RESUMING_OPERATION:
                handleMessageWhenUp(message);
                break;
            case OPERATION_SUSPENDED:
            case NOT_OPERATING:
                handleMessageWhenDown(message);
                break;
            case CRASHING_OPERATION:
            default:
                throw cRuntimeError("invalid operational status: %d", (int)operational);
        }
        // do not merge switches. the handleMessageWhenUp/Down() maybe start a lifecycle operation
        switch (operational) {
            case STARTING_OPERATION:
            case STOPPING_OPERATION:
            case SUSPENDING_OPERATION:
            case RESUMING_OPERATION:
                ASSERT(activeOperation.isPending);
                if (isOperationFinished())
                    operationCompleted();
                break;
            case OPERATING:
            case OPERATION_SUSPENDED:
            case NOT_OPERATING:
                break;
            case CRASHING_OPERATION:
            default:
                throw cRuntimeError("invalid operational status: %d", (int)operational);
        }
    }
}

void OperationalBase::handleMessageWhenDown(cMessage *message)
{
    if (message->isSelfMessage())
        throw cRuntimeError("Self message '%s' received when %s is down", message->getName(), getComponentType()->getName());
    else if (simTime() == lastChange)
        EV_WARN << getComponentType()->getName() << " is down, dropping '" << message->getName() << "' message\n";
    else
        throw cRuntimeError("Message '%s' received when %s is down", message->getName(), getComponentType()->getName());
    delete message;
}

bool OperationalBase::handleOperationStage(LifecycleOperation *operation, IDoneCallback *doneCallback)
{
    Enter_Method_Silent();
    int stage = operation->getCurrentStage();
    if (dynamic_cast<ModuleStartOperation *>(operation)) {
        if (isModuleStartStage(stage)) {
            operational = STARTING_OPERATION;
            operationStarted(operation, doneCallback, OPERATING);
            handleStartOperation(operation);
            bool done = isOperationFinished();
            if (done)
                operationCompleted();
            else
                operationPending();
            return done;
        }
    }
    else if (dynamic_cast<ModuleStopOperation *>(operation)) {
        if (isModuleStopStage(stage)) {
            operational = STOPPING_OPERATION;
            operationStarted(operation, doneCallback, NOT_OPERATING);
            handleStopOperation(operation);
            bool done = isOperationFinished();
            if (done)
                operationCompleted();
            else
                operationPending();
            return done;
        }
    }
    else if (dynamic_cast<ModuleCrashOperation *>(operation)) {
        if (stage == ModuleCrashOperation::STAGE_CRASH) {
            operational = CRASHING_OPERATION;
            operationStarted(operation, doneCallback, NOT_OPERATING);
            handleCrashOperation(operation);
            operationCompleted();   // set operational, doesn't call invoke
            return true;
        }
    }
    else
        throw cRuntimeError("unaccepted Lifecycle operation: (%s)%s", operation->getClassName(), operation->getName());
    return true;
}

void OperationalBase::handleSuspendOperation(LifecycleOperation *operation)
{
}

void OperationalBase::handleResumeOperation(LifecycleOperation *operation)
{
}

void OperationalBase::scheduleOperationTimeout(simtime_t timeout)
{
    ASSERT(activeOperation.isPending);
    ASSERT(operationTimeoutMsg == nullptr);
    operationTimeoutMsg = new cMessage("OperationTimeout");
    operationTimeoutMsg->setContextPointer(activeOperation.operation);
    scheduleAt(simTime() + timeout, operationTimeoutMsg);
    // TODO: schedule timer and use module parameter
}

bool OperationalBase::isOperationTimeout(cMessage *message)
{
    return message == operationTimeoutMsg;
}

void OperationalBase::handleOperationTimeout(cMessage *message)
{
    handleCrashOperation(nullptr);
    operationCompleted();
}

bool OperationalBase::hasMessageScheduledForNow()
{
    cFutureEventSet *fes = getSimulation()->getFES();
    int fesLen = fes->getLength();
    auto myModuleId = this->getId();
    auto now = simTime();
    for (int i = 0; i < fesLen; i++) {
        cEvent *event = fes->get(i);
        if (cMessage *msg = dynamic_cast<cMessage *>(event)) {
            if (msg->getArrivalModuleId() == myModuleId && msg->getArrivalTime() == now)
                return true;
        }
    }
    return false;
}

void OperationalBase::operationStarted(LifecycleOperation *operation, IDoneCallback *doneCallback, State endOperation)
{
    ASSERT(activeOperation.operation == nullptr);
    activeOperation.set(operation, doneCallback, endOperation);
}

void OperationalBase::operationPending()
{
    ASSERT(activeOperation.operation != nullptr);
    activeOperation.pending();
    scheduleOperationTimeout(2.0); //TODO timeout parameter instead of 2.0s
}

bool OperationalBase::isOperationFinished()
{
    switch (operational) {
        case STOPPING_OPERATION:
        case SUSPENDING_OPERATION:
            return ! hasMessageScheduledForNow();
        default:
            return true;
    }
}

void OperationalBase::operationCompleted()
{
    setOperational(activeOperation.endOperation);
    if (activeOperation.isPending) {
        cancelAndDelete(operationTimeoutMsg);
        operationTimeoutMsg = nullptr;
        activeOperation.doneCallback->invoke();
    }
    activeOperation.clear();
}

void OperationalBase::setOperational(State newState)
{
    operational = newState;
    lastChange = simTime();
}

void OperationalBase::refreshDisplay() const
{
    switch (operational) {
    case STARTING_OPERATION:
    case RESUMING_OPERATION:
        getDisplayString().setTagArg("i2", 0, "status/up");
        break;
    case OPERATING:
        getDisplayString().removeTag("i2");
        break;
    case STOPPING_OPERATION:
    case CRASHING_OPERATION:
    case SUSPENDING_OPERATION:
        getDisplayString().setTagArg("i2", 0, "status/down");
        break;
    case NOT_OPERATING:
        getDisplayString().setTagArg("i2", 0, "status/cross");
        break;
    case OPERATION_SUSPENDED:
        getDisplayString().setTagArg("i2", 0, "status/stop");
        break;
    default:
        break;
    }
}

} // namespace inet

