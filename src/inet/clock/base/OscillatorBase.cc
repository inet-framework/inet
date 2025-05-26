//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/clock/base/OscillatorBase.h"

namespace inet {

simsignal_t OscillatorBase::driftRateChangedSignal = cComponent::registerSignal("driftRateChanged");

void OscillatorBase::initialize(int stage)
{
    SimpleModule::initialize(stage);
}

std::string OscillatorBase::resolveDirective(char directive) const
{
    switch (directive) {
        case 'n':
            return getNominalTickLength().str() + " s";
        case 'o':
            return getComputationOrigin().str() + " s";
        default:
            return SimpleModule::resolveDirective(directive);
    }
}

} // namespace inet

