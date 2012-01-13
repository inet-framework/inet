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


package inet.examples.mpls.net37;

import inet.networklayer.autorouting.NetworkConfigurator;
import inet.nodes.mpls.RSVP_LSR;
import ned.DatarateChannel;


network R37orig
{
    types:
        channel line extends DatarateChannel
        {
            delay = 15ms;
            datarate = 60kbps;
        }
    submodules:
        configurator: NetworkConfigurator;
        LSR0: RSVP_LSR {
            parameters:
                @display("p=489,376");
        }
        LSR1: RSVP_LSR {
            parameters:
                @display("p=999,983");
        }
        LSR2: RSVP_LSR {
            parameters:
                @display("p=362,787");
        }
        LSR3: RSVP_LSR {
            parameters:
                @display("p=890,698");
        }
        LSR4: RSVP_LSR {
            parameters:
                @display("p=677,381");
        }
        LSR5: RSVP_LSR {
            parameters:
                @display("p=360,282");
        }
        LSR6: RSVP_LSR {
            parameters:
                @display("p=320,657");
        }
        LSR7: RSVP_LSR {
            parameters:
                @display("p=476,439");
        }
        LSR8: RSVP_LSR {
            parameters:
                @display("p=853,581");
        }
        LSR9: RSVP_LSR {
            parameters:
                @display("p=670,237");
        }
        LSR10: RSVP_LSR {
            parameters:
                @display("p=222,284");
        }
        LSR11: RSVP_LSR {
            parameters:
                @display("p=531,437");
        }
        LSR12: RSVP_LSR {
            parameters:
                @display("p=588,466");
        }
        LSR13: RSVP_LSR {
            parameters:
                @display("p=348,152");
        }
        LSR14: RSVP_LSR {
            parameters:
                @display("p=584,357");
        }
        LSR15: RSVP_LSR {
            parameters:
                @display("p=905,32");
        }
        LSR16: RSVP_LSR {
            parameters:
                @display("p=857,479");
        }
        LSR17: RSVP_LSR {
            parameters:
                @display("p=25,866");
        }
        LSR18: RSVP_LSR {
            parameters:
                @display("p=373,394");
        }
        LSR19: RSVP_LSR {
            parameters:
                @display("p=488,651");
        }
        LSR20: RSVP_LSR {
            parameters:
                @display("p=185,838");
        }
        LSR21: RSVP_LSR {
            parameters:
                @display("p=453,748");
        }
        LSR22: RSVP_LSR {
            parameters:
                @display("p=595,662");
        }
        LSR23: RSVP_LSR {
            parameters:
                @display("p=647,542");
        }
        LSR24: RSVP_LSR {
            parameters:
                @display("p=643,58");
        }
        LSR25: RSVP_LSR {
            parameters:
                @display("p=686,999");
        }
        LSR26: RSVP_LSR {
            parameters:
                @display("p=413,502");
        }
        LSR27: RSVP_LSR {
            parameters:
                @display("p=766,518");
        }
        LSR28: RSVP_LSR {
            parameters:
                @display("p=696,837");
        }
        LSR29: RSVP_LSR {
            parameters:
                @display("p=104,950");
        }
        LSR30: RSVP_LSR {
            parameters:
                @display("p=995,778");
        }
        LSR31: RSVP_LSR {
            parameters:
                @display("p=763,109");
        }
        LSR32: RSVP_LSR {
            parameters:
                @display("p=544,535");
        }
        LSR33: RSVP_LSR {
            parameters:
                @display("p=757,563");
        }
        LSR34: RSVP_LSR {
            parameters:
                @display("p=884,379");
        }
        LSR35: RSVP_LSR {
            parameters:
                @display("p=767,675");
        }
        LSR36: RSVP_LSR {
            parameters:
                @display("p=544,603");

        }
    connections:
        LSR7.pppg++ <--> line <--> LSR0.pppg++;   // cost 259
        LSR8.pppg++ <--> line <--> LSR3.pppg++;   // cost 474
        LSR9.pppg++ <--> line <--> LSR4.pppg++;   // cost 540
        LSR11.pppg++ <--> line <--> LSR7.pppg++;  // cost 264
        LSR12.pppg++ <--> line <--> LSR11.pppg++; // cost 275
        LSR13.pppg++ <--> line <--> LSR0.pppg++;  // cost 1067
        LSR13.pppg++ <--> line <--> LSR5.pppg++;  // cost 610
        LSR13.pppg++ <--> line <--> LSR10.pppg++; // cost 462
        LSR14.pppg++ <--> line <--> LSR0.pppg++;  // cost 552
        LSR14.pppg++ <--> line <--> LSR4.pppg++;  // cost 381
        LSR14.pppg++ <--> line <--> LSR12.pppg++; // cost 592
        LSR16.pppg++ <--> line <--> LSR8.pppg++;  // cost 436
        LSR18.pppg++ <--> line <--> LSR0.pppg++;  // cost 540
        LSR18.pppg++ <--> line <--> LSR5.pppg++;  // cost 239
        LSR18.pppg++ <--> line <--> LSR10.pppg++; // cost 690
        LSR18.pppg++ <--> line <--> LSR17.pppg++; // cost 1977
        LSR20.pppg++ <--> line <--> LSR2.pppg++;  // cost 760
        LSR20.pppg++ <--> line <--> LSR6.pppg++;  // cost 834
        LSR20.pppg++ <--> line <--> LSR17.pppg++; // cost 751
        LSR21.pppg++ <--> line <--> LSR2.pppg++;  // cost 508
        LSR21.pppg++ <--> line <--> LSR6.pppg++;  // cost 756
        LSR21.pppg++ <--> line <--> LSR19.pppg++; // cost 411
        LSR23.pppg++ <--> line <--> LSR4.pppg++;  // cost 757
        LSR23.pppg++ <--> line <--> LSR12.pppg++; // cost 456
        LSR23.pppg++ <--> line <--> LSR22.pppg++; // cost 522
        LSR24.pppg++ <--> line <--> LSR9.pppg++;  // cost 722
        LSR24.pppg++ <--> line <--> LSR15.pppg++; // cost 1182
        LSR25.pppg++ <--> line <--> LSR1.pppg++;  // cost 1363
        LSR26.pppg++ <--> line <--> LSR6.pppg++;  // cost 747
        LSR26.pppg++ <--> line <--> LSR7.pppg++;  // cost 393
        LSR26.pppg++ <--> line <--> LSR18.pppg++; // cost 514
        LSR26.pppg++ <--> line <--> LSR19.pppg++; // cost 594
        LSR27.pppg++ <--> line <--> LSR4.pppg++;  // cost 420
        LSR27.pppg++ <--> line <--> LSR8.pppg++;  // cost 668
        LSR28.pppg++ <--> line <--> LSR21.pppg++; // cost 907
        LSR28.pppg++ <--> line <--> LSR22.pppg++; // cost 720
        LSR28.pppg++ <--> line <--> LSR25.pppg++; // cost 638
        LSR29.pppg++ <--> line <--> LSR2.pppg++;  // cost 1244
        LSR29.pppg++ <--> line <--> LSR17.pppg++; // cost 471
        LSR30.pppg++ <--> line <--> LSR1.pppg++;  // cost 794
        LSR30.pppg++ <--> line <--> LSR3.pppg++;  // cost 487
        LSR31.pppg++ <--> line <--> LSR9.pppg++;  // cost 777
        LSR31.pppg++ <--> line <--> LSR15.pppg++; // cost 597
        LSR32.pppg++ <--> line <--> LSR12.pppg++; // cost 271
        LSR32.pppg++ <--> line <--> LSR26.pppg++; // cost 600
        LSR33.pppg++ <--> line <--> LSR23.pppg++; // cost 534
        LSR33.pppg++ <--> line <--> LSR27.pppg++; // cost 376
        LSR34.pppg++ <--> line <--> LSR4.pppg++;  // cost 775
        LSR34.pppg++ <--> line <--> LSR15.pppg++; // cost 1370
        LSR34.pppg++ <--> line <--> LSR16.pppg++; // cost 383
        LSR35.pppg++ <--> line <--> LSR1.pppg++;  // cost 1500
        LSR35.pppg++ <--> line <--> LSR3.pppg++;  // cost 551
        LSR35.pppg++ <--> line <--> LSR28.pppg++; // cost 783
        LSR35.pppg++ <--> line <--> LSR33.pppg++; // cost 400
        LSR36.pppg++ <--> line <--> LSR19.pppg++; // cost 507
        LSR36.pppg++ <--> line <--> LSR22.pppg++; // cost 327
        LSR36.pppg++ <--> line <--> LSR32.pppg++; // cost 218
}

