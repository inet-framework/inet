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
    bool isOperational;
    simtime_t lastChange;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;

    virtual void handleMessage(cMessage *msg) override;
    virtual void handleMessageWhenDown(cMessage *msg);
    virtual void handleMessageWhenUp(cMessage *msg) = 0;

    virtual bool handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback) override;
    virtual bool handleNodeStart(IDoneCallback *doneCallback);
    virtual bool handleNodeShutdown(IDoneCallback *doneCallback);
    virtual void handleNodeCrash();

    virtual bool isInitializeStage(int stage) = 0;
    virtual bool isNodeStartStage(int stage) = 0;
    virtual bool isNodeShutdownStage(int stage) = 0;

    virtual void setOperational(bool isOperational);

  public:
    OperationalBase();
};

} // namespace inet

#endif // ifndef __INET_OPERATIONALBASE_H

