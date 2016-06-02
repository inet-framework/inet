//
// Copyright (C) OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

package inet.node.base;

import inet.common.lifecycle.NodeStatus;
import inet.mobility.contract.IMobility;
import inet.node.inet.INetworkNode;
import inet.power.contract.IEnergyGenerator;
import inet.power.contract.IEnergyStorage;

//
// This module contains the most basic infrastructure for network nodes that is
// not strictly communication protocol related.
//
// See derived modules: ~StandardHost, ~WirelessHost, ~AdhocHost, ~Router, etc.
//
module NodeBase like INetworkNode
{
    parameters:
        @networkNode;
        @labels(node,ethernet-node,wireless-node);
        bool hasStatus = default(false);
        string mobilityType = default("");
        string energyStorageType = default("");
        string energyGeneratorType = default("");
        *.mobilityModule = default(mobilityType != "" ? absPath(".mobility") : "");
        *.energySourceModule = default(energyStorageType != "" ? absPath(".energyStorage") : "");
        *.energyGeneratorModule = default(energyGeneratorType != "" ? absPath(".energyGenerator") : "");

    submodules:
        status: NodeStatus if hasStatus {
            parameters:
                @display("p=100,100;is=s");
        }
        energyStorage: <energyStorageType> like IEnergyStorage if energyStorageType != "" {
            parameters:
                @display("p=100,300;is=s");
        }
        energyGenerator: <energyGeneratorType> like IEnergyGenerator if energyGeneratorType != "" {
            parameters:
                @display("p=100,400;is=s");
        }
        // TODO: move mobility right after status to have it in the same order as in their positions, changes fingerprints 
        mobility: <mobilityType> like IMobility if mobilityType != "" {
            parameters:
                @display("p=100,200;is=s");
        }
}
