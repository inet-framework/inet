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

import inet.base.IHook;
import inet.linklayer.IEtherMAC;
import inet.linklayer.IWiredNic;
import inet.linklayer.ITrafficConditioner;


//
// Ethernet network interface. Complements ~EtherMAC and ~EtherEncap
// with an output queue for QoS and RED support.
//
module EthernetInterface like IWiredNic
{
    parameters:
        bool csmacdSupport = default(false);  // by default CSMA/CD is turned off, so only point-to-point duplex links are supported.
        bool useSNAP = default(false);  // use EtherFrameWithSNAP instead EthernetIIFrame
        string macType = default(csmacdSupport ? "EtherMAC" : "EtherMACFullDuplex"); // ~EtherMAC or ~EtherMACFullDuplex
        string queueType = default(""); // ~DropTailQueue, or a Diffserv queue; set to "" for use of an internal queue
        string ingressTCType = default(""); // a module type implementing ~ITrafficConditioner for optional traffic conditioning of incoming traffic
        string egressTCType = default(""); // a module type implementing ~ITrafficConditioner for optional traffic conditioning of outgoing traffic
        int numOutputHooks = default(0);
        int numInputHooks = default(0);
        @display("i=block/ifcard;bgl=2");
    gates:
        input upperLayerIn;
        output upperLayerOut;
        inout phys @labels(EtherFrame);
    submodules:
        outputHook[numOutputHooks]: <default("Nop")> like IHook if numOutputHooks>0 {
            @display("p=80,98");
        }
        inputHook[numInputHooks]: <default("Nop")> like IHook if numInputHooks>0 {
            @display("p=265,83");
        }
        ingressTC: <ingressTCType> like ITrafficConditioner if ingressTCType != "" {
            @display("p=292,158");
        }
        egressTC: <egressTCType> like ITrafficConditioner if egressTCType != "" {
            @display("p=53,158");
        }
        queue: EtherQoSQueue if queueType != "" {
            parameters:
                dataQueueType = queueType;
                @display("p=107,263;q=l2queue");
        }
        mac: <macType> like IEtherMAC {
            parameters:
                queueModule = (queueType == "" ? "" : "queue");
                txQueueLimit = (queueType == "" ? 10000 : 1); // queue sends one packet at a time
                @display("p=182,321");
        }
        encap: <default("EtherEncap")> like IEtherEncap {
            parameters:
                useSNAP = useSNAP;
                @display("p=182,205");
        }
    connections:
        encap.lowerLayerOut --> queue.in if queueType != "";
        queue.out --> mac.upperLayerIn if queueType != "";
        encap.lowerLayerOut --> mac.upperLayerIn if queueType == "";
        encap.lowerLayerIn <-- mac.upperLayerOut;
        mac.phys <--> { @display("m=s"); } <--> phys;

        // chain input hooks
        for i=0..numInputHooks-2 {
            inputHook[i].out --> inputHook[i+1].in;
        }

        // no input hooks, no ingressTC
        encap.upperLayerOut --> { @display("m=n"); } --> upperLayerOut if numInputHooks == 0 && ingressTCType == "";

        // no input hooks, there is ingressTC
        encap.upperLayerOut --> ingressTC.in if numInputHooks == 0 && ingressTCType != "";
        ingressTC.out --> { @display("m=n"); } --> upperLayerOut if numInputHooks == 0 && ingressTCType != "";

        // there are input hooks, no ingressTC
        encap.upperLayerOut --> inputHook[0].in if numInputHooks > 0 && ingressTCType == "";
        inputHook[numInputHooks-1].out --> { @display("m=n"); } --> upperLayerOut if numInputHooks > 0 && ingressTCType == "";

        // there are input hooks, there is ingressTC
        encap.upperLayerOut --> ingressTC.in if numInputHooks > 0 && ingressTCType != "";
        ingressTC.out --> inputHook[0].in if numInputHooks > 0 && ingressTCType != "";
        inputHook[numInputHooks-1].out --> { @display("m=n"); } --> upperLayerOut if numInputHooks > 0 && ingressTCType == "";

        // chain output hooks
        for i=0..numOutputHooks-2 {
            outputHook[i].out --> outputHook[i+1].in;
        }

        // no output hooks, no egressTC
        upperLayerIn --> { @display("m=n"); } --> encap.upperLayerIn if numOutputHooks == 0 && egressTCType == "";

        // no output hooks, there is egressTC
        upperLayerIn --> { @display("m=n"); } --> egressTC.in if numOutputHooks == 0 && egressTCType != "";
        egressTC.out --> encap.upperLayerIn if numOutputHooks == 0 && egressTCType != "";

        // there are output hooks, no egressTC
        upperLayerIn --> { @display("m=n"); } --> outputHook[0].in if numOutputHooks > 0 && egressTCType == "";
        outputHook[numOutputHooks-1].out --> encap.upperLayerIn if numOutputHooks > 0 && egressTCType == "";

        // there are output hooks, there is egressTC
        upperLayerIn --> { @display("m=n"); } --> outputHook[0].in if numOutputHooks > 0 && egressTCType != "";
        outputHook[numInputHooks-1].out --> egressTC.in if numOutputHooks > 0 && egressTCType != "";
        egressTC.out --> encap.upperLayerIn if numOutputHooks > 0 && egressTCType != "";
}

