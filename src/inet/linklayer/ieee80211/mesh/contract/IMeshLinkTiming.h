// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef __INET_IMESHLINKTIMING_H
#define __INET_IMESHLINKTIMING_H

#include "inet/linklayer/common/MacAddress.h"
#include "inet/common/Units.h"
#include "inet/physicallayer/wireless/ieee80211/mode/IIeee80211Mode.h"

namespace inet {
namespace ieee80211 {

struct Ieee80211RateSetState;
struct Ieee80211PeerPhyCapabilities;

struct INET_API MeshLinkTiming
{
    units::values::bps rate;
    simtime_t overhead;
    const physicallayer::IIeee80211Mode *dataMode;
    const physicallayer::IIeee80211Mode *ackMode;
};

/** Read-only nominal DATA/ACK selection and initial BE access overhead. */
class INET_API IMeshLinkTiming
{
  public:
    virtual ~IMeshLinkTiming() = default;
    virtual MeshLinkTiming getMeshLinkTiming(MacAddress peer) = 0;
    virtual MeshLinkTiming previewMeshLinkTiming(MacAddress peer, const Ieee80211RateSetState& rates,
        const Ieee80211PeerPhyCapabilities& capabilities) = 0;
};

} // namespace ieee80211
} // namespace inet
#endif
