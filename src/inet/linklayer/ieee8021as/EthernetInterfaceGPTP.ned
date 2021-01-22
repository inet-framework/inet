//
// @authors: Enkhtuvshin Janchivnyambuu
//           Henning Puttnies
//           Peter Danielis
//           University of Rostock, Germany
// 

package inet.linklayer.ieee8021as;

import inet.linklayer.contract.IEtherMac;
import inet.linklayer.contract.IEthernetInterface;
import inet.linklayer.ethernet.contract.IEthernetEncapsulation;
import inet.networklayer.common.NetworkInterface;
import inet.queueing.contract.ITrafficConditioner;


module EthernetInterfaceGPTP extends NetworkInterface like IEthernetInterface
{
    parameters:
        @lifecycleSupport;
        string interfaceTableModule;
        string address = default("auto");   // MAC address as hex string (12 hex digits), or
                                            // "auto". "auto" values will be replaced by
                                            // a generated MAC address in init stage 0.
        bool duplexMode = default(true);      // selects full-duplex (true) or half-duplex (false) operation
        bool csmacdSupport = default(!duplexMode);  // by default CSMA/CD is turned off, so only point-to-point duplex links are supported.
        double bitrate @unit(bps) = default(0 bps);
        string fcsMode @enum("declared","computed") = default("declared");
        *.interfaceTableModule = default(absPath(interfaceTableModule));
        *.fcsMode = fcsMode;
        *.duplexMode = duplexMode;
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input cutthroughIn @loose;
        output cutthroughOut @loose;
        inout phys @labels(EtherFrame);
    submodules:
        egressTC: <default("")> like ITrafficConditioner if typename != "" {
            parameters:
                @display("p=300,100");
        }
        ingressTC: <default("")> like ITrafficConditioner if typename != "" {
            parameters:
                @display("p=500,100");
        }
        encap: <default("EthernetEncapsulation")> like IEthernetEncapsulation {
            parameters:
                @display("p=400,200");
        }
        mac: <default(csmacdSupport ? "EthernetCsmaMac" : "EthernetMac")> like IEtherMac {
            parameters:
                queue.packetCapacity = default(10000);
                @display("p=400,400");
        }
        etherGPTP: EtherGPTP {
            @display("p=400,300");
        }
    connections:
        etherGPTP.upperLayerOut --> encap.lowerLayerIn;
        etherGPTP.upperLayerIn <-- encap.lowerLayerOut;

        mac.upperLayerOut --> etherGPTP.lowerLayerIn;
        mac.upperLayerIn <-- etherGPTP.lowerLayerOut;

        mac.phys <--> { @display("m=s"); } <--> phys;

        encap.upperLayerOut --> { @display("m=n"); } --> upperLayerOut if !exists(ingressTC);
        encap.upperLayerOut --> ingressTC.in if exists(ingressTC);
        ingressTC.out --> { @display("m=n"); } --> upperLayerOut if exists(ingressTC);

        upperLayerIn --> { @display("m=n"); } --> encap.upperLayerIn if !exists(egressTC);
        upperLayerIn --> { @display("m=n"); } --> egressTC.in if exists(egressTC);
        egressTC.out --> encap.upperLayerIn if exists(egressTC);

}

