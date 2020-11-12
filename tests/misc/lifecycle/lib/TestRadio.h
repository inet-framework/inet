//
// Copyright (C) 2013 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#ifndef __INET_TESTRADIO_H_
#define __INET_TESTRADIO_H_


#include "inet/common/lifecycle/ILifecycle.h"

namespace inet {

class INET_API TestRadio : public cSimpleModule, public ILifecycle {
  private:
    bool receiverTurnedOn;
    bool transmitterTurnedOn;
    cMessage turnOnReceiver;
    cMessage turnOffReceiver;
    cMessage turnOnTransmitter;
    cMessage turnOffTransmitter;
    IDoneCallback * doneCallback;

  public:
    TestRadio() { }
    virtual bool handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback);

  protected:
    virtual void initialize(int stage);
    virtual void handleMessage(cMessage * message);
};

}

#endif
