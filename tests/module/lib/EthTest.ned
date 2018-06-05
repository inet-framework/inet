

import inet.linklayer.contract.IEtherMac;
import inet.linklayer.ethernet.EtherQosQueue;


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
        bool csmacdSupport = default(true);  // by default use CSMA/CD
        string queueType = default(""); // DropTailQueue, DiffServQueue, or empty to use the internal queue
        *.interfaceTableModule = "";
        @networkNode();
    gates:
        inout ethg @labels(EtherFrame-conn);
    submodules:
        app: EthTestApp {
            @display("p=155,72");
        }
        queue: EtherQosQueue if queueType != "" {
            parameters:
                dataQueueType = queueType;
                @display("p=65,171;q=l2queue");
        }
        mac: <default(csmacdSupport ? "EtherMac" : "EtherMacFullDuplex")> like IEtherMac {
            parameters:
                queueModule = (queueType == "" ? "" : "^.queue");
                @display("p=155,272;q=txQueue");
        }
    connections:
        app.out --> queue.in if queueType != "";
        queue.out --> mac.upperLayerIn if queueType != "";
        app.out --> mac.upperLayerIn if queueType == "";
        mac.upperLayerOut --> app.in;
        mac.phys <--> ethg;
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

