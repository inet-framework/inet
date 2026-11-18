// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef __INET_IEEE80211AMPDURECEPTIONDECISION_H
#define __INET_IEEE80211AMPDURECEPTIONDECISION_H

#include "inet/physicallayer/wireless/common/radio/packetlevel/ReceptionDecision.h"
#include "inet/common/packet/chunk/Chunk.h"

namespace inet {
namespace physicallayer {

class INET_API Ieee80211AmpduReceptionDecision : public ReceptionDecision
{
  protected:
    const bool headerSuccessful;
    const bool contentComplete;
    const Ptr<const Chunk> data;

  public:
    Ieee80211AmpduReceptionDecision(const IReception *reception, IRadioSignal::SignalPart part,
            bool possible, bool attempted, bool successful, bool headerSuccessful, bool contentComplete,
            const Ptr<const Chunk>& data) :
        ReceptionDecision(reception, part, possible, attempted, successful),
        headerSuccessful(headerSuccessful), contentComplete(contentComplete), data(data) {}
    bool isHeaderSuccessful() const { return headerSuccessful; }
    bool isContentComplete() const { return contentComplete; }
    const Ptr<const Chunk>& getData() const { return data; }
};

} // namespace physicallayer
} // namespace inet

#endif
