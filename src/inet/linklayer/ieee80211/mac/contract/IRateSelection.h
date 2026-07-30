//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IRATESELECTION_H
#define __INET_IRATESELECTION_H

#include "inet/common/packet/Packet.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#include "inet/linklayer/ieee80211/mac/common/StationLabelCache.h"
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

    // Emits datarateSelected on behalf of a coordination function. Unicast data frames are tagged
    // with the receiver's station label as a named details object, so that a demux(datarateSelected)
    // result filter or the statistic bar chart visualizer can key a separate per-station series on
    // it; this mirrors the condition under which ~RateSelection applies a per-receiver configured
    // rate, so the label always names the station whose rate is reported. Control, management and
    // group-addressed frames carry no per-station data rate and are emitted without details: the
    // aggregate datarateSelected statistic still records them, a bar chart ignores them.
    static void emitDatarateSelected(cComponent *emitter, StationLabelCache& stationLabels, const Ptr<const Ieee80211MacHeader>& header, const physicallayer::IIeee80211Mode *mode);

  public:
    virtual ~IRateSelection() {}

    virtual const physicallayer::IIeee80211Mode *computeResponseCtsFrameMode(Packet *packet, const Ptr<const Ieee80211RtsFrame>& rtsFrame) = 0;
    virtual const physicallayer::IIeee80211Mode *computeResponseAckFrameMode(Packet *packet, const Ptr<const Ieee80211DataOrMgmtHeader>& dataOrMgmtHeader) = 0;

    virtual const physicallayer::IIeee80211Mode *computeMode(Packet *packet, const Ptr<const Ieee80211MacHeader>& header) = 0;
};

} // namespace ieee80211
} // namespace inet

#endif

