//
// Copyright (C) 2018 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.contract;

//
// Interface for PPP network interfaces.
//
moduleinterface IPppInterface extends IWiredInterface
{
    parameters:
        @display("i=block/ifcard");
}

