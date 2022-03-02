//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.osg.integrated;

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
import inet.visualizer.contract.IPacketFlowVisualizer;
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
        @display("i=block/app");
        *.visualizationTargetModule = default("^.^");
        *.visualizationSubjectModule = default("^.^");
    submodules:
        sceneVisualizer: <default("SceneOsgVisualizer")> like ISceneVisualizer if typename != "" {
            parameters:
                @display("p=150,100");
        }
        physicalEnvironmentVisualizer: <default("PhysicalEnvironmentOsgVisualizer")> like IPhysicalEnvironmentVisualizer if typename != "" {
            parameters:
                @display("p=150,200");
        }
        networkNodeVisualizer: <default("NetworkNodeOsgVisualizer")> like INetworkNodeVisualizer if typename != "" {
            parameters:
                @display("p=150,300");
        }
        networkConnectionVisualizer: <default("NetworkConnectionOsgVisualizer")> like INetworkConnectionVisualizer if typename != "" {
            parameters:
                @display("p=150,400");
        }
        mobilityVisualizer: <default("MobilityOsgVisualizer")> like IMobilityVisualizer if typename != "" {
            parameters:
                @display("p=150,500");
        }
        statisticVisualizer: <default("StatisticOsgVisualizer")> like IStatisticVisualizer if typename != "" {
            parameters:
                @display("p=150,600");
        }
        infoVisualizer: <default("InfoOsgVisualizer")> like IInfoVisualizer if typename != "" {
            parameters:
                @display("p=150,700");
        }
        energyStorageVisualizer: <default("EnergyStorageOsgVisualizer")> like IEnergyStorageVisualizer if typename != "" {
            parameters:
                @display("p=150,800");
        }
        mediumVisualizer: <default("MediumOsgVisualizer")> like IMediumVisualizer if typename != "" {
            parameters:
                @display("p=150,900");
        }
        obstacleLossVisualizer: <default("TracingObstacleLossOsgVisualizer")> like ITracingObstacleLossVisualizer if typename != "" {
            parameters:
                @display("p=150,1000");
        }
        packetDropVisualizer: <default("PacketDropOsgVisualizer")> like IPacketDropVisualizer if typename != "" {
            parameters:
                @display("p=150,1100");
        }
        packetFlowVisualizer: <default("PacketFlowOsgVisualizer")> like IPacketFlowVisualizer if typename != "" {
            parameters:
                @display("p=150,1200");
        }
        queueVisualizer: <default("QueueOsgVisualizer")> like IQueueVisualizer if typename != "" {
            parameters:
                @display("p=450,100");
        }
        gateScheduleVisualizer: <default("GateScheduleOsgVisualizer")> like IGateScheduleVisualizer if typename != "" {
            parameters:
                @display("p=450,200");
        }
        radioVisualizer: <default("RadioOsgVisualizer")> like IRadioVisualizer if typename != "" {
            parameters:
                @display("p=450,300");
        }
        physicalLinkVisualizer: <default("PhysicalLinkOsgVisualizer")> like IPhysicalLinkVisualizer if typename != "" {
            parameters:
                @display("p=450,400");
        }
        interfaceTableVisualizer: <default("InterfaceTableOsgVisualizer")> like IInterfaceTableVisualizer if typename != "" {
            parameters:
                @display("p=450,500");
        }
        dataLinkVisualizer: <default("DataLinkOsgVisualizer")> like IDataLinkVisualizer if typename != "" {
            parameters:
                @display("p=450,600");
        }
        linkBreakVisualizer: <default("LinkBreakOsgVisualizer")> like ILinkBreakVisualizer if typename != "" {
            parameters:
                @display("p=450,700");
        }
        ieee80211Visualizer: <default("Ieee80211OsgVisualizer")> like IIeee80211Visualizer if typename != "" {
            parameters:
                @display("p=450,800");
        }
        networkRouteVisualizer: <default("NetworkRouteOsgVisualizer")> like INetworkRouteVisualizer if typename != "" {
            parameters:
                @display("p=450,900");
        }
        routingTableVisualizer: <default("RoutingTableOsgVisualizer")> like IRoutingTableVisualizer if typename != "" {
            parameters:
                @display("p=450,1000");
        }
        transportRouteVisualizer: <default("TransportRouteOsgVisualizer")> like ITransportRouteVisualizer if typename != "" {
            parameters:
                @display("p=450,1100");
        }
        transportConnectionVisualizer: <default("TransportConnectionOsgVisualizer")> like ITransportConnectionVisualizer if typename != "" {
            parameters:
                @display("p=450,1200");
        }
}

