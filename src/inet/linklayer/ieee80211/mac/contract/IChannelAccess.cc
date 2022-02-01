//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/contract/IChannelAccess.h"

namespace inet {
namespace ieee80211 {

simsignal_t IChannelAccess::channelOwnershipChangedSignal = cComponent::registerSignal("channelOwnershipChanged");

} // namespace ieee80211
} // namespace inet

