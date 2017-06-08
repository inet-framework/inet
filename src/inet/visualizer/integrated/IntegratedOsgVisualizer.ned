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
// This module integrates all osg visualizers into a single module. It supports
// disabling any submodule visualizer by providing an empty string as its type.
//
// @see ~IntegratedOsgVisualizer, ~IntegratedVisualizer, ~IIntegratedVisualizer.
//
module IntegratedOsgVisualizer like IIntegratedVisualizer
{
    parameters:
        @display("i=block/app_s");
        string sceneVisualizerType = default("SceneOsgVisualizer");
        string physicalEnvironmentVisualizerType = default("PhysicalEnvironmentOsgVisualizer");
        string networkNodeVisualizerType = default("NetworkNodeOsgVisualizer");
        string networkConnectionVisualizerType = default("NetworkConnectionOsgVisualizer");
        string mobilityVisualizerType = default("MobilityOsgVisualizer");
        string statisticVisualizerType = default("StatisticOsgVisualizer");
        string infoVisualizerType = default("InfoOsgVisualizer");
        string energyStorageVisualizerType = default("EnergyStorageOsgVisualizer");
        string mediumVisualizerType = default("MediumOsgVisualizer");
        string obstacleLossVisualizerType = default("TracingObstacleLossOsgVisualizer");
        string packetDropVisualizerType = default("PacketDropOsgVisualizer");
        string queueVisualizerType = default("QueueOsgVisualizer");
        string radioVisualizerType = default("RadioOsgVisualizer");
        string physicalLinkVisualizerType = default("PhysicalLinkOsgVisualizer");
        string interfaceTableVisualizerType = default("InterfaceTableOsgVisualizer");
        string dataLinkVisualizerType = default("DataLinkOsgVisualizer");
        string linkBreakVisualizerType = default("LinkBreakOsgVisualizer");
        string ieee80211VisualizerType = default("Ieee80211OsgVisualizer");
        string networkRouteVisualizerType = default("NetworkRouteOsgVisualizer");
        string routingTableVisualizerType = default("RoutingTableOsgVisualizer");
        string transportRouteVisualizerType = default("TransportRouteOsgVisualizer");
        string transportConnectionVisualizerType = default("TransportConnectionOsgVisualizer");
    submodules:
        sceneVisualizer: <sceneVisualizerType> like ISceneVisualizer if sceneVisualizerType != "" {
            parameters:
                @display("p=150,50");
        }
        physicalEnvironmentVisualizer: <physicalEnvironmentVisualizerType> like IPhysicalEnvironmentVisualizer if physicalEnvironmentVisualizerType != "" {
            parameters:
                @display("p=150,150");
        }
        networkNodeVisualizer: <networkNodeVisualizerType> like INetworkNodeVisualizer if networkNodeVisualizerType != "" {
            parameters:
                @display("p=150,250");
        }
        networkConnectionVisualizer: <networkConnectionVisualizerType> like INetworkConnectionVisualizer if networkConnectionVisualizerType != "" {
            parameters:
                @display("p=150,350");
        }
        mobilityVisualizer: <mobilityVisualizerType> like IMobilityVisualizer if mobilityVisualizerType != "" {
            parameters:
                @display("p=150,450");
        }
        statisticVisualizer: <statisticVisualizerType> like IStatisticVisualizer if statisticVisualizerType != "" {
            parameters:
                @display("p=150,550");
        }
        infoVisualizer: <infoVisualizerType> like IInfoVisualizer if infoVisualizerType != "" {
            parameters:
                @display("p=150,650");
        }
        energyStorageVisualizer: <energyStorageVisualizerType> like IEnergyStorageVisualizer if energyStorageVisualizerType != "" {
            parameters:
                @display("p=150,750");
        }
        mediumVisualizer: <mediumVisualizerType> like IMediumVisualizer if mediumVisualizerType != "" {
            parameters:
                @display("p=150,850");
        }
        obstacleLossVisualizer: <obstacleLossVisualizerType> like ITracingObstacleLossVisualizer if obstacleLossVisualizerType != "" {
            parameters:
                @display("p=150,950");
        }
        packetDropVisualizer: <packetDropVisualizerType> like IPacketDropVisualizer if packetDropVisualizerType != "" {
            parameters:
                @display("p=150,1050");
        }
        queueVisualizer: <queueVisualizerType> like IQueueVisualizer if queueVisualizerType != "" {
            parameters:
                @display("p=450,50");
        }
        radioVisualizer: <radioVisualizerType> like IRadioVisualizer if radioVisualizerType != "" {
            parameters:
                @display("p=450,150");
        }
        physicalLinkVisualizer: <physicalLinkVisualizerType> like IPhysicalLinkVisualizer if physicalLinkVisualizerType != "" {
            parameters:
                @display("p=450,250");
        }
        interfaceTableVisualizer: <interfaceTableVisualizerType> like IInterfaceTableVisualizer if interfaceTableVisualizerType != "" {
            parameters:
                @display("p=450,350");
        }
        dataLinkVisualizer: <dataLinkVisualizerType> like IDataLinkVisualizer if dataLinkVisualizerType != "" {
            parameters:
                @display("p=450,450");
        }
        linkBreakVisualizer: <linkBreakVisualizerType> like ILinkBreakVisualizer if linkBreakVisualizerType != "" {
            parameters:
                @display("p=450,550");
        }
        ieee80211Visualizer: <ieee80211VisualizerType> like IIeee80211Visualizer if ieee80211VisualizerType != "" {
            parameters:
                @display("p=450,650");
        }
        networkRouteVisualizer: <networkRouteVisualizerType> like INetworkRouteVisualizer if networkRouteVisualizerType != "" {
            parameters:
                @display("p=450,750");
        }
        routingTableVisualizer: <routingTableVisualizerType> like IRoutingTableVisualizer if routingTableVisualizerType != "" {
            parameters:
                @display("p=450,850");
        }
        transportRouteVisualizer: <transportRouteVisualizerType> like ITransportRouteVisualizer if transportRouteVisualizerType != "" {
            parameters:
                @display("p=450,950");
        }
        transportConnectionVisualizer: <transportConnectionVisualizerType> like ITransportConnectionVisualizer if transportConnectionVisualizerType != "" {
            parameters:
                @display("p=450,1050");
        }
}
