//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.applications.external;

import inet.applications.contract.IApp;
import inet.common.ExternalEnvironment;
import inet.common.ExternalProcess;

//
// This module provides an external application that uses a host OS external process.
//
// @see ~ExternalProcess, ~ExternalEnvironment
//
module ExternalApp like IApp
{
    parameters:
        string namespace = default(""); // name of the host OS network namespace (optional)
        *.namespace = default(this.namespace);
        @display("i=block/app");
    gates:
        input socketIn @loose;
        output socketOut @loose;
    submodules:
        environment: ExternalEnvironment {
            parameters:
                @display("p=150,150");
        }
        process: ExternalProcess {
            parameters:
                @display("p=150,300");
        }
    connections allowunconnected:
}

