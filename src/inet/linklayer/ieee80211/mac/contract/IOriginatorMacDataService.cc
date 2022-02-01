//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/contract/IOriginatorMacDataService.h"

namespace inet {
namespace ieee80211 {

simsignal_t IOriginatorMacDataService::packetFragmentedSignal = cComponent::registerSignal("packetFragmented");
simsignal_t IOriginatorMacDataService::packetAggregatedSignal = cComponent::registerSignal("packetAggregated");

} // namespace ieee80211
} // namespace inet

