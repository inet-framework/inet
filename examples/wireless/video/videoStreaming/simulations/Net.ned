//
//  VideoStreaming
// 
//  It's an example of video streaming with TCP, UDP protocols and the DASH techinique.
//  To do the video streaming with TCP we developed the TCPBasicVideoStream and with the DASH we developed the 
//  TCPVideoStreamCliApp and both are in src/inet/applications/tcpapp. 	
//  Created on 8 de dez de 2017 by Anderson Andrei da Silva & Patrick Menani Abrahão at University of São Paulo
//

package inet.examples.wireless.video.videoStreaming.simulations;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.AdhocHost;
import inet.node.inet.INetworkNode;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;
import inet.node.internetcloud.InternetCloud;
import inet.node.wireless.AccessPoint;
//import inet.physicallayer.idealradio.IdealRadioMedium;
import inet.visualizer.contract.IIntegratedVisualizer;
import inet.node.aodv.AodvRouter;
import inet.common.lifecycle.LifecycleController;
import inet.common.scenario.ScenarioManager;
//import inet.networklayer.configurator.ipv4.IPv4NetworkConfigurator;
import inet.networklayer.ipv4.RoutingTableRecorder;
//import inet.node.aodv.AODVRouter;
//import inet.physicallayer.idealradio.IdealRadioMedium;
//import inet.physicallayer.apskradio.packetlevel.ApskRadio;
import inet.physicallayer.unitdisk.UnitDiskRadioMedium;

network Net
{
    parameters:
        int numHosts;
        string hostType = default("AODVRouter");
        string mediumType = default("IdealRadioMedium");
        @display("bgb=1000,700");

    submodules:
        visualizer: <default("IntegratedCanvasVisualizer")> like IIntegratedVisualizer if hasVisualizer() {
            parameters:
                @display("p=61.1,14.3");
        }
        configurator: Ipv4NetworkConfigurator {
            parameters:
                @display("p=61.1,127.4");
        }
        radioMedium: UnitDiskRadioMedium {
            parameters:
                @display("p=59.8,67.6");
        }
        hostA: AodvRouter {
            @display("i=device/pocketpc_s;p=782.86206,544.9101");
        }
        hostB: AodvRouter {
            @display("i=device/pocketpc_s;p=145.15071,242.71104");
        }
        hostR[numHosts]: AodvRouter {
            parameters:
                @display("i=device/pocketpc_s;r=,,#707070;");
        }

}
