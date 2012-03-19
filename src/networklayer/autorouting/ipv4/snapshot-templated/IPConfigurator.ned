//
// Copyright (C) 2004 Andras Varga
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

package inet.networklayer.autorouting.ipv4;

//
// Module base for IP address configurators.
//
simple IPConfigurator
{
    parameters:
        @display("i=block/cogwheel_s");
        bool disjunctSubnetAddresses = default(true); // avoids using the same address prefix and netmask on different links when assigning IP addresses to interfaces
        bool dumpAddresses = default(true);           // prints assigned IP addresses for all interfaces to the module output
}
