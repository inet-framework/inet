
module OneNetArea
{
    parameters:
        @display("p=10,10;b=404,108");
    gates:
        input in[];
        output out[];
    submodules:
        H3: StandardHost {
            parameters:
                @display("p=372,72;i=laptop3");
            gates:
                ethIn[1];
                ethOut[1];
        };
        N3: EtherHub {
            parameters:
                @display("p=264,72");
            gates:
                in[2];
                out[2];
        };
        R5: OSPFRouter {
            parameters:
                @display("p=160,72");
            gates:
                ethIn[2];
                ethOut[2];
        };
        R4: OSPFRouter {
            parameters:
                @display("p=64,72");
            gates:
                ethIn[2];
                ethOut[2];
        };
    connections:
        in[0] --> R4.ethIn[1];
        R4.ethOut[1] --> out[0];
        R4.ethOut[0] --> R5.ethIn[1];
        R5.ethOut[1] --> R4.ethIn[0];
        R5.ethOut[0] --> N3.in[1];
        N3.out[1] --> R5.ethIn[0];
        N3.out[0] --> H3.ethIn[0];
        H3.ethOut[0] --> N3.in[0];
}

