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
 */
class INET_API ConstantDriftOscillator : public DriftingOscillatorBase
{
  protected:
    virtual void initialize(int stage) override;
};

} // namespace inet

#endif
