import
    "EtherHub",
    "OSPFRouter",
    "StandardHost";

module TwoNetsArea
    gates:
        in: in[];
        out: out[];
    submodules:
        R1: OSPFRouter;
            gatesizes:
                ethIn[3],
                ethOut[3];
            display: "p=248,124;i=srouter";
        N1: EtherHub;
            gatesizes:
                in[2],
                out[2];
            display: "p=156,72;i=device/hub";
        N2: EtherHub;
            gatesizes:
                in[2],
                out[2];
            display: "p=160,284;i=device/hub";
        R3: OSPFRouter;
            gatesizes:
                ethIn[3],
                ethOut[3];
            display: "p=360,180;i=srouter";
        H1: StandardHost;
            gatesizes:
                ethIn[1],
                ethOut[1];
            display: "p=56,72;i=device/laptop";
        H2: StandardHost;
            gatesizes:
                ethIn[1],
                ethOut[1];
            display: "p=56,288;i=device/laptop";
        R2: OSPFRouter;
            gatesizes:
                ethIn[3],
                ethOut[3];
            display: "p=248,232;i=srouter";
    connections:
        H1.ethOut[0] --> delay 0.1us --> N1.in[1];
        N1.out[1] --> delay 0.1us --> H1.ethIn[0];
        N1.out[0] --> delay 0.1us --> R1.ethIn[2];
        R1.ethOut[2] --> delay 0.1us --> N1.in[0];
        R1.ethOut[0] --> delay 0.1us --> R3.ethIn[2];
        R3.ethOut[2] --> delay 0.1us --> R1.ethIn[0];
        N2.out[1] --> delay 0.1us --> H2.ethIn[0];
        H2.ethOut[0] --> delay 0.1us --> N2.in[1];
        R1.ethOut[1] --> delay 0.1us --> R2.ethIn[0];
        R2.ethOut[0] --> delay 0.1us --> R1.ethIn[1];
        R2.ethOut[1] --> delay 0.1us --> R3.ethIn[1];
        R3.ethOut[1] --> delay 0.1us --> R2.ethIn[1];
        R2.ethOut[2] --> delay 0.1us --> N2.in[0];
        N2.out[0] --> delay 0.1us --> R2.ethIn[2];

        // towards other areas
        R3.ethOut[0] --> out[0];
        in[0] --> R3.ethIn[0];
    display: "p=10,10;b=412,316";
endmodule

