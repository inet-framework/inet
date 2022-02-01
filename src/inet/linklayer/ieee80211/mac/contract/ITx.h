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

    virtual void transmitFrame(Packet *packet, const Ptr<const Ieee80211MacHeader>& header, ICallback *callback) = 0;
    virtual void transmitFrame(Packet *packet, const Ptr<const Ieee80211MacHeader>& header, simtime_t ifs, ICallback *callback) = 0;
    virtual void radioTransmissionFinished() = 0;
};

} // namespace ieee80211
} // namespace inet

#endif

