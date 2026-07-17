//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/CoroutineEventExecution.h"
#include "inet/common/SimulationContinuation.h"

namespace inet {

static const unsigned COROUTINE_STACK_SIZE = 1048576; // 1 MB

thread_local CoroutineSlot *currentCoroutineSlot = nullptr;

static void coroutineBody(void *arg)
{
    CoroutineSlot *slot = (CoroutineSlot *)arg;
    while (true) {
        try {
            cSimulation::getActiveSimulation()->executeEvent(slot->event);
        }
        catch (...) {
            slot->exception = std::current_exception();
        }
        slot->completed = true;
        cCoroutine::switchToMain();
    }
}

class CoroutinePool
{
  protected:
    std::vector<CoroutineSlot *> available;

  public:
    ~CoroutinePool() {
        for (auto *slot : available)
            delete slot;
    }

    CoroutineSlot *acquire() {
        if (!available.empty()) {
            auto *slot = available.back();
            available.pop_back();
            return slot;
        }
        auto *slot = new CoroutineSlot();
        if (!slot->coroutine.setup(coroutineBody, slot, COROUTINE_STACK_SIZE))
            throw cRuntimeError("Cannot set up coroutine for event execution");
        return slot;
    }

    void release(CoroutineSlot *slot) {
        available.push_back(slot);
    }
};

static CoroutinePool pool;

static void coroutineEventExecutor(cSimulation *sim, cEvent *event)
{
    CoroutineSlot *slot;

    auto *ctxSwitch = dynamic_cast<SimulationContextSwitchingEvent *>(event);
    if (ctxSwitch && ctxSwitch->suspendedSlot) {
        // Wake-up or resume event: run executeEvent on main stack for
        // bookkeeping (time advance, event number, eventlog), then resume
        // the suspended coroutine.  execute() is a no-op in this mode.
        slot = ctxSwitch->suspendedSlot;
        sim->executeEvent(event);
        ctxSwitch->removeFromOwnershipTree();
        delete ctxSwitch;
        slot->completed = false;
        slot->yielded = false;
    }
    else {
        // Normal event: run on a coroutine from the pool
        slot = pool.acquire();
        slot->event = event;
        slot->completed = false;
        slot->yielded = false;
    }

    currentCoroutineSlot = slot;
    cCoroutine::switchTo(&slot->coroutine);
    currentCoroutineSlot = nullptr;

    if (slot->exception) {
        auto ex = slot->exception;
        slot->exception = nullptr;
        pool.release(slot);
        std::rethrow_exception(ex);
    }
    if (slot->completed) {
        pool.release(slot);
    }
    // else: yielded — slot stays pinned until the wake-up event resumes it
}

void installCoroutineEventExecution()
{
    cSimulation::getActiveSimulation()->setEventExecutor(coroutineEventExecutor);
}

void uninstallCoroutineEventExecution()
{
    cSimulation::getActiveSimulation()->setEventExecutor(nullptr);
}

Register_GlobalConfigOption(CFGID_TRAJECTORY_VERIFICATION, "trajectory-verification", CFG_BOOL, "false", "Delivers each pushPacket() call that replaced a send() during the intra-node communication refactoring in a separate zero-delay event, recreating the pre-refactoring event trajectories for verification. The INET_TRAJECTORY_VERIFICATION environment variable also enables it.");

class INET_API TrajectoryVerificationActivator : public cISimulationLifecycleListener
{
  public:
    virtual void lifecycleEvent(SimulationLifecycleEventType eventType, cObject *details) override
    {
        switch (eventType) {
            case LF_PRE_NETWORK_SETUP:
                if (cSimulation::getActiveEnvir()->getConfig()->getAsBool(CFGID_TRAJECTORY_VERIFICATION) || getenv("INET_TRAJECTORY_VERIFICATION") != nullptr)
                    deferPushPacketForVerification = true;
                break;
            case LF_ON_RUN_END:
                deferPushPacketForVerification = false;
                break;
            default:
                break;
        }
    }
};

EXECUTE_ON_STARTUP(
    cSimulation::getActiveEnvir()->addLifecycleListener(new TrajectoryVerificationActivator());
);

} // namespace inet
