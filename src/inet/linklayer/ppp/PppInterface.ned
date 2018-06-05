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

import inet.common.IHook;
import inet.common.queue.IOutputQueue;
import inet.linklayer.contract.IPppInterface;
import inet.linklayer.contract.ITrafficConditioner;


//
// PPP interface. Complements the ~PPP module with an output queue
// for QoS and RED support.
//
module PppInterface like IPppInterface
{
    parameters:
        @class(InterfaceEntry);
        @display("i=block/ifcard;bgb=214,249;bgl=53");
        string interfaceTableModule;
        string queueType = default("DropTailQueue"); // DropTailQueue, a Diffserv queue, or empty for use internal queue
        int numOutputHooks = default(0);
        int numInputHooks = default(0);
        *.interfaceTableModule = default(absPath(interfaceTableModule));
    gates:
        input upperLayerIn;
        output upperLayerOut;
        inout phys @labels(PppFrame);
    submodules:
        outputHook[numOutputHooks]: <default("Nop")> like IHook if numOutputHooks>0 {
            @display("p=42,33");
        }
        inputHook[numInputHooks]: <default("Nop")> like IHook if numInputHooks>0 {
            @display("p=177,33");
        }
        ingressTC: <default("")> like ITrafficConditioner if typename!="" {
            @display("p=177,100");
        }
        egressTC: <default("")> like ITrafficConditioner if typename!="" {
            @display("p=42,100");
        }
        queue: <queueType> like IOutputQueue if queueType != "" {
            parameters:
                @display("p=42,161;q=l2queue");
        }
        ppp: Ppp {
            parameters:
                queueModule = (queueType == "" ? "" : "^.queue");
                txQueueLimit = (queueType == "" ? 10000 : 1); // queue sends one packet at a time
                @display("p=98,207");
        }
    connections:
        upperLayerIn --> { @display("m=n"); } --> outputHook[0].in if numOutputHooks > 0;
        upperLayerIn --> { @display("m=n"); } --> egressTC.in if numOutputHooks == 0 && exists(egressTC);
        upperLayerIn --> { @display("m=n"); } --> queue.in if numOutputHooks == 0 && !exists(egressTC) && queueType != "";
        upperLayerIn --> { @display("m=n"); } --> ppp.upperLayerIn if numOutputHooks == 0 && !exists(egressTC) && queueType == "";

        egressTC.out --> queue.in if egressTCType != "" && queueType != "";
        egressTC.out --> ppp.upperLayerIn if exists(egressTC) && queueType == "";

        for i=0..numOutputHooks-2 {
            outputHook[i].out --> outputHook[i+1].in;
        }
        outputHook[numOutputHooks-1].out --> egressTC.in if numOutputHooks > 0 && exists(egressTC);
        outputHook[numOutputHooks-1].out --> queue.in if numOutputHooks > 0 && !exists(egressTC) && queueType != "";
        outputHook[numOutputHooks-1].out --> ppp.upperLayerIn if numOutputHooks > 0 && !exists(egressTC) && queueType == "";

        queue.out --> ppp.upperLayerIn if queueType != "";

        phys <--> { @display("m=s"); } <--> ppp.phys;

        ppp.upperLayerOut --> { @display("m=n"); } --> upperLayerOut if numInputHooks == 0 && !exists(ingressTC);

        ppp.upperLayerOut --> ingressTC.in if exists(ingressTC);
        ingressTC.out --> { @display("m=n"); } --> upperLayerOut if exists(ingressTC) && numInputHooks == 0;

        ppp.upperLayerOut --> inputHook[0].in if numInputHooks > 0 && !exists(ingressTC);
        ingressTC.out --> inputHook[0].in if numInputHooks > 0 && exists(ingressTC);

        for i=0..numInputHooks-2 {
            inputHook[i].out --> inputHook[i+1].in;
        }
        inputHook[numInputHooks-1].out --> { @display("m=n"); } --> upperLayerOut if numInputHooks > 0;
}

