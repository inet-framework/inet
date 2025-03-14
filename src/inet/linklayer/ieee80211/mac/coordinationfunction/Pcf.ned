//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.coordinationfunction;

//
// IEEE 802.11 Point Coordination Function (PCF) that provides contention-free frame 
// transfer for infrastructure networks. PCF operates during the Contention-Free Period 
// (CFP) where the Point Coordinator (PC), typically implemented in the Access Point (AP), 
// controls all medium access by polling stations in a round-robin fashion. This 
// centralized control eliminates contention and provides deterministic access with 
// bounded latency, making it suitable for time-sensitive applications. PCF alternates 
// with the Distributed Coordination Function (DCF) in a superframe structure, providing 
// both contention-free and contention-based access periods.
//
// @see ~Dcf, ~Hcf, ~Mcf
//
module Pcf
{
    parameters:
        @class(Pcf);
        @display("i=block/queue");
}
