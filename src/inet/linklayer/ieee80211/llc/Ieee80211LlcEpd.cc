//
// Copyright (C) 2018 Raphael Riebl, TH Ingolstadt
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/linklayer/ieee80211/llc/Ieee80211LlcEpd.h"

#include "inet/common/ProtocolGroup.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/Simsignals.h"
#include "inet/common/Simsignals_m.h"
#include "inet/linklayer/ieee802/Ieee802EpdHeader_m.h"

namespace inet {
namespace ieee80211 {

Define_Module(Ieee80211LlcEpd);

const Protocol *Ieee80211LlcEpd::getProtocol() const
{
    return &Protocol::ieee802epd;
}

} // namespace ieee80211
} // namespace inet

