import
    "OSPFAreaTest_Area1",
    "OSPFAreaTest_Area2",
    "ChannelInstaller";


module OSPF_AreaTest
    submodules:
        channelInstaller: ChannelInstaller;
            parameters:
                channelClass = "ThruputMeteringChannel",
                channelAttrs = "format=#N";
            display: "p=456,36;i=block/cogwheel_s";
        Area1: TwoNetsArea;
            gatesizes:
                in[1],
                out[1];
            display: "p=108,112;b=136,128";
        Area2: OneNetArea;
            gatesizes:
                in[1],
                out[1];
            display: "p=324,112;b=136,128";
    connections:
        Area1.out[0] --> delay 0.1us --> Area2.in[0];
        Area2.out[0] --> delay 0.1us --> Area1.in[0];
endmodule

network OSPFAreaTest : OSPF_AreaTest
endnetwork

