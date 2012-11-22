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

#include "INETDefs.h"


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
 * Checks if the module is node, i.e. it has a @node attribute.
 */
bool isNode(cModule *mod);

/**
 * Find the node containing the given module.
 * Returns NULL, if no containing node.
 */
INET_API cModule *findContainingNode(cModule *from);


/**
 * Finds and returns the pointer to a module of type T and name N.
 * Uses findModuleWhereverInNode(). See usage e.g. at RoutingTableAccess.
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
    ModuleAccess(const char *n) {name = n; p = NULL;}
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
        if (!m) opp_error("Module (%s)%s not found", opp_typename(typeid(T)), name);
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
        if (!from) opp_error("Invalid argument: module must not be NULL");
        cModule *m = findModuleWhereverInNode(name, from);
        return dynamic_cast<T*>(m);
    }
};

#endif

