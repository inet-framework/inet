//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.node.inet;

//
// A wireless host containing routing, mobility, and energy components. Supports
// the IPv4 network protocol, TCP, UDP, and SCTP as transport protocols.
// This is a typical mobile node that can participate in adhoc routing
// and may have TCP/UDP applications installed. It also supports ICMP (ping).
//
// - By default, it contains a single wireless card, but it can be configured
//   using the numWlanInterfaces parameter. The wireless card type is configured by the
//   **.wlan[*].typename parameter. See: inet.linklayer.ieee80211 or other
//   modules implementing ~IWirelessInterface.
// - Node mobility can be set using the **.mobility.typename
//   see: inet.mobility and ~IMobility.
//
module AdhocHost extends WirelessHost
{
    parameters:
        @display("i=device/cellphone");
        wlan[*].mgmt.typename = default("Ieee80211MgmtAdhoc");
        wlan[*].agent.typename = default("");
        forwarding = default(true);
}

