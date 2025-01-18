//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_CLOCKBASE_H
#define __INET_CLOCKBASE_H

#include "inet/clock/common/ClockTime.h"
#include "inet/clock/common/ClockEvent.h"
#include "inet/clock/contract/IClock.h"
#include "inet/common/StringFormat.h"
#include "inet/common/ModuleRefByPar.h"

namespace inet {

class INET_API ClockBase : public cSimpleModule, public IClock, public StringFormat::IDirectiveResolver, public cListener
{
  public:
    struct ClockJumpDetails : public cObject {
        clocktime_t oldClockTime;
        clocktime_t newClockTime;
    };

  public:
    static simsignal_t timeChangedSignal; // Called every time there is a change in the speed of the clock oscillator
    static simsignal_t timeDifferenceToReferenceSignal; // Called every time there is a change in the speed of the clock oscillator
    static simsignal_t timeJumpedSignal; // Only called when the clock performs an immediate jump to a new time
    ModuleRefByPar<IClock> referenceClockModule;

  protected:
    clocktime_t clockEventTime = -1;
    const char *displayStringTextFormat = nullptr;
    simtime_t emitClockTimeInterval;
    cMessage *timer = nullptr;

  protected:
    virtual ~ClockBase() { cancelAndDelete(timer); }
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override;
    void emitTimeDifferenceToReference();
    virtual void finish() override;
    virtual void refreshDisplay() const override;
    virtual void updateDisplayString() const;

    cSimpleModule *getTargetModule() const {
        cSimpleModule *target = getSimulation()->getContextSimpleModule();
        if (target == nullptr)
            throw cRuntimeError("scheduleAt()/cancelEvent() must be called with a simple module in context");
        return target;
    }

  public:
    virtual clocktime_t getClockTime() const override;

    virtual void scheduleClockEventAt(clocktime_t time, ClockEvent *event) override;
    virtual void scheduleClockEventAfter(clocktime_t time, ClockEvent *event) override;
    virtual ClockEvent *cancelClockEvent(ClockEvent *event) override;
    virtual void handleClockEvent(ClockEvent *event) override;
    virtual void receiveSignal(cComponent *source, int signal, const simtime_t& time, cObject *details) override;

    virtual std::string resolveDirective(char directive) const override;
};

} // namespace inet

#endif

