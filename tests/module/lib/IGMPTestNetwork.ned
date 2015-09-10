import inet.node.ethernet.EtherSwitch;
import ned.DatarateChannel;
import inet.node.inet.StandardHost;
import inet.node.inet.Router;
import inet.networklayer.configurator.ipv4.IPv4NetworkConfigurator;
import inet.common.scenario.ScenarioManager;

network IGMPTestNetwork
{
    types:
        channel ethernetline extends DatarateChannel
        {
            delay = 0.1us;
            datarate = 10Mbps;
        }
    submodules:
        configurator: IPv4NetworkConfigurator {
            config = default(xml("\
               <config>\
                 <interface hosts='*' address='10.x.x.x' netmask='255.0.0.0'/>\
                 <interface hosts='router1' address='10.0.0.1' netmask='255.0.0.0'/>\
               </config>"));
            @display("p=347,31");
        }
        scenarioManager: ScenarioManager {
            @display("p=73,42");
        }
        host: StandardHost {
            @display("p=221,50");
        }
        host2: StandardHost {
            @display("p=81,152");
        }
        // Querier router
        router: Router {
            @display("p=384,152");
            multicastForwarding = true;
        }
        etherSwitch: EtherSwitch {
            @display("p=221,151");
        }
        // Non-querier router
        nqrouter: Router {
            @display("p=221,275");
            multicastForwarding = true;
        }
    connections:
        host.ethg++ <--> ethernetline <--> etherSwitch.ethg++;
        host2.ethg++ <--> ethernetline <--> etherSwitch.ethg++;
        etherSwitch.ethg++ <--> ethernetline <--> router.ethg++;
        etherSwitch.ethg++ <--> ethernetline <--> nqrouter.ethg++;
}
