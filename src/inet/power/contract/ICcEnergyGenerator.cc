//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/power/contract/ICcEnergyGenerator.h"

namespace inet {

namespace power {

simsignal_t ICcEnergyGenerator::currentGenerationChangedSignal = cComponent::registerSignal("currentGenerationChanged");

} // namespace power

} // namespace inet

