package inet.test.unittest.lib;

import inet.clock.contract.IClock;
import inet.common.scenario.ScenarioManager;

simple ClockTestApp
{
    parameters:
        string clockModule; // relative path of a module that implements IClock; optional
        @class(::inet::ClockTestApp);
}

module ClockTestHost
{
    submodules:
        app: ClockTestApp {
            @display("p=100,100");
            clockModule = "^.clock";
        }
        clock: <> like IClock {
            parameters:
                @display("p=200,100");
        }
}

network ClockTestNetwork
{
    submodules:
        host: ClockTestHost {
            @display("p=200,100");
        }
        scenarioManager: ScenarioManager {
            @display("p=100,100;is=s");
        }
}

