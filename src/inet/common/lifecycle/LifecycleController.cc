//
// (C) 2013 Opensim Ltd.
//
// This library is free software, you can redistribute it
// and/or modify
// it under  the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation;
// either version 2 of the License, or any later version.
// The library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// Author: Andras Varga (andras@omnetpp.org)
//

#include <algorithm>

#include "inet/common/INETUtils.h"
#include "inet/common/lifecycle/LifecycleController.h"
#include "inet/common/lifecycle/LifecycleOperation.h"
#include "inet/networklayer/common/InterfaceEntry.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/networklayer/contract/IInterfaceTable.h"

namespace inet {

void LifecycleController::Callback::init(LifecycleController *controller, LifecycleOperation *operation, cModule *module)
{
    this->controller = controller;
    this->operation = operation;
    this->module = module;
}

void LifecycleController::Callback::invoke()
{
    if (!controller)
        throw cRuntimeError("Usage error: callback may only be invoked if (and after) you returned 'false' from initiateStateChange()");
    controller->moduleOperationStageCompleted(this);
}

//----

template<typename T>
void vector_delete_element(std::vector<T *>& v, T *p)
{
    auto it = std::find(v.begin(), v.end(), p);
    ASSERT(it != v.end());
    v.erase(it);
    delete p;
}

bool LifecycleController::initiateOperation(LifecycleOperation *operation, IDoneCallback *completionCallback)
{
    operation->currentStage = 0;
    operation->operationCompletionCallback = completionCallback;
    operation->insideInitiateOperation = true;
    return resumeOperation(operation);
}

bool LifecycleController::resumeOperation(LifecycleOperation *operation)
{
    int numStages = operation->getNumStages();
    while (operation->currentStage < numStages) {
        EV << "Doing stage " << operation->currentStage << "/" << operation->getNumStages()
           << " of operation " << operation->getClassName() << " on " << operation->rootModule->getFullPath() << endl;
        doOneStage(operation, operation->rootModule);
        if (operation->pendingList.empty())
            operation->currentStage++;
        else
            return false; // pending
    }

    // done: invoke callback (unless we are still under initiateOperation())
    if (operation->operationCompletionCallback && !operation->insideInitiateOperation)
        operation->operationCompletionCallback->invoke();
    delete operation;
    return true;    // done
}

void LifecycleController::doOneStage(LifecycleOperation *operation, cModule *submodule)
{
    ILifecycle *subject = dynamic_cast<ILifecycle *>(submodule);
    if (subject) {
        Callback *callback = spareCallback ? spareCallback : new Callback();
        bool done = subject->handleOperationStage(operation, callback);
        if (!done) {
            callback->init(this, operation, submodule);
            operation->pendingList.push_back(callback);
            spareCallback = nullptr;
        }
        else
            spareCallback = callback;
    }

    for (cModule::SubmoduleIterator i(submodule); !i.end(); i++) {
        cModule *child = *i;
        doOneStage(operation, child);
    }
}

void LifecycleController::moduleOperationStageCompleted(Callback *callback)
{
    LifecycleOperation *operation = callback->operation;
    std::string moduleFullPath = callback->module->getFullPath();
    vector_delete_element(operation->pendingList, (IDoneCallback *)callback);

    EV << "Module " << moduleFullPath << " completed stage "
       << operation->currentStage << " of operation " << operation->getClassName() << ", "
       << operation->pendingList.size() << " more module(s) pending"
       << (operation->pendingList.empty() ? ", stage completed" : "") << endl;

    if (operation->pendingList.empty()) {
        operation->currentStage++;
        operation->insideInitiateOperation = false;
        resumeOperation(operation);
    }
}

} // namespace inet

