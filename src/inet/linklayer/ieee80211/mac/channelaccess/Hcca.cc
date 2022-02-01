//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/channelaccess/Hcca.h"

namespace inet {
namespace ieee80211 {

Define_Module(Hcca);

void Hcca::initialize(int stage)
{
}

bool Hcca::isOwning()
{
    return false;
}

void Hcca::requestChannel(IChannelAccess::ICallback *callback)
{
    throw cRuntimeError("Unimplemented!");
}

void Hcca::releaseChannel(IChannelAccess::ICallback *callback)
{
    throw cRuntimeError("Unimplemented!");
}

} // namespace ieee80211
} // namespace inet

