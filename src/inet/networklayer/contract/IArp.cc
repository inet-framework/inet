//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/networklayer/contract/IArp.h"

namespace inet {

Register_Abstract_Class(IArp::Notification);

const simsignal_t IArp::arpResolutionInitiatedSignal = cComponent::registerSignal("arpResolutionInitiated");
const simsignal_t IArp::arpResolutionCompletedSignal = cComponent::registerSignal("arpResolutionCompleted");
const simsignal_t IArp::arpResolutionFailedSignal = cComponent::registerSignal("arpResolutionFailed");

} // namespace inet

