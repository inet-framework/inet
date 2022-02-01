//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_TESTMOBILITY_H_
#define __INET_TESTMOBILITY_H_

#include "inet/common/lifecycle/ILifecycle.h"

namespace inet {

class INET_API TestMobility : public cSimpleModule, public ILifecycle {
  private:
    bool moving;
    cMessage stopMoving;
    cMessage startMoving;
    IDoneCallback * doneCallback;

  public:
    TestMobility() { }
    virtual bool handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback);
  protected:
    virtual void initialize(int stage);
    virtual void handleMessage(cMessage * message);
};
}

#endif
