package inet.examples.rtp.shutdownrestart;

import inet.base.LifecycleController;
import inet.networklayer.autorouting.ipv4.IPv4NetworkConfigurator;
import inet.transport.rtp.RTPHost;
import inet.world.scenario.ScenarioManager;


network RTPNetwork
{
    types:
        channel ethernet extends ned.DatarateChannel
        {
            parameters:
                delay = normal(0.00015s, 0.00005s);
                datarate = 10Mbps;
                ber = 0.00001;
        }

    submodules:
        sender: RTPHost {
            parameters:
                profileName = "inet.transport.rtp.RTPAVProfile";
                destinationAddress = "receiver";
                portNumber = 5004;
                bandwidth = 8000;
                @display("p=91,227");
        }
        receiver: RTPHost {
            parameters:
                profileName = "inet.transport.rtp.RTPAVProfile";
                destinationAddress = "sender";
                portNumber = 5004;
                bandwidth = 8000;
                @display("p=295,227");
        }
        configurator: IPv4NetworkConfigurator {
            @display("p=83,139");
        }
        scenarioManager: ScenarioManager {
            @display("p=83,31");
            script = xml("<script>\n" +
                "<at t='30s'><tell module='lifecycleController' operation='NodeShutdownOperation' target='sender'/></at>\n" +
                "<at t='60s'><tell module='lifecycleController' operation='NodeStartOperation' target='sender'/></at>\n" +
                "<at t='90s'><tell module='lifecycleController' operation='NodeShutdownOperation' target='receiver'/></at>\n" +
                "<at t='120s'><tell module='lifecycleController' operation='NodeStartOperation' target='receiver'/></at>\n" +
                "<at t='150s'><tell module='lifecycleController' operation='NodeShutdownOperation' target='receiver'/></at>\n" +
                "<at t='180s'><tell module='lifecycleController' operation='NodeStartOperation' target='receiver'/></at>\n" +
                "<at t='210s'><tell module='lifecycleController' operation='NodeShutdownOperation' target='sender'/></at>\n" +
                "<at t='240s'><tell module='lifecycleController' operation='NodeStartOperation' target='sender'/></at>\n" +
                "<at t='270s'><tell module='lifecycleController' operation='NodeShutdownOperation' target='sender'/></at>\n" +
                "<at t='300s'><tell module='lifecycleController' operation='NodeStartOperation' target='sender'/></at>\n" +
                "</script>");
        }
        lifecycleController: LifecycleController {
            @display("p=83,91");
        }
    connections:
        sender.pppg++ <--> ethernet <--> receiver.pppg++;
}
