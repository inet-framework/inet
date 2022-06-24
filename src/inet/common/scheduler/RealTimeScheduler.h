//
// Copyright (C) 2005 Andras Varga,
//                    Christian Dankbar, Irene Ruengeler, Michael Tuexen
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_REALTIMESCHEDULER_H
#define __INET_REALTIMESCHEDULER_H

#define WANT_WINSOCK2

#include "inet/common/INETDefs.h"

namespace inet {

/**
 * This class implements the event scheduler interface of OMNeT++ to provide a
 * real time scheduling mechanism. The events are scheduled so that the events
 * are executed according to the time of the underlying operating system. All
 * time values are measured in nanoseconds.
 */
class INET_API RealTimeScheduler : public cScheduler
{
  public:
    class INET_API ICallback {
      public:
        virtual ~ICallback() {}
        virtual bool notify(int fd) = 0;
    };

    class INET_API Entry {
      public:
        int fd = -1;
        ICallback *callback = nullptr;

      public:
        Entry(int fd, ICallback *callback) : fd(fd), callback(callback) {}
    };

    int64_t baseTime = 0; // in nanoseconds, as returned by opp_get_monotonic_clock_nsecs()

  protected:
    class INET_API BeginSimulationEvent : public cEvent {
      public:
        BeginSimulationEvent(const char *name) : cEvent(name) {}
        virtual cEvent *dup() const override { return new BeginSimulationEvent(getName()); }
        virtual cObject *getTargetObject() const override { return nullptr; }
        virtual void execute() override { delete this; }
    };

  protected:
    std::vector<Entry> callbackEntries;

  protected:
    virtual void advanceSimTime();
    virtual bool receiveWithTimeout(int64_t timeout);
    virtual int receiveUntil(int64_t targetTime); // in nanoseconds, as returned by opp_get_monotonic_clock_nsecs()

  public:
    RealTimeScheduler();
    virtual ~RealTimeScheduler();

    /**
     * To be called from the module which wishes to receive data from the
     * fd. The method must be called from the module's initialize() function.
     */
    void addCallback(int fd, ICallback *callback);
    void removeCallback(int fd, ICallback *callback);

    /**
     * Called at the beginning of a simulation run.
     */
    virtual void startRun() override;

    /**
     * Called at the end of a simulation run.
     */
    virtual void endRun() override;

    /**
     * Recalculates "base time" from current wall clock time.
     */
    virtual void executionResumed() override;

    /**
     * Returns the first event in the Future Event Set.
     */
    virtual cEvent *guessNextEvent() override;

    /**
     * Scheduler function -- it comes from the cScheduler interface.
     */
    virtual cEvent *takeNextEvent() override;

    /**
     * Scheduler function -- it comes from the cScheduler interface.
     */
    virtual void putBackEvent(cEvent *event) override;
};

} // namespace inet

#endif

