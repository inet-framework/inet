// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef INET_IMESHPOWERBUFFER_H
#define INET_IMESHPOWERBUFFER_H

#include "inet/linklayer/common/MacAddress.h"
#include "inet/common/Units.h"

namespace inet::ieee80211 {

struct INET_API MeshBufferedFrame
{
    enum class Kind { PAYLOAD = 0, MANAGEMENT = 1, POWER_CONTROL = 2, TIMING = 3, PROBE_RESPONSE = 4 };
    int64_t id;
    MacAddress receiver;
    b length;
    simtime_t deadline;
    int accessCategory;
    Kind kind;
};

/** The MAC queue owner supplies copied logical inventory and retains all packets. */
class INET_API IMeshPowerBuffer
{
  public:
    virtual ~IMeshPowerBuffer() = default;
    virtual std::vector<MeshBufferedFrame> getMeshBufferedFrames() const = 0;
    virtual void meshPowerEligibilityChanged() = 0;
    // Retire logical membership of an indication. On-air storage remains
    // borrowed until real completion; its PPDU is never cancelled.
    virtual void withdrawMeshPowerControl(int64_t packetId) = 0;
    virtual simtime_t getGroupDeliveryIdleTime() const = 0;
};

} // namespace inet::ieee80211
#endif
