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
    class DoneCallback : public IDoneCallback
    {
        OperationalBase *module = nullptr;
        IDoneCallback *orig = nullptr;
        State state = static_cast<State>(-1);
      public:
        DoneCallback(OperationalBase *module) : module(module) { }
        void init(IDoneCallback *newOrig, State newState);
        void done();
        IDoneCallback * getOrig() { return orig; }
        virtual void invoke() override;
    };
    State operational = static_cast<State>(-1);
    simtime_t lastChange;
    DoneCallback *spareCallback = nullptr;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void refreshDisplay() const override;

    virtual void handleMessage(cMessage *msg) override;
    virtual void handleMessageWhenDown(cMessage *msg);
    virtual void handleMessageWhenUp(cMessage *msg) = 0;

    virtual bool handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback) override;
    virtual bool handleStartOperation(IDoneCallback *doneCallback);
    virtual bool handleStopOperation(IDoneCallback *doneCallback);
    virtual void handleCrashOperation();
    virtual bool handleSuspendOperation(IDoneCallback *doneCallback);
    virtual bool handleResumeOperation(IDoneCallback *doneCallback);

    virtual bool isInitializeStage(int stage) = 0;
    virtual bool isModuleStartStage(int stage) = 0;
    virtual bool isModuleStopStage(int stage) = 0;

    virtual bool isWorking() const { return operational != NOT_OPERATING && operational != OPERATION_SUSPENDED; }

    virtual void setOperational(State newState);
    virtual void operationalInvoked(DoneCallback *callback);

    DoneCallback *newDoneCallback(OperationalBase *module);
    void deleteDoneCallback(DoneCallback *callback);

  public:
    OperationalBase();
    ~OperationalBase();
};

} // namespace inet

#endif // ifndef __INET_OPERATIONALBASE_H

