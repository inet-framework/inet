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

package inet.linklayer.ieee8021as;

import inet.applications.contract.IApp;
import inet.queueing.classifier.PacketClassifier;
import inet.queueing.common.PacketMultiplexer;

module MultiDomainGptp like IApp
{
    parameters:
        string clockModule;
        string interfaceTableModule;
        int numDomains;
        @display("i=block/app");
    gates:
        input socketIn;
        output socketOut;
    submodules:
        domain[numDomains]: <default("Gptp")> like IApp {
            clockModule = default(absPath(parent.clockModule) + ".clock[" + string(this.index) + "]");
            interfaceTableModule = default(absPath(parent.interfaceTableModule));
            domainNumber = default(this.index);
            @display("p=150,150,row,150");
        }
        multiplexer: PacketMultiplexer {
            @display("p=150,350");
        }
        classifier: PacketClassifier {
            classifierClass = default("inet::GptpDomainNumberClassifier");
            @display("p=300,350");
        }
    connections:
        for i=0..numDomains-1
        {
            domain[i].socketOut --> multiplexer.in++;
            classifier.out++ --> domain[i].socketIn;
        }

        socketIn --> { @display("m=s"); } --> classifier.in;
        multiplexer.out --> { @display("m=s"); } --> socketOut;
}

