


module OSPF_Area1
{
    parameters:
        @display("p=10,42;b=432,504");
    gates:
        input in[];
        output out[];
    submodules:
        RT1: OSPFRouter {
            parameters:
                @display("p=232,136");
            gates:
                ethIn[2];
                ethOut[2];
        };
        RT2: OSPFRouter {
            parameters:
                @display("p=236,348");
            gates:
                ethIn[2];
                ethOut[2];
        };
        N1: EtherHub {
            parameters:
                @display("p=140,136");
            gates:
                in[3];
                out[3];
        };
        N2: EtherHub {
            parameters:
                @display("p=140,348");
            gates:
                in[3];
                out[3];
        };
        N3: EtherHub {
            parameters:
                @display("p=360,252");
            gates:
                in[5];
                out[5];
        };
        N4: EtherHub {
            parameters:
                @display("p=360,464");
            gates:
                in[3];
                out[3];
        };
        H1: StandardHost {
            parameters:
                @display("p=56,96;i=comp2_s");
            gates:
                ethIn[1];
                ethOut[1];
        };
        H2: StandardHost {
            parameters:
                @display("p=56,172;i=comp2_s");
            gates:
                ethIn[1];
                ethOut[1];
        };
        H3: StandardHost {
            parameters:
                @display("p=56,316;i=comp2_s");
            gates:
                ethIn[1];
                ethOut[1];
        };
        H4: StandardHost {
            parameters:
                @display("p=56,388;i=comp2_s");
            gates:
                ethIn[1];
                ethOut[1];
        };
        H5: StandardHost {
            parameters:
                @display("p=360,188;i=comp2_s");
            gates:
                ethIn[1];
                ethOut[1];
        };
        H6: StandardHost {
            parameters:
                @display("p=316,520;i=comp2_s");
            gates:
                ethIn[1];
                ethOut[1];
        };
        H7: StandardHost {
            parameters:
                @display("p=400,520;i=comp2_s");
            gates:
                ethIn[1];
                ethOut[1];
        };
    connections:
        H1.ethOut[0] --> N1.in[0];
        N1.out[0] --> H1.ethIn[0];
        N1.out[1] --> H2.ethIn[0];
        H2.ethOut[0] --> N1.in[1];
        N2.out[0] --> H3.ethIn[0];
        H3.ethOut[0] --> N2.in[0];
        N2.out[1] --> H4.ethIn[0];
        H4.ethOut[0] --> N2.in[1];
        N1.out[2] --> RT1.ethIn[0];
        RT1.ethOut[0] --> N1.in[2];
        RT1.ethOut[1] --> N3.in[1];
        N3.out[1] --> RT1.ethIn[1];
        N3.out[0] --> H5.ethIn[0];
        H5.ethOut[0] --> N3.in[0];
        N4.out[0] --> H6.ethIn[0];
        H6.ethOut[0] --> N4.in[0];
        N4.out[1] --> H7.ethIn[0];
        H7.ethOut[0] --> N4.in[1];
        N2.out[2] --> RT2.ethIn[0];
        RT2.ethOut[0] --> N2.in[2];
        RT2.ethOut[1] --> N3.in[2];
        N3.out[2] --> RT2.ethIn[1];
        N3.out[4] --> { @display("m=m,100,44,100,32"); } --> out[0];
        in[0] --> { @display("m=m,100,32,100,48"); } --> N3.in[4];
        N3.out[3] --> { @display("m=m,96,96,100,52"); } --> out[1];
        in[1] --> { @display("m=m,100,52,89,64"); } --> N3.in[3];
        N4.out[2] --> out[2];
        in[2] --> N4.in[2];
}

