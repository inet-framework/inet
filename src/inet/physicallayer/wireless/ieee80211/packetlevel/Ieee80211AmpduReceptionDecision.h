// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef __INET_IEEE80211AMPDURECEPTIONDECISION_H
#define __INET_IEEE80211AMPDURECEPTIONDECISION_H

#include "inet/physicallayer/wireless/common/radio/packetlevel/ReceptionDecision.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211Tag_m.h"

namespace inet {
namespace physicallayer {

class INET_API Ieee80211AmpduReceptionDecision : public ReceptionDecision
{
  protected:
    const bool headerSuccessful;
    const bool contentComplete;
    const std::vector<Ieee80211AmpduRange> ranges;

  public:
    Ieee80211AmpduReceptionDecision(const IReception *reception, IRadioSignal::SignalPart part,
            bool possible, bool attempted, bool successful, bool headerSuccessful, bool contentComplete,
            const std::vector<Ieee80211AmpduRange>& ranges) :
        ReceptionDecision(reception, part, possible, attempted, successful),
        headerSuccessful(headerSuccessful), contentComplete(contentComplete), ranges(ranges) {}
    bool isHeaderSuccessful() const { return headerSuccessful; }
    bool isContentComplete() const { return contentComplete; }
    const std::vector<Ieee80211AmpduRange>& getRanges() const { return ranges; }
};

} // namespace physicallayer
} // namespace inet

#endif
