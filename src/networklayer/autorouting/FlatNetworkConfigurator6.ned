//
// Copyright (C) 2004 Eric Wu
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


package inet.networklayer.autorouting;

//
// Configures \IPv6 addresses and routing tables for a "flat" network,
// "flat" meaning that all hosts and routers will have the same
// network address and will only differ in the host part.
//
// FIXME: add documentation!
//
// @see FlatNetworkConfigurator
//
simple FlatNetworkConfigurator6
{
    parameters:
        @display("i=block/cogwheel");
        @labels(node);
}

