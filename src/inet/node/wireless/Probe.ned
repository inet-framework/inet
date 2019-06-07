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

package inet.node.wireless;

import inet.mobility.contract.IMobility;
import inet.node.inet.INetworkNode;
import inet.physicallayer.contract.packetlevel.IAntenna;

module Probe like INetworkNode
{
    parameters:
        @networkNode;
        @display("i=device/antennatower");
    submodules:
        mobility: <default("StationaryMobility")> like IMobility {
            parameters:
	            subjectModule = default("^");
                @display("p=100,100");
        }
        antenna: <default("IsotropicAntenna")> like IAntenna {
            parameters:
                mobilityModule = default("^.mobility");
                @display("p=100,200");
        }
}

