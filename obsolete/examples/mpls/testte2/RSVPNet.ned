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



//
// An example of Hop-by-Hop routing. The network consists of 5 LSRs and
// 2 hosts. Host1 establishes path simply based on \IP routing.
//
// See the README file in the model directory for scenario description.
//
network RSVPNet
{
    submodules:
        LSR1: RSVP_LSR {
            parameters:
                isIR = true;
                isER = false;
                @display("p=139,50");
            gates:
                in[4];
                out[4];
        };
        LSR2: RSVP_LSR {
            parameters:
                isIR = false;
                isER = false;
                @display("p=177,226");
            gates:
                in[3];
                out[3];
        };
        LSR3: RSVP_LSR {
            parameters:
                isIR = false;
                isER = false;
                @display("p=325,50");
            gates:
                in[4];
                out[4];
        };
        LSR4: RSVP_LSR {
            parameters:
                isIR = false;
                isER = false;
                @display("p=457,226");
            gates:
                in[3];
                out[3];
        };
        LSR5: RSVP_LSR {
            parameters:
                isIR = false;
                isER = true;
                @display("p=535,50");
            gates:
                in[4];
                out[4];
        };
        host1: StandardHost { // client
            parameters:
                @display("p=49,115;i=device/pc2");
            gates:
                in[1];
                out[1];
        };
        host3: StandardHost { // server
            parameters:
                @display("p=617,123;i=device/server");
            gates:
                in[1];
                out[1];
        };
        ted: TED {
            parameters:
                moduleTypes = "StandardHost RSVP_LSR";
                @display("p=89,241");
        };
    connections allowunconnected:
        host1.in[0] <-- {  delay = 0ms; datarate = 9000; } <-- LSR1.out[0];
        host1.out[0] --> {  delay = 0ms; datarate = 9000; } --> LSR1.in[0];

        LSR1.out[2] --> {  delay = 1ms; datarate = 2000; } --> LSR2.in[0];
        LSR1.in[2] <-- {  delay = 1ms; datarate = 1000; } <-- LSR2.out[0];

        LSR1.out[3] --> {  delay = 5ms; datarate = 1000; } --> LSR3.in[0];
        LSR1.in[3] <-- {  delay = 5ms; datarate = 1000; } <-- LSR3.out[0];

        LSR2.out[1] --> {  delay = 5ms; datarate = 1000; } --> LSR3.in[1];
        LSR2.in[1] <-- {  delay = 5ms; datarate = 2000; } <-- LSR3.out[1];

        LSR2.out[2] --> {  delay = 15ms; datarate = 4000; } --> LSR4.in[0];
        LSR2.in[2] <-- {  delay = 15ms; datarate = 1000; } <-- LSR4.out[0];

        LSR3.out[2] --> {  delay = 15ms; datarate = 2000; } --> LSR4.in[1];
        LSR3.in[2] <-- {  delay = 15ms; datarate = 1000; } <-- LSR4.out[1];

        LSR3.out[3] --> {  delay = 5ms; datarate = 2000; } --> LSR5.in[0];
        LSR3.in[3] <-- {  delay = 5ms; datarate = 2000; } <-- LSR5.out[0];

        LSR4.out[2] --> {  delay = 2ms; datarate = 1000; } --> LSR5.in[1];
        LSR4.in[2] <-- {  delay = 2ms; datarate = 2000; } <-- LSR5.out[1];


        LSR5.out[2] --> {  delay = 0ms; datarate = 9000; } --> host3.in[0];
        LSR5.in[2] <-- {  delay = 10ms; datarate = 9000; } <-- host3.out[0];

}

