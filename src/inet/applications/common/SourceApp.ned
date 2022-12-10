//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.applications.common;

import inet.applications.contract.IApp;
import inet.queueing.contract.IActivePacketSource;

//
// This module implements a generic source application.
//
module SourceApp like IApp
{
    parameters:
        @application;
        @display("i=block/app");
    gates:
        input socketIn;
        output socketOut;
    submodules:
        source: <default("ActivePacketSource")> like IActivePacketSource {
            @display("p=150,150");
        }
    connections allowunconnected:
        source.out --> socketOut;
}

