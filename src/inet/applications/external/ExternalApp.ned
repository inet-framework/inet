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
// Enables integration of real-world applications running as external processes on
// the host operating system with the simulation environment. Creates a bridge between
// the simulated network and actual executable applications, allowing realistic
// application behavior to be incorporated into network simulations without reimplementing
// the application logic.
//
// Leverages the external environment and process management capabilities to establish
// communication channels between the simulation and host OS processes. Supports optional
// network namespace configuration for isolating the external application's network
// stack, particularly useful for containerized or virtualized testing scenarios.
// The module handles the lifecycle management of the external process, including
// startup, communication, and termination, synchronizing its execution with the
// simulation timeline.
//
// @see ~ExternalProcess, ~ExternalEnvironment, ~ExternalInterface
//
module ExternalApp like IApp
{
    parameters:
        string namespace = default(""); // Name of the host OS network namespace (optional)
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
