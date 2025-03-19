//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.node.inet;

//
// A mobile wireless node that participates in ad hoc networks without
// centralized infrastructure. Extends WirelessHost by configuring the wireless interface
// for ad hoc mode operation and enabling packet forwarding.
//
// In ad hoc mode, nodes communicate directly with each other without requiring an access point.
// This model is suitable for simulating mobile ad hoc networks (MANETs), wireless mesh networks,
// and other decentralized wireless scenarios.
//
module AdhocHost extends WirelessHost
{
    parameters:
        @display("i=device/cellphone");
        wlan[*].mgmt.typename = default("Ieee80211MgmtAdhoc");
        wlan[*].agent.typename = default("");
        forwarding = default(true);
}

