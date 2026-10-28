//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IEEE80211RADIOTAPPCAPRECORDERHELPER_H
#define __INET_IEEE80211RADIOTAPPCAPRECORDERHELPER_H

#include "inet/common/packet/recorder/PcapRecorder.h"
#include "inet/physicallayer/wireless/ieee80211/radiotap/Ieee80211RadiotapHeader_m.h"

namespace inet {

namespace physicallayer {

/**
 * Lets a PcapRecorder write IEEE 802.11 frames as Radiotap records (link type
 * 127). The Radiotap header shows the PHY values of the frame: the data rate
 * or the MCS, the channel, the signal power or the transmit power, and the FCS
 * flags. The helper reads the values of a received frame from the indication
 * tags of the receiver's copy, and the values of a sent frame from the request
 * tags that Ieee80211Radio completes. The header exists only in the record.
 */
class INET_API Ieee80211RadiotapPcapRecorderHelper : public cObject, public PcapRecorder::IHelper
{
  protected:
    virtual Ptr<Ieee80211RadiotapHeader> createHeader(const Packet *packet, Direction direction) const;

  public:
    virtual PcapLinkType protocolToLinkType(const Protocol *protocol) const override;
    virtual bool matchesLinkType(PcapLinkType pcapLinkType, const Protocol *protocol) const override;
    virtual Packet *tryConvertToLinkType(const Packet *packet, b frontOffset, b backOffset, PcapLinkType pcapLinkType, const Protocol *protocol, Direction direction) const override;
};

} // namespace physicallayer

} // namespace inet

#endif
