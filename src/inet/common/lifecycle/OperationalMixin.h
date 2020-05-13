//
// Copyright (C) OpenSim Ltd
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

#ifndef __INET_OPERATIONALMIXIN_H
#define __INET_OPERATIONALMIXIN_H

#include "inet/common/lifecycle/ILifecycle.h"

namespace inet {

template <typename T>
class INET_API OperationalMixin : public T, public ILifecycle
{
  protected:
    enum State {
        STARTING_OPERATION, OPERATING, STOPPING_OPERATION, CRASHING_OPERATION, NOT_OPERATING,
        /*SUSPENDING_OPERATION, OPERATION_SUSPENDED, RESUMING_OPERATION */
    };
    State operationalState = NOT_OPERATING;
    simtime_t lastChange;

    class INET_API Operation
    {
      public:
        LifecycleOperation *operation = nullptr;
        IDoneCallback *doneCallback = nullptr;
        State endState = (State)(-1);
        bool isDelayedFinish = false;
        void set(LifecycleOperation *operation_, IDoneCallback *doneCallback_, State endState_) { operation = operation_; doneCallback = doneCallback_; endState = endState_; isDelayedFinish = false; }
        void clear() { operation = nullptr; doneCallback = nullptr; endState = (State)(-1); isDelayedFinish = false; }
        void delayFinish() { isDelayedFinish = true; }
    };
    Operation activeOperation;
    cMessage *activeOperationTimeout = nullptr;
    cMessage *activeOperationExtraTimer = nullptr;

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

    /**
     * Returns initial operational state: OPERATING or NOT_OPERATING.
     */
    virtual State getInitialOperationalState() const;

    virtual bool isInitializeStage(int stage) = 0;
    virtual bool isModuleStartStage(int stage) = 0;
    virtual bool isModuleStopStage(int stage) = 0;

    virtual void handleActiveOperationTimeout(cMessage *message);

    /// @{ utility functions
    virtual bool isUp() const { return operationalState != NOT_OPERATING /* && operationalState != OPERATION_SUSPENDED */; }
    virtual bool isDown() const { return operationalState == NOT_OPERATING /* || operationalState == OPERATION_SUSPENDED */; }
    virtual void setOperationalState(State newState);
    virtual void scheduleOperationTimeout(simtime_t timeout);
    virtual void setupActiveOperation(LifecycleOperation *operation, IDoneCallback *doneCallback, State);

    virtual void delayActiveOperationFinish(simtime_t timeout);
    virtual void startActiveOperationExtraTime(simtime_t delay = SIMTIME_ZERO);
    virtual void startActiveOperationExtraTimeOrFinish(simtime_t extraTime);
    virtual void finishActiveOperation();
    /// }@

  public:
    virtual ~OperationalMixin();
};

} // namespace inet

#endif // ifndef __INET_OPERATIONALMIXIN_H

