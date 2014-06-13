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
import inet.linklayer.ITrafficConditioner;
import inet.linklayer.IWiredNic;


//
// PPP interface. Complements the ~PPP module with an output queue
// for QoS and RED support.
//
module PPPInterface like IWiredNic
{
    parameters:
        @display("i=block/ifcard;bgb=214,249;bgl=53");
        string interfaceTableModule;
        string queueType = default("DropTailQueue"); // DropTailQueue, a Diffserv queue, or empty for use internal queue
        string ingressTCType = default(""); // a module type implementing ~ITrafficConditioner for optional traffic conditioning of incoming traffic
        string egressTCType = default(""); // a module type implementing ~ITrafficConditioner for optional traffic conditioning of outgoing traffic
        int numOutputHooks = default(0);
        int numInputHooks = default(0);
        *.interfaceTableModule = default(absPath(interfaceTableModule));
    gates:
        input upperLayerIn;
        output upperLayerOut;
        inout phys @labels(PPPFrame);
    submodules:
        outputHook[numOutputHooks]: <default("Nop")> like IHook if numOutputHooks>0 {
            @display("p=42,33");
        }
        inputHook[numInputHooks]: <default("Nop")> like IHook if numInputHooks>0 {
            @display("p=177,33");
        }
        ingressTC: <ingressTCType> like ITrafficConditioner if ingressTCType != "" {
            @display("p=177,100");
        }
        egressTC: <egressTCType> like ITrafficConditioner if egressTCType != "" {
            @display("p=42,100");
        }
        queue: <queueType> like IOutputQueue if queueType != "" {
            parameters:
                @display("p=42,161;q=l2queue");
        }
        ppp: PPP {
            parameters:
                queueModule = (queueType == "" ? "" : "queue");
                txQueueLimit = (queueType == "" ? 10000 : 1); // queue sends one packet at a time
                @display("p=98,207");
        }
    connections:
        upperLayerIn --> { @display("m=n"); } --> outputHook[0].in if numOutputHooks > 0;
        upperLayerIn --> { @display("m=n"); } --> egressTC.in if numOutputHooks == 0 && egressTCType != "";
        upperLayerIn --> { @display("m=n"); } --> queue.in if numOutputHooks == 0 && egressTCType == "" && queueType != "";
        upperLayerIn --> { @display("m=n"); } --> ppp.netwIn if numOutputHooks == 0 && egressTCType == "" && queueType == "";

        egressTC.out --> queue.in if egressTCType != "" && queueType != "";
        egressTC.out --> ppp.netwIn if egressTCType != "" && queueType == "";

        for i=0..numOutputHooks-2 {
            outputHook[i].out --> outputHook[i+1].in;
        }
        outputHook[numOutputHooks-1].out --> egressTC.in if numOutputHooks > 0 && egressTCType != "";
        outputHook[numOutputHooks-1].out --> queue.in if numOutputHooks > 0 && egressTCType == "" && queueType != "";
        outputHook[numOutputHooks-1].out --> ppp.netwIn if numOutputHooks > 0 && egressTCType == "" && queueType == "";

        queue.out --> ppp.netwIn if queueType != "";

        phys <--> { @display("m=s"); } <--> ppp.phys;

        ppp.netwOut --> { @display("m=n"); } --> upperLayerOut if numInputHooks == 0 && ingressTCType == "";

        ppp.netwOut --> ingressTC.in if ingressTCType != "";
        ingressTC.out --> { @display("m=n"); } --> upperLayerOut if ingressTCType != "" && numInputHooks == 0;

        ppp.netwOut --> inputHook[0].in if numInputHooks > 0 && ingressTCType == "";
        ingressTC.out --> inputHook[0].in if numInputHooks > 0 && ingressTCType != "";

        for i=0..numInputHooks-2 {
            inputHook[i].out --> inputHook[i+1].in;
        }
        inputHook[numInputHooks-1].out --> { @display("m=n"); } --> upperLayerOut if numInputHooks > 0;
}

