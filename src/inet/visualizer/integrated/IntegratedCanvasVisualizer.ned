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
module IntegratedCanvasVisualizer like IIntegratedVisualizer
{
    parameters:
        @display("i=block/app_s");
    submodules:
        sceneVisualizer: <default("SceneCanvasVisualizer")> like ISceneVisualizer if typename != "" {
            parameters:
                @display("p=150,50");
        }
        physicalEnvironmentVisualizer: <default("PhysicalEnvironmentCanvasVisualizer")> like IPhysicalEnvironmentVisualizer if typename != "" {
            parameters:
                @display("p=150,150");
        }
        networkNodeVisualizer: <default("NetworkNodeCanvasVisualizer")> like INetworkNodeVisualizer if typename != "" {
            parameters:
                @display("p=150,250");
        }
        networkConnectionVisualizer: <default("NetworkConnectionCanvasVisualizer")> like INetworkConnectionVisualizer if typename != "" {
            parameters:
                @display("p=150,350");
        }
        mobilityVisualizer: <default("MobilityCanvasVisualizer")> like IMobilityVisualizer if typename != "" {
            parameters:
                @display("p=150,450");
        }
        statisticVisualizer: <default("StatisticCanvasVisualizer")> like IStatisticVisualizer if typename != "" {
            parameters:
                @display("p=150,550");
        }
        infoVisualizer: <default("InfoCanvasVisualizer")> like IInfoVisualizer if typename != "" {
            parameters:
                @display("p=150,650");
        }
        energyStorageVisualizer: <default("EnergyStorageCanvasVisualizer")> like IEnergyStorageVisualizer if typename != "" {
            parameters:
                @display("p=150,750");
        }
        mediumVisualizer: <default("MediumCanvasVisualizer")> like IMediumVisualizer if typename != "" {
            parameters:
                @display("p=150,850");
        }
        obstacleLossVisualizer: <default("TracingObstacleLossCanvasVisualizer")> like ITracingObstacleLossVisualizer if typename != "" {
            parameters:
                @display("p=150,950");
        }
        packetDropVisualizer: <default("PacketDropCanvasVisualizer")> like IPacketDropVisualizer if typename != "" {
            parameters:
                @display("p=150,1050");
        }
        queueVisualizer: <default("QueueCanvasVisualizer")> like IQueueVisualizer if typename != "" {
            parameters:
                @display("p=450,50");
        }
        radioVisualizer: <default("RadioCanvasVisualizer")> like IRadioVisualizer if typename != "" {
            parameters:
                @display("p=450,150");
        }
        physicalLinkVisualizer: <default("PhysicalLinkCanvasVisualizer")> like IPhysicalLinkVisualizer if typename != "" {
            parameters:
                @display("p=450,250");
        }
        interfaceTableVisualizer: <default("InterfaceTableCanvasVisualizer")> like IInterfaceTableVisualizer if typename != "" {
            parameters:
                @display("p=450,350");
        }
        dataLinkVisualizer: <default("DataLinkCanvasVisualizer")> like IDataLinkVisualizer if typename != "" {
            parameters:
                @display("p=450,450");
        }
        linkBreakVisualizer: <default("LinkBreakCanvasVisualizer")> like ILinkBreakVisualizer if typename != "" {
            parameters:
                @display("p=450,550");
        }
        ieee80211Visualizer: <default("Ieee80211CanvasVisualizer")> like IIeee80211Visualizer if typename != "" {
            parameters:
                @display("p=450,650");
        }
        networkRouteVisualizer: <default("NetworkRouteCanvasVisualizer")> like INetworkRouteVisualizer if typename != "" {
            parameters:
                @display("p=450,750");
        }
        routingTableVisualizer: <default("RoutingTableCanvasVisualizer")> like IRoutingTableVisualizer if typename != "" {
            parameters:
                @display("p=450,850");
        }
        transportRouteVisualizer: <default("TransportRouteCanvasVisualizer")> like ITransportRouteVisualizer if typename != "" {
            parameters:
                @display("p=450,950");
        }
        transportConnectionVisualizer: <default("TransportConnectionCanvasVisualizer")> like ITransportConnectionVisualizer if typename != "" {
            parameters:
                @display("p=450,1050");
        }
}
