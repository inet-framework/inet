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
#include "inet/common/lifecycle/NodeOperations.h"
#include "inet/common/lifecycle/NodeStatus.h"

namespace inet {

OperationalBase::OperationalBase() :
    operational(DOWN)
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
        setOperational((!nodeStatus || nodeStatus->getState() == NodeStatus::UP) ? UP : DOWN);
        if (operational != DOWN)
            handleNodeStart(nullptr);
    }
}

void OperationalBase::handleMessage(cMessage *message)
{
    if (operational != DOWN)
        handleMessageWhenUp(message);
    else
        handleMessageWhenDown(message);
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
    if (dynamic_cast<NodeStartOperation *>(operation)) {
        if (isNodeStartStage(stage)) {
            setOperational(GOING_UP);
            bool done = handleNodeStart(doneCallback);
            if (done)
                setOperational(UP);
            return done;
        }
    }
    else if (dynamic_cast<NodeShutdownOperation *>(operation)) {
        if (isNodeShutdownStage(stage)) {
            setOperational(GOING_DOWN);
            bool done = handleNodeShutdown(doneCallback);
            if (done)
                setOperational(DOWN);
            return done;
        }
    }
    else if (dynamic_cast<NodeCrashOperation *>(operation)) {
        if (stage == NodeCrashOperation::STAGE_CRASH) {
            handleNodeCrash();
            setOperational(DOWN);
            return true;
        }
    }
    return true;
}

bool OperationalBase::handleNodeStart(IDoneCallback *doneCallback)
{
    return true;
}

bool OperationalBase::handleNodeShutdown(IDoneCallback *doneCallback)
{
    return true;
}

void OperationalBase::handleNodeCrash()
{
}

void OperationalBase::setOperational(State newState)
{
    operational = newState;
    lastChange = simTime();
}

} // namespace inet

