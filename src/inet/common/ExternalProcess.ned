//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.common;

import inet.common.SimpleModule;

//
// Launches an external OS process in the background, using the command line
// given in the 'command' parameter. The process is terminated when the module
// is deleted. This module requires using the ~RealTimeScheduler class as the
// simulation event scheduler.
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
// @see ~ExternalApp, ~ExternalEnvironment
//
simple ExternalProcess extends SimpleModule
{
    parameters:
        @class(ExternalProcess);
        string namespace = default(""); // Name of the OS network namespace (optional)
        string command; // Executable with arguments (e.g. "ping google.com"); not a shell command as it will be passed to execvp
        string onExit @enum("terminateSimulation","relaunch","ignore") = default("ignore"); // Determines what happens when the process exits
        double startTime @unit(s) = default(0s);
        double relaunchDelay @unit(s) = default(0s);
        bool printStdout = default(false); // Print STDOUT to the simulation's STDOUT
        bool printStderr = default(true); // Print STDERR to the simulation's STDERR
        @display("i=block/app");
}

