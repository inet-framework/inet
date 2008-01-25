
module OSPF_Area2
{
    parameters:
        @display("p=10,10;b=424,400");
    gates:
        input in[];
        output out[];
    submodules:
        RT8: OSPFRouter {
            parameters:
                @display("p=304,196");
            gates:
                ethIn[2];
                ethOut[2];
        };
        N6: EtherHub {
            parameters:
                @display("p=304,100");
            gates:
                in[4];
                out[4];
        };
        N7: EtherHub {
            parameters:
                @display("p=304,300");
            gates:
                in[3];
                out[3];
        };
        N8: EtherHub {
            parameters:
                @display("p=104,100");
            gates:
                in[3];
                out[3];
        };
        H8: StandardHost {
            parameters:
                @display("p=392,100;i=comp2_s");
            gates:
                ethIn[1];
                ethOut[1];
        };
        H9: StandardHost {
            parameters:
                @display("p=348,364;i=comp2_s");
            gates:
                ethIn[1];
                ethOut[1];
        };
        H10: StandardHost {
            parameters:
                @display("p=260,364;i=comp2_s");
            gates:
                ethIn[1];
                ethOut[1];
        };
        H11: StandardHost {
            parameters:
                @display("p=104,156;i=comp2_s");
            gates:
                ethIn[1];
                ethOut[1];
        };
    connections:
        N8.out[0] --> out[0];
        in[0] --> N8.in[0];
        N8.out[2] --> { @display("m=m,52,64,40,0"); } --> out[1];
        in[1] --> { @display("m=m,40,0,64,8"); } --> N8.in[2];
        N8.out[1] --> H11.ethIn[0];
        H11.ethOut[0] --> N8.in[1];
        N6.out[1] --> { @display("m=m,28,36,48,0"); } --> out[2];
        in[2] --> { @display("m=m,48,0,28,28"); } --> N6.in[1];
        N6.out[2] --> RT8.ethIn[0];
        RT8.ethOut[0] --> N6.in[2];
        N6.out[3] --> H8.ethIn[0];
        H8.ethOut[0] --> N6.in[3];
        N6.out[0] --> out[3];
        in[3] --> N6.in[0];
        RT8.ethOut[1] --> N7.in[0];
        N7.out[0] --> RT8.ethIn[1];
        N7.out[1] --> H10.ethIn[0];
        H10.ethOut[0] --> N7.in[1];
        N7.out[2] --> H9.ethIn[0];
        H9.ethOut[0] --> N7.in[2];
}

