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
// Demonstrates displaying several values of a statistic at once with the
// ~StatisticVisualizer, with several UDP traffic streams flowing from the server
// to the receiver. A bar chart above the receiver shows a per-stream quantity;
// the ini file selects what, and what displays it:
//
//  - `Packets`    - number of packets received per stream (one bar per sink app,
//                   `groupBy = "networkNode"`, `statisticExpression = "count"`).
//  - `Throughput` - throughput per stream (same, `statisticExpression = "throughput"`).
//  - `Flow`       - throughput per named flow, demultiplexed from a single signal
//                   by the packet's flow tag (`splitBy = "flow"`).
//  - `Gauge`      - throughput of a single stream on a gauge instead of a bar
//                   chart, to show that the figure is a configuration choice.
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
