//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/contract/IRecoveryProcedure.h"

namespace inet {
namespace ieee80211 {

simsignal_t IRecoveryProcedure::contentionWindowChangedSignal = cComponent::registerSignal("contentionWindowChanged");
simsignal_t IRecoveryProcedure::retryLimitReachedSignal = cComponent::registerSignal("retryLimitReached");

} // namespace ieee80211
} // namespace inet

