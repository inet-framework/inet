//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.networks.base;

network Ieee80211NetworkBase extends WirelessNetworkBase
{
    parameters:
        radioMedium.typename = default("Ieee80211ScalarRadioMedium");
}
