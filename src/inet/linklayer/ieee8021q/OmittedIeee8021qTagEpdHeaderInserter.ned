//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee8021q;

//
// Placeholder module that implements the IEEE 802.1Q VLAN tag header inserter interface
// but removes itself from the module hierarchy during initialization. Provides a direct
// connection between input and output gates, effectively bypassing VLAN tag insertion
// functionality when VLAN processing is not needed in a particular configuration.
//
// This module is useful in network configurations where the same model structure is
// maintained but VLAN tag processing is selectively disabled in certain paths.
//
// @see IIeee8021qTagEpdHeaderInserter, Ieee8021qTagEpdHeaderInserter
//
module OmittedIeee8021qTagEpdHeaderInserter like IIeee8021qTagEpdHeaderInserter
{
    parameters:
        string vlanTagType = default("");
        @class(::inet::OmittedModule);
        @display("i=block/inserter,black,50");
    gates:
        input in;
        output out;
    connections:
        in --> out;
}
