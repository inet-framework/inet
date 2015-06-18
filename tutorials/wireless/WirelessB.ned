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

package inet.tutorials.wireless;

import inet.node.inet.INetworkNode;


// - drop two standard hosts named hostR1 and hostR2 at the 100,400 and 400,100 positions
network WirelessB extends WirelessA
{
    submodules:
        hostR1: <hostType> like INetworkNode {
            @display("p=250,300");
        }
        hostR2: <hostType> like INetworkNode {
            @display("p=150,450");
        }
        hostR3: <hostType> like INetworkNode {
            @display("p=350,450");
        }
}
