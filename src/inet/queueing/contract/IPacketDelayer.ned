//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.contract;

//
// This module interface is implemented by packet delayers.
//
moduleinterface IPacketDelayer extends IPassivePacketSink, IActivePacketSource
{
    parameters:
        @omittedTypename(OmittedPacketDelayer);
        @display("i=block/delay");
}
