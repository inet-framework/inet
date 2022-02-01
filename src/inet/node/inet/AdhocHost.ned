//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.node.inet;

//
// A wireless host containing routing, mobility and energy components. Supports
// IPv4 network protocol, TCP, UDP, and SCTP as transport protocol.
// This is a typical mobile node which can participate in adhoc routing
// and may have TCP/UDP applications installed. Supports ICMP (ping) too.
//
// - By default contains a single wireless card, however it can be configured
//   by the numWlanInterfaces parameter. Wirless card type is configured by the
//   **.wlan[*].typename parameter. see:  inet.linklayer.ieee80211 or other
//   modules implementing ~IWirelessInterface
// - Node mobility can be set using **.mobility.typename
//   see: inet.mobility and ~IMobility
//
module AdhocHost extends WirelessHost
{
    parameters:
        @display("i=device/cellphone");
        wlan[*].mgmt.typename = default("Ieee80211MgmtAdhoc");
        wlan[*].agent.typename = default("");
        forwarding = default(true);
}

