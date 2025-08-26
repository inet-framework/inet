//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ICLOCK_H
#define __INET_ICLOCK_H

#include "inet/clock/contract/ClockEvent.h"
#include "inet/clock/contract/ClockTime.h"

namespace inet {

struct NullStream {
    template<typename T>
    NullStream& operator<<(const T&) { return *this; }
    NullStream& operator<<(std::ostream& (*)(std::ostream&)) { return *this; }
};

extern NullStream nullStream;

#ifndef CLOCK_LOG_IMPLEMENTATION
  #include <sstream>
  #define CLOCK_COUT nullStream
#else
  #include <iostream>
  #include <iomanip>
  #define CLOCK_COUT !stdcoutenabled ? devnull : std::cout << std::setprecision(24) << std::string(stdcoutindent * 3, ' ')
#endif

#define CLOCK_CHECK_IMPLEMENTATION

#ifdef CLOCK_CHECK_IMPLEMENTATION
#define ASSERTCMP(cmp, o1, o2) { \
    ClockCoutDisabledBlock b; auto _o1 = (o1); auto _o2 = (o2); \
    if (!(_o1 cmp _o2)) { \
        std::ostringstream oss; \
        oss << "ASSERT: Condition '" << #o1 << " " << #cmp << " " << #o2 << "' as '" << _o1 << " " << #cmp << " " << _o2 << "' does not hold in function '%s' at %s:%d"; \
        throw omnetpp::cRuntimeError(oss.str().c_str(), __FUNCTION__, __FILE__, __LINE__); \
    } \
}
#else
#define ASSERTCMP(cmp, o1, o2) { (void)(o1); (void)(o2); }
#endif

extern bool clockCoutEnabled;
extern int clockCoutIndentLevel;

class ClockCoutIndent {
public:
    ClockCoutIndent() { ++clockCoutIndentLevel; }
    ~ClockCoutIndent() { --clockCoutIndentLevel; }
};

class ClockCoutDisabledBlock {
public:
    ClockCoutDisabledBlock() { clockCoutEnabled = false; }
    ~ClockCoutDisabledBlock() { clockCoutEnabled = true; }
};

/**
 * This class defines the interface for clocks. See the corresponding NED file for details.
 *
 * The typical way to use a clock is to derive your class or module from either
 * ClockUserModuleBase or ClockUserModuleMixin. Then you can use the inherited
 * clock related methods or the methods of this interface on the inherited clock
 * field.
 *
 * The following properties always hold for all clocks and clock events:
 *
 * 1. The clock time of a clock increases monotonically with simulation time
 *    unless the clock time is explicitly modified.
 *
 * 2. When a clock event is executed, the clock time is equal to the arrival
 *    clock time of the event.
 *
 * 3. The arrival clock time of a scheduled clock event is always greater than
 *    or equal to the current clock time.
 *
 * 4. The arrival simulation time of a scheduled clock event is equal to the
 *    expected simulation time when the clock will be at the arrival clock time
 *    of the clock event.
 *
 * 5. The arrival clock time of a scheduled clock event is equal to the expected
 *    clock time of the clock at the arrival simulation time of the clock event.
 */
class INET_API IClock
{
  public:
    virtual ~IClock() {}

    /**
     * Returns the current clock time. Note that the clock time is not necessarily
     * monotonous in execution order. For example, the clock time may decrease
     * even at the same simulation time.
     */
    virtual clocktime_t getClockTime() const = 0;

    /**
     * Returns the clock time for the specified future simulation time according
     * to the current state of the clock. This method implements a monotonic
     * function with respect to the simulation time argument. It's allowed to
     * return a different value for the same argument value if the clock is set
     * between calls. The time argument must be greater than or equal to the current
     * simulation time, otherwise an error is raised. See SIMTIME_AS_CLOCKTIME
     * macro for simple type conversion.
     */
    virtual clocktime_t computeClockTimeFromSimTime(simtime_t time) const = 0;

    /**
     * Returns the simulation time for the specified future clock time according
     * to the current state of the clock. The clock time may be the same for a
     * non-zero length of simulation interval. This method can return both lower
     * and upper bounds of the corresponding simulation interval. This method
     * implements a monotonic function with respect to the clock time argument.
     * It's allowed to return a different value for the same argument value if
     * the clock is set between calls. The time argument must be greater or than
     * equal to the current clock time, otherwise an error is raised. See
     * CLOCKTIME_AS_SIMTIME macro for simple type conversion.
     */
    virtual simtime_t computeSimTimeFromClockTime(clocktime_t time, bool lowerBound = true) const = 0;

    /**
     * Schedules an event to be delivered to the caller module (i.e. the context
     * module) at the specified clock time. The event is anchored to a specific
     * clock time value, so the actual simulation time when this event is executed
     * will be affected if the clock time is set later.
     */
    virtual void scheduleClockEventAt(clocktime_t time, ClockEvent *event) = 0;

    /**
     * Schedules an event to be delivered to the caller module (i.e. the context
     * module) after the given clock time delay has elapsed. The event is anchored
     * to a specific clock time duration, so the actual simulation time when this
     * event is executed is not affected if the clock time is set later. On the
     * other hand, setting the clock drift still affects the simulation time of
     * the event execution.
     */
    virtual void scheduleClockEventAfter(clocktime_t delay, ClockEvent *event) = 0;

    /**
     * Cancels a previously scheduled clock event. The clock event ownership is
     * transferred to the caller.
     */
    virtual ClockEvent *cancelClockEvent(ClockEvent *event) = 0;

    /**
     * Called by the clock event to be executed in the context of this clock.
     * This method is primarily useful for clock implementations to update their
     * internal data structures related to individual clock events.
     */
    virtual void handleClockEvent(ClockEvent *event) = 0;

    virtual bool isScheduledClockEvent(ClockEvent *event) const = 0;
};

} // namespace inet

#endif

