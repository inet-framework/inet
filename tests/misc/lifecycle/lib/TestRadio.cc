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

#include "TestRadio.h"
#include "TestOperation.h"

Define_Module(TestRadio);

bool TestRadio::initiateStateChange(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback)
{
    Enter_Method_Silent();
    if (dynamic_cast<TestNodeStartOperation *>(operation)) {
        if (stage == 0) {
            scheduleAt(simTime() + 1, &turnOnTransmitter);
            EV << getFullPath() << " turning on transmitter" << endl;
        }
        else if (stage == 1) {
            scheduleAt(simTime() + 2, &turnOnReceiver);
            EV << getFullPath() << " turning on receiver" << endl;
        }
        else if (stage == 2 || stage == 3)
            return true;
        else
            throw cRuntimeError("Unknown stage");
        this->doneCallback = doneCallback;
        return false;
    }
    else if (dynamic_cast<TestNodeShutdownOperation *>(operation)) {
        if (stage == 0 || stage == 3)
            return true;
        else if (stage == 1) {
            scheduleAt(simTime() + 2, &turnOffReceiver);
            EV << getFullPath() << " turning off receiver" << endl;
        }
        else if (stage == 2) {
            scheduleAt(simTime() + 1, &turnOffTransmitter);
            EV << getFullPath() << " turning off transmitter" << endl;
        }
        else
            throw cRuntimeError("Unknown stage");
        this->doneCallback = doneCallback;
        return false;
    }
    else
        return true;
}

void TestRadio::initialize(int stage)
{
    receiverTurnedOn = false;
    transmitterTurnedOn = false;
}

void TestRadio::handleMessage(cMessage * message)
{
    if (message == &turnOnTransmitter) {
        transmitterTurnedOn = true;
        EV << getFullPath() << " transmitter turned on" << endl;
        doneCallback->invoke();
    }
    else if (message == &turnOnReceiver) {
        receiverTurnedOn = true;
        EV << getFullPath() << " receiver turned on" << endl;
        doneCallback->invoke();
    }
    else if (message == &turnOffTransmitter) {
        transmitterTurnedOn = false;
        EV << getFullPath() << " transmitter turned off" << endl;
        doneCallback->invoke();
    }
    else if (message == &turnOffReceiver) {
        receiverTurnedOn = false;
        EV << getFullPath() << " receiver turned off" << endl;
        doneCallback->invoke();
    }
    else
        throw cRuntimeError("Unknown message");
}
