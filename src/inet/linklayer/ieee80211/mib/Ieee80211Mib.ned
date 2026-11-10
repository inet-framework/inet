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
        // True after a committed change to rates, HT/VHT capabilities, or channel operation.
        // Equal updates emit nothing. Listeners query the MIB for current state.
        @signal[rateStateChanged](type=bool);
        @signal[meshPeerChanged](type=bool); // details: MeshPeerChange; state/rates committed before emission
        // Model-backed subset of IEEE Std 802.11-2024 HT capability/operation state; not a full Annex C MIB.
        int htMaxAmpduLengthExponent = default(0); // maximum received A-MPDU length exponent (0..3)
        int htSecondaryChannelOffset = default(0); // BSS operation policy: 0=20 MHz, 1=40 MHz above, 3=40 MHz below; bounded by the mode set
        int htProtectionMode = default(0); // 0=none, 1=nonmember, 2=20 MHz, 3=non-HT mixed
        int meshRxSpatialStreams = default(-1); // -1: antenna/catalog limit; opt-in mesh capability ceiling
        int meshTxSpatialStreams = default(-1);
        int meshHtRxMaxMcs = default(7); // per-stream ceiling;0..7
        int meshHtTxMaxMcs = default(7);
        object meshVhtRxMcsMap = default([]); // optional8 entries:-1,7,8,9; bounded by catalog/antennas
        object meshVhtTxMcsMap = default([]);
        bool meshShortGuardInterval = default(true); // advertised receive support
        bool meshShortGuardIntervalTransmissionEnabled = default(true); // local Tx activation, independently of advertised Rx support
        bool meshVht160MhzCapable = default(false); // optional device capability, independent of current operating allocation
        int meshMaxAmsduLength = default(3839);
        int meshMinimumMpduStartSpacing = default(0);
        int meshVhtMaxMpduLength = default(3895);
        int meshVhtMaxAmpduLengthExponent = default(0);
        int meshVhtBasicMcsMap = default(65532); // NSS1 MCS0..7, otherNSS not required
        displayStringTextFormat = default("Address: {address}{ssidStr}\n{modeStr}{stationTypeStr}{qosStr}{associatedStr}");
        @display("i=block/table");
}
