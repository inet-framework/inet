//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IRATESELECTION_H
#define __INET_IRATESELECTION_H

#include "inet/common/packet/Packet.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#include "inet/physicallayer/wireless/ieee80211/mode/IIeee80211Mode.h"
#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211ModeSet.h"

namespace inet {
namespace ieee80211 {

/**
 * Abstract interface for rate selection. Rate selection decides what bit rate
 * (or MCS) should be used for any particular frame. The rules of rate selection
 * is described in the 802.11 specification in the section titled "Multirate Support".
 */
class INET_API IRateSelection
{
  public:
    static simsignal_t datarateSelectedSignal;

  public:
    virtual ~IRateSelection() {}

    virtual const physicallayer::IIeee80211Mode *computeResponseCtsFrameMode(Packet *packet, const Ptr<const Ieee80211RtsFrame>& rtsFrame) = 0;
    virtual const physicallayer::IIeee80211Mode *computeResponseAckFrameMode(Packet *packet, const Ptr<const Ieee80211DataOrMgmtHeader>& dataOrMgmtHeader) = 0;

    virtual const physicallayer::IIeee80211Mode *computeMode(Packet *packet, const Ptr<const Ieee80211MacHeader>& header) = 0;
};

} // namespace ieee80211
} // namespace inet

#endif

