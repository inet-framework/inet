//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/contract/IContention.h"

namespace inet {
namespace ieee80211 {

simsignal_t IContention::backoffPeriodGeneratedSignal = cComponent::registerSignal("backoffPeriodGenerated");
simsignal_t IContention::backoffStartedSignal = cComponent::registerSignal("backoffStarted");
simsignal_t IContention::backoffStoppedSignal = cComponent::registerSignal("backoffStopped");
simsignal_t IContention::channelAccessGrantedSignal = cComponent::registerSignal("channelAccessGranted");

} // namespace ieee80211
} // namespace inet

