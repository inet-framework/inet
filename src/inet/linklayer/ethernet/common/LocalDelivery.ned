//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ethernet.common;

simple LocalDelivery
{
    parameters:
        string interfaceTableModule;
    gates:
        input in;
        output deliveryOut;
        output forwardingOut;
}

