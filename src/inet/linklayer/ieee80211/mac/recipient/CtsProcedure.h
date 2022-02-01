//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_CTSPROCEDURE_H
#define __INET_CTSPROCEDURE_H

#include "inet/linklayer/ieee80211/mac/contract/ICtsProcedure.h"

namespace inet {
namespace ieee80211 {

/*
 * This class implements 9.3.2.6 CTS procedure
 */
class INET_API CtsProcedure : public ICtsProcedure
{
  protected:
    int numReceivedRts = 0;
    int numSentCts = 0;

  protected:
    virtual const Ptr<Ieee80211CtsFrame> buildCts(const Ptr<const Ieee80211RtsFrame>& rtsFrame) const;

  public:
    virtual void processReceivedRts(Packet *rtsPacket, const Ptr<const Ieee80211RtsFrame>& rtsFrame, ICtsPolicy *ctsPolicy, IProcedureCallback *callback) override;
    virtual void processTransmittedCts(const Ptr<const Ieee80211CtsFrame>& ctsFrame) override;
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif

