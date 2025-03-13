//
// Copyright (C) 2015 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211;

import inet.linklayer.contract.IMacProtocol;

//
// Defines the interface for IEEE 802.11 MAC (Medium Access Control) protocol
// implementations. This interface specifies the minimum requirements for any
// module that implements the MAC sublayer functionality of the IEEE 802.11
// wireless LAN standard.
//
// The IEEE 802.11 MAC layer is responsible for:
// - Channel access coordination using CSMA/CA
// - Frame transmission and reception
// - Acknowledgment handling
// - Retransmission of unacknowledged frames
// - Fragmentation and reassembly
// - Frame addressing and filtering
// - Security (encryption/decryption)
//
// This interface extends the basic ~IMacProtocol interface with additional
// gates for management frame exchange, allowing communication with the
// management entity (~IIeee80211Mgmt) that handles operations such as
// scanning, authentication, and association.
//
// Modules implementing this interface can support various IEEE 802.11 amendments
// including 802.11a/b/g/n/ac/p with different channel access methods such as
// DCF (Distributed Coordination Function) and EDCA (Enhanced Distributed
// Channel Access) for QoS support.
//
// @see ~Ieee80211Mac, ~IMacProtocol, ~IIeee80211Mgmt
//
moduleinterface IIeee80211Mac extends IMacProtocol
{
    parameters:
        @display("i=block/layer");
    gates:
        input mgmtIn;
        output mgmtOut;
}
