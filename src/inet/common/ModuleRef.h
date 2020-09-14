//
// Copyright (C) OpenSim Ltd.
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
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//

#ifndef __INET_MODULE_REF_H
#define __INET_MODULE_REF_H

#include "inet/common/ModuleAccess.h"

namespace inet {

template <typename T>
class INET_API ModuleRef : public cListener
{
  private:
    T *referencedModule = nullptr;

#ifndef NDEBUG
    cModule *referencingModule = nullptr;
    const char *parameterName = nullptr;
#endif

    void checkReference() const {
        if (referencedModule == nullptr)
#ifndef NDEBUG
            throw cRuntimeError("Dereferencing nullptr from '%s' with parameter '%s'", referencingModule != nullptr ? referencingModule->getFullPath().c_str() : "", parameterName != nullptr ? parameterName : "");
#else
            throw cRuntimeError("Dereferencing nullptr");
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
        checkReference();
        return referencedModule;
    }

    explicit operator bool() const { return referencedModule != nullptr; }

    void set(cModule *referencingModule, const char *parameterName, bool mandatory = false) {
#ifndef NDEBUG
        this->referencingModule = referencingModule;
        this->parameterName = parameterName;
#endif
        referencedModule = findModuleFromPar<T>(referencingModule->par(parameterName), referencingModule);
        if (referencedModule != nullptr)
            getModule()->subscribe(PRE_MODEL_CHANGE, this);
    }

    T *find() { return referencedModule; }

    const T *find() const { return referencedModule; }

    T *get() {
        checkReference();
        return find();
    }

    const T *get() const {
        checkReference();
        return find();
    }

    cModule *findModule() {
        return dynamic_cast<cModule *>(referencedModule);
    }

    const cModule *findModule() const {
        return dynamic_cast<cModule *>(referencedModule);
    }

    cModule *getModule() {
        checkReference();
        return findModule();
    }

    const cModule *getModule() const {
        checkReference();
        return findModule();
    }

  public:
    virtual void receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details) override {
        if (signal == PRE_MODEL_CHANGE) {
            if (auto preModuleDeleteNotification = dynamic_cast<cPreModuleDeleteNotification *>(object)) {
                if (preModuleDeleteNotification->module == findModule())
                    referencedModule = nullptr;
            }
        }
    }
};

} // namespace inet

#endif

