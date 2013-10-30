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
// author: Zoltan Bojthe
//

#ifndef __INET_APPBASE_H
#define __INET_APPBASE_H

#include "INETDefs.h"

#include "ILifecycle.h"
#include "NodeStatus.h"


/**
 * UDP application. See NED for more info.
 */
class INET_API AppBase : public InetSimpleModule, public ILifecycle
{
  protected:
    bool isOperational;

  public:
    AppBase();
    virtual ~AppBase();

  protected:
    virtual void initialize(int stage);
    virtual int numInitStages() const { return NUM_INIT_STAGES; }
    virtual void finish();
    virtual void handleMessage(cMessage *msg);
    virtual void handleMessageWhenUp(cMessage *msg) = 0;
    virtual void handleMessageWhenDown(cMessage *msg);

    virtual bool handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback);
    virtual bool startApp(IDoneCallback *doneCallback) = 0;
    virtual bool stopApp(IDoneCallback *doneCallback) = 0;
    virtual bool crashApp(IDoneCallback *doneCallback) = 0;
};

#endif  // __INET_APPBASE_H

