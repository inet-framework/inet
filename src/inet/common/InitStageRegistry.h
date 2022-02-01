//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_INITSTAGEREGISTRY_H
#define __INET_INITSTAGEREGISTRY_H

// KLUDGE: avoid including InitStages.h from INETDefs.h
#define __INET_INITSTAGES_H

#include "inet/common/INETDefs.h"

namespace inet {

class InitStage;

class INET_API InitStageRegistry
{
  protected:
    int numInitStages = -1;
    std::vector<InitStage *> stages;
    std::vector<std::pair<const char *, const char *>> dependencies;

  protected:
    InitStage* getInitStage(const char *name);

    void assignInitStageNumbers();

  public:
    void addInitStage(InitStage &initStage);

    void addInitStageDependency(const char *source, const char *target);

    int getNumInitStages();

    void ensureInitStageNumbersAssigned() {
        if (numInitStages == -1)
            assignInitStageNumbers();
    }
};

extern INET_API InitStageRegistry globalInitStageRegistry;

/**
 * This class provides constants for initialization stages for modules overriding
 * cComponent::initialize(int stage). The numbers are assigned lazily on the first
 * access to any initialization stage.
 */
class INET_API InitStage
{
  public:
    int number = -1;
    const char *name = nullptr;
    std::vector<InitStage *> precedingStages;
    std::vector<InitStage *> followingStages;

  public:
    InitStage(const char *name) : name(name) {}

    operator int() const {
        globalInitStageRegistry.ensureInitStageNumbersAssigned();
        return number;
    }
};

#define Define_InitStage(name) InitStage INITSTAGE_##name(#name); EXECUTE_ON_STARTUP(globalInitStageRegistry.addInitStage(INITSTAGE_##name))

#define Define_InitStage_Dependency(source, target) EXECUTE_ON_STARTUP(globalInitStageRegistry.addInitStageDependency(#source, #target))

#define NUM_INIT_STAGES globalInitStageRegistry.getNumInitStages()

} // namespace inet

#endif

