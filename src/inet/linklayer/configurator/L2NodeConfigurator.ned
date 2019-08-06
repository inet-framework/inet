//
// Copyright (C) 2013 OpenSim Ltd.
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
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

package inet.linklayer.configurator;

//
// This module has one instance per network node, and it acts like a bridge
// between the node and the network's global configurator module,
// ~L2NetworkConfigurator.
//
// ~L2NetworkConfigurator only computes and remembers configuration data
// (port priority, link cost, etc.), but doesn't configure the nodes'
// routing tables and interfaces accordingly. The actual configuration is
// carried out by this module, at network initialization time and also when
// the node restarts (i.e. after a shutdown or a crash.)
//
// This module is needed because of failure/restart scenarios:
// ~L2NetworkConfigurator would not be able to configure the node after
// a restart, because being a global module, it doesn't know about node
// lifecycle events.
//
simple L2NodeConfigurator
{
    parameters:
        @display("i=block/cogwheel");
        string interfaceTableModule;
        string l2ConfiguratorModule = default("l2NetworkConfigurator"); // the absolute path to the ~L2NetworkConfigurator; use "" if there is no configurator
}

