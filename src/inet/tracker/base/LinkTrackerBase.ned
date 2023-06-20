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
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//

package inet.tracker.base;

//
// TODO
//
// This module subscribes for signals at the provided subscription module. When
// determining active links, it only considers packets matched by the provided
// filters. The filters use the format of cMatchExpression with the packet name
// as the default attribute.
//
simple LinkTrackerBase extends TrackerBase
{
    parameters:
        string fileName = default("");
        bool trackLinks = default(false);
        string moduleMode @enum("networkNode", "networkInterface", "module") = default("networkNode");
        string activityLevel @enum("service", "peer", "protocol") = default("service"); // determines what level of activity is tracked, service level by default

        string nodeFilter = default("*"); // which nodes are considered, matches all nodes by default
        string interfaceFilter = default("*"); // which interfaces are considered, matches all interfaces by default
        object packetFilter = default("*"); // which packets are considered, matches all packets by default

        @class(LinkTrackerBase);
}

