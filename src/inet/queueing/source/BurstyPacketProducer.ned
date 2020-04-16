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
import inet.queueing.scheduler.MarkovScheduler;

//
// This module mixes two different packet sources to generate bursty traffic.
//
module BurstyPacketProducer like IActivePacketSource
{
    parameters:
        @display("i=block/source");
    gates:
        output out;
    submodules:
        normalProducer: ActivePacketSource {
            @display("p=100,100");
        }
        burstProducer: ActivePacketSource {
            @display("p=300,100");
        }
        scheduler: MarkovScheduler {
            @display("p=200,200");
        }
    connections allowunconnected:
        normalProducer.out --> scheduler.in++;
        burstProducer.out --> scheduler.in++;
        scheduler.out --> out;
}
