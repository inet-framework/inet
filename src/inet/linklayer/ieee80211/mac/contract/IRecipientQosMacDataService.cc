//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/contract/IRecipientQosMacDataService.h"

namespace inet {
namespace ieee80211 {

simsignal_t IRecipientQosMacDataService::packetDefragmentedSignal = cComponent::registerSignal("packetDefragmented");
simsignal_t IRecipientQosMacDataService::packetDeaggregatedSignal = cComponent::registerSignal("packetDeaggregated");

} // namespace ieee80211
} // namespace inet

