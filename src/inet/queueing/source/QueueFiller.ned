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
// along with this program; if not, see http://www.gnu.org/licenses/.
//

package inet.queueing.source;

import inet.queueing.contract.IActivePacketSource;
import inet.queueing.contract.IPacketServer;
import inet.queueing.contract.IPassivePacketSource;
import inet.queueing.tokengenerator.QueueBasedTokenGenerator;

//
// This module produces packets in order to prevent a queue from becoming empty.
//
module QueueFiller like IActivePacketSource
{
    parameters:
        @display("i=block/source");
    gates:
        output out @labels(pull);
    submodules:
        provider: <default("PassivePacketSource")> like IPassivePacketSource {
            parameters:
                providingInterval = default(0s);
                @display("p=250,100");
        }
        server: <default("TokenBasedServer")> like IPacketServer {
            parameters:
                @display("p=250,200");
        }
        tokenGenerator: QueueBasedTokenGenerator {
            parameters:
                serverModule = default("^.server");
                @display("p=100,100");
        }
    connections:
        provider.out --> server.in;
        server.out --> { @display("m=s"); } --> out;
}
