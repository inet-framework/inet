//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.contract;

//
// Interface for IEEE 802.11 Distributed Coordination Function (DCF) implementations. 
// DCF provides contention-based access to the wireless medium using CSMA/CA with 
// binary exponential backoff. Serves as the foundation for basic medium access in 
// IEEE 802.11 networks, handling frame transmission, reception, and retransmission 
// with collision avoidance mechanisms.
//
// @see ~Dcf, ~Dcaf, ~Contention
//
moduleinterface IDcf
{
    parameters:
        @display("i=block/layer");
}
