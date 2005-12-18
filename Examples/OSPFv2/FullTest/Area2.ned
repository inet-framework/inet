import
    "EtherHub",
    "OSPFRouter",
    "StandardHost";

module OSPF_Area2
    gates:
        in: in[];
        out: out[];
    submodules:
        RT8: OSPFRouter;
            gatesizes:
                ethIn[2],
                ethOut[2];
            display: "p=304,196;i=srouter";
        N6: EtherHub;
            gatesizes:
                in[4],
                out[4];
            display: "p=304,100;i=device/hub";
        N7: EtherHub;
            gatesizes:
                in[3],
                out[3];
            display: "p=304,300;i=device/hub";
        N8: EtherHub;
            gatesizes:
                in[3],
                out[3];
            display: "p=104,100;i=device/hub";
        H8: StandardHost;
            gatesizes:
                ethIn[1],
                ethOut[1];
            display: "p=392,100;i=device/pc_s";
        H9: StandardHost;
            gatesizes:
                ethIn[1],
                ethOut[1];
            display: "p=348,364;i=device/pc_s";
        H10: StandardHost;
            gatesizes:
                ethIn[1],
                ethOut[1];
            display: "p=260,364;i=device/pc_s";
        H11: StandardHost;
            gatesizes:
                ethIn[1],
                ethOut[1];
            display: "p=104,156;i=device/pc_s";
    connections:
        N8.out[1] --> delay 0.1us --> H11.ethIn[0];
        H11.ethOut[0] --> delay 0.1us --> N8.in[1];
        N6.out[2] --> delay 0.1us --> RT8.ethIn[0];
        RT8.ethOut[0] --> delay 0.1us --> N6.in[2];
        N6.out[3] --> delay 0.1us --> H8.ethIn[0];
        H8.ethOut[0] --> delay 0.1us --> N6.in[3];
        RT8.ethOut[1] --> delay 0.1us --> N7.in[0];
        N7.out[0] --> delay 0.1us --> RT8.ethIn[1];
        N7.out[1] --> delay 0.1us --> H10.ethIn[0];
        H10.ethOut[0] --> delay 0.1us --> N7.in[1];
        N7.out[2] --> delay 0.1us --> H9.ethIn[0];
        H9.ethOut[0] --> delay 0.1us --> N7.in[2];

        // towards other areas
        N8.out[0] --> out[0];
        in[0] --> N8.in[0];
        N8.out[2] --> out[1] display "m=m,52,64,40,0";
        in[1] --> N8.in[2] display "m=m,40,0,64,8";
        N6.out[1] --> out[2] display "m=m,28,36,48,0";
        in[2] --> N6.in[1] display "m=m,48,0,28,28";
        N6.out[0] --> out[3];
        in[3] --> N6.in[0];
    display: "p=10,10;b=424,400";
endmodule

