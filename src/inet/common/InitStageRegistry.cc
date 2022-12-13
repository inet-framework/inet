//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/InitStageRegistry.h"

#include <algorithm>

namespace inet {

InitStageRegistry::~InitStageRegistry()
{
    for (auto stage : stages)
        delete stage;
}

InitStageRegistry::Stage *InitStageRegistry::getInitStage(const char *name)
{
    for (auto stage : stages)
        if (!strcmp(stage->stageDecl->getName(), name))
            return stage;
    throw cRuntimeError("Cannot find initialization stage: %s", name);
}

void InitStageRegistry::addInitStage(const InitStage *initStage)
{
    stages.push_back(new Stage{initStage});
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

int InitStageRegistry::getNumber(const InitStage *initStage)
{
    ensureInitStageNumbersAssigned();
    for (auto stage : stages)
        if (stage->stageDecl == initStage)
            return stage->number;
    throw cRuntimeError("Cannot find initialization stage: %s", initStage->getName());
}

void InitStageRegistry::ensureInitStageNumbersAssigned()
{
    if (numInitStages == -1)
        assignInitStageNumbers();
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
    std::sort(stages.begin(), stages.end(), [] (const Stage *s1, const Stage *s2) -> bool {
        return s1->number < s2->number;
    });
    EV_STATICCONTEXT;
    for (auto stage : stages)
        EV_DEBUG << "Initialization stage: " << stage->stageDecl->getName() << " = " << stage->number << std::endl;
    EV_DEBUG << "Total number of initialization stages: " << numInitStages << std::endl;
}

InitStageRegistry& InitStageRegistry::getInstance()
{
    static int handle = cSimulationOrSharedDataManager::registerSharedVariableName("inet::InitStageRegistry::instance");
    return getSimulationOrSharedDataManager()->getSharedVariable<InitStageRegistry>(handle);
}

} // namespace inet

