//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_IEEE80211RADIOTAPPCAPCAPTUREADAPTER_H
#define __INET_IEEE80211RADIOTAPPCAPCAPTUREADAPTER_H

#include "inet/common/packet/recorder/IPcapCaptureAdapter.h"

namespace inet {
namespace ieee80211 {

class INET_API Ieee80211RadiotapPcapCaptureAdapter : public IPcapCaptureAdapter
{
  public:
    virtual PcapLinkType getLinkType() const override { return LINKTYPE_IEEE802_11_RADIOTAP; }
    virtual std::optional<std::pair<b, b>> tryResolvePacket(const Packet *packet, b frontOffset, b backOffset) const override;
    virtual std::vector<PcapCaptureRecord> createRecords(const PcapCaptureObservation& observation, b frontOffset, b backOffset) const override;
};

} // namespace ieee80211
} // namespace inet

#endif
