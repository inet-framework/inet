//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/FunctionalEvent.h"
#include "inet/common/SimulationTask.h"

namespace inet {

SimulationTask::~SimulationTask()
{
    delete joinChildrenContinuation;
}

bool SimulationTask::isChildrenFinished()
{
    return std::all_of(children.begin(), children.end(), [] (SimulationTask *task) { return task->isFinished; });
}

void SimulationTask::spawnChild(std::function<void()> f)
{
    SimulationTask *childTask = new SimulationTask();
    children.push_back(childTask);
    inet::scheduleAfter("TaskStartEvent", 0, [=] () {
        f();
        childTask->isFinished = true;
        if (isChildrenFinished() && joinChildrenContinuation != nullptr)
            joinChildrenContinuation->resume();
    });
}

void SimulationTask::joinChildren()
{
    if (!isChildrenFinished()) {
        joinChildrenContinuation = new SimulationContinuation();
        joinChildrenContinuation->suspend();
    }
    for (auto childTask : children)
        delete childTask;
    children.clear();
}

} // namespace inet

