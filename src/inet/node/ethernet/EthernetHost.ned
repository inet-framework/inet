//
// Copyright (C) 2003 Andras Varga; CTIE, Monash University, Australia
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
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//

package inet.node.ethernet;

import inet.applications.ethernet.EtherAppClient;
import inet.applications.ethernet.EtherAppServer;
import inet.common.MessageDispatcher;
import inet.common.lifecycle.NodeStatus;
import inet.linklayer.contract.IEthernetInterface;
import inet.linklayer.ethernet.contract.IEthernetEncapsulation;
import inet.linklayer.ethernet.contract.IEthernetLayer;
import inet.linklayer.ieee8021q.IIeee8021qLayer;
import inet.linklayer.ieee8021r.IIeee8021rLayer;
import inet.linklayer.ieee8022.IIeee8022Llc;
import inet.networklayer.common.InterfaceTable;


//
// An example host with one Ethernet port and a traffic generator that
// generates request-reply traffic directly over Ethernet. This host model
// does not contain higher layer protocols (IP, TCP). By default it is
// configured to use half-duplex MAC (CSMA/CD).
//
module EthernetHost
{
    parameters:
        bool hasStatus = default(false);
        bool csmacdSupport = default(true);  // by default use CSMA/CD
        string fcsMode @enum("declared","computed") = default("declared");
        @networkNode();
        @labels(node,ethernet-node);
        @display("i=device/pc2");
        eth.csmacdSupport = this.csmacdSupport;
        *.interfaceTableModule = default(absPath(".interfaceTable"));
        *.fcsMode = this.fcsMode;
        eth.encap.typename = "OmittedEthernetEncapsulation";
        llc.registerProtocol = true;
    gates:
        inout ethg @labels(EtherFrame-conn);
    submodules:
        status: NodeStatus if hasStatus {
            @display("p=100,100;is=s");
        }
        interfaceTable: InterfaceTable {
            parameters:
                @display("p=100,240;is=s");
        }
        cli: EtherAppClient {
            parameters:
                interface = "eth";
                @display("p=350,100");
        }
        srv: EtherAppServer {
            parameters:
                @display("p=550,100");
        }
        dp: MessageDispatcher {
            parameters:
                @display("p=450,150;b=500,5,,,,1");
        }
        encap: <default("EthernetEncapsulation")> like IEthernetEncapsulation if typename != "" {
            parameters:
                registerProtocol = true;
                @display("p=350,250");
        }
        llc: <default("")> like IIeee8022Llc if typename != "" {
            parameters:
                @display("p=550,200");
        }
        ethernet: <default("")> like IEthernetLayer if typename != "" {
            @display("p=550,300");
        }
        li: MessageDispatcher {
            parameters:
                @display("p=450,350;b=500,5");
        }
        eth: <default("EthernetInterface")> like IEthernetInterface {
            parameters:
                @display("p=350,400,row,150;q=txQueue");
        }
    connections:
        cli.out --> dp.in++;
        dp.out++ --> cli.in;
        srv.out --> dp.in++;
        dp.out++ --> srv.in;

        dp.out++ --> encap.upperLayerIn if exists(encap);
        encap.upperLayerOut --> dp.in++ if exists(encap);
        encap.lowerLayerOut --> li.in++ if exists(encap);
        li.out++ --> encap.lowerLayerIn if exists(encap);

        dp.out++ --> llc.upperLayerIn if exists(llc);
        llc.upperLayerOut --> dp.in++ if exists(llc);
        llc.lowerLayerOut --> ethernet.upperLayerIn if exists(llc) && exists(ethernet);
        ethernet.upperLayerOut --> llc.lowerLayerIn if exists(llc) && exists(ethernet);
        ethernet.lowerLayerOut --> li.in++ if exists(ethernet);
        li.out++ --> ethernet.lowerLayerIn if exists(ethernet);

        li.out++ --> eth.upperLayerIn;
        eth.upperLayerOut --> li.in++;

        eth.phys <--> { @display("m=s"); } <--> ethg;
}

