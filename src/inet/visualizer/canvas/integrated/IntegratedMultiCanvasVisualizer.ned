//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.canvas.integrated;

import inet.visualizer.contract.IDataLinkVisualizer;
import inet.visualizer.contract.IEnergyStorageVisualizer;
import inet.visualizer.contract.IGateScheduleVisualizer;
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
import inet.visualizer.contract.IStreamRedundancyConfigurationVisualizer;
import inet.visualizer.contract.ITracingObstacleLossVisualizer;
import inet.visualizer.contract.ITransportConnectionVisualizer;
import inet.visualizer.contract.ITransportRouteVisualizer;
import inet.visualizer.contract.IFailureProtectionConfigurationVisualizer;

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
        @display("i=block/app");
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
        int numGateScheduleVisualizers = default(1);
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
        int numFailureProtectionConfigurationVisualizers = default(1);
        int numStreamRedundancyConfigurationVisualizers = default(1);
        *.visualizationTargetModule = default("^.^");
        *.visualizationSubjectModule = default("^.^");
        *.networkNodeVisualizerModule = default(absPath(".networkNodeVisualizer[0]"));
    submodules:
        sceneVisualizer[numSceneVisualizers]: <default("SceneCanvasVisualizer")> like ISceneVisualizer {
            parameters:
                @display("p=150,100,row,100");
        }
        physicalEnvironmentVisualizer[numPhysicalEnvironmentVisualizers]: <default("PhysicalEnvironmentCanvasVisualizer")> like IPhysicalEnvironmentVisualizer {
            parameters:
                @display("p=150,200,row,100");
        }
        networkNodeVisualizer[numNetworkNodeVisualizers]: <default("NetworkNodeCanvasVisualizer")> like INetworkNodeVisualizer {
            parameters:
                @display("p=150,300,row,100");
        }
        networkConnectionVisualizer[numNetworkConnectionVisualizers]: <default("NetworkConnectionCanvasVisualizer")> like INetworkConnectionVisualizer {
            parameters:
                @display("p=150,400,row,100");
        }
        mobilityVisualizer[numMobilityVisualizers]: <default("MobilityCanvasVisualizer")> like IMobilityVisualizer {
            parameters:
                @display("p=150,500,row,100");
        }
        statisticVisualizer[numStatisticVisualizers]: <default("StatisticCanvasVisualizer")> like IStatisticVisualizer {
            parameters:
                @display("p=150,600,row,100");
        }
        infoVisualizer[numInfoVisualizers]: <default("InfoCanvasVisualizer")> like IInfoVisualizer {
            parameters:
                @display("p=150,700,row,100");
        }
        energyStorageVisualizer[numEnergyStorageVisualizers]: <default("EnergyStorageCanvasVisualizer")> like IEnergyStorageVisualizer {
            parameters:
                @display("p=150,800,row,100");
        }
        mediumVisualizer[numMediumVisualizers]: <default("MediumCanvasVisualizer")> like IMediumVisualizer {
            parameters:
                @display("p=150,900,row,100");
        }
        obstacleLossVisualizer[numObstacleLossVisualizers]: <default("TracingObstacleLossCanvasVisualizer")> like ITracingObstacleLossVisualizer {
            parameters:
                @display("p=150,1000,row,100");
        }
        packetDropVisualizer[numPacketDropVisualizers]: <default("PacketDropCanvasVisualizer")> like IPacketDropVisualizer {
            parameters:
                @display("p=150,1100,row,100");
        }
        queueVisualizer[numQueueVisualizers]: <default("QueueCanvasVisualizer")> like IQueueVisualizer {
            parameters:
                @display("p=450,100,row,100");
        }
        gateScheduleVisualizer[numGateScheduleVisualizers]: <default("GateScheduleCanvasVisualizer")> like IGateScheduleVisualizer {
            parameters:
                @display("p=450,200,row,100");
        }
        radioVisualizer[numRadioVisualizers]: <default("RadioCanvasVisualizer")> like IRadioVisualizer {
            parameters:
                @display("p=450,300,row,100");
        }
        physicalLinkVisualizer[numPhysicalLinkVisualizers]: <default("PhysicalLinkCanvasVisualizer")> like IPhysicalLinkVisualizer {
            parameters:
                @display("p=450,400,row,100");
        }
        interfaceTableVisualizer[numInterfaceTableVisualizers]: <default("InterfaceTableCanvasVisualizer")> like IInterfaceTableVisualizer {
            parameters:
                @display("p=450,500,row,100");
        }
        dataLinkVisualizer[numDataLinkVisualizers]: <default("DataLinkCanvasVisualizer")> like IDataLinkVisualizer {
            parameters:
                @display("p=450,600,row,100");
        }
        linkBreakVisualizer[numLinkBreakVisualizers]: <default("LinkBreakCanvasVisualizer")> like ILinkBreakVisualizer {
            parameters:
                @display("p=450,700,row,100");
        }
        ieee80211Visualizer[numIeee80211Visualizers]: <default("Ieee80211CanvasVisualizer")> like IIeee80211Visualizer {
            parameters:
                @display("p=450,800,row,25,row,100");
        }
        networkRouteVisualizer[numNetworkRouteVisualizers]: <default("NetworkRouteCanvasVisualizer")> like INetworkRouteVisualizer {
            parameters:
                @display("p=450,900,row,100");
        }
        routingTableVisualizer[numRoutingTableVisualizers]: <default("RoutingTableCanvasVisualizer")> like IRoutingTableVisualizer {
            parameters:
                @display("p=450,1000,row,100");
        }
        transportRouteVisualizer[numTransportRouteVisualizers]: <default("TransportRouteCanvasVisualizer")> like ITransportRouteVisualizer {
            parameters:
                @display("p=450,1100,row,100");
        }
        transportConnectionVisualizer[numTransportConnectionVisualizers]: <default("TransportConnectionCanvasVisualizer")> like ITransportConnectionVisualizer {
            parameters:
                @display("p=450,1200,row,100");
        }
        failureProtectionConfigurationVisualizer[numFailureProtectionConfigurationVisualizers]: <default("FailureProtectionConfigurationCanvasVisualizer")> like IFailureProtectionConfigurationVisualizer {
            parameters:
                @display("p=750,100");
        }
        streamRedundancyConfigurationVisualizer[numStreamRedundancyConfigurationVisualizers]: <default("StreamRedundancyConfigurationCanvasVisualizer")> like IStreamRedundancyConfigurationVisualizer {
            parameters:
                @display("p=750,200");
        }
}

