//
// Copyright (C) 2004 Andras Varga
//               2010 Zoltan Bojthe
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

package inet.linklayer.ethernet;

import inet.common.IHook;
import inet.common.queue.IOutputQueue;
import inet.linklayer.contract.IEtherMac;
import inet.linklayer.contract.IEthernetInterface;
import inet.linklayer.contract.ITrafficConditioner;

//
// Ethernet network interface. Complements ~EtherMac and ~EtherEncap
// with an output queue for QoS and RED support.
//
module EthernetInterface like IEthernetInterface
{
    parameters:
        @class(InterfaceEntry);
        string interfaceTableModule;
        bool csmacdSupport = default(false);  // by default CSMA/CD is turned off, so only point-to-point duplex links are supported.
        string fcsMode @enum("declared", "computed") = default("declared");
        int numOutputHooks = default(0);
        int numInputHooks = default(0);
        @display("i=block/ifcard;bgl=2");
        *.interfaceTableModule = default(absPath(interfaceTableModule));
        *.fcsMode = fcsMode;
    gates:
        input upperLayerIn;
        output upperLayerOut;
        inout phys @labels(EtherFrame);
    submodules:
        outputHook[numOutputHooks]: <default("Nop")> like IHook if numOutputHooks>0 {
            @display("p=100,100;is=s");
        }
        inputHook[numInputHooks]: <default("Nop")> like IHook if numInputHooks>0 {
            @display("p=400,100;is=s");
        }
        ingressTC: <default("")> like ITrafficConditioner if typename != "" {
            @display("p=400,200");
        }
        egressTC: <default("")> like ITrafficConditioner if typename != "" {
            @display("p=100,200");
        }
        queue: <default("")> like IOutputQueue if typename != "" {
            parameters:
                @display("p=100,400;q=l2queue");
        }
        mac: <default(csmacdSupport ? "EtherMac" : "EtherMacFullDuplex")> like IEtherMac {
            parameters:
                queueModule = (exists(queue) ? "^.queue" : "");
                txQueueLimit = (exists(queue) ? 1 : 10000); // queue sends one packet at a time
                @display("p=200,500");
        }
        encap: <default("EtherEncap")> like IEtherEncap {
            parameters:
                @display("p=250,300");
        }
    connections:
        mac.upperLayerOut --> encap.lowerLayerIn;
        mac.phys <--> { @display("m=s"); } <--> phys;

        if exists(queue) {
            encap.lowerLayerOut --> queue.in;
            queue.out --> mac.upperLayerIn;
        }

        encap.lowerLayerOut --> mac.upperLayerIn if !exists(queue);

        // no input hooks, no ingressTC
        encap.upperLayerOut --> { @display("m=n"); } --> upperLayerOut if numInputHooks == 0 && !exists(ingressTC);

        // no input hooks, there is ingressTC
        if numInputHooks == 0 && exists(ingressTC) {
            encap.upperLayerOut --> ingressTC.in;
            ingressTC.out --> { @display("m=n"); } --> upperLayerOut;
        }

        // there are input hooks
        if numInputHooks > 0 {
            inputHook[numInputHooks-1].out --> { @display("m=n"); } --> upperLayerOut;
        }
        for i=0..numInputHooks-2 {
            inputHook[i].out --> inputHook[i+1].in;
        }

        // there are input hooks, no ingressTC
        if numInputHooks > 0 && exists(ingressTC) {
            encap.upperLayerOut --> inputHook[0].in;
        }

        // there are input hooks, there is ingressTC
        if numInputHooks > 0 && exists(ingressTC) {
            encap.upperLayerOut --> ingressTC.in;
            ingressTC.out --> inputHook[0].in;
        }

        // chain output hooks
        for i=0..numOutputHooks-2 {
            outputHook[i].out --> outputHook[i+1].in;
        }

        // no output hooks, no egressTC
        upperLayerIn --> { @display("m=n"); } --> encap.upperLayerIn if numOutputHooks == 0 && !exists(egressTC);

        // there are output hooks
        if numOutputHooks > 0 {
            upperLayerIn --> { @display("m=n"); } --> outputHook[0].in;
        }

        // no output hooks, there is egressTC
        if numOutputHooks == 0 && exists(egressTC) {
            upperLayerIn --> { @display("m=n"); } --> egressTC.in;
        }

        // there is egressTC
        if exists(egressTC) {
            egressTC.out --> encap.upperLayerIn;
        }

        // there are output hooks, no egressTC
        if numOutputHooks > 0 && exists(egressTC) {
            outputHook[numOutputHooks-1].out --> encap.upperLayerIn;
        }

        // there are output hooks, there is egressTC
        if numOutputHooks > 0 && exists(egressTC) {
            outputHook[numOutputHooks-1].out --> egressTC.in;
        }
}

