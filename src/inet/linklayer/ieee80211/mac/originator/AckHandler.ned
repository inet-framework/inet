//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.linklayer.ieee80211.mac.originator;

//
// Manages acknowledgment (ACK) status tracking for transmitted frames in IEEE
// 802.11 networks. Maintains a record of frame transmission states, processes
// received ACK frames, and determines when frames need retransmission. Supports
// the reliable frame delivery mechanism by tracking which frames have been
// successfully acknowledged and which require retransmission.
//
simple AckHandler
{
    parameters:
        @display("i=block/table2");
}
