//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/INETDefs.h"

#include <cstdio>
#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

#include "inet/common/IPrintableObject.h"

namespace inet {

Register_PerRunConfigOption(CFGID_INET_LOG_FORMATTING, "inet-log-formatting", CFG_CUSTOM, "auto", "Whether INET's log output and printed objects contain ANSI escape sequences for bold, faint, colored, etc. text. Qtenv renders them; in Cmdenv output and in log files they appear as raw escape sequences. Possible values: true, false, auto. `auto` enables them in graphical user interfaces, and elsewhere when the standard output is a terminal and `cmdenv-redirect-output` is off.");

OPP_THREAD_LOCAL int evFlags = 1;

static bool isLogFormattingEnabled()
{
    cEnvir *envir = cSimulation::getActiveEnvir();
    cConfiguration *config = envir->getConfig();
    std::string value = config->getAsCustom(CFGID_INET_LOG_FORMATTING);
    if (value == "true")
        return true;
    else if (value == "false")
        return false;
    else if (value == "auto") {
        if (envir->isGUI())
            return true;
        // Cmdenv redirects its output stream, not the standard output file descriptor
        cConfigOption *redirectOutputOption = cConfigOption::find("cmdenv-redirect-output");
        if (redirectOutputOption != nullptr && config->getAsBool(redirectOutputOption))
            return false;
        return isatty(fileno(stdout));
    }
    else
        throw cRuntimeError("Unrecognized value '%s' for option '%s', possible values: true, false, auto", value.c_str(), CFGID_INET_LOG_FORMATTING->getName());
}

EXECUTE_PRE_NETWORK_SETUP(
    if (isLogFormattingEnabled())
        evFlags |= IPrintableObject::PRINT_FLAG_FORMATTED;
    else
        evFlags &= ~IPrintableObject::PRINT_FLAG_FORMATTED;
);

} // namespace inet

