//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/clock/contract/IOscillator.h"

namespace inet {

simsignal_t IOscillator::preOscillatorStateChangedSignal = cComponent::registerSignal("preOscillatorStateChanged");
simsignal_t IOscillator::postOscillatorStateChangedSignal = cComponent::registerSignal("postOscillatorStateChanged");

} // namespace inet

