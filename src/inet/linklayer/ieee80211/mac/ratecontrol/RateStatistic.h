//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_RATESTATISTIC_H
#define __INET_RATESTATISTIC_H

#include "inet/common/INETMath.h"
#include "inet/common/SimpleModule.h"
#include "inet/linklayer/common/MacAddress.h"

namespace inet {
namespace ieee80211 {

/**
 * Records the IEEE 802.11 data rate currently used towards a single peer
 * station. A rate control module (see RateControlBase) creates one instance
 * per receiver when per-station rate recording is enabled, and calls setRate()
 * whenever the rate to that station changes. Each instance emits its own
 * datarateChanged signal, so the data rate to each station becomes a separate,
 * plottable statistic. See the corresponding NED file for more details.
 */
class INET_API RateStatistic : public SimpleModule
{
  protected:
    static simsignal_t datarateChangedSignal;

    MacAddress peerAddress; // the station this submodule tracks the rate towards
    double currentRate = NaN; // most recently reported data rate to the peer, in bps

  protected:
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void refreshDisplay() const override;

  public:
    void setPeerAddress(const MacAddress& address) { peerAddress = address; }

    // Reports the current data rate (bps) to the peer; emits datarateChanged when it changes.
    void setRate(double bitrate);
};

} // namespace ieee80211
} // namespace inet

#endif
