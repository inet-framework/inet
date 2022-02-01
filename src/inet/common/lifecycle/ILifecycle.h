//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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

#endif

