//
// Copyright (C) 2005 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.examples.mobility;

import inet.common.scenario.ScenarioManager;
import inet.environment.common.PhysicalEnvironment;
import inet.visualizer.canvas.integrated.IntegratedCanvasVisualizer;

network MobileNetwork
{
    parameters:
        int numHosts;
        bool hasVisualizer = default(false);
        bool hasScenarioManager = default(false);
        bool hasEnvironment = default(false);
        @display("bgb=600,400");
    submodules:
        visualizer: IntegratedCanvasVisualizer if hasVisualizer {
            parameters:
                @display("p=-100,100");
        }
        host[numHosts]: MobileHost {
            parameters:
                @display("p=300,200;r=,,#707070");
        }
        environment: PhysicalEnvironment if hasEnvironment {
            parameters:
                @display("p=-100,200");
        }
        scenarioManager: ScenarioManager if hasScenarioManager {
            parameters:
                @display("p=-100,300;is=s");
        }
}
