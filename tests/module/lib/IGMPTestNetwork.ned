import inet.nodes.ethernet.EtherSwitch;
import ned.DatarateChannel;
import inet.nodes.inet.StandardHost;
import inet.nodes.inet.Router;
import inet.networklayer.autorouting.ipv4.IPv4Configurator;
import inet.world.scenario.ScenarioManager;

network IGMPTestNetwork
{
    types:
        channel ethernetline extends DatarateChannel
        {
            delay = 0.1us;
            datarate = 10Mbps;
        }
    submodules:
        configurator: IPv4Configurator
        {
            config = default(xml("\
               <config>\
                 <interface hosts='*' address='10.x.x.x' netmask='255.0.0.0'/>\
                 <interface hosts='router1' address='10.0.0.1' netmask='255.0.0.0'/>\
               </config>"));
        };
        scenarioManager: ScenarioManager {
            @display("p=73,42");
        }
        tester: IGMPTester {
            @display("p=178,50");
        }
        host: StandardHost {
            @display("p=122,152");
        }
        // Querier router
        router: Router {
            @display("p=416,152");
        }
        etherSwitch: EtherSwitch {
            @display("p=278,151");
        }
        // Non-querier router
        nqrouter: Router {
            @display("p=278,269");
        }
    connections:
        host.ethg++ <--> ethernetline <--> etherSwitch.ethg++;
        etherSwitch.ethg++ <--> ethernetline <--> router.ethg++;
        etherSwitch.ethg++ <--> ethernetline <--> nqrouter.ethg++;
}
