package inet.tutorials.queueing;

import inet.applications.contract.IApp;
import inet.applications.tcpapp.TcpServerSocketIo;
import inet.applications.tcpapp.TelnetClientApp;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.ethernet.Eth100M;
import inet.node.inet.StandardHost;
import inet.queueing.common.PacketCloner;
import inet.queueing.common.PacketMultiplexer;
import inet.queueing.contract.IActivePacketSource;
import inet.queueing.contract.IPacketClassifier;
import inet.queueing.contract.IPacketScheduler;
import inet.queueing.contract.IPacketServer;
import inet.queueing.contract.IPassivePacketSink;
import inet.queueing.contract.IPassivePacketSource;
import inet.queueing.tokengenerator.PacketBasedTokenGenerator;


network TelnetNetwork_
{
    submodules:
        client: StandardHost {
            @display("p=100,100");
        }
        server: StandardHost {
            @display("p=200,100");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=50,50");
        }
    connections:
        client.ethg++ <--> Eth100M <--> server.ethg++;
}

//network Tel2
//{
//    submodules:
//        client: TelnetClientApp;
//        server: TelnetServerConnection;
//    connections:
//        client.socketOut --> server.socketIn;
//        client.socketIn <-- server.socketOut;
//}

network TelnetNetwork
{
    @display("bgb=400,200");
    submodules:
        client: TelnetClientTraffic {
            @display("p=100,100");
        }
        server: TelnetServerTraffic
        {
            @display("p=300,100");
        }
    connections:
        client.out --> server.in;
        client.in <-- server.out;
}

module TelnetClientTraffic
{
    parameters:
        @display("i=block/app");
    gates:
        input in;
        output out;
    submodules:
        characterProducer: <default("ActivePacketSource")> like IActivePacketSource {
            parameters:
                packetLength = 1B;
                packetData = intuniform(97, 122); // lower case ASCII characters
                productionInterval = uniform(0.1s, 0.2s); // typing speed between 5 and 10 characters per second
                @display("p=100,100");
        }
        enterProducer: <default("ActivePacketSource")> like IActivePacketSource {
            parameters:
                packetLength = 1B;
                packetData = 13; // enter character
                productionInterval = 0.1s;
                @display("p=300,100");
        }
        emptyProducer: <default("EmptyPacketSource")> like IActivePacketSource {
            parameters:
                @display("p=500,100");
        }
        scheduler: <default("MarkovScheduler")> like IPacketScheduler {
            parameters:
                transitionProbabilities = "0 1 0 0 0 1 1 0 0"; // character -> enter -> wait -> character
                waitIntervals = "uniform(0,3) 0 uniform(10,30)";
                @display("p=300,200");
        }
        consumer: <default("PassivePacketSink")> like IPassivePacketSink {
            parameters:
                @display("p=100,200");
        }
    connections:
        characterProducer.out --> scheduler.in++;
        enterProducer.out --> scheduler.in++;
        emptyProducer.out --> scheduler.in++;
        scheduler.out --> { @display("m=s"); } --> out;
        in --> { @display("m=s"); } --> consumer.in;
}

module TelnetServerTraffic
{
    parameters:
        @display("i=block/app");
    gates:
        input in;
        output out;
    submodules:
        cloner: PacketCloner {
            parameters:
                @display("p=300,200");
        }
        responseProvider: <default("PassivePacketSource")> like IPassivePacketSource {
            parameters:
                @display("p=100,100");
        }
        responseServer: <default("TokenBasedServer")> like IPacketServer {
            parameters:
                @display("p=100,200");
        }
        multiplexer: PacketMultiplexer {
            parameters:
                @display("p=200,300");
        }
        classifier: <default("PacketClassifier")> like IPacketClassifier {
            parameters:
                classifierClass = default("inet::queueing::PacketCharacterOrEnterClassifier");
                @display("p=500,200");
        }
        characterConsumer: <default("PassivePacketSink")> like IPassivePacketSink {
            parameters:
                @display("p=400,300");
        }
        enterTokenGenerator: PacketBasedTokenGenerator {
            parameters:
                serverModule = default("^.responseServer");
                @display("p=600,300");
        }
    connections:
        in --> { @display("m=s"); } --> cloner.in;
        cloner.out++ --> classifier.in;
        cloner.out++ --> multiplexer.in++;
        responseProvider.out --> responseServer.in;
        responseServer.out --> multiplexer.in++;
        classifier.out++ --> characterConsumer.in;
        classifier.out++ --> enterTokenGenerator.in;
        multiplexer.out --> { @display("m=s"); } --> out;
}
