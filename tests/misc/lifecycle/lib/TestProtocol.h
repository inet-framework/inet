//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_TESTPROTOCOL_H_
#define __INET_TESTPROTOCOL_H_


#include "inet/common/lifecycle/ILifecycle.h"

namespace inet {

class INET_API TestProtocol : public cSimpleModule, public ILifecycle {
  private:
    bool connectionOpen;
    bool dataSent;
    cMessage sendOpen;
    cMessage sendClose;
    cMessage sendData;
    IDoneCallback * doneCallback;

  public:
    TestProtocol() { }
    virtual bool handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback);

  protected:
    virtual void initialize(int stage);
    virtual void handleMessage(cMessage * message);
};

}

#endif
