//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.test.moduletest.lib.ieee80211retransmissiontest;

import inet.linklayer.ieee80211.mac.Ieee80211Mac;

module Ieee80211TesterMac extends Ieee80211Mac
{
    parameters:
        // A means accept
        // B means block
        string actions;

    @class(Ieee80211TesterMac);
}
