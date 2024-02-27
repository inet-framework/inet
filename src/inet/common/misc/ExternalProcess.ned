//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.common.misc;

//
// Launches an external OS process in the background, using the command line
// given in the 'command' parameter. The process is terminated when the module
// is deleted.
//
simple ExternalProcess
{
    parameters:
        string command = default(""); // don't include the "&" in the command
        string cleanupCommand = default("");
}

