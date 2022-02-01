//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/mobility/contract/IMobility.h"

namespace inet {

simsignal_t IMobility::mobilityStateChangedSignal = cComponent::registerSignal("mobilityStateChanged");

} // namespace inet

