//
// Protocol Test Framework for INET -- a radio medium that is a tap.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.protocoltest;

import inet.physicallayer.wireless.common.medium.RadioMedium;

//
// A radio medium that applies the interception rules of a test program to the frames on
// the air, as a PacketTap does on a wired link (see TapRadioMedium.h). A test names it with
// tap("radioMedium"). Every radio medium type uses the C++ class of RadioMedium, so a test
// that needs another medium type declares a NED type of one line that extends that type
// with @class(TapRadioMedium), as Ieee80211ScalarTapRadioMedium does.
//
module TapRadioMedium extends RadioMedium
{
    parameters:
        @class(TapRadioMedium);
}
