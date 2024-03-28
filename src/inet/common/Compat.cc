//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/INETDefs.h"
#include "inet/common/Compat.h"

namespace inet {


#if OMNETPP_BUILDNUM < 2000

using namespace omnetpp;

const size_t MAX_NUM_COUNTERS = 1000;  // fixed size to prevent array reallocation -- increase if necessary

SharedDataManager::SharedDataHandles *SharedDataManager::sharedVariableHandles;
SharedDataManager::SharedDataHandles *SharedDataManager::sharedCounterHandles;

int SharedDataManager::SharedDataHandles::registerName(const char *name)
{
    auto it = nameToHandle.find(name);
    if (it != nameToHandle.end())
        return it->second;
    else {
        nameToHandle[name] = ++lastHandle;
        return lastHandle;
    }
}

const char *SharedDataManager::SharedDataHandles::getNameFor(int handle)
{
    // linear search should suffice here (small number of items, non-performance-critical)
    for (auto& pair : nameToHandle)
        if (pair.second == handle)
            return pair.first.c_str();
    return nullptr;
}

SharedDataManager::SharedDataManager()
{
    sharedCounters.resize(MAX_NUM_COUNTERS, INVALID);
}

sharedvarhandle_t SharedDataManager::registerSharedVariableName(const char *name)
{
    if (sharedVariableHandles == nullptr)
        sharedVariableHandles = new SharedDataHandles();
    return sharedVariableHandles->registerName(name);
}

const char *SharedDataManager::getSharedVariableName(sharedvarhandle_t handle)
{
    return sharedVariableHandles ? sharedVariableHandles->getNameFor(handle) : nullptr;
}

sharedcounterhandle_t SharedDataManager::registerSharedCounterName(const char *name)
{
    if (sharedCounterHandles == nullptr)
        sharedCounterHandles = new SharedDataHandles();
    return sharedCounterHandles->registerName(name);
}

const char *SharedDataManager::getSharedCounterName(sharedcounterhandle_t handle)
{
    return sharedCounterHandles ? sharedCounterHandles->getNameFor(handle) : nullptr;
}

uint64_t& SharedDataManager::getSharedCounter(sharedcounterhandle_t handle, uint64_t initialValue)
{
    if (sharedCounters.size() <= handle)
        throw cRuntimeError("getSharedCounter(): invalid handle %lu, or number of shared counters exhausted (you can increase table size in " __FILE__ ")", (unsigned long)handle); // fixed size to prevent array reallocation which would invalidate returned references -- increase if necessary
    uint64_t& counter = sharedCounters[handle];
    if (counter == INVALID)
        counter = initialValue;
    return counter;
}

uint64_t& SharedDataManager::getSharedCounter(const char *name, uint64_t initialValue)
{
    return getSharedCounter(registerSharedCounterName(name), initialValue);
}

void SharedDataManager::clear()
{
    for (auto& item : sharedVariables)
        if (item.second)
            item.second(); // call stored destructor
    sharedVariables.clear();
    sharedCounters.clear();
    sharedCounters.resize(MAX_NUM_COUNTERS, INVALID);
}

void SharedDataManager::lifecycleEvent(SimulationLifecycleEventType eventType, cObject *details)
{
    if (eventType == LF_POST_NETWORK_DELETE)
        clear();

    CodeFragment::executeAll(eventType);
}

SharedDataManager *SharedDataManager::getInstance()
{
    static SharedDataManager *instance = nullptr;
    if (instance == nullptr) {
        instance = new SharedDataManager;
        getEnvir()->addLifecycleListener(instance);
    }
    return instance;
}

EXECUTE_ON_STARTUP(SharedDataManager::getInstance());

//---

CodeFragment *CodeFragment::head = nullptr;

CodeFragment::CodeFragment(void (*code)(), SimulationLifecycleEventType lifecycleEvent) : lifecycleEvent(lifecycleEvent), code(code)
{
    next = head;
    head = this;
}

void CodeFragment::executeAll(SimulationLifecycleEventType lifecycleEvent)
{
    CodeFragment *p = CodeFragment::head;
    while (p) {
        if (p->lifecycleEvent == lifecycleEvent)
            p->code();
        p = p->next;
    }
}

#endif



#ifdef _MSC_VER

//
// Implementation of the error function, from the Mobility Framework
//
// Unfortunately the windows math library does not provide an
// implementation of the error function, so we use an own
// implementation (Thanks to Jirka Klaue)
// Author Jirka Klaue
//
double INET_API erfc(double x)
{
    double t, u, y;

    if (x <= -6)
        return 2;
    if (x >= 6)
        return 0;

    t = 3.97886080735226 / (fabs(x) + 3.97886080735226);
    u = t - 0.5;
    y = (((((((((0.00127109764952614092 * u + 1.19314022838340944e-4) * u
                - 0.003963850973605135) * u - 8.70779635317295828e-4) * u
              + 0.00773672528313526668) * u + 0.00383335126264887303) * u
            - 0.0127223813782122755) * u - 0.0133823644533460069) * u
          + 0.0161315329733252248) * u + 0.0390976845588484035) * u
        + 0.00249367200053503304;
    y = ((((((((((((y * u - 0.0838864557023001992) * u
                   - 0.119463959964325415) * u + 0.0166207924969367356) * u
                 + 0.357524274449531043) * u + 0.805276408752910567) * u
               + 1.18902982909273333) * u + 1.37040217682338167) * u
             + 1.31314653831023098) * u + 1.07925515155856677) * u
           + 0.774368199119538609) * u + 0.490165080585318424) * u
         + 0.275374741597376782) * t * exp(-x * x);

    return x < 0 ? 2 - y : y;
}

#endif // ifdef _MSC_VER

} // namespace inet

