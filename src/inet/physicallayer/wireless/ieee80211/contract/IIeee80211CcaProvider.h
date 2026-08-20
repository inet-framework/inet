//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_IIEEE80211CCAPROVIDER_H
#define __INET_IIEEE80211CCAPROVIDER_H

#include "inet/common/INETDefs.h"

namespace inet {
namespace physicallayer {

class INET_API Ieee80211CcaSnapshot : public cObject
{
  protected:
    bool ht40 = false;
    bool primaryBusy = false;
    bool secondaryBusy = false;

  public:
    Ieee80211CcaSnapshot(bool ht40 = false, bool primaryBusy = false, bool secondaryBusy = false) :
        ht40(ht40), primaryBusy(primaryBusy), secondaryBusy(secondaryBusy) {}

    virtual Ieee80211CcaSnapshot *dup() const override { return new Ieee80211CcaSnapshot(*this); }

    bool isHt40() const { return ht40; }
    bool isPrimaryBusy() const { return primaryBusy; }
    bool isSecondaryBusy() const { return secondaryBusy; }
};

/**
 * Provides the local IEEE 802.11 per-channel PHY-CCA state. The snapshot is
 * local PHY/MAC control information and is not transmitted on air.
 */
class INET_API IIeee80211CcaProvider
{
  public:
    static simsignal_t ccaStateChangedSignal;

    virtual ~IIeee80211CcaProvider() {}

    virtual const Ieee80211CcaSnapshot& getCcaSnapshot() const = 0;
};

} // namespace physicallayer
} // namespace inet

#endif
