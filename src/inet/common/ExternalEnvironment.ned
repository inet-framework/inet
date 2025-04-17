//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.common;

import inet.common.SimpleModule;

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
// To run a command that requires elevated privileges without sudo, user and
// network namespaces need to be unshared from the host OS (unshare-namespaces = true).
// In this case, the state of the host OS's external processes are not accessible
// from the unshared namespace in the usual way (e.g. displaying routing tables with `ip route`).
// To work around this, use `nsenter`:
//
//    sudo nsenter -t <PID> -n <command>
//
// PID is the ID of the process running in the unshared namespace.
//
// To list process IDs in all network namespaces:
//
//    sudo lsns -t net
//
//
// @see ~ExternalApp, ~ExternalProcess
//
simple ExternalEnvironment extends SimpleModule
{
    parameters:
        @class(ExternalEnvironment);
        int initStage = default(0); // Initialization stage for the setup command
        bool globalNamespace = default(false); // Create new network namespaces under /var/run/netns/ (Linux only)
        string namespace = default(""); // Name of the host OS network namespace (optional) (Linux only)
        string setupCommand = default(""); // A host OS system command
        string teardownCommand = default(""); // A host OS system command
        @display("i=block/app");
}

