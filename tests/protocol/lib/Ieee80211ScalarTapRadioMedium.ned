//
// Protocol Test Framework for INET -- the IEEE 802.11 scalar radio medium as a tap.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.protocoltest;

import inet.physicallayer.wireless.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;

//
// Ieee80211ScalarRadioMedium with the interception rules of TapRadioMedium. Select it with
// one ini line: *.radioMedium.typename = "Ieee80211ScalarTapRadioMedium".
//
module Ieee80211ScalarTapRadioMedium extends Ieee80211ScalarRadioMedium
{
    parameters:
        @class(TapRadioMedium);
}
