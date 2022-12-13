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
    struct Stage {
        const InitStage *stageDecl;
        int number = -1;
        std::vector<Stage *> precedingStages;
        std::vector<Stage *> followingStages;
    };

    int numInitStages = -1;
    std::vector<Stage *> stages;
    std::vector<std::pair<const char *, const char *>> dependencies;

  protected:
    Stage *getInitStage(const char *name);
    void assignInitStageNumbers();
    void ensureInitStageNumbersAssigned();

  public:
    InitStageRegistry() {}
    ~InitStageRegistry();

    void addInitStage(const InitStage *initStage);
    void addInitStageDependency(const char *source, const char *target);

    int getNumInitStages();
    int getNumber(const InitStage *initStage);

    static InitStageRegistry& getInstance();
};

/**
 * This class provides constants for initialization stages for modules overriding
 * cComponent::initialize(int stage). The numbers are assigned lazily on the first
 * access to any initialization stage.
 */
class INET_API InitStage
{
  public:
    const char *name = nullptr;
  public:
    InitStage(const char *name) : name(name) {}
    const char *getName() const { return name; }
    operator int() const { return InitStageRegistry::getInstance().getNumber(this); }
};

#define Define_InitStage(name) const InitStage INITSTAGE_##name(#name); EXECUTE_PRE_NETWORK_SETUP(InitStageRegistry::getInstance().addInitStage(&INITSTAGE_##name))

#define Define_InitStage_Dependency(source, target) EXECUTE_PRE_NETWORK_SETUP(InitStageRegistry::getInstance().addInitStageDependency(#source, #target))

#define NUM_INIT_STAGES InitStageRegistry::getInstance().getNumInitStages()

} // namespace inet

#endif

