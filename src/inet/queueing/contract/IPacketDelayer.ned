//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.contract;

//
// Interface for packet delayers.
//
moduleinterface IPacketDelayer extends IPassivePacketSink, IActivePacketSource
{
    parameters:
        @omittedTypename(OmittedPacketDelayer);
        @display("i=block/delay");
}
