//
// This library is free software, you can redistribute it 
// and/or modify 
// it under  the terms of the GNU Lesser General Public License 
// as published by the Free Software Foundation; 
// either version 2 of the License, or any later version.
// The library is distributed in the hope that it will be useful, 
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
// See the GNU Lesser General Public License for more details.
//


import   
    "TCPClientTest", 
    "TCPServerTest",
    "RSVP_LSR_Node",
    "Tester",
    "TED"; 

           
//
// FIXME: add documentation here
//
module RSVPNet 
    parameters: 
                
    submodules: 
        LSR1: RSVP_LSR_Node; 
            parameters: 
                isIR = true, 
                isER = false, 
                numOfProcessors = 1, 
                numOfPorts = 4; 
            gatesizes: 
                in[4], 
                out[4]; 
            display: "b=32,32;p=104,95;i=router";
        LSR2: RSVP_LSR_Node; 
            parameters: 
                isIR = false, 
                isER = false, 
                numOfProcessors = 1, 
                numOfPorts = 3; 
            gatesizes: 
                in[3], 
                out[3]; 
            display: "b=32,32;p=142,271;i=router";
        LSR3: RSVP_LSR_Node; 
            parameters: 
                isIR = false, 
                isER = false, 
                numOfProcessors = 1, 
                numOfPorts = 4; 
            gatesizes: 
                in[4], 
                out[4]; 
            display: "b=32,32;p=266,95;i=router";
        LSR4: RSVP_LSR_Node; 
            parameters: 
                isIR = false, 
                isER = false, 
                numOfProcessors = 1, 
                numOfPorts = 3; 
            gatesizes: 
                in[3], 
                out[3]; 
            display: "b=32,32;p=422,271;i=router";
        LSR5: RSVP_LSR_Node; 
            parameters: 
                isIR = false, 
                isER = true, 
                numOfProcessors = 1, 
                numOfPorts = 4; 
            gatesizes: 
                in[4], 
                out[4]; 
            display: "b=32,32;p=500,95;i=router";
        host1: TCPClientTest; 
            parameters: 
                numOfPorts = 1, 
                numOfProcessors = 1; 
            display: "b=18,16;p=26,98;i=comp_s";
        host2: TCPClientTest;
            parameters:
                numOfPorts = 1,
                numOfProcessors = 1;
            display: "b=18,16;p=26,256;i=comp_s";
        host3: TCPServerTest; 
            parameters: 
                numOfPorts = 1, 
                numOfProcessors = 1; 
            display: "b=18,16;p=570,88;i=comp_s";
        host4: TCPServerTest;
            parameters:
                numOfPorts = 1,
                numOfProcessors = 1;
            display: "b=18,16;p=578,240;i=comp_s";
        ted: TED; 
            display: "p=276,320;b=24,16";
    connections nocheck: 
        host1.in[0] <-- delay 0 error 0 datarate 9000 <-- LSR1.out[0]; 
        host1.out[0] --> delay 0 error 0 datarate 9000 --> LSR1.in[0]; 
                                     
        host2.in[0] <-- delay 0 error 0 datarate 9000 <-- LSR1.out[1];
        host2.out[0] --> delay 0 error 0 datarate 9000 --> LSR1.in[1];
                
        LSR1.out[2] --> delay 1 error 0 datarate 2000 --> LSR2.in[0];
        LSR1.in[2] <-- delay 1 error 0 datarate 1000 <-- LSR2.out[0];
                                                                     
        LSR1.out[3] --> delay 5 error 0 datarate 1000 --> LSR3.in[0];
        LSR1.in[3] <-- delay 5 error 0 datarate 1000 <-- LSR3.out[0];
                                                                     
        LSR2.out[1] --> delay 5 error 0 datarate 1000 --> LSR3.in[1];
        LSR2.in[1] <-- delay 5 error 0 datarate 2000 <-- LSR3.out[1];
                                                                     
        LSR2.out[2] --> delay 15 error 0 datarate 4000 --> LSR4.in[0];
        LSR2.in[2] <-- delay 15 error 0 datarate 1000 <-- LSR4.out[0];
                                                                     
        LSR3.out[2] --> delay 15 error 0 datarate 2000 --> LSR4.in[1];
        LSR3.in[2] <-- delay 15 error 0 datarate 1000 <-- LSR4.out[1];
                                                                     
        LSR3.out[3] --> delay 5 error 0 datarate 2000 --> LSR5.in[0];
        LSR3.in[3] <-- delay 5 error 0 datarate 2000 <-- LSR5.out[0];
                                                                     
        LSR4.out[2] --> delay 2 error 0 datarate 1000 --> LSR5.in[1];
        LSR4.in[2] <-- delay 2 error 0 datarate 2000 <-- LSR5.out[1];
                                                                     
                                                                     
        LSR5.out[2] --> delay 0 error 0 datarate 9000 --> host3.in[0];
        LSR5.in[2] <-- delay 10 error 0 datarate 9000 <-- host3.out[0];
                                     
        LSR5.out[3] --> delay 0 error 0 datarate 9000 --> host4.in[0];
        LSR5.in[3] <-- delay 10 error 0 datarate 9000 <-- host4.out[0];
                                                              
    display: "p=10,10;b=736,398";
endmodule 

network RSVPTest : RSVPNet 
endnetwork 


