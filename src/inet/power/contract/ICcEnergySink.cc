//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/power/contract/ICcEnergySink.h"

namespace inet {

namespace power {

simsignal_t ICcEnergySink::currentGenerationChangedSignal = cComponent::registerSignal("currentGenerationChanged");

} // namespace power

} // namespace inet

