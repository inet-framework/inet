//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "TestProtocol.h"
#include "TestOperation.h"

namespace inet {

Define_Module(TestProtocol);

bool TestProtocol::handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback)
{
    Enter_Method("handleOperationStage");
    if (dynamic_cast<TestNodeStartOperation *>(operation)) {
        if (stage == 0 || stage == 3)
            return true;
        else if (stage == 1) {
            scheduleAfter(3, &sendOpen);
            EV << getFullPath() << " opening connection" << endl;
        }
        else if (stage == 2) {
            scheduleAfter(2, &sendData);
            EV << getFullPath() << " sending initial data" << endl;
        }
        else
            throw cRuntimeError("Unknown stage");
        this->doneCallback = doneCallback;
        return false;
    }
    else if (dynamic_cast<TestNodeShutdownOperation *>(operation)) {
        if (stage == 0) {
            scheduleAfter(2, &sendData);
            EV << getFullPath() << " sending final data" << endl;
        }
        else if (stage == 1) {
            scheduleAfter(3, &sendClose);
            EV << getFullPath() << " closing connection" << endl;
        }
        else if (stage == 2 || stage == 3)
            return true;
        else
            throw cRuntimeError("Unknown stage");
        this->doneCallback = doneCallback;
        return false;
    }
    else
        return true;
}

void TestProtocol::initialize(int stage)
{
    connectionOpen = false;
    dataSent = false;
}

void TestProtocol::handleMessage(cMessage * message)
{
    if (message == &sendOpen) {
        connectionOpen = true;
        EV << getFullPath() << " connection open" << endl;
        doneCallback->invoke();
    }
    else if (message == &sendData) {
        dataSent = true;
        EV << getFullPath() << " data sent" << endl;
        doneCallback->invoke();
    }
    else if (message == &sendClose) {
        connectionOpen = false;
        EV << getFullPath() << " connection closed" << endl;
        doneCallback->invoke();
    }
    else
        throw cRuntimeError("Unknown message");
}

}