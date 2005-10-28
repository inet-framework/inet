import
    "EtherHub",
    "OSPFRouter",
    "StandardHost";

module SimpleTest
    submodules:
        H1: StandardHost;
            gatesizes:
                ethIn[1],
                ethOut[1];
            display: "p=56,92;i=laptop3";
        N1: EtherHub;
            gatesizes:
                in[2],
                out[2];
            display: "p=184,92;i=switch2";
        R1: OSPFRouter;
            gatesizes:
                ethIn[2],
                ethOut[2];
            display: "p=296,92;i=srouter";
        R2: OSPFRouter;
            gatesizes:
                ethIn[2],
                ethOut[2];
            display: "p=416,92;i=srouter";
        N2: EtherHub;
            gatesizes:
                in[2],
                out[2];
            display: "p=532,92;i=switch2";
        H2: StandardHost;
            gatesizes:
                ethIn[1],
                ethOut[1];
            display: "p=660,92;i=laptop3";
    connections:
        H1.ethOut[0] --> N1.in[0];
        N1.out[0] --> H1.ethIn[0];
        N1.out[1] --> R1.ethIn[0];
        R1.ethOut[0] --> N1.in[1];
        R1.ethOut[1] --> R2.ethIn[0];
        R2.ethOut[0] --> R1.ethIn[1];
        R2.ethOut[1] --> N2.in[0];
        N2.out[0] --> R2.ethIn[1];
        N2.out[1] --> H2.ethIn[0];
        H2.ethOut[0] --> N2.in[1];
    display: "p=10,10;b=712,152";
endmodule

network OSPFSimpleTest : SimpleTest
endnetwork
