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
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

package inet.node.inet;

import inet.applications.generic.ExtApp;
import inet.common.packet.recorder.PcapRecorder;
import inet.emulation.linklayer.ethernet.ExtUpperEthernetInterface;
import inet.emulation.networklayer.ipv4.ExtLowerIpv4;
import inet.networklayer.common.InterfaceTable;
import inet.node.base.NodeBase;


module ExtAppNode extends NodeBase
{
    parameters:

        int numApps = default(0);
        energyStorage.typename = default("IdealEpEnergyStorage");

        int numPcapRecorders = default(0);
        int numEthInterfaces = default(0);  // minimum number of ethernet interfaces

        mobility.typename = default("");
        *.interfaceTableModule = default(absPath(".interfaceTable"));

    gates:
        inout ethg[numEthInterfaces] @labels(EtherFrame-conn) @allowUnconnected;

    submodules:
        app[numApps]: ExtApp {
            parameters:
                @display("p=375,76,row,150");
        }

        pcapRecorder[numPcapRecorders]: PcapRecorder {
            parameters:
                @display("p=125,560;is=s");
        }
        interfaceTable: InterfaceTable {
            parameters:
                @display("p=125,240;is=s");
        }
        eth[sizeof(ethg)]: ExtUpperEthernetInterface {
            parameters:
                @display("p=900,526,row,150;q=txQueue");
        }

    connections allowunconnected:

        for i=0..sizeof(ethg)-1 {
            ethg[i] <--> { @display("m=s"); } <--> eth[i].phys;
        }
}
