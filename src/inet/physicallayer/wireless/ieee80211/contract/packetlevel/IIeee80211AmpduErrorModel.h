// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef __INET_IIEEE80211AMPDUERRORMODEL_H
#define __INET_IIEEE80211AMPDUERRORMODEL_H

#include "inet/physicallayer/wireless/common/contract/packetlevel/ISnir.h"
#include "inet/physicallayer/wireless/ieee80211/mode/IIeee80211Mode.h"

namespace inet {
namespace physicallayer {

class INET_API IIeee80211AmpduErrorModel
{
  public:
    virtual ~IIeee80211AmpduErrorModel() = default;
    // This query calculates no probability and consumes no random number.
    virtual bool supportsAmpduMode(const IIeee80211Mode *mode) const = 0;
    // Invalid input and unsupported modes cause a runtime error. The result is in [0, 1].
    virtual double computeAmpduRangeSuccessRate(const ISnir *snir, const IIeee80211Mode *mode, B length) const = 0;
};

} // namespace physicallayer
} // namespace inet

#endif
