//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_EDCA_H
#define __INET_EDCA_H

#include "inet/linklayer/ieee80211/mac/channelaccess/Edcaf.h"
#include "inet/linklayer/ieee80211/mac/common/Ieee80211Defs.h"
#include "inet/linklayer/ieee80211/mac/lifetime/EdcaTransmitLifetimeHandler.h"
#include "inet/linklayer/ieee80211/mac/originator/NonQosRecoveryProcedure.h"

namespace inet {
namespace ieee80211 {

/**
 * Implements IEEE 802.11 Enhanced Distributed Channel Access.
 */
class INET_API Edca : public cSimpleModule
{
  protected:
    int numEdcafs = -1;
    Edcaf **edcafs = nullptr;
    EdcaTransmitLifetimeHandler *lifetimeHandler = nullptr;
    NonQosRecoveryProcedure *mgmtAndNonQoSRecoveryProcedure = nullptr;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;

    virtual AccessCategory mapTidToAc(Tid tid);

  public:
    virtual ~Edca();

    virtual AccessCategory classifyFrame(const Ptr<const Ieee80211DataHeader>& header);
    virtual Edcaf *getEdcaf(AccessCategory ac) const { return edcafs[ac]; }
    virtual Edcaf *getChannelOwner();
    virtual std::vector<Edcaf *> getInternallyCollidedEdcafs();
    virtual NonQosRecoveryProcedure *getMgmtAndNonQoSRecoveryProcedure() const { return mgmtAndNonQoSRecoveryProcedure; }

    virtual void requestChannelAccess(AccessCategory ac, IChannelAccess::ICallback *callback);
    virtual void releaseChannelAccess(AccessCategory ac, IChannelAccess::ICallback *callback);
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif

