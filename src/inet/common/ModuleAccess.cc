//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/ModuleAccess.h"

namespace inet {

inline bool _isNetworkNode(const cModule *mod)
{
    cProperties *props = mod->getProperties();
    return props && props->getAsBool("networkNode");
}

bool isNetworkNode(const cModule *mod)
{
    return (mod != nullptr) ? _isNetworkNode(mod) : false;
}

cModule *findModuleSomewhereUp(const char *name, const cModule *from)
{
    cModule *mod = nullptr;
    for (const cModule *curmod = from; !mod && curmod; curmod = curmod->getParentModule())
        mod = curmod->getSubmodule(name);
    return mod;
}

cModule *findContainingNode(const cModule *from)
{
    for (cModule *curmod = const_cast<cModule *>(from); curmod; curmod = curmod->getParentModule()) {
        if (_isNetworkNode(curmod))
            return curmod;
    }
    return nullptr;
}

cModule *getContainingNode(const cModule *from)
{
    cModule *curmod = findContainingNode(from);
    if (!curmod)
        throw cRuntimeError("getContainingNode(): node module not found (it should have a property named networkNode) for module '%s'", from ? from->getFullPath().c_str() : "<nullptr>");
    return curmod;
}

cModule *findModuleUnderContainingNode(const cModule *from)
{
    cModule *prevmod = nullptr;
    for (cModule *curmod = const_cast<cModule *>(from); curmod; curmod = curmod->getParentModule()) {
        if (_isNetworkNode(curmod))
            return prevmod;
        prevmod = curmod;
    }
    return nullptr;
}

cNEDValue nedf_nodeFullName(cComponent *context, cNEDValue argv[], int argc)
{
    cModule *node = getContainingNode(check_and_cast<cModule *>(context));
    return node->getFullName();
}

Define_NED_Function2(nedf_nodeFullName,
        "string nodeFullName()",
        "node",
        "Returns the full name of the containing network node"
        );

cNEDValue nedf_nodeIndex(cComponent *context, cNEDValue argv[], int argc)
{
    cModule *node = getContainingNode(check_and_cast<cModule *>(context));
    return node->getIndex();
}

Define_NED_Function2(nedf_nodeIndex,
        "int nodeIndex()",
        "node",
        "Returns the submodule index of the containing network node"
        );

} // namespace inet

