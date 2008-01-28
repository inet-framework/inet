
network SimpleTest
{
    parameters:
        @display("p=10,10;b=712,152");
    submodules:
        H1: StandardHost {
            parameters:
                @display("p=56,92;i=laptop3");
            gates:
                ethIn[1];
                ethOut[1];
        };
        N1: EtherHub {
            parameters:
                @display("p=184,92");
            gates:
                in[2];
                out[2];
        };
        R1: OSPFRouter {
            parameters:
                @display("p=296,92");
            gates:
                ethIn[2];
                ethOut[2];
        };
        R2: OSPFRouter {
            parameters:
                @display("p=416,92");
            gates:
                ethIn[2];
                ethOut[2];
        };
        N2: EtherHub {
            parameters:
                @display("p=532,92");
            gates:
                in[2];
                out[2];
        };
        H2: StandardHost {
            parameters:
                @display("p=660,92;i=laptop3");
            gates:
                ethIn[1];
                ethOut[1];
        };
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
}

