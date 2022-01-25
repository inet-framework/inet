//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

package inet.queueing.policing;

import inet.queueing.contract.IPacketFilter;
import inet.queueing.contract.IPacketMeter;
import inet.queueing.contract.IPacketPolicing;

//
// This module combines a packet meter and a packet filter into a packet policing
// module. The meter measures the packet flow that is passing through and attaches
// some meta information to the packets. The filter drops the packets that don't
// match the implemented filter criteria.
//
module PacketPolicing like IPacketPolicing
{
    gates:
        input in @labels(push);
        output out @labels(push);
    submodules:
        meter: <default("DualRateThreeColorMeter")> like IPacketMeter {
            parameters:
                @display("p=125,120");
        }
        filter: <default("LabelFilter")> like IPacketFilter {
            parameters:
                @display("p=325,120");
        }
    connections:
        in --> { @display("m=w"); } --> meter.in;
        meter.out --> filter.in;
        filter.out --> { @display("m=e"); } --> out;
}

