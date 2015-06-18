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

#include "inet/common/ModuleAccess.h"

namespace inet {

inline bool _isNetworkNode(cModule *mod)
{
    cProperties *props = mod->getProperties();
    return props && props->getAsBool("networkNode");
}

bool isNetworkNode(cModule *mod)
{
    return (mod != nullptr) ? _isNetworkNode(mod) : false;
}

/*
static cModule *findSubmodRecursive(cModule *curmod, const char *name)
{
    for (cModule::SubmoduleIterator i(curmod); !i.end(); i++) {
        cModule *submod = i();
        if (!strcmp(submod->getFullName(), name))
            return submod;
        cModule *foundmod = findSubmodRecursive(submod, name);
        if (foundmod)
            return foundmod;
    }
    return nullptr;
}
*/

cModule *findModuleSomewhereUp(const char *name, cModule *from)
{
    cModule *mod = nullptr;
    for (cModule *curmod = from; !mod && curmod; curmod = curmod->getParentModule())
        mod = curmod->getSubmodule(name);
    return mod;
}

cModule *findContainingNode(cModule *from)
{
    for (cModule *curmod = from; curmod; curmod = curmod->getParentModule()) {
        if (_isNetworkNode(curmod))
            return curmod;
    }
    return nullptr;
}

cModule *getContainingNode(cModule *from)
{
    cModule *curmod = findContainingNode(from);
    if (!curmod)
        throw cRuntimeError("getContainingNode(): node module not found (it should have a property named networkNode) for module '%s'", from ? from->getFullPath().c_str() : "<nullptr>");
    return curmod;
}

cModule *findModuleUnderContainingNode(cModule *from)
{
    cModule *prevmod = nullptr;
    for (cModule *curmod = from; curmod; curmod = curmod->getParentModule()) {
        if (_isNetworkNode(curmod))
            return prevmod;
        prevmod = curmod;
    }
    return nullptr;
}

} // namespace inet

