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

void OperationalBase::DoneCallback::init(IDoneCallback *orig, State state)
{
    this->orig = orig;
    this->state = state;
}

void OperationalBase::DoneCallback::done()
{
    module->setOperational(state);
}
void OperationalBase::DoneCallback::invoke()
{
    done();
    module->operationalInvoked(this);
}

OperationalBase::OperationalBase() :
    operational(NOT_OPERATING)
{
}

OperationalBase::~OperationalBase()
{
    delete spareCallback;
}

OperationalBase::DoneCallback *OperationalBase::newDoneCallback(OperationalBase *module)
{
    if (spareCallback != nullptr) {
        auto callback = spareCallback;
        spareCallback = nullptr;
        return callback;
    }
    else
        return new DoneCallback(this);
}

void OperationalBase::deleteDoneCallback(OperationalBase::DoneCallback *callback)
{
    if (spareCallback != nullptr)
        delete callback;
    else
        spareCallback = callback;
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
            handleStartOperation(nullptr);
    }
}

void OperationalBase::operationalInvoked(OperationalBase::DoneCallback *callback)
{
    IDoneCallback *origCallback = callback->getOrig();
    deleteDoneCallback(callback);
    origCallback->invoke();
}

void OperationalBase::handleMessage(cMessage *message)
{
    switch (operational) {
        case STARTING_OPERATION:
        case OPERATING:
        case STOPPING_OPERATION:
        case CRASHING_OPERATION:
        case SUSPENDING_OPERATION:
        case RESUMING_OPERATION:
            handleMessageWhenUp(message);
            break;
        case OPERATION_SUSPENDED:
        case NOT_OPERATING:
            handleMessageWhenDown(message);
            break;
        default:
            throw cRuntimeError("invalid operational status: %d", (int)operational);
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

bool OperationalBase::handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback)
{
    Enter_Method_Silent();
    if (dynamic_cast<ModuleStartOperation *>(operation)) {
        if (isModuleStartStage(stage)) {
            operational = STARTING_OPERATION;
            DoneCallback *callback = newDoneCallback(this);
            callback->init(doneCallback, OPERATING);
            bool done = handleStartOperation(callback);
            if (done) {
                callback->done();
                deleteDoneCallback(callback);
            }
            return done;
        }
    }
    else if (dynamic_cast<ModuleStopOperation *>(operation)) {
        if (isModuleStopStage(stage)) {
            operational = STOPPING_OPERATION;
            DoneCallback *callback = newDoneCallback(this);
            callback->init(doneCallback, NOT_OPERATING);
            bool done = handleStopOperation(callback);
            if (done) {
                callback->done();
                deleteDoneCallback(callback);
            }
            return done;
        }
    }
    else if (dynamic_cast<ModuleCrashOperation *>(operation)) {
        if (stage == ModuleCrashOperation::STAGE_CRASH) {
            operational = CRASHING_OPERATION;
            handleCrashOperation();
            setOperational(NOT_OPERATING);
            return true;
        }
    }
    return true;
}

bool OperationalBase::handleStartOperation(IDoneCallback *doneCallback)
{
    return true;
}

bool OperationalBase::handleStopOperation(IDoneCallback *doneCallback)
{
    return true;
}

bool OperationalBase::handleSuspendOperation(IDoneCallback *doneCallback)
{
    return true;
}

bool OperationalBase::handleResumeOperation(IDoneCallback *doneCallback)
{
    return true;
}

void OperationalBase::handleCrashOperation()
{
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

