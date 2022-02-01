//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ICTSPROCEDURE_H
#define __INET_ICTSPROCEDURE_H

#include "inet/common/packet/Packet.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#include "inet/linklayer/ieee80211/mac/contract/ICtsPolicy.h"
#include "inet/linklayer/ieee80211/mac/contract/IProcedureCallback.h"

namespace inet {
namespace ieee80211 {

class INET_API ICtsProcedure
{
  public:
    virtual ~ICtsProcedure() {}

    virtual void processReceivedRts(Packet *rtsPacket, const Ptr<const Ieee80211RtsFrame>& rtsFrame, ICtsPolicy *ctsPolicy, IProcedureCallback *callback) = 0;
    virtual void processTransmittedCts(const Ptr<const Ieee80211CtsFrame>& ctsFrame) = 0;
};

} // namespace ieee80211
} // namespace inet

#endif

