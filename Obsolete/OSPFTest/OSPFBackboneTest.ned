import
    "EtherHub",
    "OSPFRouter",
    "StandardHost";

module Backbone
    submodules:
        channelInstaller : ChannelInstaller;
            parameters:
                channelClass = "ThruputMeteringChannel",
                channelAttrs = "format=#N";
            display: "i=block/cogwheel";
        R1: OSPFRouter;
            gatesizes:
                ethIn[3],
                ethOut[3];
            display: "p=372,340;i=srouter";
        N1: EtherHub;
            gatesizes:
                in[2],
                out[2];
            display: "p=368,268;i=switch2";
        N2: EtherHub;
            gatesizes:
                in[3],
                out[3];
            display: "p=452,384;i=switch2";
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
            display: "p=368,100;i=switch2";
        N5: EtherHub;
            gatesizes:
                in[3],
                out[3];
            display: "p=612,476;i=switch2";
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
            display: "p=136,476;i=switch2";
        N3: EtherHub;
            gatesizes:
                in[2],
                out[2];
            display: "p=292,384;i=switch2";
        H1: StandardHost;
            gatesizes:
                ethIn[1],
                ethOut[1];
            display: "p=372,420;i=laptop3";
        H2: StandardHost;
            gatesizes:
                ethIn[1],
                ethOut[1];
            display: "p=368,40;i=laptop3";
    connections:
        R1.ethOut[0] --> N1.in[1];
        N1.out[1] --> R1.ethIn[0];
        R1.ethOut[1] --> N2.in[2];
        N2.out[2] --> R1.ethIn[1];
        N1.out[0] --> R2.ethIn[1];
        R2.ethOut[1] --> N1.in[0];
        R2.ethOut[0] --> N4.in[2];
        N4.out[2] --> R2.ethIn[0];
        N4.out[1] --> R5.ethIn[1];
        R5.ethOut[1] --> N4.in[1];
        R5.ethOut[0] --> N5.in[2];
        N5.out[2] --> R5.ethIn[0];
        N2.out[0] --> R3.ethIn[1];
        R3.ethOut[1] --> N2.in[0];
        R3.ethOut[0] --> N5.in[1];
        N5.out[1] --> R3.ethIn[0];
        N5.out[0] --> R6.ethIn[0];
        R6.ethOut[0] --> N5.in[0];
        N6.out[2] --> R6.ethIn[1];
        R6.ethOut[1] --> N6.in[2];
        R1.ethOut[2] --> N3.in[0];
        N3.out[0] --> R1.ethIn[2];
        N3.out[1] --> R4.ethIn[0];
        R4.ethOut[0] --> N3.in[1];
        R4.ethOut[1] --> N6.in[1];
        N6.out[1] --> R4.ethIn[1];
        N4.out[3] --> R7.ethIn[0];
        R7.ethOut[0] --> N4.in[3];
        R7.ethOut[1] --> N6.in[0];
        N6.out[0] --> R7.ethIn[1];
        N4.out[0] --> H2.ethIn[0];
        H2.ethOut[0] --> N4.in[0];
        N2.out[1] --> H1.ethIn[0];
        H1.ethOut[0] --> N2.in[1];
    display: "p=10,10;b=736,568";
endmodule

network OSPFBackboneTest : Backbone
endnetwork

