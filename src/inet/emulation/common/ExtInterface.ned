//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.emulation.common;

import inet.linklayer.contract.IExternalInterface;
import inet.networklayer.common.NetworkInterface;

//
// This module provides a base module for external network interfaces.
//
module ExtInterface extends NetworkInterface like IExternalInterface
{
    parameters:
        bool isWireless = default(false);
        string interfaceTableModule;
        string device;
        string namespace = default("");
        string copyConfiguration @enum("","copyToExt","copyFromExt") = default("");
        string address @mutable = default("auto");
        bool broadcast = default(true);
        bool multicast = default(true);
        bool pointToPoint = default(false);
        int mtu @unit(B) = default(4470B);
        *.interfaceTableModule = default(absPath(this.interfaceTableModule));
        *.device = default(this.device);
        *.namespace = default(this.namespace);
        @class(ExtInterface);
    gates:
        input upperLayerIn @loose;
        output upperLayerOut @loose;
}

