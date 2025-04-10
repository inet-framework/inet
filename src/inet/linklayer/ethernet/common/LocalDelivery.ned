//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ethernet.common;

import inet.common.SimpleModule;

simple LocalDelivery extends SimpleModule
{
    parameters:
        @class(LocalDelivery);
        string interfaceTableModule;
    gates:
        input in;
        output deliveryOut;
        output forwardingOut;
}

