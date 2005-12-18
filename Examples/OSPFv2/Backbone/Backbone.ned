import
    "EtherHub",
    "OSPFRouter",
    "StandardHost",
    "ChannelInstaller";

module Backbone
    submodules:
        channelInstaller : ChannelInstaller;
            parameters:
                channelClass = "ThruputMeteringChannel",
                channelAttrs = "format=#N";
            display: "p=98,50;i=block/cogwheel_s";
        R1: OSPFRouter;
            gatesizes:
                ethIn[3],
                ethOut[3];
            display: "p=372,340;i=srouter";
        N1: EtherHub;
            gatesizes:
                in[2],
                out[2];
            display: "p=368,268;i=device/hub";
        N2: EtherHub;
            gatesizes:
                in[3],
                out[3];
            display: "p=452,384;i=device/hub";
        R2: OSPFRouter;
            gatesizes:
                ethIn[2],
                ethOut[2];
            display: "p=368,188;i=srouter";
        R4: OSPFRouter;
            gatesizes:
                ethIn[2],
                ethOut[2];
            display: "p=216,432;i=srouter";
        R3: OSPFRouter;
            gatesizes:
                ethIn[2],
                ethOut[2];
            display: "p=532,428;i=srouter";
        N4: EtherHub;
            gatesizes:
                in[4],
                out[4];
            display: "p=368,100;i=device/hub";
        N5: EtherHub;
            gatesizes:
                in[3],
                out[3];
            display: "p=612,476;i=device/hub";
        R5: OSPFRouter;
            gatesizes:
                ethIn[2],
                ethOut[2];
            display: "p=488,284;i=srouter";
        R7: OSPFRouter;
            gatesizes:
                ethIn[2],
                ethOut[2];
            display: "p=256,284;i=srouter";
        R6: OSPFRouter;
            gatesizes:
                ethIn[2],
                ethOut[2];
            display: "p=372,476;i=srouter";
        N6: EtherHub;
            gatesizes:
                in[3],
                out[3];
            display: "p=136,476;i=device/hub";
        N3: EtherHub;
            gatesizes:
                in[2],
                out[2];
            display: "p=292,384;i=device/hub";
        H1: StandardHost;
            gatesizes:
                ethIn[1],
                ethOut[1];
            display: "p=372,420;i=device/laptop";
        H2: StandardHost;
            gatesizes:
                ethIn[1],
                ethOut[1];
            display: "p=368,40;i=device/laptop";
    connections:
        R1.ethOut[0] --> delay 0.1us --> N1.in[1];
        N1.out[1] --> delay 0.1us --> R1.ethIn[0];
        R1.ethOut[1] --> delay 0.1us --> N2.in[2];
        N2.out[2] --> delay 0.1us --> R1.ethIn[1];
        N1.out[0] --> delay 0.1us --> R2.ethIn[1];
        R2.ethOut[1] --> delay 0.1us --> N1.in[0];
        R2.ethOut[0] --> delay 0.1us --> N4.in[2];
        N4.out[2] --> delay 0.1us --> R2.ethIn[0];
        N4.out[1] --> delay 0.1us --> R5.ethIn[1];
        R5.ethOut[1] --> delay 0.1us --> N4.in[1];
        R5.ethOut[0] --> delay 0.1us --> N5.in[2];
        N5.out[2] --> delay 0.1us --> R5.ethIn[0];
        N2.out[0] --> delay 0.1us --> R3.ethIn[1];
        R3.ethOut[1] --> delay 0.1us --> N2.in[0];
        R3.ethOut[0] --> delay 0.1us --> N5.in[1];
        N5.out[1] --> delay 0.1us --> R3.ethIn[0];
        N5.out[0] --> delay 0.1us --> R6.ethIn[0];
        R6.ethOut[0] --> delay 0.1us --> N5.in[0];
        N6.out[2] --> delay 0.1us --> R6.ethIn[1];
        R6.ethOut[1] --> delay 0.1us --> N6.in[2];
        R1.ethOut[2] --> delay 0.1us --> N3.in[0];
        N3.out[0] --> delay 0.1us --> R1.ethIn[2];
        N3.out[1] --> delay 0.1us --> R4.ethIn[0];
        R4.ethOut[0] --> delay 0.1us --> N3.in[1];
        R4.ethOut[1] --> delay 0.1us --> N6.in[1];
        N6.out[1] --> delay 0.1us --> R4.ethIn[1];
        N4.out[3] --> delay 0.1us --> R7.ethIn[0];
        R7.ethOut[0] --> delay 0.1us --> N4.in[3];
        R7.ethOut[1] --> delay 0.1us --> N6.in[0];
        N6.out[0] --> delay 0.1us --> R7.ethIn[1];
        N4.out[0] --> delay 0.1us --> H2.ethIn[0];
        H2.ethOut[0] --> delay 0.1us --> N4.in[0];
        N2.out[1] --> delay 0.1us --> H1.ethIn[0];
        H1.ethOut[0] --> delay 0.1us --> N2.in[1];
    display: "p=10,10;b=736,568";
endmodule

network OSPFBackboneTest : Backbone
endnetwork

