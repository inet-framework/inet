//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_MODULEOPERATIONS_H
#define __INET_MODULEOPERATIONS_H

#include "inet/common/lifecycle/LifecycleOperation.h"

namespace inet {

/**
 * This operation represents the process of turning on a module
 * after a stop or crash operation.
 *
 * The operation should be applied to the module of a network node. Operation
 * stages are organized bottom-up similarly to the OSI network layers.
 */
class INET_API ModuleStartOperation : public LifecycleOperation
{
  public:
    enum Stage {
        STAGE_LOCAL, // for changes that don't depend on other modules
        STAGE_PHYSICAL_LAYER,
        STAGE_LINK_LAYER,
        STAGE_NETWORK_LAYER,
        STAGE_TRANSPORT_LAYER,
        STAGE_ROUTING_PROTOCOLS,
        STAGE_APPLICATION_LAYER,
        STAGE_LAST
    };

  public:
    virtual int getNumStages() const override { return STAGE_LAST + 1; }
};

/**
 * This operation represents the process of orderly stopping down a module.
 *
 * Operation stages are organized top-down similarly to the OSI network layers.
 */
class INET_API ModuleStopOperation : public LifecycleOperation
{
  public:
    enum Stage {
        STAGE_LOCAL, // for changes that don't depend on other modules
        STAGE_APPLICATION_LAYER,
        STAGE_ROUTING_PROTOCOLS,
        STAGE_TRANSPORT_LAYER,
        STAGE_NETWORK_LAYER,
        STAGE_LINK_LAYER,
        STAGE_PHYSICAL_LAYER,
        STAGE_LAST // for changes that others shouldn't depend on
    };

  public:
    virtual int getNumStages() const override { return STAGE_LAST + 1; }
};

/**
 * This operation represents the process of crashing a module. The
 * difference between this operation and ShutdownOperation is that the
 * module will not do a graceful shutdown (e.g. routing protocols will
 * not have chance of notifying peers about broken routes).
 *
 * The operation has only one stage, and the execution must finish in zero
 * simulation time.
 */
class INET_API ModuleCrashOperation : public LifecycleOperation
{
  public:
    enum Stage {
        STAGE_CRASH, // the only stage, must execute within zero simulation time
        STAGE_LAST
    };

  public:
    virtual int getNumStages() const override { return STAGE_LAST + 1; }
};

} // namespace inet

#endif

