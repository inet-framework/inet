//
// Copyright (C) 2018 Raphael Riebl, TH Ingolstadt
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/linklayer/ieee80211/llc/Ieee80211LlcLpd.h"

#include "inet/common/ProtocolGroup.h"

namespace inet {
namespace ieee80211 {

Define_Module(Ieee80211LlcLpd);

const Protocol *Ieee80211LlcLpd::getProtocol() const
{
    return &Protocol::ieee8022llc;
}

} // namespace ieee80211
} // namespace inet

