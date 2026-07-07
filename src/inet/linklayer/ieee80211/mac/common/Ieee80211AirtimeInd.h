//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IEEE80211AIRTIMEIND_H
#define __INET_IEEE80211AIRTIMEIND_H

#include "inet/common/INETDefs.h"
#include "inet/linklayer/common/MacAddress.h"

namespace inet {
namespace ieee80211 {

/**
 * Carries the on-air time actually consumed by a transmitted frame, together with
 * the receiver it was sent to. Emitted by the coordination function (~Dcf, ~Hcf)
 * as the object value of the `frameTransmittedAirtime` signal, once per on-air
 * transmission (so retries are counted), and consumed by ~AirtimeFairnessQueue to
 * charge the receiver's airtime deficit a-posteriori.
 */
class INET_API Ieee80211AirtimeInd : public cObject
{
  public:
    MacAddress receiverAddress;
    simtime_t airtime = SIMTIME_ZERO;

  public:
    Ieee80211AirtimeInd() {}
    Ieee80211AirtimeInd(const MacAddress& receiverAddress, simtime_t airtime) :
        receiverAddress(receiverAddress), airtime(airtime) {}

    virtual Ieee80211AirtimeInd *dup() const override { return new Ieee80211AirtimeInd(*this); }

    virtual std::string str() const override
    {
        std::stringstream out;
        out << "receiver=" << receiverAddress << ", airtime=" << airtime;
        return out.str();
    }
};

} // namespace ieee80211
} // namespace inet

#endif
