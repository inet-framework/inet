//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_SIMULATIONCONTINUATION_H
#define __INET_SIMULATIONCONTINUATION_H

#include "inet/common/FunctionalEvent.h"
#include "inet/common/INETDefs.h"

namespace inet {

struct CoroutineSlot;

/**
 * Stops the execution of the current event and schedules resuming the execution after the specified simulation time.
 */
INET_API void sleepSimulationTime(simtime_t duration);

/**
 * This class supports temporarily stopping and later resuming the execution of the current event.
 */
class INET_API SimulationContinuation
{
  protected:
    bool isStopped = false;
    CoroutineSlot *suspendedSlot = nullptr;

  public:
    virtual ~SimulationContinuation() { }

    virtual void suspend();
    virtual void resume();
};

class INET_API SimulationContextSwitchingEvent : public cEvent
{
  public:
    CoroutineSlot *suspendedSlot = nullptr;

  public:
    SimulationContextSwitchingEvent(const char *name) : cEvent(name) { }

    virtual cEvent *dup() const override { throw cRuntimeError("Cannot duplicate SimulationContextSwitchingEvent"); }
    virtual cObject *getTargetObject() const override { return nullptr; }

    virtual void execute() override;
};

/**
 * When enabled, each pushPacket() call that replaced a send() during the
 * intra-node communication refactoring delivers its packet in a separate
 * zero-delay FunctionalEvent while the caller continues immediately. This
 * recreates the pre-refactoring send() semantics event by event, for
 * trajectory verification (see the trajectory-verification config option).
 */
INET_API extern bool deferPushPacketForVerification;

template<typename TSink, typename TPacket>
inline void deferrablePushPacket(TSink& sink, TPacket *packet)
{
    if (!deferPushPacketForVerification)
        sink.pushPacket(packet);
    else {
        // Deliver in a separate event, like the pre-refactoring send() did. The
        // caller's module context is restored around the deferred push so the
        // sink's Enter_Method sees the same caller as a direct call would.
        auto context = cSimulation::getActiveSimulation()->getContext();
        scheduleAt(packet->getName(), simTime(), [&sink, packet, context]() {
            cContextSwitcher switcher(context);
            sink.pushPacket(packet);
        });
    }
}

} // namespace inet

#endif

