//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.node.inet;

import inet.node.base.ApplicationLayerNodeBase;

//
// IPv4 host with SCTP, TCP, UDP layers and applications.
// IP forwarding is disabled by default (see forwarding).
// - Can be connected via ethernet interface to other nodes using
//   the ethg gate. By default full-duplex connections are supported
//   only (twisted pair). Set **.eth[*].typename="EthernetInterface" for
//   a full/half-duplex CSMA/CD implementation (coaxial cable)
// - By default contains no wireless cards, however it can be configured
//   by the numWlanInterfaces parameter. Wireless card type is configured by the
//   **.wlan[*].typename parameter. see:  inet.linklayer.ieee80211 or other
//   modules implementing ~IWirelessInterface
// - Also external interfaces can be configured for HW in the loop simulation
//   using the numEthInterfaces parameter and setting the interface type using
//   **.eth[*].typename in the INI file. see: ~ExtInterface derived modules and ~IExternalInterface
// - If wireless card is present, node mobility can be set using *.*.mobility.typename
//   see: inet.mobility and ~IMobility
//
module StandardHost extends ApplicationLayerNodeBase
{
    parameters:
        @display("i=device/pc2");
        @figure[submodules];
        forwarding = default(false);  // disable routing by default
        ipv4.arp.proxyArpInterfaces = default(""); // proxy arp is disabled on hosts by default
        *.routingTableModule = default("^.ipv4.routingTable");
}

