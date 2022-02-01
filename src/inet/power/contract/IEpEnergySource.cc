//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/power/contract/IEpEnergySource.h"

namespace inet {

namespace power {

simsignal_t IEpEnergySource::powerConsumptionChangedSignal = cComponent::registerSignal("powerConsumptionChanged");

} // namespace power

} // namespace inet

