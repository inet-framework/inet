//
// WiFi conformance suite -- 802.11ac (WiFi 5) VHT PHY mode.
//
// The defining WiFi-5 assertion: a transmitted QoS data frame uses a VHT mode at
// 80 MHz. Read via a C++ lambda over the Ieee80211ModeReq tag (see WifiTestSupport.h),
// since the PHY mode is an opaque pointer the PacketFilter string engine cannot follow.
//
// Required (mandatory) for 802.11ac. SPDX-License-Identifier: LGPL-3.0-or-later
//
#include "ProtocolTest.h"
#include "../common/WifiTestSupport.h"

namespace inet {
namespace protocoltest {

// 40 = QoS data frame (ST_DATA_WITH_QOS).
Define_ProtocolTest(wifi_11ac_vht_80mhz)
{
    return ProtocolTest("wifi_11ac_vht_80mhz")
        .once(on("sta1.wlan[0].mac").signal("packetSentToLower")
                  .packet("ieee80211mac.type == 40")
                  .match([](const MatchContext& c) { return isVht(c.event, MHz(80), 1); })
                  .describe("a data frame transmitted in VHT mode at 80 MHz")
                  .within(2.5));
}

} // namespace protocoltest
} // namespace inet
