package inet.test.moduletest.lib;

import inet.linklayer.contract.IEthernetInterface;
import inet.linklayer.ethernet.EthernetQosQueue;


simple EthTestApp
{
    parameters:
        string destAddr;    // MacAddress
        string script;      // space separated sequence of seconds:bytes pairs
    gates:
        input in;
        output out;
}

module EthTestHost
{
    parameters:
        *.interfaceTableModule = "";
        @networkNode();
    gates:
        inout ethg @labels(EtherFrame-conn);
    submodules:
        app: EthTestApp {
            @display("p=155,72");
        }
        eth: <default("EthernetInterface")> like IEthernetInterface {
            parameters:
                @display("p=200,239,row,150;q=txQueue");
        }
    connections:
        app.out --> eth.upperLayerIn;
        app.in <-- eth.upperLayerOut;
        eth.phys <--> { @display("m=s"); } <--> ethg;
}

network EthTestNetwork
{
    types:
        channel C1 extends ned.DatarateChannel
        {
            delay = 0s;
        }
        channel C2 extends PacketLoggerChannel
        {
            delay = 0s;
        }
    submodules:
        host1: EthTestHost {
            @display("p=80,72");
        }
        host2: EthTestHost {
            @display("p=340,72");
        }
    connections:
        host1.ethg$i <-- ethch1:C1 <-- host2.ethg$o;
        host1.ethg$o --> ethch2:C2 --> host2.ethg$i;
}

