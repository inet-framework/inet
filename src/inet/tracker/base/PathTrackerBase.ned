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
// This module subscribes for packetReceivedFromUpper, packetSentToUpper, and
// packetReceivedFromLower on the module determined by the trackingSubjectModule
// parameter. If the tracker receives a packetReceivedFromUpper from a
// protocol module, then it stores the corresponding packet id. If later on the
// tracker receives a packetSentToUpper from another corresponding protocol
// module with the same packet id, then it adds an active route between the
// source and the destination nodes. During the route discovery if the
// tracker receives a packetReceivedFromLower from a corresponding protocol
// module, then it adds the node to the intermediate nodes of the route. After
// a certain amount of time the route becomes inactive unless it is reinforced
// by another packet.
//
simple PathTrackerBase extends TrackerBase
{
    parameters:
        string fileName = default("");
        bool trackPaths = default(false); // track paths, disabled by default

        string startPathSignal = default("packetReceivedFromUpper");
        string extendPathSignal = default("packetReceivedFromLower");
        string endPathSignal = default("packetSentToUpper");

        string nodeFilter = default("*"); // which nodes are considered, matches all nodes by default
        string packetFilter = default("*"); // which packets are considered, matches all packets by default

        @class(PathTrackerBase);
}

