//
// Copyright (C) 2005 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.showcases.wireless.ratecontrol;

import inet.environment.common.PhysicalEnvironment;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.WirelessHost;
import inet.physicallayer.wireless.ieee80211.packetlevel.Ieee80211RadioMedium;
import inet.visualizer.canvas.integrated.IntegratedCanvasVisualizer;


network RateControlShowcase
{
    parameters:
        @display("b=297,203;bgb=1300,460");
//        @statistic[bitrate](source=sourceHost.wlan[0].mac.dcf.rateControl.datarateChanged/1000000; record=figure; targetFigure=bitrateLinearGauge);
//        @figure[bitrateLinearGauge](type=linearGauge; pos=320,65; size=250,30; minValue=0; maxValue=54; tickSize=6);
        @statistic[throughput](source=liveThroughput(sinkHost.app[0].packetReceived)/1000000; record=figure; targetFigure=throughputPlot);
        @figure[throughputPlot](type=plot; pos=300,280; size=450,60; timeWindow=25; maxValue=30; valueTickSize=30; timeTickSize=5; label="Application level throughput"; numberSizeFactor=1.5);
        @statistic[numRcvdPk](source=count(sinkHost.app[0].packetReceived); record=figure; targetFigure=numRcvdPkCounter; checkSignals=false);
        @figure[numRcvdPkCounter](type=counter; pos=785,285; label="Packets received"; decimalPlaces=5);


    submodules:
        sourceHost: WirelessHost {
            parameters:
                @display("r=,,#707070");
        }
        sinkHost: WirelessHost {
            parameters:
                @display("p=200,200;r=,,#707070");
        }
        physicalEnvironment: PhysicalEnvironment {
            parameters:
                @display("p=131.53847,86.15385");
        }
        radioMedium: Ieee80211RadioMedium {
            parameters:
                @display("p=130.76924,168.46155");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=131.53847,253.84616");
        }
        visualizer: IntegratedCanvasVisualizer {
            @display("p=131.53847,336.15387");
        }
}

