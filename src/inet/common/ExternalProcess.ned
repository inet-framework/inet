//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.common;

//
// Launches an external OS process in the background, using the command line
// given in the 'command' parameter. The process is terminated when the module
// is deleted. This module requires using the ~RealTimeScheduler class as the
// simulation event scheduler.
//
// @see ~ExternalApp, ~ExternalEnvironment
//
simple ExternalProcess
{
    parameters:
        string namespace = default(""); // Name of the OS network namespace (optional)
        string command; // Executable with arguments (e.g. "ping google.com"); not a shell command as it will be passed to execvp
        string onExit @enum("terminateSimulation","relaunch","ignore") = default("ignore"); // Determines what happens when the process exits
        double startTime @unit(s) = default(0s);
        double relaunchDelay @unit(s) = default(0s);
        bool printStdout = default(false); // Print STDOUT to the simulation's STDOUT
        bool printStderr = default(true); // Print STDERR to the simulation's STDERR
        @display("i=block/app");
}

