//
// Copyright (C) 2013 Opensim Ltd.
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
// Author: Levente Meszaros (levy@omnetpp.org)
//

#include <algorithm>
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/lifecycle/NodeOperations.h"

namespace inet {

Register_Enum(inet::NodeStatus, (NodeStatus::UP, NodeStatus::DOWN, NodeStatus::GOING_UP, NodeStatus::GOING_DOWN));
Define_Module(NodeStatus);

simsignal_t NodeStatus::nodeStatusChangedSignal = registerSignal("nodeStatusChanged");

void NodeStatus::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        state = getStateByName(par("initialStatus"));
        origIcon = getDisplayString().getTagArg("i", 0);
        updateDisplayString();
    }
}

NodeStatus::State NodeStatus::getStateByName(const char *name)
{
    std::string temp = name;
    std::transform(temp.begin(), temp.end(), temp.begin(), ::toupper);
    cEnum *e = cEnum::get("inet::NodeStatus");
    int state = e->lookup(temp.c_str(), -1);
    if (state == -1)
        throw cRuntimeError("Invalid state name '%s'", name);
    return (State)state;
}

bool NodeStatus::handleOperationStage(LifecycleOperation *operation, int opStage, IDoneCallback *doneCallback)
{
    Enter_Method_Silent();
    cModule *node = getContainingNode(this);
    if (dynamic_cast<NodeStartOperation *>(operation)) {
        if (opStage == 0) {
            EV << node->getFullPath() << " starting up" << endl;
            if (getState() != DOWN)
                throw cRuntimeError("Current node status is not 'down' at NodeStartOperation");
            setState(GOING_UP);
        }
        // NOTE: this is not an 'else if' so that it works if there's only 1 stage
        if (opStage == operation->getNumStages() - 1) {
            ASSERT(getState() == GOING_UP);
            setState(UP);
            EV << node->getFullPath() << " started" << endl;
        }
    }
    else if (dynamic_cast<NodeShutdownOperation *>(operation)) {
        if (opStage == 0) {
            EV << node->getFullPath() << " shutting down" << endl;
            if (getState() != UP)
                throw cRuntimeError("Current node status is not 'up' at NodeShutdownOperation");
            setState(GOING_DOWN);
        }
        // NOTE: this is not an 'else if' so that it works if there's only 1 stage
        if (opStage == operation->getNumStages() - 1) {
            ASSERT(getState() == GOING_DOWN);
            setState(DOWN);
            EV << node->getFullPath() << " shut down" << endl;
        }
    }
    else if (dynamic_cast<NodeCrashOperation *>(operation)) {
        if (opStage == 0) {
            EV << node->getFullPath() << " crashing" << endl;
            if (getState() != UP)
                throw cRuntimeError("Current node status is not 'up' at NodeCrashOperation");
            setState(GOING_DOWN);
        }
        // NOTE: this is not an 'else if' so that it works if there's only 1 stage
        if (opStage == operation->getNumStages() - 1) {
            ASSERT(getState() == GOING_DOWN);
            setState(DOWN);
            EV << node->getFullPath() << " crashed" << endl;
        }
    }
    return true;
}

void NodeStatus::setState(State s)
{
    state = s;
    emit(nodeStatusChangedSignal, this);
    updateDisplayString();
}

void NodeStatus::updateDisplayString()
{
    const char *icon;
    switch (state) {
        case UP:
            icon = "";
            break;

        case DOWN:
            icon = "status/cross";
            break;

        case GOING_UP:
            icon = "status/execute";
            break;

        case GOING_DOWN:
            icon = "status/execute";
            break;

        default:
            throw cRuntimeError("Unknown status");
    }
    cModule *node = getContainingNode(this);
    const char *myicon = state == UP ? origIcon.c_str() : icon;
    getDisplayString().setTagArg("i", 0, myicon);
    if (*icon)
        node->getDisplayString().setTagArg("i2", 0, icon);
    else
        node->getDisplayString().removeTag("i2");
}

} // namespace inet

