//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/power/contract/IEpEnergySink.h"

namespace inet {

namespace power {

simsignal_t IEpEnergySink::powerGenerationChangedSignal = cComponent::registerSignal("powerGenerationChanged");

} // namespace power

} // namespace inet

