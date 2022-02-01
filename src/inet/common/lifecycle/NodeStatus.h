//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_NODESTATUS_H
#define __INET_NODESTATUS_H

#include "inet/common/lifecycle/ILifecycle.h"

namespace inet {

/**
 * Keeps track of the status of network node (up, down, etc.) for other
 * modules, and also displays it as a small overlay icon on this module
 * and on the module of the network node.
 *
 * Other modules can obtain the network node's status by calling the
 * getState() method.
 *
 * See NED file for more information.
 */
class INET_API NodeStatus : public cSimpleModule, public ILifecycle
{
  public:
    enum State { UP, DOWN, GOING_UP, GOING_DOWN };
    static simsignal_t nodeStatusChangedSignal; // the signal used to notify subscribers about status changes

  private:
    State state;

  public:
    virtual State getState() const { return state; }

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override { throw cRuntimeError("This module doesn't handle messages"); }
    virtual bool handleOperationStage(LifecycleOperation *operation, IDoneCallback *doneCallback) override;
    virtual void setState(State state);
    virtual void refreshDisplay() const override;
    static State getStateByName(const char *name);
};

} // namespace inet

#endif

