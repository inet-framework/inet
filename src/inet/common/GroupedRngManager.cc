//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/GroupedRngManager.h"

#if OMNETPP_VERSION >= 0x0604

namespace omnetpp {
extern cConfigOption *CFGID_NUM_RNGS;    // registered in crngmanager.cc
extern cConfigOption *CFGID_RNG_CLASS;   // registered in crngmanager.cc
extern cConfigOption *CFGID_SEED_SET;    // registered in crngmanager.cc
}

namespace inet {

Register_Class(GroupedRngManager);
Register_PerRunConfigOption(CFGID_RNG_GROUPING, "rng-grouping", CFG_STRING, "module",
    "Determines how modules are grouped for RNG assignment when using GroupedRngManager. "
    "'module' = each module gets its own RNG, "
    "'node' = all submodules of a network node share one RNG, "
    "'network' = all modules share a single RNG. "
    "Ignored if a custom key function is installed via setKeyFunction().");

GroupedRngManager::~GroupedRngManager()
{
    for (auto *rng : allRngs)
        delete rng;
}

void GroupedRngManager::configure(cConfiguration *config)
{
    cfg = dynamic_cast<cConfigurationEx *>(config);
    if (!cfg)
        throw cRuntimeError("GroupedRngManager: Configuration object is not a cConfigurationEx");

    // clean up state from previous run
    for (auto *rng : allRngs)
        delete rng;
    allRngs.clear();
    keyToBaseIndex.clear();
    componentBaseIndex.clear();

    rngClass = cfg->getAsString(CFGID_RNG_CLASS);
    if (rngClass.empty())
        rngClass = "omnetpp::cMersenneTwister";

    seedSet = cfg->getAsInt(CFGID_SEED_SET);
    numRngsPerKey = cfg->getAsInt(CFGID_NUM_RNGS);

    grouping = cfg->getAsString(CFGID_RNG_GROUPING);
    if (grouping != "module" && grouping != "node" && grouping != "network")
        throw cRuntimeError("GroupedRngManager: Invalid rng-grouping='%s', must be 'module', 'node', or 'network'", grouping.c_str());

    // run RNG self-test
    cRNG *testRng = createByClassName<cRNG>(rngClass.c_str(), "random number generator");
    testRng->selfTest();
    delete testRng;
}

void GroupedRngManager::configureRngs(cComponent *component)
{
    std::string key = getKeyForComponent(component);

    auto it = keyToBaseIndex.find(key);
    if (it == keyToBaseIndex.end()) {
        // first component with this key — allocate numRngsPerKey RNGs
        int baseIndex = (int)allRngs.size();
        for (int k = 0; k < numRngsPerKey; k++)
            allRngs.push_back(createRng(baseIndex + k));
        it = keyToBaseIndex.emplace(key, baseIndex).first;
    }

    // store base index for this component (O(1) lookup by component ID)
    int baseIndex = it->second;
    int id = component->getId();
    if (id >= (int)componentBaseIndex.size())
        componentBaseIndex.resize(id + 1, -1);
    componentBaseIndex[id] = baseIndex;
}

int GroupedRngManager::getNumRngs(const cComponent *component) const
{
    return numRngsPerKey;
}

static cModule *getContainingNode(cComponent *component)
{
    cModule *mod = dynamic_cast<cModule *>(component);
    if (!mod)
        mod = component->getParentModule();
    while (mod && mod->getParentModule() && mod->getParentModule()->getParentModule())
        mod = mod->getParentModule();
    return mod;
}

std::string GroupedRngManager::getKeyForComponent(cComponent *component)
{
    if (keyFunction)
        return keyFunction(component);
    if (grouping == "node") {
        cModule *node = getContainingNode(component);
        return node ? node->getFullPath() : component->getFullPath();
    }
    if (grouping == "network")
        return getSimulation()->getSystemModule()->getFullPath();
    // default: "module" — each component gets its own RNG set
    return component->getFullPath();
}

cRNG *GroupedRngManager::createRng(int rngId)
{
    cRNG *rng = createByClassName<cRNG>(rngClass.c_str(), "random number generator");
    rng->initialize(seedSet, rngId, numRngsPerKey,
                    getEnvir()->getParsimProcId(),
                    getEnvir()->getParsimNumPartitions(),
                    cfg);
    return rng;
}

cRNG *GroupedRngManager::getRng(const cComponent *component, int k)
{
    // fast path: flat vector lookup by component ID
    int id = component->getId();
    if (id < 0 || id >= (int)componentBaseIndex.size() || componentBaseIndex[id] < 0)
        throw cRuntimeError("GroupedRngManager: No RNG configured for component '%s'", component->getFullPath().c_str());
    int rngId = componentBaseIndex[id] + k;
    if (k < 0 || k >= numRngsPerKey || rngId >= (int)allRngs.size())
        throw cRuntimeError("GroupedRngManager: RNG index %d out of range (num-rngs=%d)", k, numRngsPerKey);
    return allRngs[rngId];
}

uint32_t GroupedRngManager::getHash() const
{
    cHasher hasher;
    for (auto *rng : allRngs)
        hasher << rng->getNumbersDrawn();
    return hasher.getHash();
}

int GroupedRngManager::getTotalNumRngs() const
{
    return (int)allRngs.size();
}

cRNG *GroupedRngManager::getGlobalRng(int rngId)
{
    if (rngId < 0 || rngId >= (int)allRngs.size())
        throw cRuntimeError("GroupedRngManager: Global RNG index %d out of range (total=%d)", rngId, (int)allRngs.size());
    return allRngs[rngId];
}

} // namespace inet

#endif // OMNETPP_VERSION >= 0x0604
