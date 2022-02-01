//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/InitStageRegistry.h"

#include <algorithm>

namespace inet {

InitStageRegistry globalInitStageRegistry;

inet::InitStage* InitStageRegistry::getInitStage(const char *name)
{
    for (auto stage : stages)
        if (!strcmp(stage->name, name))
            return stage;
    throw cRuntimeError("Cannot find initialization stage: %s", name);
}

void InitStageRegistry::addInitStage(InitStage &initStage)
{
    stages.push_back(&initStage);
    numInitStages = -1;
}

void InitStageRegistry::addInitStageDependency(const char *source, const char *target)
{
    dependencies.push_back({source, target});
    numInitStages = -1;
}

int InitStageRegistry::getNumInitStages()
{
    ensureInitStageNumbersAssigned();
    return numInitStages;
}

void InitStageRegistry::assignInitStageNumbers()
{
    EV_DEBUG << "Assigning initialization stage numbers" << EV_ENDL;
    for (auto stage : stages) {
        stage->number = -1;
        stage->followingStages.clear();
        stage->precedingStages.clear();
    }
    for (auto &dependency : dependencies) {
        auto following = getInitStage(dependency.first);
        auto preceding = getInitStage(dependency.second);
        preceding->followingStages.push_back(following);
        following->precedingStages.push_back(preceding);
    }
    numInitStages = 0;
    while (numInitStages < stages.size()) {
        bool assigned = false;
        for (auto stage : stages) {
            if (stage->number == -1) {
                for (auto precedingStage : stage->precedingStages)
                    if (precedingStage->number == -1)
                        goto next;
                stage->number = numInitStages++;
                assigned = true;
            }
            next:;
        }
        if (!assigned)
            throw cRuntimeError("Circle detected in initialization stage dependency graph");
    }
    std::sort(stages.begin(), stages.end(), [] (const InitStage *s1, const InitStage *s2) -> bool {
        return s1->number < s2->number;
    });
    EV_STATICCONTEXT;
    for (auto stage : stages)
        EV_DEBUG << "Initialization stage: " << stage->name << " = " << stage->number << std::endl;
    EV_DEBUG << "Total number of initialization stages: " << numInitStages << std::endl;
}

} // namespace inet

