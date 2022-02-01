//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.contract;

//
// This module interface is implemented by packet filter modules. A packet filter
// connects one input to one output. It can operate in both active and passive
// mode. Packets can be pushed into its input or packets can be pulled from its
// output. Processed packets are either dropped, if they don't match the filter
// criteria, or they are passed through without any delay or reordering. Packets
// are not modified in any way.
//
moduleinterface IPacketFilter extends IPacketFlow
{
    parameters:
        @omittedTypename(OmittedPacketFilter);
        @display("i=block/filter");
}
