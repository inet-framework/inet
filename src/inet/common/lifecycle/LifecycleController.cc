//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/lifecycle/LifecycleController.h"

#include <algorithm>

#include "inet/common/INETUtils.h"
#include "inet/common/lifecycle/LifecycleOperation.h"
#include "inet/common/stlutils.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/networklayer/common/NetworkInterface.h"
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

// ----

template<typename T>
void vector_delete_element(std::vector<T *>& v, T *p)
{
    auto it = find(v, p);
    ASSERT(it != v.end());
    v.erase(it);
    delete p;
}

bool LifecycleController::initiateOperation(LifecycleOperation *operation, IDoneCallback *completionCallback)
{
    ASSERT(cSimulation::getActiveSimulation()->getContextModule() == check_and_cast<cComponent *>(this));
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
    return true; // done
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
    omnetpp::cMethodCallContextSwitcher __ctx(check_and_cast<cComponent *>(this)); __ctx.methodCall(__FUNCTION__);
#ifdef INET_WITH_SELFDOC
    __Enter_Method_SelfDoc(__FUNCTION__);
#endif

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

