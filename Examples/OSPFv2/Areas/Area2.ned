import
    "EtherHub",
    "OSPFRouter",
    "StandardHost";

module OneNetArea
    gates:
        in: in[];
        out: out[];
    submodules:
        H3: StandardHost;
            gatesizes:
                ethIn[1],
                ethOut[1];
            display: "p=372,72;i=device/laptop";
        N3: EtherHub;
            gatesizes:
                in[2],
                out[2];
            display: "p=264,72;i=device/hub";
        R5: OSPFRouter;
            gatesizes:
                ethIn[2],
                ethOut[2];
            display: "p=160,72;i=srouter";
        R4: OSPFRouter;
            gatesizes:
                ethIn[2],
                ethOut[2];
            display: "p=64,72;i=srouter";
    connections:
        R4.ethOut[0] --> delay 0.1us --> R5.ethIn[1];
        R5.ethOut[1] --> delay 0.1us --> R4.ethIn[0];
        R5.ethOut[0] --> delay 0.1us --> N3.in[1];
        N3.out[1] --> delay 0.1us --> R5.ethIn[0];
        N3.out[0] --> delay 0.1us --> H3.ethIn[0];
        H3.ethOut[0] --> delay 0.1us --> N3.in[0];

        // towards other areas
        in[0] --> R4.ethIn[1];
        R4.ethOut[1] --> out[0];
    display: "p=10,10;b=404,108";
endmodule

