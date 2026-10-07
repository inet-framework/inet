//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ITX_H
#define __INET_ITX_H

#include "inet/common/packet/Packet.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#include "inet/linklayer/ieee80211/mac/common/TxRequestId.h"

namespace inet {
namespace ieee80211 {

/**
 * Tx accepts an identified frame request. It transmits the frame immediately or after the specified interframe space (IFS). The callback checks permission before the request reaches the medium. Hypothetical: a permission callback replaces request A with request B. Tx rechecks identity and cannot transmit A after that replacement.
 */
class INET_API ITx
{
  public:
    class INET_API ICallback {
      public:
        virtual ~ICallback() {}

        virtual void beginCallback() = 0;
        virtual void endCallback() = 0;
        virtual bool isTransmissionPermitted(TxRequestId id) = 0;
        virtual void transmissionStarted(TxRequestId id) = 0;
        virtual void transmissionCanceled(TxRequestId id) = 0;
        virtual void transmissionComplete(TxRequestId id, Packet *packet, const Ptr<const Ieee80211MacHeader>& header) = 0;
    };

  public:
    virtual ~ITx() {}

    enum class Cancellation { CANCELED, TOO_LATE, NOT_FOUND };
    virtual void transmitFrame(TxRequestId id, Packet *packet, const Ptr<const Ieee80211MacHeader>& header, simtime_t ifs, ICallback *callback) = 0;
    virtual Cancellation cancelPendingTransmission(TxRequestId id) = 0;
    virtual void resetForLifecycle(uint64_t epoch) = 0;
    /**
     * The query returns true while Tx retains an accepted frame, including any wait before transmission and the transmission itself. Tx clears this state before it calls ICallback::transmissionComplete(), so the callback can release pending radio commands. The query returns false after completion, cancellation, or lifecycle reset releases the request. For example, an accepted ACK with a SIFS delay keeps this query true until transmission ends.
     */
    [[nodiscard]] virtual bool hasTransmission() const = 0;
    virtual void radioTransmissionFinished() = 0;
};

} // namespace ieee80211
} // namespace inet

#endif
