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
// @see EthernetInterface2, ~EthernetInterfaceNoQueue
//
module EthernetInterface like IWiredNic
{
    parameters:
        bool csmacdSupport = default(false);  // by default no CSMA/CD so only only point-to-point duplex links are supported.
        string macType = default(csmacdSupport ? "EtherMAC" : "EtherMACFullDuplex"); // ~EtherMAC or ~EtherMACFullDuplex
        string queueType = default("DropTailQueue"); // DropTailQueue, DropTailQoSQueue, REDQueue, NoQueue
        int numOutputHooks = default(0);
        int numInputHooks = default(0);
        @display("i=block/ifcard;bgb=273,285;bgl=2");
    gates:
        input netwIn;
        output netwOut;
        inout phys @labels(EtherFrame);
    submodules:
        outputHook[numOutputHooks]: <default("Nop")> like IHook if numOutputHooks>0;
        inputHook[numInputHooks]: <default("Nop")> like IHook if numInputHooks>0;
        queue: <queueType> like IOutputQueue {
            parameters:
                @display("p=53,151;q=l2queue");
        }
        mac: <macType> like IEtherMAC {
            parameters:
                queueModule = (queueType == "NoQueue" ? "" : "queue");
                txQueueLimit = (queueType == "NoQueue" ? 10000 : 1); // queue sends one packet at a time
                @display("p=116,231");
        }
        encap: EtherEncap {
            parameters:
                @display("p=116,151");
        }
    connections:
        queue.out --> encap.upperLayerIn;
        encap.lowerLayerOut --> mac.upperLayerIn;
        encap.lowerLayerIn <-- mac.upperLayerOut;
        mac.phys <--> { @display("m=s"); } <--> phys;

        netwIn --> { @display("m=n"); } --> queue.in if numOutputHooks == 0;
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
        outputHook[numOutputHooks-1].out --> queue.in if numOutputHooks > 0;
}
