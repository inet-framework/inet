//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.originator;

import inet.common.SimpleModule;

//
// Manages acknowledgment handling for QoS frames in IEEE 802.11 networks. Tracks
// the status of transmitted frames, processes received acknowledgments (both
// normal ACKs and Block ACKs), and handles different acknowledgment policies.
// Supports QoS-specific features including Traffic Identifier (TID) based frame
// tracking and Block ACK mechanisms, enabling reliable frame delivery with
// Quality of Service guarantees.
//
simple QosAckHandler extends SimpleModule
{
    parameters:
        @class(QosAckHandler);
        @display("i=block/table2");
}

