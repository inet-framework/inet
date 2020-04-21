//
// Copyright (C) 2004 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __INET_MODULEACCESS_H
#define __INET_MODULEACCESS_H

#include "inet/common/INETDefs.h"
#include "inet/networklayer/contract/IInterfaceTable.h"

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
 * Finds a module in the module tree, given by its absolute or relative path
 * defined by 'par' parameter.
 * Returns nullptr if the 'par' parameter is empty.
 * Returns the pointer to a module of type T or throws an error if module not found
 * or type mismatch.
 */
template<typename T>
INET_API T *findModuleFromPar(cPar& par, const cModule *from);

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
INET_API T *getModuleFromPar(cPar& par, const cModule *from);

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
 * Find the nic module (inside the networkNode) containing the given module.
 * Returns nullptr, if no containing nic module.
 */
INET_API InterfaceEntry *findContainingNicModule(const cModule *from);

/**
 * Find the nic module (inside the networkNode) containing the given module.
 * throws error if no containing nic module.
 */
INET_API InterfaceEntry *getContainingNicModule(const cModule *from);

/**
 * Returns a gate of a module with type T that is on the path starting at the given gate.
 * Returns nullptr if no such module is found along the path.
 */
template<typename T>
cGate *findConnectedGate(cGate *gate, int direction = 0)
{
    if (direction < 0 || (direction == 0 && gate->getType() == cGate::INPUT)) {
        auto g = gate;
        while (g != nullptr) {
            auto gateType = g->getType();
            auto previousGate = g->getPreviousGate();
            if (dynamic_cast<T *>(g->getOwnerModule()) &&
                ((gateType == cGate::OUTPUT && previousGate == nullptr) ||
                 (gateType == cGate::INPUT && g != gate && previousGate != nullptr)))
                break;
            else
                g = previousGate;
        }
        return g;
    }
    else if (direction > 0 || (direction == 0 && gate->getType() == cGate::OUTPUT)) {
        auto g = gate;
        while (g != nullptr) {
            auto gateType = g->getType();
            auto nextGate = g->getNextGate();
            if (dynamic_cast<T *>(g->getOwnerModule()) &&
                ((gateType == cGate::INPUT && nextGate == nullptr) ||
                 (gateType == cGate::OUTPUT && g != gate && nextGate != nullptr)))
                break;
            else
                g = nextGate;
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

#endif // ifndef __INET_MODULEACCESS_H

