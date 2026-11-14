// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef INET_IMESHPATHSELECTION_H
#define INET_IMESHPATHSELECTION_H

#include <optional>
#include "inet/linklayer/common/MacAddress.h"
#include "inet/linklayer/ieee80211/mgmt/Ieee80211MeshFrame_m.h"

namespace inet::ieee80211 {

struct INET_API MeshPath
{
    MacAddress nextHop;
    simtime_t expiry;
};

/** Path selection owns validation, initial lifetimes and precursor state.
 * Queries return snapshots; expiry is exclusive. A self destination still
 * requires precursor admission. Refreshing a precursor retains the later expiry.
 */
class INET_API IMeshPathSelection
{
  public:
    virtual ~IMeshPathSelection() = default;
    virtual std::optional<MeshPath> findPath(MacAddress destination) const = 0;
    virtual simtime_t getPrecursorExpiry(MacAddress destination, MacAddress precursor) const = 0;
    virtual void refreshPath(MacAddress destination) = 0;
    virtual void refreshPrecursor(MacAddress destination, MacAddress precursor) = 0;
    virtual void startDiscovery(MacAddress destination) = 0;
    virtual void cancelDiscovery(MacAddress destination) = 0;
    virtual void processHwmp(const Ptr<const Ieee80211HwmpFrame>& body, MacAddress transmitter, MacAddress receiver) = 0;
    virtual void reportUnknownTransit(MacAddress transmitter, MacAddress destination) = 0;
    virtual void sourcePathNeeded(MacAddress destination) = 0;
};

} // namespace inet::ieee80211
#endif
