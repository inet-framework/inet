//
// Copyright (C) 2013 OpenSim Ltd
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

#ifndef __INET_OPERATIONALBASE_H
#define __INET_OPERATIONALBASE_H

#include "inet/common/lifecycle/ILifecycle.h"

namespace inet {

class INET_API OperationalBase : public cSimpleModule, public ILifecycle
{
  protected:
    enum State { STARTING_OPERATION, OPERATING, STOPPING_OPERATION, CRASHING_OPERATION, NOT_OPERATING, SUSPENDING_OPERATION, OPERATION_SUSPENDED, RESUMING_OPERATION };
    State operational = static_cast<State>(-1);
    simtime_t lastChange;

    class INET_API Operation
    {
      public:
        LifecycleOperation *operation = nullptr;
        IDoneCallback *doneCallback = nullptr;
        State endOperation = (State)(-1);
        bool isPending = false;
        bool isExtraPending = false;
        void set(LifecycleOperation *operation_, IDoneCallback *doneCallback_, State endOperation_) { operation = operation_; doneCallback = doneCallback_; endOperation = endOperation_; isPending = false; isExtraPending = false; }
        void clear() { operation = nullptr; doneCallback = nullptr; endOperation = (State)(-1); isPending = false; isExtraPending = false; }
        void pending() { isPending = true; }
        void extraPending() { isPending = true; isExtraPending = true; }
    };
    Operation activeOperation;
    cMessage *operationTimeoutMsg = nullptr;
    cMessage *delayedOperationDoneMsg = nullptr;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void refreshDisplay() const override;

    virtual void handleMessage(cMessage *msg) override;
    virtual void handleMessageWhenDown(cMessage *msg);
    virtual void handleMessageWhenUp(cMessage *msg) = 0;

    virtual bool handleOperationStage(LifecycleOperation *operation, IDoneCallback *doneCallback) override;
    virtual void handleStartOperation(LifecycleOperation *operation) = 0;
    virtual void handleStopOperation(LifecycleOperation *operation) = 0;
    virtual void handleCrashOperation(LifecycleOperation *operation) = 0;
    virtual void handleSuspendOperation(LifecycleOperation *operation);
    virtual void handleResumeOperation(LifecycleOperation *operation);
    virtual bool isOperationFinished();

    virtual bool isInitializeStage(int stage) = 0;
    virtual bool isModuleStartStage(int stage) = 0;
    virtual bool isModuleStopStage(int stage) = 0;

    virtual void handleOperationTimeout(cMessage *message);

    /// @{ utility functions
    virtual bool isWorking() const { return operational != NOT_OPERATING && operational != OPERATION_SUSPENDED; }
    virtual void setOperational(State newState);
    virtual void scheduleOperationTimeout(simtime_t timeout);
    virtual bool checkOperationFinished();
    virtual bool isOperationTimeout(cMessage *message);
    virtual void operationStarted(LifecycleOperation *operation, IDoneCallback *doneCallback, State);
    virtual void operationPending();
    virtual void operationCompleted();
    virtual void delayDoneCallbackInvocation(simtime_t delay = SIMTIME_ZERO);
    /// }@

  public:
    OperationalBase();
    ~OperationalBase();
};

} // namespace inet

#endif // ifndef __INET_OPERATIONALBASE_H

