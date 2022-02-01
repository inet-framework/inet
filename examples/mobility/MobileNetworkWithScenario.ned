//
// Copyright (C) 2005 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.examples.mobility;

import inet.common.scenario.ScenarioManager;


network MobileNetworkWithScenario
{
    parameters:
        int numHosts;
        @display("bgb=600,400");
    submodules:
        host[numHosts]: MobileHost {
            parameters:
                @display("p=300,200;r=,,#707070");
        }
        scenarioManager: ScenarioManager {
            parameters:
                @display("p=100,100;is=s");
        }
}
