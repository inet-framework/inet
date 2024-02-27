//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include <cstdlib>
#include "inet/common/misc/ExternalProcess.h"

namespace inet {

//TODO drawback of fork+exec approach: if command is incorrect, the module will not be notified and cannot throw an error
// Alternative approach: use system() to launch the process using system(), but then it's not possible to get the
// process id of the external process (instead, stopProcess() could kill ALL child processes of the current process?)
// how to kill all processes in the process tree? it seems impossible (e.g. if an intermediate process dies, its children are reassigned to PID=1)
//TODO drawback: if the external process dies, this module will not be notified
//TODO stopProcess: after SIGTERM, check if process still running, and if so, send a SIGKILL signal to terminate it

Define_Module(ExternalProcess);

void ExternalProcess::initialize()
{
    WATCH(command);
    WATCH(pidFile);
    WATCH(cleanupCommand);

    startProcess();
}

void ExternalProcess::handleMessage(cMessage *msg)
{
    // This module does not handle messages
    delete msg;
}

ExternalProcess::~ExternalProcess()
{
    stopProcess();
}

static std::string quoteForShell(const std::string& input)
{
    std::string output = "'";
    for (char c : input) {
        if (c == '\'')
            output += "'\\''";
        else
            output += c;
    }
    output += "'";
    return output;
}

void ExternalProcess::startProcess()
{
    command = par("command").stringValue();
    cleanupCommand = par("cleanupCommand").stringValue();
    pidFile = std::string(".") + getName() + "-" + std::to_string(getId())+".pid";

    if (!command.empty()) {
        command = "echo $$ >" + pidFile + "; echo " + quoteForShell(command) + "; " + command + "; sh";
        bool useXterm = true; // always use xterm, because cleanup command does not work outside of it
        if (useXterm)
            command = "xterm -e " + quoteForShell(command);
        // TODO else: use "sh -c" instead of xterm, plus use group PID of shell for cleanup command
        command += " &";

        if (cleanupCommand.empty())
            cleanupCommand = "cat " + pidFile + " | xargs kill 2>/dev/null; rm -f " + pidFile;

        EV << "Running: " << command << "\n";
        int res = system(command.c_str());
        if (res == -1)
            throw cRuntimeError("Command '%s' failed to start", command.c_str());
    }
}

void ExternalProcess::stopProcess()
{
    if (!cleanupCommand.empty()) {
        EV << "Running: " << cleanupCommand << "\n";
        int res = system(cleanupCommand.c_str());
        if (res == -1)
            throw cRuntimeError("Cleanup command '%s' failed to start", cleanupCommand.c_str());
    }
}

} // namespace inet
