// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef INET_IMESHRADIOCONTROL_H
#define INET_IMESHRADIOCONTROL_H

#include "inet/physicallayer/wireless/common/contract/packetlevel/IRadio.h"

namespace inet::ieee80211 {

/** Mesh policy queries copied physical facts; the MAC alone arbitrates modes. */
class INET_API IMeshRadioControl
{
  public:
    virtual ~IMeshRadioControl() = default;
    virtual void requestRadioReview() = 0;
    virtual std::optional<physicallayer::IRadio::RadioModeState> getMeshRadioState() const = 0;
    virtual std::optional<simtime_t> getMeshRadioSwitchingTime(
            physicallayer::IRadio::RadioMode from, physicallayer::IRadio::RadioMode to) const = 0;
    virtual bool hasMeshRadioActivity() const = 0;
    virtual bool isMeshPhyIdle() const = 0;
};

} // namespace inet::ieee80211
#endif
