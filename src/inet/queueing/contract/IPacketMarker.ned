//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.contract;

//
// This module interface is implemented by packet marker modules. A packet marker
// connects one input to one output. It can operate in both active and passive
// mode. Packets can be pushed into its input or packets can be pulled from its
// output. When processing a packet, a marker attaches some information (such as
// a label) to the packet based on the data and meta data it contains. The packets
// are processed individually.
//
moduleinterface IPacketMarker extends IPacketFlow
{
    parameters:
        @display("i=block/star");
}
