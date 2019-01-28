//
// Copyright (C) 2005 Andras Varga
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

package inet.linklayer.ppp;

import inet.common.queue.IOutputQueue;
import inet.linklayer.contract.IPppInterface;
import inet.linklayer.contract.ITrafficConditioner;
import inet.networklayer.common.InterfaceEntry;

//
// PPP interface. Complements the ~PPP module with an output queue
// for QoS and RED support.
//
module PppInterface extends InterfaceEntry like IPppInterface
{
    parameters:
        @class(InterfaceEntry);
        @display("i=block/ifcard;bgl=53");
        string interfaceTableModule;
        *.interfaceTableModule = default(absPath(interfaceTableModule));
    gates:
        input upperLayerIn;
        output upperLayerOut;
        inout phys @labels(PppFrame);
    submodules:
        ingressTC: <default("")> like ITrafficConditioner if typename != "" {
            @display("p=500,200");
        }
        egressTC: <default("")> like ITrafficConditioner if typename != "" {
            @display("p=100,200");
        }
        queue: <default("DropTailQueue")> like IOutputQueue if typename != "" {
            parameters:
                @display("p=219,247;q=l2queue");
        }
        ppp: Ppp {
            parameters:
                queueModule = (!exists(queue) ? "" : "^.queue");
                txQueueLimit = (!exists(queue) ? 10000 : 1); // queue sends one packet at a time
                @display("p=300,400");
        }
    connections:
        upperLayerIn --> { @display("m=n"); } --> egressTC.in if exists(egressTC);
        upperLayerIn --> { @display("m=n"); } --> queue.in if !exists(egressTC) && exists(queue);
        upperLayerIn --> { @display("m=n"); } --> ppp.upperLayerIn if !exists(egressTC) && !exists(queue);

        egressTC.out --> queue.in if exists(egressTC) && exists(queue);
        egressTC.out --> ppp.upperLayerIn if exists(egressTC) && !exists(queue);

        queue.out --> ppp.upperLayerIn if exists(queue);

        phys <--> { @display("m=s"); } <--> ppp.phys;

        ppp.upperLayerOut --> { @display("m=n"); } --> upperLayerOut if !exists(ingressTC);

        ppp.upperLayerOut --> ingressTC.in if exists(ingressTC);
        ingressTC.out --> { @display("m=n"); } --> upperLayerOut if exists(ingressTC);
}

