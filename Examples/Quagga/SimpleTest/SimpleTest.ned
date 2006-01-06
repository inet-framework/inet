import
    "EtherHub",
    "QuaggaRouter",
    "StandardHost",
    "ChannelInstaller";

module SimpleTest
    submodules:
        channelInstaller : ChannelInstaller;
            parameters:
                channelClass = "ThruputMeteringChannel",
                channelAttrs = "format=#N";
            display: "p=130,40;i=block/cogwheel_s";
        H1: StandardHost;
            gatesizes:
                ethIn[1],
                ethOut[1];
            display: "p=56,92;i=device/laptop";
        N1: EtherHub;
            gatesizes:
                in[2],
                out[2];
            display: "p=184,92;i=device/hub";
        R1: QuaggaRouter;
            gatesizes:
                ethIn[1],
                ethOut[1];
            display: "p=296,92;i=srouter";
        R2: QuaggaRouter;
            gatesizes:
                ethIn[1],
                ethOut[1];
            display: "p=416,92;i=srouter";
        N2: EtherHub;
            gatesizes:
                in[2],
                out[2];
            display: "p=532,92;i=device/hub";
        H2: StandardHost;
            gatesizes:
                ethIn[1],
                ethOut[1];
            display: "p=660,92;i=device/laptop";
    connections:
        H1.ethOut[0] --> delay 0.1us --> N1.in[0];
        N1.out[0] --> delay 0.1us --> H1.ethIn[0];
        N1.out[1] --> delay 0.1us --> R1.ethIn[0];
        R1.ethOut[0] --> delay 0.1us --> N1.in[1];

        R1.out++ --> datarate 10000000 delay 0.1us --> R2.in++;
        R2.out++ --> datarate 10000000 delay 0.1us --> R1.in++;

        R2.ethOut[0] --> delay 0.1us --> N2.in[0];
        N2.out[0] --> delay 0.1us --> R2.ethIn[0];
        N2.out[1] --> delay 0.1us --> H2.ethIn[0];
        H2.ethOut[0] --> delay 0.1us --> N2.in[1];
    display: "p=10,10;b=712,152";
endmodule

network OSPFSimpleTest : SimpleTest
endnetwork
