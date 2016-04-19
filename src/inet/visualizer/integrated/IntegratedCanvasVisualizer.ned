//
// Copyright (C) 2006 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

package inet.visualizer.integrated;

import inet.visualizer.contract.IIntegratedVisualizer;
import inet.visualizer.contract.ILinkVisualizer;
import inet.visualizer.contract.IMediumVisualizer;
import inet.visualizer.contract.IMobilityVisualizer;
import inet.visualizer.contract.INetworkConnectionVisualizer;
import inet.visualizer.contract.INetworkNodeVisualizer;
import inet.visualizer.contract.IPhysicalEnvironmentVisualizer;
import inet.visualizer.contract.IRouteVisualizer;
import inet.visualizer.contract.ISceneVisualizer;
import inet.visualizer.contract.IStatisticVisualizer;
import inet.visualizer.contract.ITracingObstacleLossVisualizer;

//
// This module integrates all canvas visualizers into a single module. It
// supports disabling any submodule visualizer by providing an empty string as
// its type.
//
// @see ~IntegratedOsgVisualizer, ~IntegratedVisualizer, and
// ~IIntegratedVisualizer.
//
module IntegratedCanvasVisualizer like IIntegratedVisualizer
{
    parameters:
        @display("bgb=0,0;i=block/app_s");
        string sceneVisualizerType = default("SceneCanvasVisualizer");
        string physicalEnvironmentVisualizerType = default("PhysicalEnvironmentCanvasVisualizer");
        string networkNodeVisualizerType = default("NetworkNodeCanvasVisualizer");
        string networkConnectionVisualizerType = default("NetworkConnectionCanvasVisualizer");
        string mobilityVisualizerType = default("MobilityCanvasVisualizer");
        string mediumVisualizerType = default("MediumCanvasVisualizer");
        string obsacleLossVisualizerType = default("TracingObstacleLossCanvasVisualizer");
        string linkVisualizerType = default("LinkCanvasVisualizer");
        string routeVisualizerType = default("RouteCanvasVisualizer");
        string statisticVisualizerType = default("StatisticCanvasVisualizer");
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
        mediumVisualizer: <mediumVisualizerType> like IMediumVisualizer if mediumVisualizerType != "" {
            parameters:
                @display("p=150,550");
        }
        obsacleLossVisualizer: <obsacleLossVisualizerType> like ITracingObstacleLossVisualizer if obsacleLossVisualizerType != "" {
            parameters:
                @display("p=150,650");
        }
        linkVisualizer: <linkVisualizerType> like ILinkVisualizer if linkVisualizerType != "" {
            parameters:
                @display("p=150,750");
        }
        routeVisualizer: <routeVisualizerType> like IRouteVisualizer if routeVisualizerType != "" {
            parameters:
                @display("p=150,850");
        }
        statisticVisualizer: <statisticVisualizerType> like IStatisticVisualizer if statisticVisualizerType != "" {
            parameters:
                @display("p=150,950");
        }
}
