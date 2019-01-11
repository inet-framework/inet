//
// (C) 2013 Opensim Ltd.
//
// This library is free software, you can redistribute it
// and/or modify
// it under  the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation;
// either version 2 of the License, or any later version.
// The library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// Author: Andras Varga (andras@omnetpp.org)
//

#ifndef __INET_ILIFECYCLE_H
#define __INET_ILIFECYCLE_H

#include "inet/common/INETDefs.h"

namespace inet {

class LifecycleOperation;

/**
 * Callback object used by the ILifecycle interface.
 *
 * @see LifecycleController, ILifecycle
 */
class INET_API IDoneCallback
{
  public:
    virtual ~IDoneCallback() {}
    virtual void invoke() = 0;
};

/**
 * Interface to be implemented by modules that want to support failure/recovery,
 * shutdown/restart, suspend/resume, and similar scenarios.
 *
 * @see LifecycleController
 */
class INET_API ILifecycle
{
  public:
    virtual ~ILifecycle() {}

    /**
     * Perform one stage of a lifecycle operation. Processing may be done
     * entirely within this method, or may be a longer process that involves
     * nonzero simulation time or several events, and is triggered by this
     * method call.
     *
     * Return value: true = "done"; false = "not yet done, will invoke
     * doneCallback when done"
     */
    virtual bool handleOperationStage(LifecycleOperation *operation, IDoneCallback *doneCallback) = 0;
};

} // namespace inet

#endif // ifndef __INET_ILIFECYCLE_H

