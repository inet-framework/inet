//
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ICLOCKSERVO_H
#define __INET_ICLOCKSERVO_H

#include "inet/clock/contract/IClock.h"
#include "inet/common/Units.h"

namespace inet {

using namespace units::values;

/**
 * Interface for clock servos that discipline an IClock against a reference.
 *
 * Terminology:
 * - C_this(t): the local clock time produced by the controlled clock.
 * - C_ref(t): the reference clock time (external, measured, or computed).
 * - timeDifference Δc := C_this(now) − C_ref(now).
 *     Δc > 0 ⇒ local clock is AHEAD of the reference (reads a larger time).
 * - rateDifference Δf_ppm := 10^6 * (f_this − f_ref) / f_ref, in parts per million (ppm).
 *     Δf_ppm > 0 ⇒ local clock runs FASTER than the reference.
 *
 * Purpose:
 * - Implementations should use the provided differences to steer the underlying IClock
 *   (e.g., by stepping, slewing, or adjusting drift) so that Δc → 0 and Δf_ppm → 0 over time.
 *
 * Notes:
 * - This interface does not prescribe the control law (PI/PLL/FLL, etc.). It only defines the
 *   measurement convention and the adjustment request entry point.
 * - A servo may combine the instantaneous Δc and Δf_ppm with its internal state (filters,
 *   loop gains, limiters) and then apply changes to the controlled clock (e.g., set time,
 *   change rate/drift).
 * - Implementations should avoid violating the IClock invariants (e.g., monotone behavior
 *   except at explicit steps) and should document whether they step or slew.
 */
class INET_API IClockServo
{
  public:
    virtual ~IClockServo() {}

    /**
     * Provide the servo with a new measurement of:
     *   - timeDifference (Δc): C_this(now) − C_ref(now)
     *       Δc > 0 ⇒ local clock is ahead; Δc < 0 ⇒ local clock is behind.
     *   - rateDifference (Δf_ppm): ppm-scale fractional frequency offset
     *       Δf_ppm > 0 ⇒ local clock runs faster; Δf_ppm < 0 ⇒ slower.
     *
     * Semantics:
     * - The servo should translate these differences into suitable adjustments on the
     *   controlled IClock (e.g., by rate/offset updates). The method can be called
     *   periodically or aperiodically whenever new estimates are available.
     *
     * Expectations:
     * - Implementations should be robust to noisy inputs and may internally filter Δc/Δf_ppm.
     * - If the servo elects to *step* time, it must preserve IClock’s documented behavior
     *   (e.g., well-defined left/right bounds at discontinuities).
     * - If it *slews* (rate correction), it should ensure that drift/rate parameters remain
     *   within documented bounds and that scheduled events behave as specified by IClock.
     */
    virtual void adjustClockForDifference(clocktime_t timeDifference, ppm rateDifference) = 0;
};

} // namespace inet

#endif
