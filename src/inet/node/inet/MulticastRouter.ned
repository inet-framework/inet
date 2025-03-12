//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.node.inet;

//
// A router with multicast routing capabilities that can forward multicast traffic between
// different network segments.
//
// Multicast routing allows efficient delivery of data to multiple recipients by sending
// packets only once over each link, creating copies only when paths to destinations split.
// This router has Protocol Independent Multicast (PIM) enabled by default and multicast
// forwarding activated.
//
// @see ~StandardHost
//
module MulticastRouter extends Router
{
    parameters:
        hasPim = default(true);
        multicastForwarding = true;
}

