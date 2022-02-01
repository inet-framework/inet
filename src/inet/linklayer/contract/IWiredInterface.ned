//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.linklayer.contract;

//
// This module interface is implemented by wired network interfaces.
//
moduleinterface IWiredInterface extends INetworkInterface
{
    parameters:
        @display("i=block/ifcard");
    gates:
        inout phys;
}

