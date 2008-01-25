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
// Network for the LSP preemption example.
//
// See the README file in the model directory for scenario description.
//
network RSVPTE4
{
    submodules:
        LSR1: RSVP_LSR {
            parameters:
                isIR = true;
                isER = false;
                @display("p=159,133");
            gates:
                in[4];
                out[4];
        };
        LSR2: RSVP_LSR {
            parameters:
                isIR = false;
                isER = false;
                @display("p=248,228");
            gates:
                in[3];
                out[3];
        };
        LSR3: RSVP_LSR {
            parameters:
                isIR = false;
                isER = false;
                @display("p=241,55");
            gates:
                in[2];
                out[2];
        };
        LSR4: RSVP_LSR {
            parameters:
                isIR = false;
                isER = false;
                @display("p=336,127");
            gates:
                in[3];
                out[3];
        };
        LSR5: RSVP_LSR {
            parameters:
                isIR = false;
                isER = true;
                @display("p=438,127");
            gates:
                in[3];
                out[3];
        };
        host1: StandardHost { // client
            parameters:
                @display("p=73,56;i=device/pc2");
            gates:
                in[1];
                out[1];
        };
        host2: StandardHost { // client
            parameters:
                @display("p=73,214;i=device/pc2");
            gates:
                in[1];
                out[1];
        };
        host3: StandardHost { // server
            parameters:
                @display("p=530,64;i=device/server");
            gates:
                in[1];
                out[1];
        };
        host4: StandardHost { // server
            parameters:
                @display("p=530,216;i=device/server");
            gates:
                in[1];
                out[1];
        };
        ted: TED {
            parameters:
                moduleTypes = "StandardHost RSVP_LSR";
                @display("p=390,258");
        };
    connections allowunconnected:
        host1.in[0] <-- {  delay = 0ms; datarate = 700; } <-- LSR1.out[0];
        host1.out[0] --> {  delay = 0ms; datarate = 700; } --> LSR1.in[0];
        host2.in[0] <-- {  delay = 0ms; datarate = 700; } <-- LSR1.out[1];
        host2.out[0] --> {  delay = 0ms; datarate = 700; } --> LSR1.in[1];

        LSR1.out[2] --> {  delay = 25ms; datarate = 600; } --> LSR2.in[0];
        LSR1.in[2] <-- {  delay = 25ms; datarate = 600; } <-- LSR2.out[0];

        LSR1.out[3] --> {  delay = 5ms; datarate = 120; } --> LSR3.in[0];
        LSR1.in[3] <-- {  delay = 5ms; datarate = 120; } <-- LSR3.out[0];

        LSR2.out[2] --> {  delay = 5ms; datarate = 200; } --> LSR4.in[0];
        LSR2.in[2] <-- {  delay = 5ms; datarate = 200; } <-- LSR4.out[0];


        LSR3.out[1] --> {  delay = 5ms; datarate = 200; } --> LSR4.in[1];
        LSR3.in[1] <-- {  delay = 5ms; datarate = 200; } <-- LSR4.out[1];

        LSR4.out[2] --> {  delay = 5ms; datarate = 150; } --> LSR5.in[0];
        LSR4.in[2] <-- {  delay = 5ms; datarate = 150; } <-- LSR5.out[0];


        LSR5.out[1] --> {  delay = 0ms; datarate = 600; } --> host3.in[0];
        LSR5.in[1] <-- {  delay = 10ms; datarate = 600; } <-- host3.out[0];

        LSR5.out[2] --> {  delay = 0ms; datarate = 600; } --> host4.in[0];
        LSR5.in[2] <-- {  delay = 10ms; datarate = 600; } <-- host4.out[0];

}

