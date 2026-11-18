//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_MPDUDEAGGREGATION_H
#define __INET_MPDUDEAGGREGATION_H

#include "inet/linklayer/ieee80211/mac/contract/IMpduDeaggregation.h"

namespace inet {
namespace ieee80211 {

class INET_API MpduDeaggregation : public IMpduDeaggregation, public cObject
{
  public:
    /**
     * Visits the A-MPDU subframes (IEEE Std 802.11-2024, 9.7.1) of the PSDU at
     * offset and length in packet, in packet order. Each subframe gives its MPDU
     * delimiter to visitDelimiter, and its MPDU, if present, to visitMpdu.
     * Offsets are relative to the PSDU, without the PHY header or PHY padding.
     * Delimiter validity includes chunk flags and member bounds. It does not
     * establish MPDU FCS acceptance. Returns false if a delimiter, bounds, or
     * alignment check fails.
     * After an invalid delimiter, the search continues at four-octet intervals.
     */
    static bool visitSubframes(const Packet *packet, B offset, B length,
            const std::function<void(const Ptr<const Ieee80211MpduSubframeHeader>&, B, bool)>& visitDelimiter,
            const std::function<void(const Ptr<const Chunk>&, B)>& visitMpdu);

    virtual std::vector<Packet *> *deaggregateFrame(Packet *frame) override;
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif

