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

#ifndef __INET_MODULEREFBYPAR_H
#define __INET_MODULEREFBYPAR_H

#include "inet/common/ModuleAccess.h"

namespace inet {

/**
 * This template class implements a module reference that is parameterized by
 * a string module parameter of the referencing module. The reference can be
 * set by providing the referencing module and the name of the module parameter.
 * After the reference is set, it can be used similarly to a simple C++ pointer.
 * The pointer is automatically set to nullptr when the referenced module is
 * deleted.
 *
 * TODO: follow parameter change
 */
template <typename T>
class INET_API ModuleRefByPar
{
  private:
    opp_component_ptr<T> referencedModule;

#ifndef NDEBUG
    cModule *referencingModule = nullptr;
    const char *parameterName = nullptr;
#endif

    void checkReference() const {
#ifndef NDEBUG
        if (referencedModule.getNullable() == nullptr) {
            if (referencingModule == nullptr)
                throw cRuntimeError("Dereferencing uninitialized reference of type '%s'", opp_typename(typeid(T)));
            else
                throw cRuntimeError("Dereferencing nullptr of type '%s' referenced by module '(%s)%s' with a string module parameter '%s'",
                                    opp_typename(typeid(T)), referencingModule->getClassName(), referencingModule->getFullPath().c_str(), parameterName);

        }
#else
        if (referencedModule.getNullable() == nullptr)
            throw cRuntimeError("Dereferencing nullptr of type '%s'", opp_typename(typeid(T)));
#endif
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
#ifndef NDEBUG
        if (this->referencingModule != nullptr)
            throw cRuntimeError("Reference is already initialized");
        this->referencingModule = referencingModule;
        this->parameterName = parameterName;
#endif
        auto& parameter = referencingModule->par(parameterName);
        referencedModule = mandatory ? getModuleFromPar<T>(parameter, referencingModule) : findModuleFromPar<T>(parameter, referencingModule);
    }
};

} // namespace inet

#endif

