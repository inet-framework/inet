//
// Copyright (C) 2020 OpenSim Ltd.
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

package inet.queueing.contract;

//
// This module interface must be implemented by passive packet source modules.
// A passive packet source, also called a provider, has one output that must be
// connected to an active packet sink module. The connected active packet sink
// is expected to pull packets from the passive packet source over time.
//
// @see ~PassivePacketSource, ~PacketQueue, ~PacketScheduler, ~IActivePacketSink
//
moduleinterface IPassivePacketSource
{
    parameters:
        @display("i=block/source");
    gates:
        output out @labels(pull);
}
