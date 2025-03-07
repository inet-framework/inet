//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.filter;

import inet.queueing.contract.IPacketFilter;

//
// Placeholder module that implements the IPacketFilter interface but removes itself
// from the module hierarchy during initialization. This module enables flexible
// network configurations with optional filtering components.
//
// Operation mechanism:
// - Directly connects its input gate to its output gate
// - Removes itself from the module hierarchy during initialization
// - Effectively creates a transparent pass-through for packets
// - No filtering is performed on packets passing through
//
// The OmittedPacketFilter is particularly useful for:
// - Creating configurable network models where filtering is optional
// - Maintaining consistent module interfaces across different configurations
// - Simplifying parameter studies by allowing filters to be easily enabled/disabled
// - Supporting modular design with optional components
// - Providing backward compatibility when adding new filtering capabilities
//
// This module is part of INET's module omission mechanism that allows for
// flexible configuration of network models without changing the module structure.
//
// @see ~OmittedModule, ~OmittedPacketQueue, ~OmittedPacketServer
//
module OmittedPacketFilter like IPacketFilter
{
    parameters:
        @class(::inet::OmittedModule);
        @display("i=block/star,black,50");
    gates:
        input in;
        output out;
    connections:
        in --> out;
}
