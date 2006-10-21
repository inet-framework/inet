import
    "TestRouter";

channel ethernetline
    delay 10ms;
    datarate 100000;
endchannel

module Test
    submodules:
        channelInstaller: ChannelInstaller;
            parameters:
                channelClass = "ThruputMeteringChannel",
                channelAttrs = "format=u #N";
            display: "p=284,50;i=block/cogwheel_s";

		r1: TestRouter;
			gatesizes:
				in[2],
				out[2];
			display: "p=100,30";
				
		r2: TestRouter;
			gatesizes:
				in[4],
				out[4];
			display: "p=50,100";
				
		r3: TestRouter;
			gatesizes:
				in[4],
				out[4];
			display: "p=150,100";
				
		r4: TestRouter;
			gatesizes:
				in[2],
				out[2];
			display: "p=10,150";
				
		r5: TestRouter;
			gatesizes:
				in[3],
				out[3];
			display: "p=100,150";				
        
    connections nocheck:
        r1.out[0] --> ethernetline --> r2.in[0];
        r1.in[0] <-- ethernetline <-- r2.out[0];

        r1.out[1] --> ethernetline --> r3.in[0];
        r1.in[1] <-- ethernetline <-- r3.out[0];

        r2.out[1] --> ethernetline --> r3.in[1];
        r2.in[1] <-- ethernetline <-- r3.out[1];

        r2.out[2] --> ethernetline --> r4.in[0];
        r2.in[2] <-- ethernetline <-- r4.out[0];

        r2.out[3] --> ethernetline --> r5.in[0];
        r2.in[3] <-- ethernetline <-- r5.out[0];
        
        r3.out[2] --> ethernetline --> r5.in[1];
        r3.in[2] <-- ethernetline <-- r5.out[1];
        
        r4.out[1] --> ethernetline --> r5.in[2];
        r4.in[1] <-- ethernetline <-- r5.out[2];
endmodule

network testNetwork : Test
endnetwork

