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

package inet.applications.tcpapp;

import inet.applications.contract.IApp;
import inet.queueing.contract.IActivePacketSource;
import inet.queueing.contract.IPacketScheduler;
import inet.queueing.contract.IPassivePacketSink;

//
// This client application contains a configurable pre-composed telnet traffic
// source and traffic sink.
//
// @see ~TelnetServerApp
//
module TelnetClientApp like IApp
{
    parameters:
        @display("i=block/app");
    gates:
        input socketIn;
        output socketOut;
    submodules:
        characterProducer: <default("ActivePacketSource")> like IActivePacketSource {
            parameters:
                packetLength = 1B;
                packetData = intuniform(97, 122); // lower case ASCII characters
                productionInterval = uniform(0.1s, 0.2s); // typing speed between 5 and 10 characters per second
                @display("p=100,100");
        }
        enterProducer: <default("ActivePacketSource")> like IActivePacketSource {
            parameters:
                packetLength = 1B;
                packetData = 13; // enter character
                productionInterval = 0.1s;
                @display("p=300,100");
        }
        emptyProducer: <default("EmptyPacketSource")> like IActivePacketSource {
            parameters:
	            @display("p=500,100");
        }
        scheduler: <default("MarkovScheduler")> like IPacketScheduler {
            parameters:
                transitionProbabilities = "0 1 0 0 0 1 1 0 0"; // character -> enter -> wait -> character
                waitIntervals = "uniform(0,3) 0 uniform(10,30)";
                @display("p=300,200");
        }
        consumer: <default("PassivePacketSink")> like IPassivePacketSink {
            parameters:
	            @display("p=100,200");
        }
        io: TcpClientSocketIo {
            parameters:
	            @display("p=200,300");
        }
    connections:
        characterProducer.out --> scheduler.in++;
        enterProducer.out --> scheduler.in++;
        emptyProducer.out --> scheduler.in++;
        scheduler.out --> { @display("m=s"); } --> io.trafficIn;
        io.trafficOut --> { @display("m=s"); } --> consumer.in;
        io.socketOut --> { @display("m=s"); } --> socketOut;
        socketIn --> { @display("m=s"); } --> io.socketIn;
}
