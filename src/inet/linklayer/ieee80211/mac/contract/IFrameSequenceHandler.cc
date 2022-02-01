//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/contract/IFrameSequenceHandler.h"

namespace inet {
namespace ieee80211 {

simsignal_t IFrameSequenceHandler::frameSequenceStartedSignal = cComponent::registerSignal("frameSequenceStarted");
simsignal_t IFrameSequenceHandler::frameSequenceFinishedSignal = cComponent::registerSignal("frameSequenceFinished");

} // namespace ieee80211
} // namespace inet

