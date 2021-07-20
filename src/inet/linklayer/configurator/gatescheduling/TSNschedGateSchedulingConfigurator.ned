//
// Copyright (C) 2020 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

package inet.linklayer.configurator.gatescheduling;

import inet.linklayer.configurator.gatescheduling.base.GateSchedulingConfiguratorBase;
import inet.linklayer.configurator.gatescheduling.contract.IGateSchedulingConfigurator;

//
// This module provides a gate scheduling configurator that uses the TSNsched
// tool that is available at https://github.com/ACassimiro/TSNsched
// Tested revision: 3f3bf663d196ec6c03e81a1e1392d4aefd158e3e
//
// In order to be able to use this module, the above tool must be installed
// along with a Java SDK and all other necessary tools. The module communicates
// with TSNsched via reading and writing files and using system commands. The
// TSNsched tool is found using the TSNSCHED_HOME environment variable, which
// must be properly set to the installation folder.
//
simple TSNschedGateSchedulingConfigurator extends GateSchedulingConfiguratorBase like IGateSchedulingConfigurator
{
    parameters:
        @class(TSNschedGateSchedulingConfigurator);
}

