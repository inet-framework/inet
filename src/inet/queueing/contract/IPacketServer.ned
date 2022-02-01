//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.contract;

//
// This module interface is implemented by packet server modules. A packet
// server is an active module which has one active input and one active output.
// Packets are pulled from the connected input module and pushed into the
// connected output module with an optional delay but without reordering.
//
moduleinterface IPacketServer extends IActivePacketSink, IActivePacketSource
{
    parameters:
        @omittedTypename(OmittedPacketServer);
        @display("i=block/server");
}
