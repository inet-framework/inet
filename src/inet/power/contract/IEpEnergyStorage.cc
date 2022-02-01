//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/power/contract/IEpEnergyStorage.h"

namespace inet {

namespace power {

simsignal_t IEpEnergyStorage::residualEnergyCapacityChangedSignal = cComponent::registerSignal("residualEnergyCapacityChanged");

} // namespace power

} // namespace inet

