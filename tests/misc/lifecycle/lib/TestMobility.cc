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

#include "TestMobility.h"
#include "TestOperation.h"

Define_Module(TestMobility);

bool TestMobility::initiateStateChange(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback)
{
    Enter_Method_Silent();
    if (dynamic_cast<TestNodeStartOperation *>(operation)) {
        if (stage == 0) {
            scheduleAt(simTime() + 9, &startMoving);
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
            scheduleAt(simTime() + 9, &stopMoving);
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
