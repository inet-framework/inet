//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/power/contract/IEpEnergyConsumer.h"

namespace inet {

namespace power {

simsignal_t IEpEnergyConsumer::powerConsumptionChangedSignal = cComponent::registerSignal("powerConsumptionChanged");

} // namespace power

} // namespace inet

