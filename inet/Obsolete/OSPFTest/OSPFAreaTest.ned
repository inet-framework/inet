

network OSPF_AreaTest
{
    parameters:
        @display("p=10,10;b=360,184");
    submodules:
        Area1: TwoNetsArea {
            parameters:
                @display("p=108,112;b=136,128");
            gates:
                in[1];
                out[1];
        };
        Area2: OneNetArea {
            parameters:
                @display("p=284,112;b=136,128");
            gates:
                in[1];
                out[1];
        };
    connections:
        Area1.out[0] --> Area2.in[0];
        Area2.out[0] --> Area1.in[0];
}

