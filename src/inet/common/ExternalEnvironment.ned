//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.common;

//
// Executes a setup command on the host OS during initialization and another teardown
// command during module destruction. For example, it can be used to configure
// virtual network interfaces.
//
// The commands can be executed in the context of a network namespace (Linux only).
// The network namespace may already exist in the host OS before the simulation
// starts, or it can be created as a global namespace under /var/run/netns/, or
// it can be created as a process-local namespace.
//
// @see ~ExternalApp, ~ExternalProcess
//
simple ExternalEnvironment
{
    parameters:
        int initStage = default(0); // Initialization stage for the setup command
        bool globalNamespace = default(false); // Create new network namespaces under /var/run/netns/ (Linux only)
        string namespace = default(""); // Name of the host OS network namespace (optional) (Linux only)
        string setupCommand = default(""); // A host OS system command
        string teardownCommand = default(""); // A host OS system command
        @display("i=block/app");
}

