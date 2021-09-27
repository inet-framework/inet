//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

package _5gns.simulations;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;
import inet.visualizer.integrated.IntegratedVisualizer;
import inet.visualizer.linklayer.InterfaceTableVisualizer;
import ned.DatarateChannel;
import inet.node.internetcloud.InternetCloud;
//import inet.applications.generic.linkAdjuster;

network htbEvaluation
{
    parameters:
        @display("bgb=710,417");
        int nVID = default(1); // Number of video clients
        int nLVD = default(1); // Number of live video clients
        int nFDO = default(1); // Number of file download clients
        int nSSH = default(1); // Number of SSH clients
        int nVIP = default(1); // Number of VoIP clients
    types:
        channel C extends DatarateChannel
        {
        }
    submodules:
        configurator: Ipv4NetworkConfigurator {
            @display("p=10,10;is=s");
        }
        
       
        hostFDO[nFDO]: StandardHost {
            @display("p=140,203");
        }
        
        visualizer: IntegratedVisualizer {
            @display("p=18,71");
        }
    
        serverFDO: StandardHost {
            @display("p=565,203");
        }
  
    connections:

        hostFDO[0].pppg++ <--> connFIX0: C {  } <--> serverFDO.pppg++;

        
        
}
