//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.configurator.gatescheduling.common;

import inet.linklayer.configurator.gatescheduling.base.GateScheduleConfiguratorBase;
import inet.linklayer.configurator.gatescheduling.contract.IGateScheduleConfigurator;

//
// This module provides a gate scheduling configurator that uses the TSNsched
// tool that is available at https://github.com/ACassimiro/TSNsched
// Tested revision: 3f3bf663d196ec6c03e81a1e1392d4aefd158e3e
//
// In order to be able to use this module, the above tool must be installed
// along with a Java SDK and all other necessary tools. The module communicates
// with TSNsched via reading and writing files and using system commands. The
// TSNsched tool is found using the TSNSCHED_ROOT environment variable, which
// must be properly set to the installation folder.
//
simple TSNschedGateScheduleConfigurator extends GateScheduleConfiguratorBase like IGateScheduleConfigurator
{
    parameters:
        @class(TSNschedGateScheduleConfigurator);
}

