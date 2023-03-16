//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_MODULEACCESS_H
#define __INET_MODULEACCESS_H

#include "inet/common/INETDefs.h"

namespace inet {

/**
 * Returns true if the given module is a network node, i.e. a module
 * with the @networkNode property set.
 */
INET_API bool isNetworkNode(const cModule *mod);

/**
 * Find a module with given name, and "closest" to module "from".
 *
 * Operation: gradually rises in the module hierarchy, and looks for a submodule
 * of the given name.
 */
INET_API cModule *findModuleSomewhereUp(const char *name, const cModule *from);

/**
 * Find the node containing the given module.
 * Returns nullptr, if no containing node.
 */
INET_API cModule *findContainingNode(const cModule *from);

/**
 * Find the node containing the given module.
 * throws error if no containing node.
 */
INET_API cModule *getContainingNode(const cModule *from);

/**
 * Find the ancestor module under the node containing the given module.
 * Returns nullptr, if no such node found.
 */
INET_API cModule *findModuleUnderContainingNode(const cModule *from);

/**
 * Gets a module in the module tree, by gradually rising in the module hierarchy,
 * and looking at a submodule of a given name.
 *
 * Returns the pointer to a module of type T or throws an error if module not found
 * or type mismatch.
 */
template <typename T>
T *getModuleSomewhereUp(const char *name, const cModule *from)
{
    cModule *mod = findModuleSomewhereUp(name, from);
    if (!mod)
        throw cRuntimeError("Module '%s' not found up from module '%s'", name, from->getFullPath().c_str());
    T *m = dynamic_cast<T *>(mod);
    if (!m)
        throw cRuntimeError("Module '%s' can not cast to '%s'", mod->getFullPath().c_str(), opp_typename(typeid(T)));
    return m;
}

/**
 * Finds a module in the module tree, given by its absolute or relative path
 * defined by 'par' parameter.
 * Returns nullptr if the 'par' parameter is empty.
 * Returns the pointer to a module of type T or throws an error if module not found
 * or type mismatch.
 */
template<typename T>
T *findModuleFromPar(cPar& par, const cModule *from)
{
    const char *path = par;
    if (path && *path) {
        cModule *mod = from->findModuleByPath(path);
        if (!mod) {
            return nullptr;
        }
        T *m = dynamic_cast<T *>(mod);
        if (!m)
            throw cRuntimeError("Module can not cast to '%s' on path '%s' defined by par '%s'", opp_typename(typeid(T)), path, par.getFullPath().c_str());
        return m;
    }
    return nullptr;
}

/**
 * Gets a module in the module tree, given by its absolute or relative path
 * defined by 'par' parameter.
 * Returns the pointer to a module of type T or throws an error if module not found
 * or type mismatch.
 */
template<typename T>
T *getModuleFromPar(cPar& par, const cModule *from)
{
    const char *path = par;
    cModule *mod = from->findModuleByPath(path);
    if (!mod) {
        throw cRuntimeError("Module not found on path '%s' defined by par '%s'", path, par.getFullPath().c_str());
    }
    T *m = dynamic_cast<T *>(mod);
    if (!m)
        throw cRuntimeError("Module can not cast to '%s' on path '%s' defined by par '%s'", opp_typename(typeid(T)), path, par.getFullPath().c_str());
    return m;
}

/**
 * Returns a gate of a module with type T that is on the path starting at the given gate.
 * Returns nullptr if no such module is found along the path.
 */
template<typename T>
cGate *findConnectedGate(cGate *gate, int direction = 0)
{
    if (direction < 0 || (direction == 0 && gate->getType() == cGate::INPUT)) {
        auto g = gate->getPreviousGate();
        while (g != nullptr) {
            if (dynamic_cast<T *>(g->getOwnerModule()))
                break;
            g = g->getPreviousGate();
        }
        return g;
    }
    else if (direction > 0 || (direction == 0 && gate->getType() == cGate::OUTPUT)) {
        auto g = gate->getNextGate();
        while (g != nullptr) {
            if (dynamic_cast<T *>(g->getOwnerModule()))
                break;
            g = g->getNextGate();
        }
        return g;
    }
    else
        throw cRuntimeError("Unknown gate type");
}

/**
 * Returns a gate of a module with type T that is on the path starting at the given gate.
 * Throws an error if no such module is found along the path.
 */
template<typename T>
cGate *getConnectedGate(cGate *gate, int direction = 0)
{
    auto connectedGate = findConnectedGate<T>(gate, direction);
    if (connectedGate == nullptr)
        throw cRuntimeError("Gate %s is not connected to a module of type %s", gate->getFullPath().c_str(), opp_typename(typeid(T)));
    return connectedGate;
}

/**
 * Returns a module of type T that is on the path starting at the given gate.
 * Returns nullptr if no such module is found along the path.
 */
template<typename T>
T *findConnectedModule(cGate *gate, int direction = 0)
{
    auto connectedGate = findConnectedGate<T>(gate, direction);
    return connectedGate != nullptr ? check_and_cast<T *>(connectedGate->getOwnerModule()) : nullptr;
}

/**
 * Returns a module of type T that is on the path starting at the given gate.
 * Throws an error if no such module is found along the path.
 */
template<typename T>
T *getConnectedModule(cGate *gate, int direction = 0)
{
    auto module = findConnectedModule<T>(gate, direction);
    if (module == nullptr)
        throw cRuntimeError("Gate %s is not connected to a module of type %s", gate->getFullPath().c_str(), opp_typename(typeid(T)));
    return module;
}

} // namespace inet

#endif

