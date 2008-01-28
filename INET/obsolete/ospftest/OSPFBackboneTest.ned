
network Backbone
{
    parameters:
        @display("p=10,10;b=736,568");
    submodules:
        channelInstaller: ChannelInstaller {
            parameters:
                channelClass = "ThruputMeteringChannel";
                channelAttrs = "format=#N";
        };
        R1: OSPFRouter {
            parameters:
                @display("p=372,340");
            gates:
                ethIn[3];
                ethOut[3];
        };
        N1: EtherHub {
            parameters:
                @display("p=368,268");
            gates:
                in[2];
                out[2];
        };
        N2: EtherHub {
            parameters:
                @display("p=452,384");
            gates:
                in[3];
                out[3];
        };
        R2: OSPFRouter {
            parameters:
                @display("p=368,188");
            gates:
                ethIn[2];
                ethOut[2];
        };
        R4: OSPFRouter {
            parameters:
                @display("p=216,432");
            gates:
                ethIn[2];
                ethOut[2];
        };
        R3: OSPFRouter {
            parameters:
                @display("p=532,428");
            gates:
                ethIn[2];
                ethOut[2];
        };
        N4: EtherHub {
            parameters:
                @display("p=368,100");
            gates:
                in[4];
                out[4];
        };
        N5: EtherHub {
            parameters:
                @display("p=612,476");
            gates:
                in[3];
                out[3];
        };
        R5: OSPFRouter {
            parameters:
                @display("p=488,284");
            gates:
                ethIn[2];
                ethOut[2];
        };
        R7: OSPFRouter {
            parameters:
                @display("p=256,284");
            gates:
                ethIn[2];
                ethOut[2];
        };
        R6: OSPFRouter {
            parameters:
                @display("p=372,476");
            gates:
                ethIn[2];
                ethOut[2];
        };
        N6: EtherHub {
            parameters:
                @display("p=136,476");
            gates:
                in[3];
                out[3];
        };
        N3: EtherHub {
            parameters:
                @display("p=292,384");
            gates:
                in[2];
                out[2];
        };
        H1: StandardHost {
            parameters:
                @display("p=372,420;i=laptop3");
            gates:
                ethIn[1];
                ethOut[1];
        };
        H2: StandardHost {
            parameters:
                @display("p=368,40;i=laptop3");
            gates:
                ethIn[1];
                ethOut[1];
        };
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
}

