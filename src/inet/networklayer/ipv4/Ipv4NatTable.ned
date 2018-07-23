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

package inet.networklayer.ipv4;

import inet.networklayer.contract.IRoutingTable;

//
// Stores the network address translation table.
//
simple Ipv4NatTable
{
    parameters:
        string networkProtocolModule = default("^.ip");
        xml config = default(xml("<config></config>")); // XML configuration parameters for network address translation
        @display("i=block/table");
}
