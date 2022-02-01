//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/contract/IRateSelection.h"

namespace inet {
namespace ieee80211 {

simsignal_t IRateSelection::datarateSelectedSignal = cComponent::registerSignal("datarateSelected");

} // namespace ieee80211
} // namespace inet

