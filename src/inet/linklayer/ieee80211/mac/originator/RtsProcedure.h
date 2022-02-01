//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_RTSPROCEDURE_H
#define __INET_RTSPROCEDURE_H

#include "inet/linklayer/ieee80211/mac/contract/IRtsProcedure.h"

namespace inet {
namespace ieee80211 {

class INET_API RtsProcedure : public IRtsProcedure
{
  protected:
    int numSentRts = 0;

  public:
    virtual const Ptr<Ieee80211RtsFrame> buildRtsFrame(const Ptr<const Ieee80211DataOrMgmtHeader>& dataOrMgmtHeader) const override;
    virtual void processTransmittedRts(const Ptr<const Ieee80211RtsFrame>& rtsFrame) override;
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif

