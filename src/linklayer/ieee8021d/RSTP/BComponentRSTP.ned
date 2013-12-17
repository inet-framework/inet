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

package inet.linklayer.ieee8021d.RSTP;
import inet.linklayer.ieee8021d.ieee8021Q.PortFilt;
import inet.linklayer.ethernet.switch.MACAddressTable;
module BComponentRSTP
{
    parameters:
        @display("i=block/broadcast;bgb=425,266");
        int sizeEthgB;

    gates:
        output BportsOut[];  // To the backbone and the IComponents. That is ethgC+ethgB
        input BportsIn[];	// From the backbone and the IComponents. That is ethgC+ethgB
        inout RSTPPort;	// To the RSTP module

    submodules:
        relay: RelayRSTP {
            parameters:
                @display("p=148,50");
            gates:
                GatesIn[sizeof(BportsIn)];
                GatesOut[sizeof(BportsOut)];
                RSTPGate;
        }
        										// Relay unit
        cache: MACAddressTable {
            parameters:
                @display("p=276,35");
        }

        PortFilt[sizeEthgB]: PortFiltRSTP {
            parameters:
                @display("p=148,152,row");
            gates:
                GatesIn[];
                GatesOut[];

        }

    connections:
        for i=0..sizeof(BportsOut)-1 {
            BportsOut[i] <-- PortFilt[i].GatesOut[0];
            PortFilt[i].GatesIn[1] <-- relay.GatesOut[i];
            BportsIn[i] --> PortFilt[i].GatesIn[0];
            PortFilt[i].GatesOut[1]--> relay.GatesIn[i];
        }
        relay.RSTPGate <--> RSTPPort;
}
