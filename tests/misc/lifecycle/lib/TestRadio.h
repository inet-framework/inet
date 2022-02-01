//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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
