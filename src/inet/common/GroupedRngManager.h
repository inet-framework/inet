//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_GROUPEDRNGMANAGER_H
#define __INET_GROUPEDRNGMANAGER_H

#include <map>
#include <vector>
#include <functional>
#include "inet/common/INETDefs.h"

#if OMNETPP_VERSION >= 0x0604

namespace inet {

/**
 * An RNG manager that assigns RNGs to components based on a string key.
 * A user-supplied key function maps each component to a string key.
 * Components that map to the same key share the same set of RNGs,
 * giving deterministic, isolated random number streams per module,
 * per network node, or any other grouping the key function defines.
 *
 * Usage in omnetpp.ini:
 *   rngmanager-class = "inet::GroupedRngManager"
 *
 * The grouping mode is selected via the "rng-grouping" per-run config option:
 *   - "module"  — each module gets its own RNG (default)
 *   - "node"    — all submodules of a network node share one RNG
 *   - "network" — all modules share a single RNG
 *
 * Alternatively, a custom key function can be installed programmatically
 * via setKeyFunction(), which overrides the rng-grouping option.
 */
class INET_API GroupedRngManager : public cIRngManager
{
  public:
    using KeyFunction = std::function<std::string(cComponent *)>;

  private:
    cConfigurationEx *cfg = nullptr;
    std::string rngClass;
    int seedSet = 0;
    int numRngsPerKey = 1;

    KeyFunction keyFunction;
    std::string grouping = "module";

    // key -> base index into allRngs (RNGs for key are at baseIndex..baseIndex+numRngsPerKey-1)
    std::map<std::string, int> keyToBaseIndex;
    // component ID -> base index into allRngs (O(1) lookup for getRNG)
    std::vector<int> componentBaseIndex;
    std::vector<cRNG *> allRngs;

  protected:
    virtual cRNG *createRng(int rngId);
    virtual std::string getKeyForComponent(cComponent *component);

  public:
    GroupedRngManager() {}
    virtual ~GroupedRngManager();

    /**
     * Sets the key function that maps a component to a string key.
     * Components with the same key share the same RNG(s).
     */
    virtual void setKeyFunction(KeyFunction f) { keyFunction = f; }

    /**
     * Returns the currently installed key function.
     */
    virtual KeyFunction getKeyFunction() const { return keyFunction; }

    /** @name cIRngManager interface. */
    //@{
    virtual void configure(cConfiguration *cfg) override;
    virtual void configureRngs(cComponent *component) override;
    virtual int getNumRngs(const cComponent *component) const override;
    virtual cRNG *getRng(const cComponent *component, int k) override;
    virtual int getTotalNumRngs() const override;
    virtual cRNG *getGlobalRng(int rngId) override;
    virtual uint32_t getHash() const override;
    //@}
};

} // namespace inet

#endif // OMNETPP_VERSION >= 0x0604

#endif
