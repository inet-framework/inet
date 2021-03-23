//
// Copyright (C) 2013 OpenSim Ltd.
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

#ifndef __INET_INITSTAGEREGISTRY_H
#define __INET_INITSTAGEREGISTRY_H

#include "inet/common/INETDefs.h"

namespace inet {

class INET_API InitStage
{
  public:
    int *number;
    const char *name;
    std::vector<InitStage *> precedingStages;
    std::vector<InitStage *> followingStages;

  public:
    InitStage(int *number, const char *name) : number(number), name(name) {}
};

class INET_API InitStageRegistry
{
  protected:
    int numInitStages = -1;
    std::vector<InitStage> stages;
    std::vector<std::pair<const char *, const char *>> dependencies;

  protected:
    InitStage* getInitStage(const char *name);

    void assignInitStageNumbers();

  public:
    void addInitStage(const InitStage &initStage);

    void addInitStageDependency(const char *source, const char *target);

    int getNumInitStages();
};

extern INET_API InitStageRegistry globalInitStageRegistry;

#define Define_InitStage(name) int INITSTAGE_ ## name = -1; EXECUTE_ON_STARTUP(globalInitStageRegistry.addInitStage(InitStage(&INITSTAGE_ ## name, #name)))

#define Define_InitStage_Dependency(source, target) EXECUTE_ON_STARTUP(globalInitStageRegistry.addInitStageDependency(#source, #target))

#define NUM_INIT_STAGES globalInitStageRegistry.getNumInitStages()

} // namespace inet

#endif

