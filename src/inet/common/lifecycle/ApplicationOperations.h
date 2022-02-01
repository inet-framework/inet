//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_APPLICATIONOPERATIONS_H
#define __INET_APPLICATIONOPERATIONS_H

#include "inet/common/lifecycle/LifecycleOperation.h"

namespace inet {

/**
 * Base class for lifecycle operations that manipulate a application.
 */
class INET_API ApplicationOperationBase : public LifecycleOperation
{
  public:
    enum Stage { STAGE_LOCAL, STAGE_LAST };

  public:
    ApplicationOperationBase() {}
    virtual void initialize(cModule *module, StringMap& params);
    virtual int getNumStages() const { return STAGE_LAST + 1; }
};

/**
 * Lifecycle operation to start an application.
 */
class INET_API ApplicationStartOperation : public ApplicationOperationBase
{
};

/**
 * Lifecycle operation to stop an application.
 */
class INET_API ApplicationStopOperation : public ApplicationOperationBase
{
};

} // namespace inet

#endif

