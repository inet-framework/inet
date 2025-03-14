//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee8021q;

//
// Placeholder module that implements the IEEE 802.1Q VLAN tag header checker interface
// but removes itself from the module hierarchy during initialization. Provides a direct
// connection between input and output gates, effectively bypassing TPID-based VLAN tag
// checking functionality when VLAN processing is not needed in a particular configuration.
//
// This module is useful in network configurations where the same model structure is
// maintained but VLAN tag processing is selectively disabled in certain paths.
//
// @see IIeee8021qTagTpidHeaderChecker, Ieee8021qTagTpidHeaderChecker
//
module OmittedIeee8021qTagTpidHeaderChecker like IIeee8021qTagTpidHeaderChecker
{
    parameters:
        string vlanTagType = default("");
        @class(::inet::OmittedModule);
        @display("i=block/checker,black,50");
    gates:
        input in;
        output out;
    connections:
        in --> out;
}
