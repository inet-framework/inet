//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.linklayer.contract;

import inet.protocolelement.contract.IProtocolLayer;

//
// Module interface for IEEE 802.1r protocol layer implementations. Defines the
// contract for modules that process frame replication and elimination for
// reliability in Ethernet networks. Implementations handle sequence numbering to
// enable duplicate detection and elimination of frames. Based on IEEE Std
// 802.1CB-2017, this protocol adds a tag header with a sequence number field and
// a TPID value of 0xF1C1. Enables redundant transmission paths for critical
// traffic to increase network reliability and availability.
//
moduleinterface IIeee8021rLayer extends IProtocolLayer
{
    parameters:
        @display("i=block/layer");
}
