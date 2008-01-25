


network OSPF_TestNetwork
{
    parameters:
        @display("p=10,10;b=704,560");
    submodules:
        Area1: OSPF_Area1 {
            parameters:
                @display("p=120,172;b=176,168");
            gates:
                out[3];
                in[3];
        };
        Area2: OSPF_Area2 {
            parameters:
                @display("p=448,468;b=176,168");
            gates:
                in[4];
                out[4];
        };
        Area3: OSPF_Area3 {
            parameters:
                @display("p=128,468;b=176,168");
            gates:
                in[1];
                out[1];
        };
        RT3: OSPFRouter {
            parameters:
                @display("p=276,208");
            gates:
                ethIn[3];
                ethOut[3];
        };
        RT4: OSPFRouter {
            parameters:
                @display("p=276,144");
            gates:
                ethIn[2];
                ethOut[2];
        };
        RT5: OSPFRouter {
            parameters:
                @display("p=496,144");
            gates:
                ethIn[6];
                ethOut[6];
        };
        RT6: OSPFRouter {
            parameters:
                @display("p=424,208");
            gates:
                ethIn[3];
                ethOut[3];
        };
        RT7: OSPFRouter {
            parameters:
                @display("p=496,328");
            gates:
                ethIn[4];
                ethOut[4];
        };
        RT10: OSPFRouter {
            parameters:
                @display("p=424,328");
            gates:
                ethIn[3];
                ethOut[3];
        };
        RT11: OSPFRouter {
            parameters:
                @display("p=284,468");
            gates:
                ethIn[2];
                ethOut[2];
        };
        N12: EtherHub {
            parameters:
                @display("p=608,296");
            gates:
                in[3];
                out[3];
        };
        N13: EtherHub {
            parameters:
                @display("p=500,92");
            gates:
                in[2];
                out[2];
        };
        N14: EtherHub {
            parameters:
                @display("p=608,92");
            gates:
                in[2];
                out[2];
        };
        N15: EtherHub {
            parameters:
                @display("p=608,356");
            gates:
                in[2];
                out[2];
        };
        H18: StandardHost {
            parameters:
                @display("p=496,36;i=comp2_s");
            gates:
                ethIn[1];
                ethOut[1];
        };
        H19: StandardHost {
            parameters:
                @display("p=612,36;i=comp2_s");
            gates:
                ethIn[1];
                ethOut[1];
        };
        H20: StandardHost {
            parameters:
                @display("p=688,296;i=comp2_s");
            gates:
                ethIn[1];
                ethOut[1];
        };
        H21: StandardHost {
            parameters:
                @display("p=688,356;i=comp2_s");
            gates:
                ethIn[1];
                ethOut[1];
        };
    connections:
        RT11.ethOut[0] --> Area3.in[0];
        Area3.out[0] --> RT11.ethIn[0];
        RT11.ethOut[1] --> Area2.in[0];
        Area2.out[0] --> RT11.ethIn[1];
        RT3.ethOut[0] --> { @display("m=m,41,50,88,40"); } --> Area1.in[1];
        Area1.out[1] --> { @display("m=m,88,40,64,79"); } --> RT3.ethIn[0];
        RT3.ethOut[1] --> { @display("m=m,77,42,92,96"); } --> Area1.in[2];
        Area1.out[2] --> { @display("m=m,83,100,60,50"); } --> RT3.ethIn[1];
        N13.out[1] --> RT5.ethIn[0];
        RT5.ethOut[0] --> N13.in[1];
        N12.out[0] --> RT5.ethIn[4];
        RT5.ethOut[4] --> N12.in[0];
        RT5.ethOut[5] --> N14.in[1];
        N14.out[1] --> RT5.ethIn[5];
        RT3.ethOut[2] --> RT6.ethIn[0];
        RT6.ethOut[0] --> RT3.ethIn[2];
        RT10.ethOut[0] --> RT6.ethIn[1];
        RT6.ethOut[1] --> RT10.ethIn[0];
        RT7.ethOut[3] --> N12.in[1];
        N12.out[1] --> RT7.ethIn[3];
        RT7.ethOut[2] --> N15.in[0];
        N15.out[0] --> RT7.ethIn[2];
        RT5.ethOut[3] --> RT7.ethIn[0];
        RT7.ethOut[0] --> RT5.ethIn[3];
        RT5.ethOut[2] --> RT6.ethIn[2];
        RT6.ethOut[2] --> RT5.ethIn[2];
        RT10.ethOut[1] --> { @display("m=m,50,50,0,30"); } --> Area2.in[1];
        Area2.out[1] --> { @display("m=m,12,7,50,50"); } --> RT10.ethIn[1];
        RT10.ethOut[2] --> { @display("m=m,50,50,64,12"); } --> Area2.in[2];
        Area2.out[2] --> { @display("m=m,76,33,50,50"); } --> RT10.ethIn[2];
        RT7.ethOut[1] --> Area2.in[3];
        Area2.out[3] --> RT7.ethIn[1];
        RT4.ethOut[1] --> RT5.ethIn[1];
        RT5.ethOut[1] --> RT4.ethIn[1];
        RT4.ethOut[0] --> Area1.in[0];
        Area1.out[0] --> RT4.ethIn[0];
        N14.out[0] --> H19.ethIn[0];
        H19.ethOut[0] --> N14.in[0];
        H20.ethOut[0] --> N12.in[2];
        N12.out[2] --> H20.ethIn[0];
        H21.ethOut[0] --> N15.in[1];
        N15.out[1] --> H21.ethIn[0];
        H18.ethOut[0] --> N13.in[0];
        N13.out[0] --> H18.ethIn[0];
}

