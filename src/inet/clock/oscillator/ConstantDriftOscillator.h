//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_CONSTANTDRIFTOSCILLATOR_H
#define __INET_CONSTANTDRIFTOSCILLATOR_H

#include "inet/clock/base/DriftingOscillatorBase.h"

namespace inet {

/**
 * @brief Oscillator with a constant fractional frequency offset (drift).
 *
 * Overview
 * --------
 * Provides a strictly periodic tick sequence whose rate is a fixed factor faster
 * or slower than the nominal tick length. No per-tick events are generated; the
 * ~IOscillator mappings N(Δt)/I(n) are computed analytically in the base.
 *
 * Semantics
 * ---------
 * - Let L be the nominal tick length and r be the configured drift (ppm).
 * - Effective factor: d ≈ 1 + r·1e−6  (r > 0 ⇒ faster; r < 0 ⇒ slower).
 * - Current tick length: L_current = L / d.
 * - Phase/Origin handling follows DriftingOscillatorBase; the first tick strictly
 *   after the origin is preserved across (re)initialization.
 *
 * Configuration
 * -------------
 * - Reads `driftRate` (ppm) from the NED parameter during initialize() and applies
 *   it via setDriftRate(). After initialization the drift remains constant unless
 *   changed explicitly from C++/scenario.
 *
 * Notes
 * -----
 * - Signals about rate changes are emitted by the base when setDriftRate() updates d.
 * - Use the inherited `tickOffset` parameter to set the initial phase.
 */
class INET_API ConstantDriftOscillator : public DriftingOscillatorBase
{
  protected:
    /**
     * Reads `driftRate` (ppm) and applies it via setDriftRate(). Also chains to
     * the base to establish origin/phase and any internal housekeeping expected
     * by DriftingOscillatorBase.
     */
    virtual void initialize(int stage) override;
};

} // namespace inet

#endif
