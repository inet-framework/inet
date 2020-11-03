//
// Copyright (C) 2020 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#ifndef __INET_MODULEREF_H
#define __INET_MODULEREF_H

#include "inet/common/ModuleAccess.h"

namespace inet {

/**
 * This template class implements a module reference that is parameterized by
 * a gate of the referencing module. The reference can be set by providing the
 * gate of the referencing module. After the reference is set, it can be used
 * similarly to simple a C++ pointer. The pointer is automatically set to nullptr
 * when the referenced module is deleted.
 *
 * TODO: follow connection
 */
template <typename T>
class INET_API ModuleRef
{
  private:
    opp_component_ptr<T> referencedModule;

    void checkReference() const {
        if (referencedModule.getNullable() == nullptr)
            throw cRuntimeError("Dereferencing uninitialized reference of type '%s'", opp_typename(typeid(T)));
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

    T *get() { checkReference(); return get(); }
    const T *get() const { checkReference(); return get(); }

    void reference(cModule *referencingModule, const char *parameterName, bool mandatory) {
        if (referencingModule == nullptr)
            throw cRuntimeError("Referencing module is nullptr");
        if (parameterName == nullptr)
            throw cRuntimeError("Parameter name is nullptr");
        auto& parameter = referencingModule->par(parameterName);
        referencedModule = mandatory ? getModuleFromPar<T>(parameter, referencingModule) : findModuleFromPar<T>(parameter, referencingModule);
    }

    void reference(cGate *gate, bool mandatory) {
        if (gate == nullptr)
            throw cRuntimeError("Gate is nullptr");
        referencedModule = mandatory ? getConnectedModule<T>(gate) : findConnectedModule<T>(gate);
    }
};

} // namespace inet

#endif

