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

import inet.networklayer.queue.IOutputQueue;
import inet.linklayer.IWiredNic;
import inet.linklayer.IEtherMAC;
import inet.base.IHook;


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
        string queueType = default("EthDropTailQoSQueue"); // ~DropTailQueue, ~DropTailQoSQueue, ~EthDropTailQoSQueue, ~REDQueue, or empty for use internal queue
        int numOutputHooks = default(0);
        int numInputHooks = default(0);
        @display("i=block/ifcard;bgl=2");
    gates:
        input netwIn;
        output netwOut;
        inout phys @labels(EtherFrame);
    submodules:
        outputHook[numOutputHooks]: <default("Nop")> like IHook if numOutputHooks>0;
        inputHook[numInputHooks]: <default("Nop")> like IHook if numInputHooks>0;
        queue: <queueType> like IOutputQueue if queueType != "" {
            parameters:
                @display("p=53,196;q=l2queue");
        }
        mac: <macType> like IEtherMAC {
            parameters:
                queueModule = (queueType == "" ? "" : "queue");
                txQueueLimit = (queueType == "" ? 10000 : 1); // queue sends one packet at a time
                @display("p=143,243");
        }
        encap: EtherEncap {
            parameters:
                useSNAP = useSNAP;
                @display("p=143,151");
        }
    connections:
        encap.lowerLayerOut --> queue.in if queueType != "";
        queue.out --> mac.upperLayerIn if queueType != "";
        encap.lowerLayerOut --> mac.upperLayerIn if queueType == "";
        encap.lowerLayerIn <-- mac.upperLayerOut;
        mac.phys <--> { @display("m=s"); } <--> phys;

        netwIn --> { @display("m=n"); } --> encap.upperLayerIn if numOutputHooks == 0;
        netwOut <-- { @display("m=n"); } <-- encap.upperLayerOut if numInputHooks == 0;

        encap.upperLayerOut --> inputHook[0].in if numInputHooks > 0;
        for i=0..numInputHooks-2 {
            inputHook[i].out --> inputHook[i+1].in;
        }
        inputHook[numInputHooks-1].out --> { @display("m=n"); } --> netwOut if numInputHooks > 0;

        netwIn --> { @display("m=n"); } --> outputHook[0].in if numOutputHooks > 0;
        for i=0..numOutputHooks-2 {
            outputHook[i].out --> outputHook[i+1].in;
        }
        outputHook[numOutputHooks-1].out --> encap.upperLayerIn if numOutputHooks > 0;
}

