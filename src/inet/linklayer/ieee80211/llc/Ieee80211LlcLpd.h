//
// Copyright (C) 2018 Raphael Riebl, TH Ingolstadt
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_IEEE80211LLCLPD_H
#define __INET_IEEE80211LLCLPD_H

#include "inet/common/packet/Packet.h"
#include "inet/linklayer/ieee80211/llc/IIeee80211Llc.h"
#include "inet/linklayer/ieee8022/Ieee8022Llc.h"

namespace inet {
namespace ieee80211 {

class INET_API Ieee80211LlcLpd : public Ieee8022Llc, public IIeee80211Llc
{
  public:
    const Protocol *getProtocol() const override;
};

} // namespace ieee80211
} // namespace inet

#endif

