//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "TestMobility.h"
#include "TestOperation.h"

namespace inet {

Define_Module(TestMobility);

bool TestMobility::handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback)
{
    Enter_Method("handleOperationStage");
    if (dynamic_cast<TestNodeStartOperation *>(operation)) {
        if (stage == 0) {
            scheduleAfter(9, &startMoving);
            EV << getFullPath() << " starting to move" << endl;
            return true;
        }
        else if (stage == 1 || stage == 3)
            return true;
        else if (stage == 2) {
            this->doneCallback = doneCallback;
            return false;
        }
        else
            throw cRuntimeError("Unknown stage");
    }
    else if (dynamic_cast<TestNodeShutdownOperation *>(operation)) {
        if (stage == 0) {
            scheduleAfter(9, &stopMoving);
            EV << getFullPath() << " stopping to move" << endl;
            return true;
        }
        else if (stage == 1 || stage == 3)
            return true;
        else if (stage == 2) {
            this->doneCallback = doneCallback;
            return false;
        }
        else
            throw cRuntimeError("Unknown stage");
    }
    else
        return true;
}

void TestMobility::initialize(int stage)
{
    moving = false;
}

void TestMobility::handleMessage(cMessage * message)
{
    if (message == &startMoving) {
        moving = true;
        EV << getFullPath() << " moving started" << endl;
        doneCallback->invoke();
    }
    else if (message == &stopMoving) {
        moving = false;
        EV << getFullPath() << " moving stopped" << endl;
        doneCallback->invoke();
    }
}

}