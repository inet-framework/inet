//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/clock/oscillator/ConstantDriftOscillator.h"

namespace inet {

Define_Module(ConstantDriftOscillator);

void ConstantDriftOscillator::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL)
        driftRate = ppm(par("driftRate"));
    DriftingOscillatorBase::initialize(stage);
}

} // namespace inet

