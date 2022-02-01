//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/power/contract/ICcEnergyStorage.h"

namespace inet {

namespace power {

simsignal_t ICcEnergyStorage::residualChargeCapacityChangedSignal = cComponent::registerSignal("residualChargeCapacityChanged");

} // namespace power

} // namespace inet

