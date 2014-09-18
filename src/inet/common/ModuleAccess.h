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
#include "inet/networklayer/common/IInterfaceTable.h"

namespace inet {

/**
 * Returns true if the given module is a network node, i.e. a module
 * with the @node property set.
 */
INET_API bool isNetworkNode(cModule *mod);

/**
 * Find a module with given name, and "closest" to module "from".
 *
 * Operation: gradually rises in the module hierarchy, and searches
 * recursively among all submodules at every level.
 */
INET_API cModule *findModuleWherever(const char *name, cModule *from);

/**
 * Find a module with given name, and "closest" to module "from".
 *
 * Operation: gradually rises in the module hierarchy up to the @node
 * module, and searches recursively among all submodules at every level.
 */
INET_API cModule *findModuleWhereverInNode(const char *name, cModule *from);

/**
 * Find a module with given name, and "closest" to module "from".
 *
 * Operation: gradually rises in the module hierarchy, and looks for a submodule
 * of the given name.
 */
INET_API cModule *findModuleSomewhereUp(const char *name, cModule *from);

/**
 * Find the node containing the given module.
 * Returns NULL, if no containing node.
 */
INET_API cModule *findContainingNode(cModule *from);

/**
 * Find the node containing the given module.
 * throws error if no containing node.
 */
INET_API cModule *getContainingNode(cModule *from);

/**
 * Find the ancestor module under the node containing the given module.
 * Returns NULL, if no such node found.
 */
INET_API cModule *findModuleUnderContainingNode(cModule *from);

/**
 * Finds a module in the module tree, given by its absolute or relative path
 * defined by 'par' parameter.
 * Returns NULL if the 'par' parameter is empty.
 * Returns the pointer to a module of type T or throws an error if module not found
 * or type mismatch.
 */
template<typename T>
INET_API T *findModuleFromPar(cPar& par, cModule *from)
{
    const char *path = par.stringValue();
    if (path && *path) {
        cModule *mod = from->getModuleByPath(path);
        if (!mod)
            throw cRuntimeError("Module not found on path '%s' defined by par '%s'", path, par.getFullPath().c_str());
        T *m = dynamic_cast<T *>(mod);
        if (!m)
            throw cRuntimeError("Module can not cast to '%s' on path '%s' defined by par '%s'", opp_typename(typeid(T)), path, par.getFullPath().c_str());
        return m;
    }
    return NULL;
}

/**
 * Gets a module in the module tree, given by its absolute or relative path
 * defined by 'par' parameter.
 * Returns the pointer to a module of type T or throws an error if module not found
 * or type mismatch.
 */
template<typename T>
INET_API T *getModuleFromPar(cPar& par, cModule *from)
{
    const char *path = par.stringValue();
    cModule *mod = from->getModuleByPath(path);
    if (!mod)
        throw cRuntimeError("Module not found on path '%s' defined by par '%s'", path, par.getFullPath().c_str());
    T *m = dynamic_cast<T *>(mod);
    if (!m)
        throw cRuntimeError("Module can not cast to '%s' on path '%s' defined by par '%s'", opp_typename(typeid(T)), path, par.getFullPath().c_str());
    return m;
}

/**
 * Finds and returns the pointer to a module of type T and name N.
 * Uses findModuleWhereverInNode(). See usage e.g. at IPv4RoutingTableAccess.
 */
template<typename T>
class INET_API ModuleAccess
{
    // Note: MSVC 6.0 doesn't like const char *N as template parameter,
    // so we have to pass it via the ctor...

  private:
    const char *name;
    T *p;

  public:
    ModuleAccess(const char *n) { name = n; p = NULL; }
    virtual ~ModuleAccess() {}

    virtual T *get()
    {
        if (!p)
            p = get(simulation.getContextModule());
        return p;
    }

    virtual T *get(cModule *from)
    {
        T *m = getIfExists(from);
        if (!m) throw cRuntimeError("Module (%s)%s not found", opp_typename(typeid(T)), name);
        return m;
    }

    virtual T *getIfExists()
    {
        if (!p)
            p = getIfExists(simulation.getContextModule());
        return p;
    }

    virtual T *getIfExists(cModule *from)
    {
        if (!from) throw cRuntimeError("Invalid argument: module must not be NULL");
        cModule *m = findModuleWhereverInNode(name, from);
        return dynamic_cast<T *>(m);
    }
};

} // namespace inet

#endif // ifndef __INET_MODULEACCESS_H

