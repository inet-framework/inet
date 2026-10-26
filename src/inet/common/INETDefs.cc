//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/INETDefs.h"

#include "inet/common/IPrintableObject.h"

namespace inet {

Register_PerRunConfigOption(CFGID_INET_LOG_FORMATTING, "inet-log-formatting", CFG_BOOL, "true", "Whether INET's log output and printed objects contain ANSI escape sequences for bold, faint, colored, etc. text. Qtenv renders them; in Cmdenv output and in log files they appear as raw escape sequences.");

OPP_THREAD_LOCAL int evFlags = 1;

EXECUTE_PRE_NETWORK_SETUP(
    if (cSimulation::getActiveEnvir()->getConfig()->getAsBool(CFGID_INET_LOG_FORMATTING))
        evFlags |= IPrintableObject::PRINT_FLAG_FORMATTED;
    else
        evFlags &= ~IPrintableObject::PRINT_FLAG_FORMATTED;
);

} // namespace inet

