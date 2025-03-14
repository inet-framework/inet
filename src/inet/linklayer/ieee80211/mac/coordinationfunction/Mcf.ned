//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.coordinationfunction;

//
// IEEE 802.11s Mesh Coordination Function (MCF) that manages medium access for mesh 
// networks. MCF combines elements of both contention-based and scheduled access methods 
// to provide efficient communication in mesh topologies. It supports peer-to-peer 
// communication between mesh stations (mesh STAs) and implements mechanisms for 
// distributed channel access, path selection, and congestion control. MCF enables 
// multi-hop wireless communication where each mesh STA can forward frames on behalf 
// of other stations, extending network coverage and improving reliability through 
// path redundancy.
//
// @see ~Dcf, ~Hcf, ~Pcf
//
module Mcf
{
    parameters:
        @class(Mcf);
        @display("i=block/queue");
}
