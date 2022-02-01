//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/power/contract/ICcEnergySource.h"

namespace inet {

namespace power {

simsignal_t ICcEnergySource::currentConsumptionChangedSignal = cComponent::registerSignal("currentConsumptionChanged");

} // namespace power

} // namespace inet

