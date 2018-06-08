//
// Copyright (C) OpenSim Ltd.
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

package inet.visualizer.integrated;

import inet.visualizer.contract.IDataLinkVisualizer;
import inet.visualizer.contract.IEnergyStorageVisualizer;
import inet.visualizer.contract.IIeee80211Visualizer;
import inet.visualizer.contract.IInfoVisualizer;
import inet.visualizer.contract.IIntegratedVisualizer;
import inet.visualizer.contract.IInterfaceTableVisualizer;
import inet.visualizer.contract.ILinkBreakVisualizer;
import inet.visualizer.contract.IMediumVisualizer;
import inet.visualizer.contract.IMobilityVisualizer;
import inet.visualizer.contract.INetworkConnectionVisualizer;
import inet.visualizer.contract.INetworkNodeVisualizer;
import inet.visualizer.contract.INetworkRouteVisualizer;
import inet.visualizer.contract.IPacketDropVisualizer;
import inet.visualizer.contract.IPhysicalEnvironmentVisualizer;
import inet.visualizer.contract.IPhysicalLinkVisualizer;
import inet.visualizer.contract.IQueueVisualizer;
import inet.visualizer.contract.IRadioVisualizer;
import inet.visualizer.contract.IRoutingTableVisualizer;
import inet.visualizer.contract.ISceneVisualizer;
import inet.visualizer.contract.IStatisticVisualizer;
import inet.visualizer.contract.ITracingObstacleLossVisualizer;
import inet.visualizer.contract.ITransportConnectionVisualizer;
import inet.visualizer.contract.ITransportRouteVisualizer;

