import
    "EtherHub",
    "OSPFRouter",
    "StandardHost",
    "OSPFTestNetworkArea1",
    "OSPFTestNetworkArea2",
    "OSPFTestNetworkArea3",
    "ChannelInstaller";



module OSPF_TestNetwork
    submodules:
        channelInstaller : ChannelInstaller;
            parameters:
                channelClass = "ThruputMeteringChannel",
                channelAttrs = "format=#N";
            display: "p=98,50;i=block/cogwheel_s";
        Area1: OSPF_Area1;
            gatesizes:
                out[3],
                in[3];
            display: "p=120,172;b=176,168";
        Area2: OSPF_Area2;
            gatesizes:
                in[4],
                out[4];
            display: "p=448,468;b=176,168";
        Area3: OSPF_Area3;
            gatesizes:
                in[1],
                out[1];
            display: "p=128,468;b=176,168";
        RT3: OSPFRouter;
            gatesizes:
                ethIn[3],
                ethOut[3];
            display: "p=276,208;i=srouter";
        RT4: OSPFRouter;
            gatesizes:
                ethIn[2],
                ethOut[2];
            display: "p=276,144;i=srouter";
        RT5: OSPFRouter;
            gatesizes:
                ethIn[6],
                ethOut[6];
            display: "p=496,144;i=srouter";
        RT6: OSPFRouter;
            gatesizes:
                ethIn[3],
                ethOut[3];
            display: "p=424,208;i=srouter";
        RT7: OSPFRouter;
            gatesizes:
                ethIn[4],
                ethOut[4];
            display: "p=496,328;i=srouter";
        RT10: OSPFRouter;
            gatesizes:
                ethIn[3],
                ethOut[3];
            display: "p=424,328;i=srouter";
        RT11: OSPFRouter;
            gatesizes:
                ethIn[2],
                ethOut[2];
            display: "p=284,468;i=srouter";
        N12: EtherHub;
            gatesizes:
                in[3],
                out[3];
            display: "p=608,296;i=device/hub";
        N13: EtherHub;
            gatesizes:
                in[2],
                out[2];
            display: "p=500,92;i=device/hub";
        N14: EtherHub;
            gatesizes:
                in[2],
                out[2];
            display: "p=608,92;i=device/hub";
        N15: EtherHub;
            gatesizes:
                in[2],
                out[2];
            display: "p=608,356;i=device/hub";
        H18: StandardHost;
            gatesizes:
                ethIn[1],
                ethOut[1];
            display: "p=496,36;i=device/pc_s";
        H19: StandardHost;
            gatesizes:
                ethIn[1],
                ethOut[1];
            display: "p=612,36;i=device/pc_s";
        H20: StandardHost;
            gatesizes:
                ethIn[1],
                ethOut[1];
            display: "p=688,296;i=device/pc_s";
        H21: StandardHost;
            gatesizes:
                ethIn[1],
                ethOut[1];
            display: "p=688,356;i=device/pc_s";
    connections:
        RT11.ethOut[0] --> delay 0.1us --> Area3.in[0];
        Area3.out[0] --> delay 0.1us --> RT11.ethIn[0];
        RT11.ethOut[1] --> delay 0.1us --> Area2.in[0];
        Area2.out[0] --> delay 0.1us --> RT11.ethIn[1];
        RT3.ethOut[0] --> delay 0.1us --> Area1.in[1] display "m=m,41,50,88,40";
        Area1.out[1] --> delay 0.1us --> RT3.ethIn[0] display "m=m,88,40,64,79";
        RT3.ethOut[1] --> delay 0.1us --> Area1.in[2] display "m=m,77,42,92,96";
        Area1.out[2] --> delay 0.1us --> RT3.ethIn[1] display "m=m,83,100,60,50";
        N13.out[1] --> delay 0.1us --> RT5.ethIn[0];
        RT5.ethOut[0] --> delay 0.1us --> N13.in[1];
        N12.out[0] --> delay 0.1us --> RT5.ethIn[4];
        RT5.ethOut[4] --> delay 0.1us --> N12.in[0];
        RT5.ethOut[5] --> delay 0.1us --> N14.in[1];
        N14.out[1] --> delay 0.1us --> RT5.ethIn[5];
        RT3.ethOut[2] --> delay 0.1us --> RT6.ethIn[0];
        RT6.ethOut[0] --> delay 0.1us --> RT3.ethIn[2];
        RT10.ethOut[0] --> delay 0.1us --> RT6.ethIn[1];
        RT6.ethOut[1] --> delay 0.1us --> RT10.ethIn[0];
        RT7.ethOut[3] --> delay 0.1us --> N12.in[1];
        N12.out[1] --> delay 0.1us --> RT7.ethIn[3];
        RT7.ethOut[2] --> delay 0.1us --> N15.in[0];
        N15.out[0] --> delay 0.1us --> RT7.ethIn[2];
        RT5.ethOut[3] --> delay 0.1us --> RT7.ethIn[0];
        RT7.ethOut[0] --> delay 0.1us --> RT5.ethIn[3];
        RT5.ethOut[2] --> delay 0.1us --> RT6.ethIn[2];
        RT6.ethOut[2] --> delay 0.1us --> RT5.ethIn[2];
        RT10.ethOut[1] --> delay 0.1us --> Area2.in[1] display "m=m,50,50,0,30";
        Area2.out[1] --> delay 0.1us --> RT10.ethIn[1] display "m=m,12,7,50,50";
        RT10.ethOut[2] --> delay 0.1us --> Area2.in[2] display "m=m,50,50,64,12";
        Area2.out[2] --> delay 0.1us --> RT10.ethIn[2] display "m=m,76,33,50,50";
        RT7.ethOut[1] --> delay 0.1us --> Area2.in[3];
        Area2.out[3] --> delay 0.1us --> RT7.ethIn[1];
        RT4.ethOut[1] --> delay 0.1us --> RT5.ethIn[1];
        RT5.ethOut[1] --> delay 0.1us --> RT4.ethIn[1];
        RT4.ethOut[0] --> delay 0.1us --> Area1.in[0];
        Area1.out[0] --> delay 0.1us --> RT4.ethIn[0];
        N14.out[0] --> delay 0.1us --> H19.ethIn[0];
        H19.ethOut[0] --> delay 0.1us --> N14.in[0];
        H20.ethOut[0] --> delay 0.1us --> N12.in[2];
        N12.out[2] --> delay 0.1us --> H20.ethIn[0];
        H21.ethOut[0] --> delay 0.1us --> N15.in[1];
        N15.out[1] --> delay 0.1us --> H21.ethIn[0];
        H18.ethOut[0] --> delay 0.1us --> N13.in[0];
        N13.out[0] --> delay 0.1us --> H18.ethIn[0];
    display: "p=10,10;b=704,560";
endmodule

network OSPFTestNetwork : OSPF_TestNetwork
endnetwork

