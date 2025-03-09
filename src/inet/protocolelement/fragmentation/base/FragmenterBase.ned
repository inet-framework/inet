//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.fragmentation.base;

import inet.protocolelement.fragmentation.contract.IFragmenterPolicy;
import inet.queueing.base.PacketPusherBase;

//
// Base module for packet fragmenters that provides common functionality for
// splitting packets into smaller fragments. Uses a pluggable fragmentation
// policy to determine fragment sizes.
//
// Operation:
// - Receives packets from upper layers
// - Consults the fragmentation policy to determine fragment sizes
// - Creates multiple fragments from each original packet
// - Adds necessary information to identify fragment relationships
// - Forwards fragments to lower layers
//
// Particularly useful for:
// - Creating protocol-specific fragmenters with shared base functionality
// - Supporting different fragmentation strategies through policy modules
// - Implementing MTU-based packet splitting
// - Building protocol stacks with fragmentation capabilities
// - Providing a foundation for specialized fragmenter implementations
//
// @see ~DefragmenterBase, ~IFragmenterPolicy
//
module FragmenterBase extends PacketPusherBase
{
    parameters:
        bool deleteSelf = default(false);
        string fragmenterPolicyClass = default("");
        string fragmenterPolicyModule = default(".fragmenterPolicy");
        @display("i=block/fork");
        @class(FragmenterBase);
    submodules:
        fragmenterPolicy: <> like IFragmenterPolicy if typename != "" {
            parameters:
                @display("p=100,100");
        }
}