//
// This module integrates all canvas visualizers into a single module. It
// supports disabling any submodule visualizer by providing an empty string as
// its type.
//
// @see ~IntegratedOsgVisualizer, ~IntegratedVisualizer, ~IIntegratedVisualizer.
//
module IntegratedMultiCanvasVisualizer like IIntegratedVisualizer
{
    parameters:
        @display("i=block/app_s");
        int numSceneVisualizers = default(1);
        int numPhysicalEnvironmentVisualizers = default(1);
        int numNetworkNodeVisualizers = default(1);
        int numNetworkConnectionVisualizers = default(1);
        int numMobilityVisualizers = default(1);
        int numStatisticVisualizers = default(1);
        int numInfoVisualizers = default(1);
        int numEnergyStorageVisualizers = default(1);
        int numMediumVisualizers = default(1);
        int numObstacleLossVisualizers = default(1);
        int numPacketDropVisualizers = default(1);
        int numQueueVisualizers = default(1);
        int numRadioVisualizers = default(1);
        int numPhysicalLinkVisualizers = default(1);
        int numInterfaceTableVisualizers = default(1);
        int numDataLinkVisualizers = default(1);
        int numLinkBreakVisualizers = default(1);
        int numIeee80211Visualizers = default(1);
        int numNetworkRouteVisualizers = default(1);
        int numRoutingTableVisualizers = default(1);
        int numTransportRouteVisualizers = default(1);
        int numTransportConnectionVisualizers = default(1);
   		*.networkNodeVisualizerModule = default(absPath(".networkNodeVisualizer[0]"));
    submodules:
        sceneVisualizer[numSceneVisualizers]: <default("SceneCanvasVisualizer")> like ISceneVisualizer {
            parameters:
                @display("p=150,50,row,25");
        }
        physicalEnvironmentVisualizer[numPhysicalEnvironmentVisualizers]: <default("PhysicalEnvironmentCanvasVisualizer")> like IPhysicalEnvironmentVisualizer {
            parameters:
                @display("p=150,150,row,25");
        }
        networkNodeVisualizer[numNetworkNodeVisualizers]: <default("NetworkNodeCanvasVisualizer")> like INetworkNodeVisualizer {
            parameters:
                @display("p=150,250,row,25");
        }
        networkConnectionVisualizer[numNetworkConnectionVisualizers]: <default("NetworkConnectionCanvasVisualizer")> like INetworkConnectionVisualizer {
            parameters:
                @display("p=150,350,row,25");
        }
        mobilityVisualizer[numMobilityVisualizers]: <default("MobilityCanvasVisualizer")> like IMobilityVisualizer {
            parameters:
                @display("p=150,450,row,25");
        }
        statisticVisualizer[numStatisticVisualizers]: <default("StatisticCanvasVisualizer")> like IStatisticVisualizer {
            parameters:
                @display("p=150,550,row,25");
        }
        infoVisualizer[numInfoVisualizers]: <default("InfoCanvasVisualizer")> like IInfoVisualizer {
            parameters:
                @display("p=150,650,row,25");
        }
        energyStorageVisualizer[numEnergyStorageVisualizers]: <default("EnergyStorageCanvasVisualizer")> like IEnergyStorageVisualizer {
            parameters:
                @display("p=150,750,row,25");
        }
        mediumVisualizer[numMediumVisualizers]: <default("MediumCanvasVisualizer")> like IMediumVisualizer {
            parameters:
                @display("p=150,850,row,25");
        }
        obstacleLossVisualizer[numObstacleLossVisualizers]: <default("TracingObstacleLossCanvasVisualizer")> like ITracingObstacleLossVisualizer {
            parameters:
                @display("p=150,950,row,25");
        }
        packetDropVisualizer[numPacketDropVisualizers]: <default("PacketDropCanvasVisualizer")> like IPacketDropVisualizer {
            parameters:
                @display("p=150,1050,row,25");
        }
        queueVisualizer[numQueueVisualizers]: <default("QueueCanvasVisualizer")> like IQueueVisualizer {
            parameters:
                @display("p=450,50,row,25");
        }
        radioVisualizer[numRadioVisualizers]: <default("RadioCanvasVisualizer")> like IRadioVisualizer {
            parameters:
                @display("p=450,150,row,25");
        }
        physicalLinkVisualizer[numPhysicalLinkVisualizers]: <default("PhysicalLinkCanvasVisualizer")> like IPhysicalLinkVisualizer {
            parameters:
                @display("p=450,250,row,25");
        }
        interfaceTableVisualizer[numInterfaceTableVisualizers]: <default("InterfaceTableCanvasVisualizer")> like IInterfaceTableVisualizer {
            parameters:
                @display("p=450,350,row,25");
        }
        dataLinkVisualizer[numDataLinkVisualizers]: <default("DataLinkCanvasVisualizer")> like IDataLinkVisualizer {
            parameters:
                @display("p=450,450,row,25");
        }
        linkBreakVisualizer[numLinkBreakVisualizers]: <default("LinkBreakCanvasVisualizer")> like ILinkBreakVisualizer {
            parameters:
                @display("p=450,550,row,25");
        }
        ieee80211Visualizer[numIeee80211Visualizers]: <default("Ieee80211CanvasVisualizer")> like IIeee80211Visualizer {
            parameters:
                @display("p=450,650,row,25,row,25");
        }
        networkRouteVisualizer[numNetworkRouteVisualizers]: <default("NetworkRouteCanvasVisualizer")> like INetworkRouteVisualizer {
            parameters:
                @display("p=450,750,row,25");
        }
        routingTableVisualizer[numRoutingTableVisualizers]: <default("RoutingTableCanvasVisualizer")> like IRoutingTableVisualizer {
            parameters:
                @display("p=450,850,row,25");
        }
        transportRouteVisualizer[numTransportRouteVisualizers]: <default("TransportRouteCanvasVisualizer")> like ITransportRouteVisualizer {
            parameters:
                @display("p=450,950,row,25");
        }
        transportConnectionVisualizer[numTransportConnectionVisualizers]: <default("TransportConnectionCanvasVisualizer")> like ITransportConnectionVisualizer {
            parameters:
                @display("p=450,1050,row,25");
        }
}
