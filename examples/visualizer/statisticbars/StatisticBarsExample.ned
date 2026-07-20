//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.examples.visualizer.statisticbars;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.ethernet.Eth100M;
import inet.node.ethernet.EthernetSwitch;
import inet.node.inet.StandardHost;
import inet.visualizer.contract.IIntegratedVisualizer;

//
// Demonstrates the ~StatisticVisualizer bar chart mode with several UDP traffic
// streams flowing from the server to the receiver. A bar chart above the
// receiver shows a per-stream quantity; the ini file selects what:
//
//  - `Packets`    - number of packets received per stream (one bar per sink app,
//                   `barSeries = "sources"`, `statisticExpression = "count"`).
//  - `Throughput` - throughput per stream (same, `statisticExpression = "throughput"`).
//  - `Flow`       - throughput per named flow, demultiplexed from a single signal
//                   by the packet's flow tag (`barSeries = "flow"`).
//
network StatisticBarsExample
{
    submodules:
        visualizer: <default("IntegratedCanvasVisualizer")> like IIntegratedVisualizer {
            @display("p=100,80;is=s");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=100,160;is=s");
        }
        server: StandardHost {
            @display("p=200,350");
        }
        switch: EthernetSwitch {
            @display("p=450,350");
        }
        receiver: StandardHost {
            @display("p=700,350");
        }
    connections:
        server.ethg++ <--> Eth100M <--> switch.ethg++;
        switch.ethg++ <--> Eth100M <--> receiver.ethg++;
}
