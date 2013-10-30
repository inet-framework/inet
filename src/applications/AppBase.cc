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

#include "AppBase.h"

#include "ModuleAccess.h"
#include "NodeOperations.h"


AppBase::AppBase()
{
    isOperational = false;
}

AppBase::~AppBase()
{
}

void AppBase::finish()
{
    stopApp(NULL);
    isOperational = false;
}

void AppBase::initialize(int stage)
{
    InetSimpleModule::initialize(stage);

    if (stage == INITSTAGE_LOCAL)
    {
        isOperational = false;
    }
    else if (stage == INITSTAGE_APPLICATION_LAYER)
    {
        NodeStatus *nodeStatus = dynamic_cast<NodeStatus *>(findContainingNode(this)->getSubmodule("status"));
        isOperational = !nodeStatus || nodeStatus->getState() == NodeStatus::UP;
        if (isOperational)
            startApp(NULL);
    }
}

void AppBase::handleMessage(cMessage *msg)
{
    if (isOperational)
        handleMessageWhenUp(msg);
    else
        handleMessageWhenDown(msg);
}

void AppBase::handleMessageWhenDown(cMessage *msg)
{
    if (msg->isSelfMessage())
        throw cRuntimeError("Model error: self msg '%s' received when isOperational is false", msg->getName());
    EV << "Application is turned off, dropping '" << msg->getName() << "' message\n";
    delete msg;
}

bool AppBase::handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback)
{
    Enter_Method_Silent();
    bool ret = true;

    if (dynamic_cast<NodeStartOperation *>(operation))
    {
        if (stage == NodeStartOperation::STAGE_APPLICATION_LAYER) {
            isOperational = true;
            ret = startApp(doneCallback);
        }
    }
    else if (dynamic_cast<NodeShutdownOperation *>(operation))
    {
        if (stage == NodeShutdownOperation::STAGE_APPLICATION_LAYER) {
            ret = stopApp(doneCallback);
            isOperational = false;
        }
    }
    else if (dynamic_cast<NodeCrashOperation *>(operation))
    {
        if (stage == NodeCrashOperation::STAGE_CRASH) {
            ret = crashApp(doneCallback);
            isOperational = false;
        }
    }
    else
    {
        throw cRuntimeError("Unsupported operation '%s'", operation->getClassName());
    }

    if (!ret)
        throw cRuntimeError("doneCallback/invoke not supported by AppBase");
    return ret;
}

