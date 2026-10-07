//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ITX_H
#define __INET_ITX_H

#include "inet/common/packet/Packet.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"

namespace inet {
namespace ieee80211 {

/**
 * Abstract interface for unconditionally transmitting a frame immediately
 * or after waiting for a specified inter-frame space (usually SIFS).
 */
class INET_API ITx
{
  public:
    class INET_API ICallback {
      public:
        virtual ~ICallback() {}

        virtual void transmissionComplete(Packet *packet, const Ptr<const Ieee80211MacHeader>& header) = 0;
    };

  public:
    virtual ~ITx() {}

    /**
     * The caller retains the frame. Tx sets its addresses and FCS, then retains a copy until completion.
     * Pass updateLocalNav=true for holder frames. Pass updateLocalNav=false for recipient responses.
     *
     * Tx retains this choice through IFS and transmission, independently of the transmitted frame type.
     * Tx saves the completed choice before the callback, which can accept another transmission.
     * For example, a recipient CTS carries its duration without a local NAV update.
     *
     * IEEE Std 802.11-2024, 10.3.2.4 defines NAV updates from received frames.
     * A received frame addressed to the station does not update its NAV.
     * Clause 10.23.2.2 defines TXNAV, the transmitted reservation timer, from holder transmissions, except PS-Poll frames.
     * Clause 10.3.2.9 defines CTS responses when NAV indicates idle, with holder and PHY conditions.
     * Model simplification: INET uses the local NAV timer for holder reservations.
     * The CTS policy does not apply the exception for received reservations from the same TXOP holder.
     */
    virtual void transmitFrame(Packet *packet, const Ptr<const Ieee80211MacHeader>& header, bool updateLocalNav, ICallback *callback) = 0;
    virtual void transmitFrame(Packet *packet, const Ptr<const Ieee80211MacHeader>& header, simtime_t ifs, bool updateLocalNav, ICallback *callback) = 0;
    /**
     * The query returns true while Tx retains an accepted frame,
     * including any wait before transmission and the transmission itself.
     * Tx clears this state before it calls ICallback::transmissionComplete(),
     * so the callback can release pending radio commands.
     * The query returns false when Tx holds no accepted transmission.
     * For example, an accepted ACK with a SIFS delay keeps this query true until transmission ends.
     */
    [[nodiscard]] virtual bool hasTransmission() const = 0;
    virtual void radioTransmissionFinished() = 0;
};

} // namespace ieee80211
} // namespace inet

#endif
