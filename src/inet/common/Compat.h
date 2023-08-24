//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_COMPAT_H
#define __INET_COMPAT_H

#include <omnetpp.h>

#include <functional>

#if OMNETPP_BUILDNUM < 2000
#include <iostream>
#endif

namespace inet {

#if OMNETPP_BUILDNUM < 2000

/**
 * @brief Handles used for accessing simulation-wide shared variables.
 *
 * @see cSimulation::registerSharedVariableName(), cSimulation::getSharedVariable()
 */
typedef int sharedvarhandle_t;

/**
 * @brief Handles used for accessing simulation-wide counters.
 *
 * @see cSimulation::registerSharedCounterName(), cSimulation::getSharedCounter(),
 * cSimulation::getNextSharedCounterValue()
 */
typedef int sharedcounterhandle_t;

/**
 * This class encapsulates the "simulation-global variables" functionality which became
 * part of the cSimulation class in OMNeT++ 7.0.
 */
class INET_API SharedDataManager : public omnetpp::cISimulationLifecycleListener
{
  private:
    struct SharedDataHandles {
        std::map<std::string,int> nameToHandle;
        int lastHandle = -1;
        int registerName(const char *);
        const char *getNameFor(int handle);
    };

    // shared variables
    static SharedDataHandles *sharedVariableHandles;
    std::vector<std::pair<omnetpp::any_ptr,std::function<void()>>> sharedVariables; // handle -> (data,destructor)

    // shared counters
    static SharedDataHandles *sharedCounterHandles;
    std::vector<uint64_t> sharedCounters; // handle -> counter
    const uint64_t INVALID = (uint64_t)-1;

  protected:
    SharedDataManager();
    virtual void lifecycleEvent(omnetpp::SimulationLifecycleEventType eventType, omnetpp::cObject *details) override;
    void clear();

  public:
    /**
     * Assigns a handle to the given name, and returns it. The handle can be
     * used as an argument for the getSharedVariable() method instead of
     * the name, to speed up lookup. It is allowed/recommended to allocate
     * handles in static initializers. E.g:
     *
     * int Foo::counterHandle = cSimulation::registerSharedVariableName("Foo::counter");
     */
    static int registerSharedVariableName(const char *name);

    /**
     * The inverse of registerSharedVariableName(): Returns the variable name
     * from the handle, or nullptr if the handle is not in use.
     */
    static const char *getSharedVariableName(int handle);

    /**
     * Like getSharedVariable(const char *name, Args&&... args), but uses a
     * a handle allocated via registerSharedVariableName() instead of a name.
     * Using a handle results in better performance due to faster lookup.
     */
    template <typename T, typename... Args>
    T& getSharedVariable(int handle, Args&&... args);

    /**
     * Returns a "simulation global" object of type T identified by the given name.
     * Any additional arguments will be passed to the constructor of type T.
     * The object will be allocated on the first call, and subsequent calls with
     * the same name will return the same object. The object will be automatically
     * deallocated on network teardown. In model code, objects allocated via getSharedVariable()
     * should be preferred to actual global variables (e.g. static class members),
     * exactly because getSharedVariable() ensures that the variables will be properly
     * initialized for each simulation (enabling repeatable simulations) and
     * deallocated at the end, without the help of any extra code. Example:
     *
     * <tt>int& globalCounter = getSharedVariable<int>("Foo::globalCounter");</tt>
     */
    template <typename T, typename... Args>
    T& getSharedVariable(const char *name, Args&&... args);

    /**
     * Assigns a handle to the given name, and returns it. The handle can be
     * used as an argument for the getNextSharedCounterValue() method.
     * It is allowed/recommended to allocate handles in static initializers. E.g:
     *
     * sharedcounterhandle_t Foo::counterHandle =
     *     cSimulation::registerSharedCounterName("Foo::counter");
     */
    static sharedcounterhandle_t registerSharedCounterName(const char *name);

    /**
     * The inverse of registerSharedCounterName(): Returns the variable name
     * from the handle, or nullptr if the handle is not in use.
     */
    static const char *getSharedCounterName(sharedcounterhandle_t handle);

    /**
     * Returns a reference to the shared counter identified by the handle.
     * The handle should be allocated via registerSharedCounterName(). The
     * initialValue argument is only used if the call is the first one to access
     * the counter.
     */
    uint64_t& getSharedCounter(sharedcounterhandle_t handle, uint64_t initialValue=0);

    /**
     * Returns a reference to the shared counter identified by the given name.
     * The initialValue argument is only used if the call is the first one to
     * access the counter.
     */
    uint64_t& getSharedCounter(const char *name, uint64_t initialValue=0);

    /**
     * Returns the next value from the shared counter identified by the handle.
     * The handle should be allocated via registerSharedCounterName(). The
     * initialValue argument is only used if the call is the first one to access
     * the counter.
     */
    uint64_t getNextSharedCounterValue(sharedcounterhandle_t handle, uint64_t initialValue=0) {return ++getSharedCounter(handle,initialValue);}

    /**
     * Returns the next value from the shared counter identified by the handle.
     * The handle should be allocated via registerSharedCounterName(). The
     * initialValue argument is only used if the call is the first one to access
     * the counter.
     */
    uint64_t getNextSharedCounterValue(const char *name, uint64_t initialValue=0) {return ++getSharedCounter(name,initialValue);}

    /**
     * Returns the shared data manager.
     */
    static SharedDataManager *getInstance();
};

template <typename T, typename... Args>
T& SharedDataManager::getSharedVariable(const char *key, Args&&... args)
{
    return getSharedVariable<T>(registerSharedVariableName(key), args...);
}

template <typename T, typename... Args>
T& SharedDataManager::getSharedVariable(int handle, Args&&... args)
{
    if (sharedVariables.size() <= handle)
        sharedVariables.resize(handle+1);
    auto& item = sharedVariables[handle];
    if (item.first != nullptr)
        return *item.first.get<T>();
    else {
        T *p = new T(args...);
        item = std::make_pair(omnetpp::any_ptr(p), [=]() {delete p;});
        return *p;
    }
}

#endif // OMNETPP_BUILDNUM

#if OMNETPP_BUILDNUM < 2000

/**
 * Supporting class for EXECUTE_PRE_NETWORK_SETUP() / EXECUTE_POST_NETWORK_DELETE(),
 * which didn't exist prior to OMNeT++ 7.0.
 */
class INET_API CodeFragment
{
  private:
    omnetpp::SimulationLifecycleEventType lifecycleEvent;
    void (*code)();
    CodeFragment *next;
    static CodeFragment *head;
  public:
    CodeFragment(void (*code)(), omnetpp::SimulationLifecycleEventType lifecycleEvent);
    static void executeAll(omnetpp::SimulationLifecycleEventType lifecycleEvent);  // called from SharedDataManager::lifecycleEvent()
};

#define EXECUTE_ON_SIMULATION_LIFECYCLE_EVENT(LF_EVENT,CODE)  \
  namespace { void __ONSTARTUP_FUNC() {CODE;} static CodeFragment __ONSTARTUP_OBJ(__ONSTARTUP_FUNC, LF_EVENT); }

#define EXECUTE_PRE_NETWORK_SETUP(CODE)       EXECUTE_ON_SIMULATION_LIFECYCLE_EVENT(LF_PRE_NETWORK_SETUP, CODE)
#define EXECUTE_POST_NETWORK_DELETE(CODE)     EXECUTE_ON_SIMULATION_LIFECYCLE_EVENT(LF_POST_NETWORK_DELETE, CODE)

#endif // OMNETPP_BUILDNUM

//
// Account for differences in OMNeT++ 6.x, and 7.0 starting from OMNETPP_BUILDNUM = 2000.
//
// - The addLifecycleListener(), removeLifecycleListener() and getUniqueNumber() methods were
//   moved from cEnvir to cSimulation. The old cEnvir methods still exists but are deprecated,
//   and they delegate to their cSimulation equivalents. The getActiveSimulationOrEnvir() function
//   hides this difference and allows compiling without the deprecation warnings.
//
// - The getSharedVariable() method of cSimulation was introduced in 7.0. On 6.x, it is emulated with
//   the SharedDataManager compatibility class, and the cSimulationOrSharedDataManager and
//   getSimulationOrSharedDataManager() macro/function were added to allow code that works with
//   both versions.
//
// - Simulation stages are represented with STAGE_xxx enum values in 7.0, and with CTX_xxx values
//   in 6.0. The STAGE(x) macro hides the difference.
//
// - The OPP_THREAD_LOCAL macro was introduced in 7.0, it didn't exist in 6.0.
//
#if OMNETPP_BUILDNUM < 2000

#define OPP_THREAD_LOCAL
#define cSimulationOrSharedDataManager  SharedDataManager
inline SharedDataManager *getSimulationOrSharedDataManager() { ASSERT(omnetpp::cSimulation::getActiveSimulation()!=nullptr); return SharedDataManager::getInstance(); }
inline omnetpp::cEnvir *getActiveSimulationOrEnvir() { return omnetpp::cSimulation::getActiveEnvir(); }
#define STAGE(x) CTX_ ## x

#else

#define cSimulationOrSharedDataManager  cSimulation
inline omnetpp::cSimulation *getSimulationOrSharedDataManager() { return omnetpp::cSimulation::getActiveSimulation(); }
inline omnetpp::cSimulation *getActiveSimulationOrEnvir() { return omnetpp::cSimulation::getActiveSimulation(); }
#define STAGE(x) cSimulation::STAGE_ ## x

#endif // OMNETPP_BUILDNUM

#define EVSTREAM    EV

#ifdef _MSC_VER
// complementary error function, not in MSVC
double INET_API erfc(double x);

// ISO C99 function, not in MSVC
inline long lrint(double x)
{
    return (long)floor(x + 0.5);
}

// ISO C99 function, not in MSVC
inline double fmin(double a, double b)
{
    return a < b ? a : b;
}

// ISO C99 function, not in MSVC
inline double fmax(double a, double b)
{
    return a > b ? a : b;
}

#endif // _MSC_VER

} // namespace inet

#endif

