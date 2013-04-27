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
// author: Levente Meszaros (levy@omnetpp.org)
//

#include "TestProtocol.h"
#include "TestOperation.h"

Define_Module(TestProtocol);

bool TestProtocol::initiateStateChange(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback)
{
    Enter_Method_Silent();
    if (dynamic_cast<TestNodeStartOperation *>(operation)) {
        if (stage == 0 || stage == 3)
            return true;
        else if (stage == 1) {
            scheduleAt(simTime() + 3, &sendOpen);
            EV << getFullPath() << " opening connection" << endl;
        }
        else if (stage == 2) {
            scheduleAt(simTime() + 2, &sendData);
            EV << getFullPath() << " sending initial data" << endl;
        }
        else
            throw cRuntimeError("Unknown stage");
        this->doneCallback = doneCallback;
        return false;
    }
    else if (dynamic_cast<TestNodeShutdownOperation *>(operation)) {
        if (stage == 0) {
            scheduleAt(simTime() + 2, &sendData);
            EV << getFullPath() << " sending final data" << endl;
        }
        else if (stage == 1) {
            scheduleAt(simTime() + 3, &sendClose);
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
