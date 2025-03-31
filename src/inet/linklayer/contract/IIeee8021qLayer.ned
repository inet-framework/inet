//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.linklayer.contract;

import inet.protocolelement.contract.IProtocolLayer;

//
// Module interface for IEEE 802.1Q protocol layer implementations. Defines the
// contract for modules that process VLAN tags in Ethernet networks, supporting
// network segmentation and traffic control. Implementations handle tag
// encapsulation/decapsulation, VLAN filtering, and priority handling. Can be
// used with both C-VLAN (0x8100) and S-VLAN (0x88A8) tag types. Enables
// creation of modular network designs with interchangeable IEEE 802.1Q
// implementations for virtual LAN functionality.
//
moduleinterface IIeee8021qLayer extends IProtocolLayer
{
    parameters:
        @display("i=block/layer");
}
