//
// Copyright (C) 2018 Raphael Riebl, TH Ingolstadt
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_IEEE80211LLCEPD_H
#define __INET_IEEE80211LLCEPD_H

#include "inet/linklayer/ieee802/Ieee802Epd.h"
#include "inet/linklayer/ieee80211/llc/IIeee80211Llc.h"

namespace inet {
namespace ieee80211 {

class INET_API Ieee80211LlcEpd : public Ieee802Epd, public IIeee80211Llc
{
  public:
    const Protocol *getProtocol() const override;
};

} // namespace ieee80211
} // namespace inet

#endif

