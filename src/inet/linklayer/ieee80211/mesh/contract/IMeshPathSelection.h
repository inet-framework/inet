// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef __INET_IMESHPATHSELECTION_H
#define __INET_IMESHPATHSELECTION_H

#include <optional>
#include "inet/linklayer/common/MacAddress.h"

namespace inet {
namespace ieee80211 {

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
};

} // namespace ieee80211
} // namespace inet
#endif
