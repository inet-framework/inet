//
// Copyright (C) 2004 Andras Varga
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

#ifndef __INET_TCPMULTITHREADAPP_H
#define __INET_TCPMULTITHREADAPP_H

#include "INETDefs.h"

#include "ILifecycle.h"
#include "LifecycleOperation.h"
#include "TCPThread.h"
#include "TCPThreadMap.h"

/**
 * Hosts a server application, to be subclassed from TCPThread (which
 * is a cSimpleModule). Creates one instance (using dynamic module creation)
 * for each incoming connection. More info in the corresponding NED file.
 */
class INET_API TCPMultithreadApp : public cSimpleModule, public ILifecycle, public ITCPMultithreadApp
{
  protected:
    ITCPAppThread *mainThread;
    TCPThreadMap threadMap;

    virtual void initialize(int stage);
    virtual int numInitStages() const { return NUM_INIT_STAGES; }
    virtual void handleMessage(cMessage *msg);
    virtual void finish();
    virtual void updateDisplay();

    virtual bool handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback)
    { Enter_Method_Silent(); throw cRuntimeError("Unsupported lifecycle operation '%s'", operation->getClassName()); return true; }

  public:
    virtual void addThread(ITCPAppThread *thread);
    virtual void removeThread(ITCPAppThread *thread);
};

#endif

