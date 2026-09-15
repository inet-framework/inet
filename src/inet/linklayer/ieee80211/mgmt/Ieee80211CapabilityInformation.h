//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_IEEE80211CAPABILITYINFORMATION_H
#define __INET_IEEE80211CAPABILITYINFORMATION_H

#include <cstdint>

namespace inet {
namespace ieee80211 {

// IEEE Std 802.11-2024, 9.4.1.4. ESS is defined only in Beacon/Probe Response.
constexpr uint16_t CAPABILITY_ESS = 1U << 0;
constexpr uint16_t CAPABILITY_QOS = 1U << 9;

} // namespace ieee80211
} // namespace inet

#endif
