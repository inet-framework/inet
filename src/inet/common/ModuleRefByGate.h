//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_MODULEREFBYGATE_H
#define __INET_MODULEREFBYGATE_H

#include "inet/common/IModuleInterfaceLookup.h"
#include "inet/common/ModuleAccess.h"

namespace inet {

/**
 * This template class implements a module reference that is parameterized by
 * a gate of the referencing module. The reference can be set by providing the
 * gate of the referencing module. After the reference is set, it can be used
 * similarly to a simple C++ pointer. The pointer is automatically set to nullptr
 * when the referenced module is deleted.
 *
 * TODO follow connection
 */
template<typename T>
class INET_API ModuleRefByGate
{
  protected:
    cGate *referencedGate = nullptr;
    opp_component_ptr<T> referencedModule;

    cGate *referencingGate = nullptr;

    void checkReference() const {
        if (referencedModule.getNullable() == nullptr) {
            if (referencingGate == nullptr)
                throw cRuntimeError("Dereferencing uninitialized reference of type '%s'", opp_typename(typeid(T)));
            else
                throw cRuntimeError("Dereferencing nullptr of type '%s' referenced by '(%s)%s' through gate '%s'",
                                    opp_typename(typeid(T)), referencingGate->getOwnerModule()->getClassName(), referencingGate->getOwnerModule()->getFullPath().c_str(), referencingGate->getFullName());
        }
    }

  public:
    T& operator*() const {
        checkReference();
        return *referencedModule;
    }

    T *operator->() const {
        checkReference();
        return referencedModule;
    }

    operator T *() const {
        return referencedModule;
    }

    explicit operator bool() const { return (bool)referencedModule; }

    T *getNullable() { return referencedModule.getNullable(); }
    const T *getNullable() const { return referencedModule.getNullable(); }

    T *get() { checkReference(); return referencedModule.get(); }
    const T *get() const { checkReference(); return referencedModule.get(); }

    cGate *getReferencingGate() { return referencingGate; }
    const cGate *getReferencingGate() const { return referencingGate; }

    cGate *getReferencedGate() { return referencedGate; }
    const cGate *getReferencedGate() const { return referencedGate; }

    void reference(cGate *referencingGate, bool mandatory, const cObject *arguments = nullptr, int direction = 0) {
        if (referencingGate == nullptr)
            throw cRuntimeError("Referencing gate is nullptr");
        if (this->referencingGate != nullptr)
            throw cRuntimeError("Reference is already initialized");
        this->referencingGate = referencingGate;
        referencedGate = findModuleInterface(referencingGate, typeid(T), arguments, direction);
        referencedModule = referencedGate != nullptr ? dynamic_cast<T *>(referencedGate->getOwnerModule()) : nullptr;
        if (mandatory && referencedModule == nullptr)
            throw cRuntimeError("Cannot find referenced module interface, module = %s, gate = %s, type = %s, arguments = %s, direction = %s",
                                printToStringIfPossible(referencingGate->getOwnerModule(), 0).c_str(),
                                printToStringIfPossible(referencingGate, 0).c_str(),
                                opp_typename(typeid(T)),
                                printToStringIfPossible(arguments, 0).c_str(),
                                printToStringIfPossible(direction, 0).c_str());
    }
};

} // namespace inet

#endif

