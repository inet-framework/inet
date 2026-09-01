//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_IFRAMETRANSMISSIONCALLBACK_H
#define __INET_IFRAMETRANSMISSIONCALLBACK_H

#include "inet/common/packet/Packet.h"

namespace inet {
namespace ieee80211 {

/**
 * Reports terminal outcomes for locally originated frame transmissions.
 *
 * The result is created synchronously by the MAC and is valid only for the
 * duration of the callback. In particular, the frame pointer must not be
 * retained by the receiver: the coordination function may release the frame
 * immediately after the callback returns.
 */
class INET_API IFrameTransmissionCallback
{
  public:
    enum class Status {
        ACKNOWLEDGED,
        RETRY_LIMIT_REACHED,
        DROPPED_BEFORE_TRANSMISSION,
    };

    class INET_API Result final
    {
      private:
        const Packet * const frame;
        const Status status;

      public:
        Result(const Packet *frame, Status status) :
            frame(frame),
            status(status)
        {}

        const Packet *getFrame() const { return frame; }
        Status getStatus() const { return status; }
    };

  public:
    virtual ~IFrameTransmissionCallback() {}

    virtual void frameTransmissionFinished(const Result& result) = 0;
};

} // namespace ieee80211
} // namespace inet

#endif
