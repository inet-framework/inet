//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/power/contract/IEpEnergyGenerator.h"

namespace inet {

namespace power {

simsignal_t IEpEnergyGenerator::powerGenerationChangedSignal = cComponent::registerSignal("powerGenerationChanged");

} // namespace power

} // namespace inet

