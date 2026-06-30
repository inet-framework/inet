//
// WiFi conformance suite -- cross-generation: infrastructure association FSM.
//
// 802.11 connection setup, observed at the STA MAC: the STA receives a Beacon from
// the AP, then completes open-system Authentication and Association. The frames are
// identical across b/a/g/n/ac, so this runs on any generation's config.
//
// Required (mandatory) behavior. SPDX-License-Identifier: LGPL-3.0-or-later
//
#include "ProtocolTest.h"

namespace inet {
namespace protocoltest {

// Frame type (combined type+subtype byte) values used here:
//   0 = Association Request, 1 = Association Response, 8 = Beacon, 11 = Authentication.
Define_ProtocolTest(wifi_association)
{
    return ProtocolTest("wifi_association")
        .once(on("sta1.wlan[0].mac").signal("packetReceivedFromLower")
                  .packet("ieee80211mac.type == 8").describe("a Beacon from the AP").within(2.0))
        .once(on("sta1.wlan[0].mac").signal("packetSentToLower")
                  .packet("ieee80211mac.type == 11").describe("an Authentication request").within(2.0))
        .once(on("sta1.wlan[0].mac").signal("packetReceivedFromLower")
                  .packet("ieee80211mac.type == 11").describe("the Authentication response").within(2.0))
        .once(on("sta1.wlan[0].mac").signal("packetSentToLower")
                  .packet("ieee80211mac.type == 0").describe("an Association Request").within(2.0))
        .once(on("sta1.wlan[0].mac").signal("packetReceivedFromLower")
                  .packet("ieee80211mac.type == 1").describe("the Association Response").within(2.0));
}

} // namespace protocoltest
} // namespace inet
