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
// Example network to demonstrate CSPF algorithm used in \RSVP-TE.
// The network consists of 5 LSR and 4 hosts. Host 1 and 2 have
// different requests of bandwidth level.
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
                @display("p=160,167");
            gates:
                in[4];
                out[4];
        };
        LSR2: RSVP_LSR {
            parameters:
                isIR = false;
                isER = false;
                @display("p=254,255");
            gates:
                in[2];
                out[2];
        };
        LSR3: RSVP_LSR {
            parameters:
                isIR = false;
                isER = false;
                @display("p=253,78");
            gates:
                in[2];
                out[2];
        };
        LSR4: RSVP_LSR {
            parameters:
                isIR = false;
                isER = false;
                @display("p=358,167");
            gates:
                in[3];
                out[3];
        };
        LSR5: RSVP_LSR {
            parameters:
                isIR = false;
                isER = true;
                @display("p=460,167");
            gates:
                in[3];
                out[3];
        };
        host1: StandardHost { // client
            parameters:
                @display("p=71,80;i=device/pc2");
        };
        host2: StandardHost { // client
            parameters:
                @display("p=72,253;i=device/pc2");
        };
        host3: StandardHost { // server
            parameters:
                @display("p=570,88;i=device/server");
        };
        host4: StandardHost { // server
            parameters:
                @display("p=562,256;i=device/server");
        };
        ted: TED {
            parameters:
                moduleTypes = "StandardHost RSVP_LSR";
                @display("p=384,284");
        };
    connections allowunconnected:
        host1.in++ <-- {  delay = 0ms; datarate = 700; } <-- LSR1.out++;
        host1.out++ --> {  delay = 0ms; datarate = 700; } --> LSR1.in++;

        host2.in++ <-- {  delay = 0ms; datarate = 700; } <-- LSR1.out++;
        host2.out++ --> {  delay = 0ms; datarate = 700; } --> LSR1.in++;

        LSR1.out++ --> {  delay = 15ms; datarate = 600; } --> LSR2.in++;
        LSR1.in++ <-- {  delay = 15ms; datarate = 600; } <-- LSR2.out++;

        LSR1.out++ --> {  delay = 5ms; datarate = 120; } --> LSR3.in++;
        LSR1.in++ <-- {  delay = 5ms; datarate = 120; } <-- LSR3.out++;

        LSR2.out++ --> {  delay = 5ms; datarate = 600; } --> LSR4.in++;
        LSR2.in++ <-- {  delay = 5ms; datarate = 600; } <-- LSR4.out++;


        LSR3.out++ --> {  delay = 5ms; datarate = 200; } --> LSR4.in++;
        LSR3.in++ <-- {  delay = 5ms; datarate = 200; } <-- LSR4.out++;

        LSR4.out++ --> {  delay = 5ms; datarate = 700; } --> LSR5.in++;
        LSR4.in++ <-- {  delay = 5ms; datarate = 700; } <-- LSR5.out++;


        LSR5.out++ --> {  delay = 0ms; datarate = 600; } --> host3.in++;
        LSR5.in++ <-- {  delay = 10ms; datarate = 600; } <-- host3.out++;

        LSR5.out++ --> {  delay = 0ms; datarate = 600; } --> host4.in++;
        LSR5.in++ <-- {  delay = 10ms; datarate = 600; } <-- host4.out++;

}

