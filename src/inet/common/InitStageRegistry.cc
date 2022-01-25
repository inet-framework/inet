//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
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

