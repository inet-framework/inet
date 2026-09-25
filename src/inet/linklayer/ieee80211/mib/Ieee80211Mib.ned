//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mib;

import inet.common.SimpleModule;

//
// Management Information Base for IEEE 802.11 networks. Stores configuration and
// state information including network mode (infrastructure, ad-hoc, mesh),
// station type (access point or station), authentication and association status,
// and BSS data (SSID, BSSID). Used by various IEEE 802.11 modules to maintain
// protocol state and control frame addressing.
//
simple Ieee80211Mib extends SimpleModule
{
    parameters:
        @class(Ieee80211Mib);
        // Model-backed subset of IEEE Std 802.11-2024 HT capability/operation state; not a full Annex C MIB.
        int htMaxAmpduLengthExponent = default(0); // maximum received A-MPDU length exponent (0..3)
        int htSecondaryChannelOffset = default(0); // BSS operation policy: 0=20 MHz, 1=40 MHz above, 3=40 MHz below; bounded by the mode set
        int htProtectionMode = default(0); // 0=none, 1=nonmember, 2=20 MHz, 3=non-HT mixed
        displayStringTextFormat = default("Address: {address}{ssidStr}\n{modeStr}{stationTypeStr}{qosStr}{associatedStr}");
        @display("i=block/table");
}
