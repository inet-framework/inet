//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_SIMULATIONTASK_H
#define __INET_SIMULATIONTASK_H

#include "inet/common/SimulationContinuation.h"

namespace inet {

/**
 * This class supports spawning child simulation tasks that run concurrently in simulation time.
 */
class INET_API SimulationTask
{
  protected:
    bool isFinished = false;
    SimulationTask *parent = nullptr;
    std::vector<SimulationTask *> children;
    SimulationContinuation *joinChildrenContinuation = nullptr;

  protected:
    virtual bool isChildrenFinished();

  public:
    virtual ~SimulationTask();

    virtual void spawnChild(std::function<void()> f);
    virtual void joinChildren();
};

} // namespace inet

#endif

