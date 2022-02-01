//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_DS_H
#define __INET_DS_H

#include "inet/common/packet/Packet.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Mac.h"
#include "inet/linklayer/ieee80211/mac/contract/IDs.h"
#include "inet/linklayer/ieee80211/mib/Ieee80211Mib.h"

namespace inet {
namespace ieee80211 {

/**
 * The default implementation of IDs.
 */
class INET_API Ds : public cSimpleModule, public IDs
{
  protected:
    Ieee80211Mib *mib = nullptr;
    Ieee80211Mac *mac = nullptr;

  protected:
    virtual void initialize(int stage) override;

    /**
     * Utility function for APs: sends back a data frame we received from a
     * STA to the wireless LAN, after tweaking fromDS/toDS bits and shuffling
     * addresses as needed.
     */
    virtual void distributeDataFrame(Packet *frame, const Ptr<const Ieee80211DataOrMgmtHeader>& header);

  public:
    virtual void processDataFrame(Packet *frame, const Ptr<const Ieee80211DataHeader>& header) override;
};

} // namespace ieee80211
} // namespace inet

#endif

