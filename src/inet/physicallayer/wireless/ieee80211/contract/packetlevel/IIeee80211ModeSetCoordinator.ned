//
// Copyright (C) 2026 INET Framework contributors
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.physicallayer.wireless.ieee80211.contract.packetlevel;

// Coordinates registered MAC/PHY state consumers before publishing a mode-set fact.
moduleinterface IIeee80211ModeSetCoordinator
{
    parameters:
        @signal[modesetChanged](type=inet::physicallayer::Ieee80211ModeSet);
}
