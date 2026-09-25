// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef __INET_TXOPADMISSIONDETAILS_H
#define __INET_TXOPADMISSIONDETAILS_H

#include "inet/linklayer/ieee80211/mac/common/AccessCategory.h"
#include "inet/linklayer/ieee80211/mac/common/TxopExchangePlan.h"

namespace inet {
namespace ieee80211 {

// The publisher owns this object. Listeners must copy values before the callback returns.
class INET_API TxopAdmissionDetails : public cObject
{
  private:
    const AccessCategory accessCategory;
    const TxopFrameIdentity identity;
    const TxopAdmissionDecision decision;
    const int fragmentNumber = -1;
    const int64_t packetId = -1;

  public:
    TxopAdmissionDetails() : accessCategory(AC_BE) {}
    TxopAdmissionDetails(AccessCategory ac, const TxopExchangePlan& plan) :
        accessCategory(ac), identity(plan.identity), decision(plan.admission), fragmentNumber(plan.fragmentNumber), packetId(plan.packetId) {}
    AccessCategory getAccessCategory() const { return accessCategory; }
    const TxopFrameIdentity& getIdentity() const { return identity; }
    const TxopAdmissionDecision& getDecision() const { return decision; }
    int getFragmentNumber() const { return fragmentNumber; }
    int64_t getPacketId() const { return packetId; }
};

} // namespace ieee80211
} // namespace inet

#endif
