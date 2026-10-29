// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef __INET_IEEE80211HTSIGNALFIELD_H
#define __INET_IEEE80211HTSIGNALFIELD_H

#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211PhyHeader_m.h"
#include "inet/physicallayer/wireless/ieee80211/mode/IIeee80211Mode.h"

namespace inet {
namespace physicallayer {

INET_API uint64_t packIeee80211HtSignalField(const Ieee80211HtPhyHeader& header);
INET_API uint8_t computeIeee80211HtSignalCrc(const Ieee80211HtPhyHeader& header);
INET_API bool isIeee80211HtSignalValid(const Ieee80211HtPhyHeader& header, const IIeee80211Mode *mode);

} // namespace physicallayer
} // namespace inet

#endif
