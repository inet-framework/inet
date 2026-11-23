// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef INET_MESHPOWERMODE_H
#define INET_MESHPOWERMODE_H

#include "inet/common/INETDefs.h"

namespace inet::ieee80211 {

// IEEE Std 802.11-2024, 14.15.2.2. Smaller values mean higher activity.
enum class MeshPowerMode { ACTIVE = 0, LIGHT_SLEEP = 1, DEEP_SLEEP = 2 };

struct INET_API MeshPeerPowerState
{
    MeshPowerMode local = MeshPowerMode::ACTIVE;
    MeshPowerMode remote = MeshPowerMode::ACTIVE;
    bool operator==(const MeshPeerPowerState& other) const { return local == other.local && remote == other.remote; }
    friend std::ostream& operator<<(std::ostream& out, const MeshPeerPowerState& state) {
        return out << "local=" << int(state.local) << ", remote=" << int(state.remote);
    }
};

// Existing peering identifiers, not an independent transition generation.
using MeshPeerInstance = std::pair<uint16_t, uint16_t>;

} // namespace inet::ieee80211
#endif
